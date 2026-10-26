#!/usr/bin/env python3
"""Buddy 1 wire format v3 (minimum-bit binary) for the dashboard / laptop side.

Written from docs/buddy1/MINIMUM_BIT_REPRESENTATION.md, NOT from the C code, so
it is an independent decoder: if the two ever disagree, one of them is wrong.

Conventions: little-endian, LSB-first. Field N bits wide starting at bit S is
(int.from_bytes(frame, 'little') >> S) & (2**N - 1). Signed fields are two's
complement. Every message starts with a 2-bit format field = 1.

Usage:
    python3 s1_wire.py --selftest                 hand-made golden vectors
    python3 s1_wire.py --telemetry 0123...        decode a hex telemetry frame
    python3 s1_wire.py --heartbeat 1504030201..   decode a hex heartbeat
    python3 s1_wire.py --command EMERGENCY_STOP   print the 1-byte command (hex)
    python3 s1_wire.py --map "MAP A=FORWARD"      print the mapping payload
"""
import sys

FORMAT = 1
TELEMETRY_BYTES = 29
HEARTBEAT_BYTES = 8
COMMANDS = {"EMERGENCY_STOP": 1, "START_RUN": 2, "PAUSE": 3, "RESUME": 4}
STATES = ["INITIALISING", "READY", "OPERATIONAL", "PAUSED", "SAFE_STOP",
          "MISSION_COMPLETE", "STOPPED", "FAULT"]
FAULT_BITS = ["MOTOR_STALL", "ENCODER", "IMU", "NO_ECHO", "WIFI_LOSS",
              "ESTOP_LATCHED", "LINE_SENSOR", "BARCODE"]
VALID_BITS = ["SPEED", "ENCODER", "LINE", "HUMP", "OBSTACLE", "IMU"]
MAP_RESULTS = ["NONE", "QUEUED", "ACCEPTED", "REJECTED"]
MAP_TEXT_MAX = 120

# name: (start bit, width, signed)
TELEMETRY = [
    ("format", 0, 2, False), ("seq", 2, 12, False), ("state", 14, 3, False),
    ("validity", 17, 6, False), ("fault", 23, 8, False),
    ("time_ms", 31, 32, False),
    ("speed_left_mm_s", 63, 12, True), ("speed_right_mm_s", 75, 12, True),
    ("encoder_left", 87, 32, True), ("encoder_right", 119, 32, True),
    ("encoder_gen", 151, 8, False), ("encoder_age_ms", 159, 10, False),
    ("line", 169, 3, False), ("line_age_ms", 172, 10, False),
    ("hump_mm", 182, 12, False), ("obstacle_mm", 194, 12, False),
    ("obstacle_age_ms", 206, 10, False), ("imu_age_ms", 216, 10, False),
    ("reserved", 226, 6, False),
]
HEARTBEAT = [
    ("format", 0, 2, False), ("estop", 2, 1, False), ("map_result", 3, 2, False),
    ("reserved_a", 5, 3, False), ("uptime_ms", 8, 32, False),
    ("map_seq", 40, 8, False), ("map_revision", 48, 8, False),
    ("reserved_b", 56, 8, False),
]


def _get(value, start, width, signed):
    raw = (value >> start) & ((1 << width) - 1)
    if signed and raw >= 1 << (width - 1):
        raw -= 1 << width
    return raw


def _put(fields, values):
    out = 0
    for name, start, width, signed in fields:
        v = values.get(name, 0)
        lo, hi = (-(1 << (width - 1)), (1 << (width - 1)) - 1) if signed else (0, (1 << width) - 1)
        if not lo <= v <= hi:
            raise ValueError("%s=%d does not fit %d bits" % (name, v, width))
        out |= (v & ((1 << width) - 1)) << start
    return out


def decode(fields, size, data):
    """Return a dict, or raise ValueError (wrong length, format or reserved bits)."""
    data = bytes(data)
    if len(data) != size:
        raise ValueError("length %d, expected %d" % (len(data), size))
    value = int.from_bytes(data, "little")
    d = {n: _get(value, s, w, sg) for n, s, w, sg in fields}
    if d["format"] != FORMAT:
        raise ValueError("unsupported format %d" % d["format"])
    if any(v for k, v in d.items() if k.startswith("reserved")):
        raise ValueError("reserved bits set")
    return d


def decode_telemetry(data):
    d = decode(TELEMETRY, TELEMETRY_BYTES, data)
    v = d["validity"]
    # A group that is not valid carries no measurement; never show a fake zero.
    for bit, names in ((0, ("speed_left_mm_s", "speed_right_mm_s")),
                       (1, ("encoder_left", "encoder_right", "encoder_gen", "encoder_age_ms")),
                       (2, ("line", "line_age_ms")), (3, ("hump_mm",)),
                       (4, ("obstacle_mm", "obstacle_age_ms")), (5, ("imu_age_ms",))):
        if not v >> bit & 1:
            for n in names:
                d[n] = None
    d["state_name"] = STATES[d["state"]]
    d["faults"] = [n for i, n in enumerate(FAULT_BITS) if d["fault"] >> i & 1]
    return d


