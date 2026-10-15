#!/usr/bin/env python3
"""
Buddy 4 - bring-up log analyser (standard library only).

Reads a terminal capture from the buddy4_imu_bringup firmware (CSV / EVT /
RAW lines) and prints the statistics needed to choose thresholds and to
fill in the terrain analysis report:

  * stationary noise per axis (RAW, first N seconds or --still t0:t1)
  * pitch / roll / vibration distribution while driving (CSV)
  * hump peaks per pass: mean, standard deviation, CV, range (EVT)
  * rejected hump candidates and their reasons (false-positive check)
  * impacts (peak, horizontal ratio) and turning events
  * suggested starting thresholds = k * measured noise (to be validated)

Usage:
  python3 analyse_log.py capture.txt [--still 0:3000] [--true-height 20]
"""

import argparse
import math
import statistics as stats
import sys

VALIDITY = {0: "VALID", 1: "PROVISIONAL", 2: "REJ_LOW_PEAK",
            3: "REJ_NO_DESCENT", 4: "REJ_TOO_SHORT", 5: "REJ_TOO_LONG",
            6: "REJ_SLOPE", 7: "REJ_ABORTED"}


def parse_kv(fields):
    out = {}
    for f in fields:
        if "=" in f:
            k, v = f.split("=", 1)
            out[k] = v
    return out


def describe(name, xs, unit=""):
    if len(xs) < 2:
        print(f"  {name:28s} n={len(xs)} (not enough data)")
        return
    print(f"  {name:28s} n={len(xs):5d} mean={stats.mean(xs):9.4f}{unit} "
          f"std={stats.stdev(xs):8.4f}{unit} min={min(xs):9.4f} "
          f"max={max(xs):9.4f}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("log")
    ap.add_argument("--still", default="0:3000",
                    help="time window (ms) known to be stationary for noise")
    ap.add_argument("--true-height", type=float, default=None,
                    help="known hump height in mm (accuracy check)")
    args = ap.parse_args()
    t0, t1 = (int(x) for x in args.still.split(":"))

    raw, csv, humps, rejects, impacts, turns = [], [], [], [], [], []
    with open(args.log, errors="replace") as fh:
        for line in fh:
            line = line.strip()
            if line.startswith("RAW,"):
                p = line.split(",")
                raw.append([float(x) for x in p[1:8]])
            elif line.startswith("CSV,"):
                p = line.split(",")
                csv.append([float(x) for x in p[1:]])
            elif line.startswith("EVT,"):
                p = line.split(",")
                kind = p[3]
                kv = parse_kv(p[4:])
                if kind == "HUMP_COMPLETED":
                    humps.append(kv)
                elif kind == "HUMP_REJECTED":
                    rejects.append(kv)
                elif kind.startswith("IMPACT"):
                    impacts.append(kv)
                elif kind == "TURNING_EVENT":
                    turns.append(kv)

    print(f"Buddy 4 log analysis: {args.log}")
    print(f"  RAW rows={len(raw)} CSV rows={len(csv)} humps={len(humps)} "
          f"rejected={len(rejects)} impacts={len(impacts)} "
          f"turn events={len(turns)}")

    still = [r for r in raw if t0 <= r[0] <= t1]
    if len(still) > 10:
        print("\n1. Stationary sensor noise (RAW, sensor frame)")
        names = ["ax [g]", "ay [g]", "az [g]", "mx [G]", "my [G]", "mz [G]"]
        sig = []
        for i, n in enumerate(names):
            col = [r[i + 1] for r in still]
            describe(n, col)
            sig.append(stats.stdev(col))
        g = [math.sqrt(r[1] ** 2 + r[2] ** 2 + r[3] ** 2) for r in still]
        describe("|a| [g]", g)
        acc_sig = max(sig[:3])
        print("\n   Suggested starting points (validate on the track!):")
        print(f"   cal_max_noise_g      >= {max(4 * acc_sig, 0.005):.4f}")
        print(f"   stationary_vib_g     ~  {max(3 * acc_sig, 0.005):.4f}")
        print(f"   pitch noise ~ {math.degrees(acc_sig):.2f} deg per sample "
              f"-> hump_ascent_enter_deg >= {max(5 * math.degrees(acc_sig), 1.5):.2f}")

    if csv:
        print("\n2. Driving statistics (CSV)")
        moving = [r for r in csv if r[11] > 0.02]
        describe("pitch [deg] (all)", [r[3] for r in csv])
        describe("roll [deg] (all)", [r[4] for r in csv])
        describe("vibration [g] (moving)", [r[11] for r in moving])
        describe("fwd accel [g] (moving)", [r[9] for r in moving])
        describe("|B| [gauss]", [r[12] for r in csv])
        if moving:
            p99 = sorted(abs(r[3]) for r in moving)[int(0.99 * len(moving)) - 1]
            print(f"   99th percentile |pitch| on flat driving = {p99:.2f} deg "
                  f"-> keep hump_ascent_enter_deg above this")

    if humps:
        print("\n3. Hump repeatability (HUMP_COMPLETED)")
        peaks = [int(h["peak_cdeg"]) / 100.0 for h in humps]
        hmm = [int(h["h_mm"]) for h in humps if int(h["h_mm"]) >= 0]
        for h in humps:
            print(f"   #{h.get('idx')}: peak {int(h['peak_cdeg'])/100:.2f} deg, "
                  f"trough {int(h['min_cdeg'])/100:.2f} deg, est {h['h_mm']} mm, "
                  f"dur {h['dur']} ms, new_max={h['new_max']}")
        describe("peak pitch [deg]", peaks)
        if len(peaks) > 1:
            cv = stats.stdev(peaks) / stats.mean(peaks) * 100.0
            print(f"   coefficient of variation = {cv:.1f} % "
                  f"(A-B4-2: consistent if << difference between humps)")
        if args.true_height and hmm:
            err = [x - args.true_height for x in hmm]
            describe("height error [mm]", err)
            gain = args.true_height / stats.mean(hmm) if stats.mean(hmm) else 1
            print(f"   single-hump gain suggestion: hump_height_gain *= {gain:.3f}"
                  " (fit gain+offset with >= 3 different heights)")

    if rejects:
        print("\n4. Rejected hump candidates (should be 0 on flat floor)")
        reasons = {}
        for r in rejects:
            v = VALIDITY.get(int(r.get("validity", -1)), "?")
            reasons[v] = reasons.get(v, 0) + 1
        for k, v in reasons.items():
            print(f"   {k:16s} {v}")

    if impacts:
        print("\n5. Impacts")
        for i in impacts:
            pk = int(i["peak_mg"])
            hz = int(i["horiz_mg"])
            print(f"   peak {pk} mg, horiz {hz} mg ({100*hz/max(pk,1):.0f} %), "
                  f"dur {i['dur']} ms, suppressed {i['supp']}")

    if turns:
        ends = [t for t in turns if t.get("phase") == "1"]
        print(f"\n6. Turns: {len(ends)} completed")
        for t in ends:
            print(f"   dir={'LEFT' if t['dir'] == '1' else 'RIGHT'} "
                  f"delta={int(t['delta'])/100:.1f} deg dur={t['dur']} ms")
    return 0


if __name__ == "__main__":
    sys.exit(main())
