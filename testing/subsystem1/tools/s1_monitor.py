#!/usr/bin/env python3
"""Laptop monitor / command sender for the Buddy 1 binary protocol.

Needs the paho-mqtt package (pip install paho-mqtt) and a broker, for example
Mosquitto on the laptop. The decoding itself is in s1_wire.py (no extra
packages), so this file is only the MQTT glue.

    python3 s1_monitor.py --broker 192.168.137.1                  print decoded telemetry + heartbeat
    python3 s1_monitor.py --broker 192.168.137.1 --send RESUME     send a 1-byte command
    python3 s1_monitor.py --broker 192.168.137.1 --map "MAP A=FORWARD B=LEFT"
                                                                    send a mapping update and
                                                                    wait for ACCEPTED / REJECTED
Topics: robot/telemetry (100 ms), robot/status (1 s, last will), robot/cmd (in).
"""
import argparse
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import s1_wire as w  # noqa: E402

try:
    import paho.mqtt.client as mqtt
except ImportError:  # pragma: no cover
    sys.exit("paho-mqtt is not installed: pip install paho-mqtt")


def make_client():
    try:  # paho-mqtt 2.x needs the callback API version, 1.x does not accept it.
        return mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
    except AttributeError:
        return mqtt.Client()


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--broker", required=True)
    ap.add_argument("--port", type=int, default=1883)
    ap.add_argument("--send", choices=sorted(w.COMMANDS))
    ap.add_argument("--map", help='mapping update text, e.g. "MAP A=FORWARD B=LEFT"')
    ap.add_argument("--seconds", type=float, default=0, help="stop after this long (0 = until Ctrl-C)")
    args = ap.parse_args()

    state = {"last_seq": None, "lost": 0, "map": None}

    def on_connect(c, _u, _f, rc, *_a):
        print("connected, rc=%s" % rc)
        c.subscribe([("robot/telemetry", 0), ("robot/status", 0)])
        if args.send:
            c.publish("robot/cmd", w.encode_command(args.send), qos=0)
            print("sent %s -> %s" % (args.send, w.encode_command(args.send).hex()))
        if args.map:
            c.publish("robot/cmd", w.encode_map(args.map), qos=0)
            print("sent mapping update: %s" % args.map)

    def on_message(_c, _u, msg):
        try:
            if msg.topic == "robot/telemetry":
                d = w.decode_telemetry(msg.payload)
                if state["last_seq"] is not None:
                    state["lost"] += (d["seq"] - state["last_seq"] - 1) % 4096
                state["last_seq"] = d["seq"]
                print("T seq=%d state=%s faults=%s t=%d ms speed=%s,%s enc=%s,%s line=%s hump=%s obst=%s lost=%d" % (
                    d["seq"], d["state_name"], d["faults"] or "-", d["time_ms"], d["speed_left_mm_s"],
                    d["speed_right_mm_s"], d["encoder_left"], d["encoder_right"], d["line"], d["hump_mm"],
                    d["obstacle_mm"], state["lost"]))
            elif msg.topic == "robot/status":
                if len(msg.payload) == w.HEARTBEAT_BYTES:
                    h = w.decode_heartbeat(msg.payload)
                    print("H uptime=%d ms estop=%d map=%s seq=%d rev=%d" % (
                        h["uptime_ms"], h["estop"], h["map_result_name"], h["map_seq"], h["map_revision"]))
                    if args.map and h["map_result_name"] in ("ACCEPTED", "REJECTED"):
                        state["map"] = h["map_result_name"]
                else:
                    print("S %r (last will / other)" % msg.payload)
        except ValueError as e:
            print("bad %s payload (%d bytes): %s" % (msg.topic, len(msg.payload), e))

    c = make_client()
    c.on_connect = on_connect
    c.on_message = on_message
    c.connect(args.broker, args.port, 30)
    c.loop_start()
    end = time.time() + args.seconds if args.seconds else None
    try:
        while end is None or time.time() < end:
            time.sleep(0.2)
            if args.map and state["map"]:
                print("mapping update result: %s" % state["map"])
                break
    except KeyboardInterrupt:
        pass
    c.loop_stop()
    return 0 if not args.map or state["map"] == "ACCEPTED" else 1


if __name__ == "__main__":
    sys.exit(main())