def decode_heartbeat(data):
    d = decode(HEARTBEAT, HEARTBEAT_BYTES, data)
    d["map_result_name"] = MAP_RESULTS[d["map_result"]]
    return d


def encode_telemetry(values):
    values = dict(values, format=FORMAT)
    return _put(TELEMETRY, values).to_bytes(TELEMETRY_BYTES, "little")


def encode_heartbeat(values):
    values = dict(values, format=FORMAT)
    return _put(HEARTBEAT, values).to_bytes(HEARTBEAT_BYTES, "little")


def encode_command(name):
    return bytes([COMMANDS[name]])


def encode_map(text):
    raw = text.encode("ascii")
    if not raw.startswith(b"MAP ") or len(raw) > MAP_TEXT_MAX or b"\0" in raw:
        raise ValueError("mapping payload must be 'MAP ...', ASCII, <= %d bytes" % MAP_TEXT_MAX)
    return raw


def selftest():
    n = 0

    def check(cond, name):
        nonlocal n
        n += 1
        if not cond:
            raise SystemExit("FAIL: " + name)

    # Golden vector 1: every field distinct, computed by hand.
    v = dict(seq=0xABC, state=2, validity=0x3F, fault=0x05, time_ms=0x01020304,
             speed_left_mm_s=-1, speed_right_mm_s=2047, encoder_left=-2 ** 31,
             encoder_right=2 ** 31 - 1, encoder_gen=0xFF, encoder_age_ms=1023,
             line=7, line_age_ms=999, hump_mm=4095, obstacle_mm=4000,
             obstacle_age_ms=1, imu_age_ms=0)
    frame = encode_telemetry(v)
    check(len(frame) == 29, "29 bytes")
    check(frame[0] == (1 | (0xABC & 0x3F) << 2) & 0xFF, "byte 0 = format | seq low 6 bits")
    d = decode_telemetry(frame)
    check(all(d[k] == val for k, val in v.items()), "round trip every field, signed extremes")
    check(d["faults"] == ["MOTOR_STALL", "IMU"], "named fault bits")
    # Golden vector 2: nothing valid -> measurements are None, never a fake 0.
    d = decode_telemetry(encode_telemetry(dict(seq=1, state=7, validity=0)))
    check(d["obstacle_mm"] is None and d["speed_left_mm_s"] is None and d["encoder_left"] is None,
          "invalid groups decode to None")
    # Malformed input is refused.
    for bad in (b"", bytes(28), bytes(30), b"\x00" * 29, bytes([2]) + bytes(28)):
        try:
            decode_telemetry(bad)
        except ValueError:
            continue
        raise SystemExit("FAIL: accepted malformed frame %r" % bad[:3])
    n += 1
    # Heartbeat golden bytes: format 1, estop 1, map ACCEPTED(2), uptime 0x01020304, seq 5, rev 7.
    hb = encode_heartbeat(dict(estop=1, map_result=2, uptime_ms=0x01020304, map_seq=5, map_revision=7))
    check(hb.hex() == "1504030201050700", "heartbeat golden bytes")
    check(decode_heartbeat(hb)["map_result_name"] == "ACCEPTED", "heartbeat map result")
    check(encode_command("EMERGENCY_STOP") == b"\x01" and encode_command("RESUME") == b"\x04", "commands are 1 byte")
    check(encode_map("MAP A=FORWARD B=LEFT") == b"MAP A=FORWARD B=LEFT", "map payload")
    for bad in ("map a=forward", "MAP " + "A" * 200, "MAP A\0"):
        try:
            encode_map(bad)
        except ValueError:
            continue
        raise SystemExit("FAIL: accepted bad map text")
    n += 1
    print("s1_wire selftest: %d checks passed" % n)


def main(argv):
    if len(argv) >= 2 and argv[1] == "--selftest":
        selftest()
    elif len(argv) == 3 and argv[1] == "--telemetry":
        print(decode_telemetry(bytes.fromhex(argv[2])))
    elif len(argv) == 3 and argv[1] == "--heartbeat":
        print(decode_heartbeat(bytes.fromhex(argv[2])))
    elif len(argv) == 3 and argv[1] == "--command":
        print(encode_command(argv[2]).hex())
    elif len(argv) == 3 and argv[1] == "--map":
        print(encode_map(argv[2]).hex())
    else:
        print(__doc__)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))