# Buddy 1 telemetry and command contract

## Production version 2 (approved for implementation, 2026-10-15)

The user authorised compact version-2 keys to preserve signed 64-bit encoder
precision, source metadata and independent validity within NFR10's 256 bytes.
This changes the dashboard contract; the project write-up's long FR17 names are
mapped below rather than emitted in v2. Confirm academic acceptance of that
representation. Legacy schema 0 still emits those original names for preserved
professor/standalone demonstration tests. No legacy demo is relabelled real data.
All firmware objects must be rebuilt together after the copied C frame changes.

The Vehicle Controller is the sole production aggregator and command consumer.
It queues a copied frame at most once per 100 ms through TMO_POL. The owning
Wi-Fi/MQTT service retains its fixed-rate publishing/heartbeat schedules; no
application task calls lwIP/CYW43. Target jitter and network delivery remain
hardware-test requirements. TEST=1 leaves the controller dormant so professor
self-tests own their queues and the existing simulator behaviour is preserved.
Normal startup disables the simulator before starting tasks.

| Key | Semantic field | Representation / units |
|---|---|---|
| z | schema version | 2 |
| q | sequence | uint32, modulo rollover |
| t | timestamp | kernel operating time modulo 2^32 ms |
| s | robot_state | existing S1 enum 0..7 |
| ml, mr | left/right_motor_speed | signed int32 mm/s, measured; positive forward |
| el, er | left/right_encoder_count | signed int64 decimal **strings**, exact including INT64_MIN |
| l | line_status | normalized raw black mask: bit0 GP2 left-front, bit1 GP4 right-front, bit2 GP27 side |
| h | hump_peak | highest validated S4 hump in cm, 0.1 cm increments; S4 mm truncated below 1 mm |
| o | obstacle_distance | forward ultrasonic cm, 2.0..400.0; no side range substituted |
| v | validity mask | bits: speed 1, counts 2, raw line 4, highest hump 8, obstacle 16, IMU snapshot 32 |
| et | encoder sample time | full uint64 kernel us, decimal string; original capture snapshot time |
| eg | encoder generation | uint32 acquisition generation; not a unique boot identifier |
| a | source ages | microseconds [line, obstacle, IMU], raw source domains, 0..999999 when valid |

Invalid fields encode JSON `null`, including count metadata and source ages.
Speed validity and count validity are independent: first encoder baseline may
have valid counts before speed. Highest-hump validity requires a healthy IMU
snapshot. Absent calibration, stale/future data and faulty acquisition do not
become valid zeros. Validity is telemetry evidence, not permission to actuate;
mission gates use their own configured tighter freshness requirements. Raw line
bits do not establish that filtered steering agrees with the current sample.

Encoder counts are the actual polarity-applied x4 transitions, never inferred
from micrometres. Boot establishes the origin; START/RESUME do not reset counts.
No live reset is supported. Saturation/overflow invalidates feedback and faults;
there is no silent count rollover. Generation restarts on reboot and does not
identify a boot globally; clients must start a new odometry history on reboot.
The codec rejects inconsistent flags, unsupported schemas and out-of-range valid
values without emitting truncated JSON. Valid hump input is 0..99999 mm; larger
or nonfinite heights are invalidated by the aggregator. Tests exercise full
signed widths, maximum source timestamp, null fields, age and capacity limits.

## Commands and safety

S1 publishes a persistent, SMP-safe emergency latch and a pulse before ordinary
queue admission or diagnostic mutex acquisition. The controller reads the latch
and pulse before ordinary providers. A consumed pulse, full queue or RESUME does
not clear the latch; current integrated recovery requires reboot. Only S2 owns
motor actuation. S1 command reception validates exact copied-message size and
known IDs; stale ordinary commands are discarded, emergency commands retain
priority. Status reads are bounded; link/latch getters never wait.

Threaded U14/U16 failures were host-test defects: the fake treated zero-valued
TWF_ANDW as a set bit, completing multi-bit waits prematurely. A direct AND/OR
regression now checks the kernel ABI. The busy reader also yields periodically
so its iteration budget measures coherence while the writer completes. All
original coherence, overlap, update totals and emergency assertions remain.
The enlarged host message storage accommodates the new copied frame.

## Build and physical verification

MQTT requires NET=lwip MQTT=1 and local credentials. Final build verification
uses the published example credentials in ignored build/profiles, not real
network credentials. Those images are compile/link evidence, not configured
network deliverables. No firmware was flashed. End-to-end emergency latency,
real two-core contention, five-minute delivery and outage CPU/timing still need
physical measurements. Results and exact hashes: testing/subsystem1/TEST_LOG.md
and testing/integration/TEST_LOG.md.
"

## Consumer implementation and exact payload bound

Repository inspection found no dashboard application/decoder (no maintained
HTML/JavaScript/TypeScript dashboard sources). The consumer is external;
there is therefore no in-repository dashboard code to update. Implement the
table above as the authoritative version-2 contract, keeping schema 0 support
only for explicitly labelled demonstrations.

Consumers must branch on z, check v before using fields, retain null as unknown,
parse el/er with BigInt or another signed-64 implementation, and parse et as
uint64/BigInt. Never pass these decimal strings through JavaScript Number.
Other integers fit JavaScript's exact integer range. Counts are independent of
speed validity. Track generation and boot/session changes; modulo q/t rollover
does not imply count reset. Do not plot unavailable fields as zero or carry old
values forward as fresh. Display acquisition ages separately from arrival time.
s retains the existing S1 robot-state enum; a bypass is OPERATIONAL, not a new
undocumented wire enum. ml/mr are measured mm/s, not PWM effort.

Exhaustive validity-mask encoding with both signed speeds/counts at their
negative width limits, maximum q/t/et/eg, longest valid h/o/ages proves a
maximum of **239 JSON payload bytes (240 including NUL)**. The largest mask is
59: unknown line is null, which can be longer than a known one-digit mask.
All valid subsets and invalid HUMP-without-IMU masks are tested. Encoding into
an insufficient buffer returns zero and clears output; truncated JSON is never
published. Original FR17 semantic fields are all mapped above; acceptance of
compact field names remains an academic/consumer agreement, not instructor
approval inferred from the user's implementation authorization.
