# Tribunal 1 and barcode reconciliation review

Reviewed on 2026-10-26 against local main HEAD
`7a61d829d8d88f0759e74968cfeb1ef9d25a33b2` plus the uncommitted overlap cleanup.
Both commits remain ancestors of main:

- Buddy 1: `dc1a2cf2846dcb36dc036b12a0ad590c6590f3a4`, PostTribunal1 Binary Wire Format v3, Map Gateway, Fault Bits.
- Barcode: `3d3ca8de61f3d0a368c0c4cc3405224a8c75b438`, barcode changes.

The working tree combines Buddy 1's transport/protocol/fault features with the
barcode commit's navigation requirements. No further production changes were
needed in this review. This is preservation of functional requirements, not
byte-for-byte preservation of both overlapping mapping implementations.

## Buddy 1 features

| Feature from dc1a2cf | Current result | Evidence |
|---|---|---|
| Fixed 29-byte telemetry, 8-byte heartbeat and 1-byte commands | Preserved; s1_codec.c is unchanged from his commit | Original codec binary suite: 133 checks pass; independent Python oracle: 4000 matching frames |
| Explicit bit packing and validation, signed field encoding | Preserved; s1_bits.h and binary codec unchanged | Original bits/codec golden and boundary tests remain |
| Shared fault flag API, owner restrictions, emergency/Wi-Fi bits | Preserved; s1_fault.h and s1_net.h unchanged | Original binary network checks and S1-U24/25 retained and passing |
| Controller IMU and line-sensor fault reporting | Preserved | vehicle_fault_bit calls and S1_FAULT_IMU/S1_FAULT_LINE_SENSOR hooks retained |
| Dedicated parsed mapping buffer, typed S3 submission and correlated reply | Preserved through s1_net_map_receive, s1_map_pump, s1_map_on_reply | Gateway suite: 43 checks pass; INT-83/84 use real s1_map.c |
| QUEUED versus ACCEPTED/REJECTED heartbeat acknowledgement | Preserved | Only matching S3 replies generate acceptance; gateway tests cover reply matching and timeout |
| Original tests, measurement tools, monitor, decoder and historical evidence | Preserved | All 29 affected files present; 20 unchanged ignoring line endings/final newline; original S1 test source retained plus S1-U27 and spacing |

The complete 29-file comparison and fresh independent-oracle run are saved in
[the preservation audit](../testing/subsystem1/logs/2026-10-26_tribunal1_preservation_audit.json).

## Barcode requirements

| Requirement from barcode changes | Current result | Evidence |
|---|---|---|
| Runtime PC mapping rather than hard-coded movement assignments | Preserved; S3 boots with every letter disabled | S3-U38; S1-U27 verifies full map and receive timestamp through dedicated buffer |
| Four distinct letters, exactly LEFT/RIGHT/FORWARD/UTURN | Preserved; accepted map completely replaces old assignments | S3-U38/39 and nav_map_install |
| Read Code 39 A-Z in either direction | Preserved | Existing decoder/navigation suites and S3-U39 |
| Store the mapped instruction and continue line following until a confirmed junction | Preserved | S3-U39 checks TRACKING with no manoeuvre ID before junction |
| Barcode observation alone does not trigger a finite manoeuvre | Preserved | INT-82 |
| Junction carries the stored action and initiates the motor-owner handshake | Preserved | S3-U39; INT-82/85 |
| All four manoeuvres, completion correlation, line recovery and repeat scanning | Preserved in software | S3-U39 exercises all four actions, both scan directions, two scan/junction/recovery cycles; existing completion/recovery tests retained |
| Pending instruction cannot be changed by a new map | Preserved | S3-U39 verifies rejected mapping while pending; original S3 owner input guard retained |
| Queue-full mapping admission retries without blocking emergency processing | Preserved through consolidated gateway | Gateway M5/M10; INT-83 and original independent emergency latch checks |

`git diff 3d3ca8d -- app_program/subsystem3` is empty: the entire S3 production
folder still matches the barcode commit. In vehicle.c, the only difference
from that commit is the restored ordinary-command maximum S1_CMD_RESUME.
Its junction trigger and manoeuvre behaviour are unchanged. Mapping now has
its own Buddy 1 buffer, so S1_CMD_MAPPING is unnecessary in that ordinary enum.

## Consolidation and explicit policy differences

The runtime flow is:

```text
PC MAP text -> S1 MQTT hook -> S1 dedicated map buffer
            -> controller calls s1_map_pump -> typed S3 mapping request
            -> S3 accepts/rejects -> s1_map_on_reply -> binary heartbeat ACK
```

The duplicate four-letter codec parser, extra ordinary-command payload and
UINT32_MAX controller submission path introduced by the barcode commit were
retired. Their runtime mapping purpose is provided by Buddy 1's gateway.
Conflicted duplicate test copies and non-C pasted text were removed. Buddy 1's
original functional tests are retained.

One deliberate change to Buddy 1's original gateway remains: an S3 queue-full
return now retries the same copied request until its original receive age
exceeds 1000 ms, instead of immediately reporting REJECTED. Stale/future
requests are rejected. This carries over the barcode commit's bounded retry
behaviour. Other submit failures, reply rejection and reply timeout remain
reported as rejection.

Buddy 1's generic partial-map syntax parser remains available. The barcode
commit intentionally made production S3 require a complete four-action map;
partial syntax examples therefore need not be accepted by S3. These two
original behaviours cannot both govern map installation. The current S3 rule
preserves the stated barcode demo requirements, and rejected updates preserve
the previous installed map.

## Verification and limits

Reviewed existing results against current source SHA-256 hashes; there are no
mismatches for the recorded S1, S3, integration or firmware source inputs:

- [S1 final results](../testing/subsystem1/logs/2026-10-26_overlap_final.json): all checks pass, including 63 binary and 45 JSON emulated-kernel checks, 68 binary network checks and 43 gateway checks.
- [S3 results](../testing/subsystem3/logs/2026-10-26_overlap_2.json): all suites pass, including S3-U38/39.
- [Integration results](../testing/integration/TEST_LOG.md): 82 groups per default and optimized/fshort-enums profile; INT-82 through INT-85 pass; eight intentional mutations detected per profile.
- [Firmware metadata](../testing/integration/logs/overlap_20261026_build_metadata.json): default, TEST=1, NET=lwip MQTT=1, and MQTT TEST=1 compile/link/UF2 profiles pass. Exact commands and build-only example header details are in the integration record.

These are static inspection, host simulation and compilation results. No
radio delivery, target SMP timing, physical barcode/junction alignment or
actual manoeuvre accuracy has been demonstrated. Production still requires
an explicit calibrated vehicle_runtime_setup profile, power authorization,
fresh providers and actual hardware acceptance tests. No commit or push was
performed; cleanup changes remain in the local main working tree.
