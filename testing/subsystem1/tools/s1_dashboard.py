#!/usr/bin/env python3
"""Live dashboard for the Buddy 1 binary protocol (FR17).

Subscribes to robot/telemetry and robot/status, decodes them with s1_wire.py
(the same layout as docs/buddy1/MINIMUM_BIT_REPRESENTATION.md section 2), and
serves a page at http://localhost:8080 that shows the robot state, speeds,
encoders, sensors, fault bits, link health and mapping-update result. The page
also sends the four commands and a mapping update to robot/cmd.

    pip install paho-mqtt
    python s1_dashboard.py --broker 192.168.137.1            real robot
    python s1_dashboard.py --demo                            no broker: synthetic frames, for a look

A frame that does not decode (wrong size, format, reserved bits) is counted and
shown as a decode error; it is never displayed as zeros.
"""
import argparse
import json
import math
import os
import sys
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import s1_wire as w  # noqa: E402

LOCK = threading.Lock()
STATE = {
    "telemetry": None, "heartbeat": None, "last_seq": None, "lost": 0,
    "frames": 0, "bad": 0, "last_bad": "", "rx_ms": 0, "hb_ms": 0,
    "offline": False, "log": [], "broker": "",
}
PUBLISH = {"fn": None}


def note(text):
    STATE["log"] = (STATE["log"] + ["%s  %s" % (time.strftime("%H:%M:%S"), text)])[-12:]


def on_telemetry(payload):
    with LOCK:
        try:
            d = w.decode_telemetry(payload)
        except ValueError as e:
            STATE["bad"] += 1
            STATE["last_bad"] = "%d bytes: %s" % (len(payload), e)
            note("bad telemetry frame (%s)" % e)
            return
        if STATE["last_seq"] is not None:
            STATE["lost"] += (d["seq"] - STATE["last_seq"] - 1) % 4096
        STATE["last_seq"] = d["seq"]
        STATE["telemetry"] = d
        STATE["frames"] += 1
        STATE["rx_ms"] = int(time.time() * 1000)


def on_status(payload):
    with LOCK:
        if len(payload) == w.HEARTBEAT_BYTES:
            try:
                STATE["heartbeat"] = w.decode_heartbeat(payload)
                STATE["hb_ms"] = int(time.time() * 1000)
                STATE["offline"] = False
                return
            except ValueError as e:
                STATE["bad"] += 1
                STATE["last_bad"] = "heartbeat: %s" % e
                return
        if payload == b"OFFLINE":
            STATE["offline"] = True
            note("robot went OFFLINE (last will)")
        else:
            STATE["bad"] += 1
            STATE["last_bad"] = "status %d bytes" % len(payload)


def snapshot():
    with LOCK:
        now = int(time.time() * 1000)
        s = dict(STATE)
        s["age_ms"] = (now - s["rx_ms"]) if s["rx_ms"] else None
        s["hb_age_ms"] = (now - s["hb_ms"]) if s["hb_ms"] else None
        return s


def start_mqtt(broker, port):
    try:
        import paho.mqtt.client as mqtt
    except ImportError:
        sys.exit("paho-mqtt is not installed: pip install paho-mqtt")
    try:
        c = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
    except AttributeError:
        c = mqtt.Client()

    def on_connect(cl, _u, _f, rc, *_a):
        with LOCK:
            note("broker connected, rc=%s" % rc)
        cl.subscribe([("robot/telemetry", 0), ("robot/status", 0)])

    def on_message(_c, _u, msg):
        (on_telemetry if msg.topic == "robot/telemetry" else on_status)(bytes(msg.payload))

    c.on_connect = on_connect
    c.on_message = on_message
    c.connect(broker, port, 30)
    c.loop_start()
    PUBLISH["fn"] = lambda payload: c.publish("robot/cmd", payload, qos=0)


def start_demo():
    def run():
        seq = 0
        t0 = time.time()
        while True:
            t = time.time() - t0
            seq = (seq + 1) & 0xFFF
            fault = 0x04 if int(t) % 20 > 16 else 0
            frame = w.encode_telemetry({
                "seq": seq, "state": 2, "validity": 0x3F, "fault": fault,
                "time_ms": int(t * 1000), "speed_left_mm_s": int(200 + 60 * math.sin(t)),
                "speed_right_mm_s": int(200 - 60 * math.sin(t)),
                "encoder_left": int(t * 200), "encoder_right": int(t * 195), "encoder_gen": 1,
                "encoder_age_ms": 12, "line": 2, "line_age_ms": 9,
                "hump_mm": 25 if int(t) % 30 > 25 else 0, "obstacle_mm": int(800 + 500 * math.sin(t / 3)),
                "obstacle_age_ms": 40, "imu_age_ms": 8})
            on_telemetry(frame)
            if seq % 10 == 0:
                on_status(w.encode_heartbeat({"estop": 0, "map_result": 0, "uptime_ms": int(t * 1000),
                                              "map_seq": 0, "map_revision": 0}))
            time.sleep(0.1)
    threading.Thread(target=run, daemon=True).start()
    PUBLISH["fn"] = lambda payload: note("demo: would publish %r" % payload)


