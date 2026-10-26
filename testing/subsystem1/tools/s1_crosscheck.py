#!/usr/bin/env python3
"""Independent oracle for the C telemetry encoder.

Reads the output of dump_frames_host (inputs + the 29 bytes the C code made),
applies the rules of docs/buddy1/MINIMUM_BIT_REPRESENTATION.md in Python,
encodes with s1_wire.py, and requires the bytes to be identical. Then decodes
the C bytes with s1_wire.py and checks every field. Exit status 0 = all agree.

    ./dump_frames_host | python3 s1_crosscheck.py
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import s1_wire as w  # noqa: E402

VALID_ALL = 63


def expected(seq, state, valid, fault, ts, sl, sr, el, er, gen, ets, line, a0, a1, a2, hump, obst):
    """The document's rules: a group that does not fit is dropped (validity bit
    cleared, fields 0), never clamped or truncated."""
    now_us = ts * 1000
    v = valid & VALID_ALL
    f = dict(seq=seq & 0xFFF, state=state, fault=fault, time_ms=ts)
    if v & 1:
        if -2048 <= sl <= 2047 and -2048 <= sr <= 2047:
            f["speed_left_mm_s"], f["speed_right_mm_s"] = sl, sr
        else:
            v &= ~1
    if v & 2:
        age = (now_us - ets) // 1000 if ets <= now_us else None
        if (gen != 0 and -2 ** 31 <= el < 2 ** 31 and -2 ** 31 <= er < 2 ** 31
                and age is not None and age <= 1023):
            f.update(encoder_left=el, encoder_right=er, encoder_gen=gen & 0xFF, encoder_age_ms=age)
        else:
            v &= ~2
    if v & 4:
        if line <= 7 and a0 // 1000 <= 999:
            f.update(line=line, line_age_ms=a0 // 1000)
        else:
            v &= ~4
    if v & 8:
        if 0 <= hump <= 4095:
            f["hump_mm"] = hump
        else:
            v &= ~8
    if v & 16:
        if 20 <= obst <= 4000 and a1 // 1000 <= 999:
            f.update(obstacle_mm=obst, obstacle_age_ms=a1 // 1000)
        else:
            v &= ~16
    if v & 32:
        if a2 // 1000 <= 999:
            f["imu_age_ms"] = a2 // 1000
        else:
            v &= ~32
    f["validity"] = v
    return f


def main():
    n = 0
    cleared = 0
    for line in sys.stdin:
        p = line.split()
        if not p or p[0] != "F":
            continue
        nums = [int(x) for x in p[1:18]]
        c_bytes = bytes.fromhex(p[18])
        exp = expected(*nums)
        py_bytes = w.encode_telemetry(exp)
        if py_bytes != c_bytes:
            print("MISMATCH line %d\n  inputs %s\n  C      %s\n  Python %s" % (n + 1, nums, c_bytes.hex(), py_bytes.hex()))
            return 1
        d = w.decode_telemetry(c_bytes)
        for k, v in exp.items():
            if d[k] != v:
                print("DECODE MISMATCH %s: %r != %r" % (k, d[k], v))
                return 1
        if exp["validity"] != (nums[2] & VALID_ALL):
            cleared += 1
        n += 1
    if n == 0:
        print("no frames read")
        return 2
    print("crosscheck: %d frames, C bytes == Python oracle bytes, %d had a group rejected as out of range" % (n, cleared))
    return 0


if __name__ == "__main__":
    sys.exit(main())