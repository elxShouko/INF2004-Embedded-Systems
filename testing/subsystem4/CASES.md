# Stable host test registry

Define cases before running them. These IDs supplement the existing monitor
S4-U01/S4-U02; they never replace those cases. Case names describe the archived
behavior exercised, not an assertion that the current firmware implements it.

Method for every row: run `bash testing/subsystem4/run_host_tests.sh normal`
or `sanitize`. Expected result: every assertion in the named C test succeeds
and the runner prints `[TEST] <ID> PASS <name>`. The test functions contain the
specific input sequences, tolerances and expected values. Those tolerances are
synthetic regression criteria, not approved physical acceptance limits.

Six wrapper cases S4-U110–S4-U115 execute sequentially and characterize the old
implementation. Their PASS status does not resolve the integration defects
listed in `REFERENCE_REVIEW.md`.

| ID | Test case | Code | Requirement / scope |
|---|---|---|---|
| S4-U003 | decode accel | [t_decode_accel](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U004 | decode mag | [t_decode_mag](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U005 | scale tables | [t_scale_tables](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U006 | init registers | [t_init_registers](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U007 | init failures | [t_init_failures](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U008 | fifo read | [t_fifo_read](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U009 | fifo overrun | [t_fifo_overrun](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U010 | single mode | [t_single_mode](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U011 | mag order and scale | [t_mag_order_and_scale](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U012 | mag overflow | [t_mag_overflow](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U013 | retries | [t_retries](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U014 | recover | [t_recover](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U015 | not ready and null | [t_not_ready_and_null](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U016 | signed and clipping | [t_signed_and_clipping](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U017 | shutdown | [t_shutdown](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U018 | other fs and odr | [t_other_fs_and_odr](host/test_unit_driver.c) | Write-up p.8: IMU integration; DR FR7 |
| S4-U019 | wrap | [t_wrap](host/test_unit_math_cal.c) | Write-up p.8: sensor calibration; DR FR7, A-B4-1 |
| S4-U020 | lpf | [t_lpf](host/test_unit_math_cal.c) | Write-up p.8: sensor calibration; DR FR7, A-B4-1 |
| S4-U021 | align to z | [t_align_to_z](host/test_unit_math_cal.c) | Write-up p.8: sensor calibration; DR FR7, A-B4-1 |
| S4-U022 | frame map | [t_frame_map](host/test_unit_math_cal.c) | Write-up p.8: sensor calibration; DR FR7, A-B4-1 |
| S4-U023 | params validate | [t_params_validate](host/test_unit_math_cal.c) | Write-up p.8: sensor calibration; DR FR7, A-B4-1 |
| S4-U024 | accel cal ok | [t_accel_cal_ok](host/test_unit_math_cal.c) | Write-up p.8: sensor calibration; DR FR7, A-B4-1 |
| S4-U025 | accel cal mount tilt | [t_accel_cal_mount_tilt](host/test_unit_math_cal.c) | Write-up p.8: sensor calibration; DR FR7, A-B4-1 |
| S4-U026 | accel cal failures | [t_accel_cal_failures](host/test_unit_math_cal.c) | Write-up p.8: sensor calibration; DR FR7, A-B4-1 |
| S4-U027 | mag cal | [t_mag_cal](host/test_unit_math_cal.c) | Write-up p.8: sensor calibration; DR FR7, A-B4-1 |
| S4-U028 | pitch roll sign | [t_pitch_roll_sign](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U029 | attitude reliability | [t_attitude_reliability](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U030 | heading cardinal | [t_heading_cardinal](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U031 | heading tilt compensated | [t_heading_tilt_compensated](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U032 | turn rate wraparound | [t_turn_rate_wraparound](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U033 | turn left right | [t_turn_left_right](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U034 | turn debounce | [t_turn_debounce](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U035 | heading disturbed | [t_heading_disturbed](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U036 | heading outlier | [t_heading_outlier](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U037 | uncalibrated mag | [t_uncalibrated_mag](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U038 | tilt detector | [t_tilt_detector](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U039 | impact detector | [t_impact_detector](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U040 | impact cooldown | [t_impact_cooldown](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U041 | classifier | [t_classifier](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U042 | classifier no flapping | [t_classifier_no_flapping](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U043 | hint validation | [t_hint_validation](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U044 | nan input | [t_nan_input](host/test_unit_processing.c) | DR FR7, FR9, NFR5; Briefing p.7 |
| S4-U045 | canonical | [t_canonical](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U046 | boundaries | [t_boundaries](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U047 | spike rejected | [t_spike_rejected](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U048 | slope and no descent | [t_slope_and_no_descent](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U049 | artefact then real hump | [t_artefact_then_real_hump](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U050 | multiple retain max | [t_multiple_retain_max](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U051 | no double count | [t_no_double_count](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U052 | rearm | [t_rearm](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U053 | speed variation | [t_speed_variation](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U054 | reset and abort | [t_reset_and_abort](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U055 | nan ignored | [t_nan_ignored](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U056 | height estimates | [t_height_estimates](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U057 | hint gating | [t_hint_gating](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U058 | names | [t_names](host/test_unit_hump.c) | Write-up p.3/p.8; DR FR8, NFR5, A-B4-2 |
| S4-U059 | seq and names | [t_seq_and_names](host/test_unit_events.c) | DR FR9, NFR13, §4 and §8.2 |
| S4-U060 | overflow policy | [t_overflow_policy](host/test_unit_events.c) | DR FR9, NFR13, §4 and §8.2 |
| S4-U061 | pending ring bound | [t_pending_ring_bound](host/test_unit_events.c) | DR FR9, NFR13, §4 and §8.2 |
| S4-U062 | rate limit | [t_rate_limit](host/test_unit_events.c) | DR FR9, NFR13, §4 and §8.2 |
| S4-U063 | sink errors | [t_sink_errors](host/test_unit_events.c) | DR FR9, NFR13, §4 and §8.2 |
| S4-U064 | critical classification | [t_critical_classification](host/test_unit_events.c) | DR FR9, NFR13, §4 and §8.2 |
| S4-U065 | init failure then success | [bb01_init_failure_then_success](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U066 | wrong device | [bb02_wrong_device](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U067 | calibration while moving | [bb03_calibration_while_moving](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U068 | stationary | [bb04_stationary](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U069 | mounting | [bb05_mounting](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U070 | straight cruise | [bb06_straight_cruise](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U071 | accel decel with hint | [bb07_accel_decel_with_hint](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U072 | accel without hint | [bb07b_accel_without_hint](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U073 | turns | [bb08_turns](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U074 | small hump | [bb09_small_hump](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U075 | large hump | [bb10_large_hump](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U076 | multiple humps | [bb11_multiple_humps](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U077 | noise no false humps | [bb12_noise_no_false_humps](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U078 | impact collision | [bb13_impact_collision](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U079 | repeated impacts | [bb14_repeated_impacts](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U080 | tilt | [bb15_tilt](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U081 | disconnect and recover | [bb16_disconnect_and_recover](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U082 | persistent fault | [bb17_persistent_fault](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U083 | invalid and frozen data | [bb18_invalid_and_frozen_data](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U084 | intermittent absorbed | [bb19_intermittent_absorbed](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U085 | queue overflow | [bb20_queue_overflow](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U086 | mission restart | [bb21_mission_restart](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U087 | hump during turn | [bb22_hump_during_turn](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U088 | fast hump no collision | [bb23_fast_hump_no_collision](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U089 | speed repeatability | [bb24_speed_repeatability](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U090 | mag calibration spin | [bb25_mag_calibration_spin](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U091 | magnetic disturbance | [bb26_magnetic_disturbance](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U092 | reset and recalibrate and stop | [bb27_reset_and_recalibrate_and_stop](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U093 | telemetry json | [bb28_telemetry_json](host/test_blackbox.c) | DR FR7–FR9, §6.4, A-B4-1/A-B4-2 |
| S4-U094 | i2c garbage fuzz | [p01_i2c_garbage_fuzz](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U095 | failure ratio sweep | [p02_failure_ratio_sweep](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U096 | extreme values | [p03_extreme_values](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U097 | api misuse | [p04_api_misuse](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U098 | command flood | [p05_command_flood](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U099 | hint spoofing | [p06_hint_spoofing](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U100 | hint replay | [p07_hint_replay](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U101 | parameter fuzz | [p08_parameter_fuzz](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U102 | time wraparound | [p09_time_wraparound](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U103 | time jumps | [p10_time_jumps](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U104 | event storm | [p11_event_storm](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U105 | nan injection | [p12_nan_injection](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U106 | mag overflow stream | [p13_mag_overflow_stream](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U107 | frozen boundary | [p14_frozen_boundary](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U108 | memory budget | [p15_memory_budget](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U109 | soak | [p16_soak](host/test_robustness.c) | DR NFR5/NFR14; §6.4 fault handling |
| S4-U110 | not started | [r01_not_started](host/test_rtos.c) | Archived wrapper characterization only; see REFERENCE_REVIEW.md |
| S4-U111 | creation failure | [r02_creation_failure](host/test_rtos.c) | Archived wrapper characterization only; see REFERENCE_REVIEW.md |
| S4-U112 | objects and attributes | [r03_objects_and_attributes](host/test_rtos.c) | Archived wrapper characterization only; see REFERENCE_REVIEW.md |
| S4-U113 | task loop and api | [r04_task_loop_and_api](host/test_rtos.c) | Archived wrapper characterization only; see REFERENCE_REVIEW.md |
| S4-U114 | restart hump collision overflow | [r05_restart_hump_collision_overflow](host/test_rtos.c) | Archived wrapper characterization only; see REFERENCE_REVIEW.md |
| S4-U115 | lock contention and timeouts | [r06_lock_contention_and_timeouts](host/test_rtos.c) | Archived wrapper characterization only; see REFERENCE_REVIEW.md |