PAGE = r"""<!doctype html><html><head><meta charset="utf-8"><title>Robot dashboard</title>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
:root{--bg:#f4f5f7;--card:#fff;--fg:#1c2230;--mut:#667085;--ok:#1a7f4b;--bad:#c0332b;--warn:#b7791f;--line:#dfe3ea}
@media(prefers-color-scheme:dark){:root{--bg:#12151c;--card:#1b2029;--fg:#e8ebf2;--mut:#98a2b3;--ok:#4cc38a;--bad:#ff7b72;--warn:#e3b341;--line:#2c3340}}
body{margin:0;background:var(--bg);color:var(--fg);font:14px/1.4 system-ui,sans-serif;padding:16px}
h1{font-size:18px;margin:0 0 12px}.grid{display:grid;gap:12px;grid-template-columns:repeat(auto-fit,minmax(240px,1fr))}
.card{background:var(--card);border:1px solid var(--line);border-radius:8px;padding:12px}
.card h2{font-size:12px;text-transform:uppercase;letter-spacing:.05em;color:var(--mut);margin:0 0 8px}
.row{display:flex;justify-content:space-between;padding:2px 0}.row span:first-child{color:var(--mut)}
.big{font-size:22px;font-weight:600}.ok{color:var(--ok)}.bad{color:var(--bad)}.warn{color:var(--warn)}
.chip{display:inline-block;border:1px solid var(--line);border-radius:12px;padding:1px 8px;margin:2px 4px 2px 0;font-size:12px;color:var(--mut)}
.chip.on{color:#fff;background:var(--bad);border-color:var(--bad)}.chip.v{color:#fff;background:var(--ok);border-color:var(--ok)}
button{font:inherit;padding:6px 12px;margin:2px 4px 2px 0;border-radius:6px;border:1px solid var(--line);background:var(--card);color:var(--fg);cursor:pointer}
button.stop{background:var(--bad);color:#fff;border-color:var(--bad);font-weight:600}
input{font:inherit;width:100%;box-sizing:border-box;padding:6px;border:1px solid var(--line);border-radius:6px;background:var(--card);color:var(--fg)}
pre{margin:0;font-size:12px;white-space:pre-wrap;color:var(--mut)}
</style></head><body>
<h1>Robot dashboard <span id="conn" class="chip">waiting</span></h1>
<div class="grid">
<div class="card"><h2>State</h2><div class="big" id="state">-</div><div id="faults"></div>
 <div class="row"><span>Emergency latch</span><span id="estop">-</span></div></div>
<div class="card"><h2>Motion</h2>
 <div class="row"><span>Speed left (mm/s)</span><span id="sl">-</span></div>
 <div class="row"><span>Speed right (mm/s)</span><span id="sr">-</span></div>
 <div class="row"><span>Encoder left</span><span id="el">-</span></div>
 <div class="row"><span>Encoder right</span><span id="er">-</span></div></div>
<div class="card"><h2>Sensors</h2>
 <div class="row"><span>Line</span><span id="line">-</span></div>
 <div class="row"><span>Hump (mm)</span><span id="hump">-</span></div>
 <div class="row"><span>Obstacle (mm)</span><span id="obs">-</span></div>
 <div id="valid"></div></div>
<div class="card"><h2>Link</h2>
 <div class="row"><span>Robot time (ms)</span><span id="time">-</span></div>
 <div class="row"><span>Last frame age (ms)</span><span id="age">-</span></div>
 <div class="row"><span>Frames / lost</span><span id="frames">-</span></div>
 <div class="row"><span>Bad frames</span><span id="bad">-</span></div>
 <div class="row"><span>Heartbeat uptime (ms)</span><span id="up">-</span></div></div>
<div class="card"><h2>Mapping update</h2>
 <div class="row"><span>Result</span><span id="map">-</span></div>
 <input id="maptext" placeholder="MAP A=FORWARD B=LEFT"><button onclick="sendMap()">Send map update</button></div>
<div class="card"><h2>Commands</h2>
 <button class="stop" onclick="cmd('EMERGENCY_STOP')">EMERGENCY STOP</button>
 <button onclick="cmd('START_RUN')">START</button><button onclick="cmd('PAUSE')">PAUSE</button>
 <button onclick="cmd('RESUME')">RESUME</button></div>
<div class="card" style="grid-column:1/-1"><h2>Events</h2><pre id="log"></pre></div></div>
<script>
const $=id=>document.getElementById(id), f=v=>v===null||v===undefined?'invalid':v;
const FAULTS=["MOTOR_STALL","ENCODER","IMU","NO_ECHO","WIFI_LOSS","ESTOP_LATCHED","LINE_SENSOR","BARCODE"];
const VALID=["SPEED","ENCODER","LINE","HUMP","OBSTACLE","IMU"];
async function cmd(n){await fetch('/cmd?name='+n,{method:'POST'});}
async function sendMap(){await fetch('/map',{method:'POST',body:$('maptext').value});}
async function tick(){
 try{const s=await (await fetch('/data')).json(), t=s.telemetry, h=s.heartbeat;
  const stale=s.age_ms===null||s.age_ms>1000||s.offline;
  $('conn').textContent=s.offline?'OFFLINE':stale?'no data':'live';$('conn').className='chip '+(stale?'on':'v');
  if(t){$('state').textContent=t.state_name;
   $('faults').innerHTML=FAULTS.map((n,i)=>'<span class="chip '+((t.fault>>i&1)?'on':'')+'">'+n+'</span>').join('');
   $('sl').textContent=f(t.speed_left_mm_s);$('sr').textContent=f(t.speed_right_mm_s);
   $('el').textContent=f(t.encoder_left);$('er').textContent=f(t.encoder_right);
   $('line').textContent=f(t.line);$('hump').textContent=f(t.hump_mm);$('obs').textContent=f(t.obstacle_mm);
   $('valid').innerHTML=VALID.map((n,i)=>'<span class="chip '+((t.validity>>i&1)?'v':'')+'">'+n+'</span>').join('');
   $('time').textContent=t.time_ms;}
  $('age').textContent=f(s.age_ms);$('frames').textContent=s.frames+' / '+s.lost;
  $('bad').innerHTML=s.bad?'<span class="bad">'+s.bad+'</span> ('+s.last_bad+')':'0';
  if(h){$('estop').innerHTML=h.estop?'<span class="bad">LATCHED</span>':'<span class="ok">clear</span>';
   $('up').textContent=h.uptime_ms;
   $('map').textContent=h.map_result_name+(h.map_seq?(' (#'+h.map_seq+', rev '+h.map_revision+')'):'');}
  $('log').textContent=s.log.join('\n');
 }catch(e){$('conn').textContent='dashboard server down';$('conn').className='chip on';}
}
setInterval(tick,250);tick();
</script></body></html>"""


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *_a):
        pass

    def _send(self, code, body, ctype="text/plain"):
        data = body if isinstance(body, bytes) else body.encode()
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        if self.path == "/data":
            self._send(200, json.dumps(snapshot()), "application/json")
        elif self.path in ("/", "/index.html"):
            self._send(200, PAGE, "text/html; charset=utf-8")
        else:
            self._send(404, "not found")

    def do_POST(self):
        publish = PUBLISH["fn"]
        if publish is None:
            self._send(503, "not connected")
            return
        try:
            if self.path.startswith("/cmd?name="):
                name = self.path.split("=", 1)[1]
                if name not in w.COMMANDS:
                    self._send(400, "unknown command")
                    return
                publish(w.encode_command(name))
                with LOCK:
                    note("sent %s" % name)
            elif self.path == "/map":
                length = min(int(self.headers.get("Content-Length", 0)), 256)
                text = self.rfile.read(length).decode("ascii", "replace").strip()
                publish(w.encode_map(text))
                with LOCK:
                    note("sent mapping update: %s" % text)
            else:
                self._send(404, "not found")
                return
            self._send(200, "ok")
        except ValueError as e:
            self._send(400, str(e))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--broker")
    ap.add_argument("--port", type=int, default=1883)
    ap.add_argument("--web-port", type=int, default=8080)
    ap.add_argument("--demo", action="store_true")
    args = ap.parse_args()
    if args.demo:
        start_demo()
    elif args.broker:
        start_mqtt(args.broker, args.port)
    else:
        ap.error("give --broker or --demo")
    srv = ThreadingHTTPServer(("127.0.0.1", args.web_port), Handler)
    print("dashboard: http://localhost:%d  (Ctrl-C to stop)" % args.web_port)
    try:
        srv.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()