# ARM application representation inventory

Baseline 93740736dd1c3407eaf7f496089a096cee70c140. Declaration inventory, not a compliance certificate.
Numeric ranges and calibration are unresolved unless explicitly justified.

## Target structure sizes

| Type | ARM bytes | Source |
|---|---:|---|
| `drv_encoder_port_t` | 16 | `app_program/subsystem2/drv_encoder.h` |
| `drv_motor_config_t` | 24 | `app_program/subsystem2/drv_motor.h` |
| `drv_motor_io_t` | 16 | `app_program/subsystem2/drv_motor.h` |
| `drv_motor_channel_t` | 24 | `app_program/subsystem2/drv_motor.h` |
| `drv_motor_t` | 104 | `app_program/subsystem2/drv_motor.h` |
| `drv_rp2040_t` | 6 | `app_program/subsystem2/drv_rp2040.h` |
| `enc_capture_sample_t` | 64 | `app_program/subsystem2/enc_capture.h` |
| `enc_capture_t` | 40 | `app_program/subsystem2/enc_capture.h` |
| `motion_bus_config_t` | 204 | `app_program/subsystem2/motion_bus.h` |
| `motion_bus_payload_t` | 208 | `app_program/subsystem2/motion_bus.h` |
| `motion_bus_request_t` | 216 | `app_program/subsystem2/motion_bus.h` |
| `motion_bus_t` | 360 | `app_program/subsystem2/motion_bus.h` |
| `motion_config_wheel_t` | 12 | `app_program/subsystem2/motion_config.h` |
| `motion_config_t` | 36 | `app_program/subsystem2/motion_config.h` |
| `motion_estimator_sample_t` | 24 | `app_program/subsystem2/motion_estimator.h` |
| `motion_estimator_measurement_t` | 32 | `app_program/subsystem2/motion_estimator.h` |
| `motion_estimator_t` | 88 | `app_program/subsystem2/motion_estimator.h` |
| `motion_pid_config_t` | 36 | `app_program/subsystem2/motion_pid.h` |
| `motion_pid_t` | 52 | `app_program/subsystem2/motion_pid.h` |
| `motion_quad_t` | 16 | `app_program/subsystem2/motion_quad.h` |
| `motion_run_status_t` | 136 | `app_program/subsystem2/motion_run.h` |
| `motion_run_t` | 968 | `app_program/subsystem2/motion_run.h` |
| `move_plan_command_t` | 40 | `app_program/subsystem2/move_plan.h` |
| `move_plan_config_t` | 28 | `app_program/subsystem2/move_plan.h` |
| `move_plan_t` | 72 | `app_program/subsystem2/move_plan.h` |
| `SAMPLE_MSG` | 20 | `app_program/subsystem2/subsystem2.h` |
| `wheel_loop_config_t` | 188 | `app_program/subsystem2/wheel_loop.h` |
| `wheel_loop_t` | 736 | `app_program/subsystem2/wheel_loop.h` |
| `vehicle_config_t` | 96 | `app_program/vehicle.h` |
| `vehicle_input_t` | 400 | `app_program/vehicle.h` |
| `vehicle_output_t` | 64 | `app_program/vehicle.h` |
| `vehicle_t` | 288 | `app_program/vehicle.h` |
| `vehicle_odometry_t` | 48 | `app_program/vehicle.h` |
| `vehicle_setup_t` | 432 | `app_program/vehicle_rt.h` |
| `gpio_bank_io_t` | 28 | `app_program/gpio_bank.h` |
| `gpio_bank_client_t` | 16 | `app_program/gpio_bank.h` |
| `gpio_bank_t` | 68 | `app_program/gpio_bank.h` |

## drv_encoder_port_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `void * p_context` | 0 | 4 | TBD | ABI pointer/callback; interpretation clarification needed |
| `bool (*p_reserve)(void * p_context, uint32_t mask, drv_encoder_sink_t p_sink, void * p_sink_context)` | 4 | 4 | 1 | false/true; preserve current ownership |
| `bool (*p_enable)(void * p_context, uint32_t mask)` | 8 | 4 | 1 | false/true; preserve current ownership |
| `void (*p_release)(void * p_context, uint32_t mask)` | 12 | 4 | TBD | ABI pointer/callback; interpretation clarification needed |

## drv_motor_config_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `uint32_t clock_hz` | 0 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t pwm_hz` | 4 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t reversal_guard_us` | 8 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_pid_value_t max_duty` | 12 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_types_polarity_t polarity[DRV_MOTOR_WHEELS]` | 16 | 2 | 2 | current enum values; invalid-input checks remain required |
| `drv_motor_stop_t zero_mode` | 18 | 1 | 1 | current enum values; invalid-input checks remain required |
| `motion_config_calibration_t calibration` | 19 | 1 | 2 | current enum values; invalid-input checks remain required |
| `bool b_resources_verified` | 20 | 1 | 1 | false/true; preserve current ownership |

## drv_motor_io_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `void * p_context` | 0 | 4 | TBD | ABI pointer/callback; interpretation clarification needed |
| `bool (*p_prepare)(void * p_context, uint16_t period, uint32_t clock_hz)` | 4 | 4 | 1 | false/true; preserve current ownership |
| `bool (*p_apply)(void * p_context, drv_motor_wheel_t wheel, uint16_t level_a, uint16_t level_b)` | 8 | 4 | 1 | false/true; preserve current ownership |
| `bool (*p_stop_all)(void * p_context)` | 12 | 4 | 1 | false/true; preserve current ownership |

## drv_motor_channel_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `drv_motor_state_t state` | 0 | 1 | 2 | current enum values; invalid-input checks remain required |
| `int8_t last_direction` | 1 | 1 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int8_t pending_direction` | 2 | 1 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint16_t pending_level` | 4 | 2 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t neutral_since_us` | 8 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_neutral` | 16 | 1 | 1 | false/true; preserve current ownership |

## drv_motor_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `drv_motor_config_t config` | 0 | 24 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `drv_motor_io_t io` | 24 | 16 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `drv_motor_channel_t channel[DRV_MOTOR_WHEELS]` | 40 | 48 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t last_time_us` | 88 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint16_t period` | 96 | 2 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint16_t duty_limit` | 98 | 2 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_initialised : 1` | None | None | 1 | false/true; preserve current ownership |
| `bool b_armed : 1` | None | None | 1 | false/true; preserve current ownership |
| `bool b_fault : 1` | None | None | 1 | false/true; preserve current ownership |
| `bool b_prepare_attempted : 1` | None | None | 1 | false/true; preserve current ownership |
| `bool b_stop_confirmed : 1` | None | None | 1 | false/true; preserve current ownership |

## drv_rp2040_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `uint16_t period` | 0 | 2 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int8_t direction[DRV_MOTOR_WHEELS]` | 2 | 2 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_claimed` | 4 | 1 | 1 | false/true; preserve current ownership |

## enc_capture_sample_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `motion_estimator_sample_t sample[2]` | 0 | 48 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t invalid[2]` | 48 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_healthy` | 56 | 1 | 1 | false/true; preserve current ownership |

## enc_capture_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `motion_quad_t decoder[2]` | 0 | 32 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t generation` | 32 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_initialised` | 36 | 1 | 1 | false/true; preserve current ownership |
| `bool b_healthy` | 37 | 1 | 1 | false/true; preserve current ownership |

## motion_bus_config_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `wheel_loop_config_t config` | 0 | 188 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `drv_encoder_port_t encoder_port` | 188 | 16 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |

## motion_bus_payload_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `motion_bus_config_t configure` | 0 | 204 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `move_plan_command_t command` | 0 | 40 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t ack_id` | 0 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |

## motion_bus_request_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `motion_bus_kind_t kind` | 0 | 1 | 2 | current enum values; invalid-input checks remain required |
| `motion_bus_payload_t payload` | 8 | 208 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |

## motion_bus_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `motion_bus_request_t request` | 0 | 216 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_run_status_t status` | 216 | 136 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t cancel_id` | 352 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_pending` | 356 | 1 | 1 | false/true; preserve current ownership |
| `bool b_inflight` | 357 | 1 | 1 | false/true; preserve current ownership |
| `bool b_stop` | 358 | 1 | 1 | false/true; preserve current ownership |
| `bool b_cancel` | 359 | 1 | 1 | false/true; preserve current ownership |

## motion_config_wheel_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `uint32_t counts_per_wheel_rev` | 0 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t wheel_circumference_um` | 4 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_types_polarity_t motor_polarity` | 8 | 1 | 2 | current enum values; invalid-input checks remain required |
| `motion_types_polarity_t encoder_polarity` | 9 | 1 | 2 | current enum values; invalid-input checks remain required |

## motion_config_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `motion_config_wheel_t left` | 0 | 12 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_config_wheel_t right` | 12 | 12 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t track_width_um` | 24 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_types_speed_mm_s_t max_wheel_speed_mm_s` | 28 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_config_calibration_t calibration` | 32 | 1 | 2 | current enum values; invalid-input checks remain required |

## motion_estimator_sample_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `motion_types_encoder_count_t count` | 0 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t timestamp_us` | 8 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t generation` | 16 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |

## motion_estimator_measurement_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `motion_types_distance_um_t position_um` | 0 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_types_speed_mm_s_t speed_mm_s` | 8 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t timestamp_us` | 16 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_position_valid` | 24 | 1 | 1 | false/true; preserve current ownership |
| `bool b_speed_valid` | 25 | 1 | 1 | false/true; preserve current ownership |

## motion_estimator_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `uint32_t counts_per_wheel_rev` | 0 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t circumference_um` | 4 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t max_elapsed_us` | 8 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_estimator_sample_t previous` | 16 | 24 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_estimator_measurement_t measurement` | 40 | 32 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int64_t remainder` | 72 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_estimator_result_t result` | 80 | 1 | 4 | current enum values; invalid-input checks remain required |
| `bool b_initialised` | 81 | 1 | 1 | false/true; preserve current ownership |
| `bool b_have_baseline` | 82 | 1 | 1 | false/true; preserve current ownership |
| `bool b_reset_required` | 83 | 1 | 1 | false/true; preserve current ownership |

## motion_pid_config_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `motion_pid_value_t proportional_gain` | 0 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_pid_value_t integral_gain` | 4 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_pid_value_t derivative_gain` | 8 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_pid_value_t output_min` | 12 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_pid_value_t output_max` | 16 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_pid_value_t integral_min` | 20 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_pid_value_t integral_max` | 24 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t max_elapsed_us` | 28 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_config_calibration_t calibration` | 32 | 1 | 2 | current enum values; invalid-input checks remain required |

## motion_pid_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `motion_pid_config_t config` | 0 | 36 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_pid_value_t integral` | 36 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_types_speed_mm_s_t previous_measurement_mm_s` | 40 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_types_speed_mm_s_t previous_target_mm_s` | 44 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_have_measurement` | 48 | 1 | 1 | false/true; preserve current ownership |
| `bool b_initialised` | 49 | 1 | 1 | false/true; preserve current ownership |

## motion_quad_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `motion_types_encoder_count_t count` | 0 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t invalid_transitions` | 8 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_types_polarity_t polarity` | 12 | 1 | 2 | current enum values; invalid-input checks remain required |
| `uint8_t previous_ab` | 13 | 1 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_initialised : 1` | None | None | 1 | false/true; preserve current ownership |
| `bool b_have_state : 1` | None | None | 1 | false/true; preserve current ownership |
| `bool b_count_limit : 1` | None | None | 1 | false/true; preserve current ownership |

## motion_run_status_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `uint64_t timestamp_us` | 0 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_run_state_t state` | 8 | 1 | 3 | current enum values; invalid-input checks remain required |
| `uint32_t active_id` | 12 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `move_plan_kind_t active_kind` | 16 | 1 | 3 | current enum values; invalid-input checks remain required |
| `uint32_t terminal_id` | 20 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_run_result_t terminal_result` | 24 | 1 | 4 | current enum values; invalid-input checks remain required |
| `motion_run_result_t fault_result` | 25 | 1 | 4 | current enum values; invalid-input checks remain required |
| `uint32_t last_request_id` | 28 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t last_request_issued_us` | 32 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_request_accepted` | 40 | 1 | 1 | false/true; preserve current ownership |
| `uint64_t measurement_timestamp_us` | 48 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t measurement_max_age_us` | 56 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int64_t position_um[2]` | 64 | 16 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t speed_mm_s[2]` | 80 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t target_mm_s[2]` | 88 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_measurement_valid` | 96 | 1 | 1 | false/true; preserve current ownership |
| `bool b_standstill` | 97 | 1 | 1 | false/true; preserve current ownership |
| `bool b_stop_command_confirmed` | 98 | 1 | 1 | false/true; preserve current ownership |
| `motion_types_encoder_count_t encoder_count[2]` | 104 | 16 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t encoder_timestamp_us` | 120 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t encoder_generation` | 128 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_encoder_valid` | 132 | 1 | 1 | false/true; preserve current ownership |

## motion_run_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `wheel_loop_t loop` | 0 | 736 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `move_plan_t plan` | 736 | 72 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_run_status_t status` | 808 | 136 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t deadline_us` | 944 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t stopping_since_us` | 952 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t last_id` | 960 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_run_result_t pending_result` | 964 | 1 | 4 | current enum values; invalid-input checks remain required |
| `bool b_initialised` | 965 | 1 | 1 | false/true; preserve current ownership |

## move_plan_command_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `uint32_t id` | 0 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `move_plan_kind_t kind` | 4 | 1 | 3 | current enum values; invalid-input checks remain required |
| `uint64_t issued_us` | 8 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t lifetime_us` | 16 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t left_mm_s` | 20 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t right_mm_s` | 24 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t distance_um` | 28 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t cruise_mm_s` | 32 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |

## move_plan_config_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `uint32_t accel_mm_s2` | 0 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t decel_mm_s2` | 4 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t tolerance_um` | 8 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t turn_scale_ppm[2]` | 12 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_pid_value_t correction_gain` | 20 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t correction_limit_mm_s` | 24 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |

## move_plan_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `move_plan_command_t command` | 0 | 40 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int64_t origin_um[2]` | 40 | 16 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t goal_um[2]` | 56 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_finite` | 64 | 1 | 1 | false/true; preserve current ownership |
| `bool b_initialised` | 65 | 1 | 1 | false/true; preserve current ownership |

## SAMPLE_MSG

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `T_MSG hdr` | 0 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `UW seq` | 4 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `UW time_ms` | 8 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `INT value` | 12 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `INT prc` | 16 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |

## wheel_loop_config_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `motion_config_t geometry` | 0 | 36 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `drv_motor_config_t motor` | 36 | 24 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_pid_config_t pid[2]` | 60 | 72 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `move_plan_config_t profile` | 132 | 28 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t period_us` | 160 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t sample_limit_us` | 164 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t stop_dwell_us` | 168 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t stop_timeout_us` | 172 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t progress_timeout_us` | 176 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t stop_speed_mm_s` | 180 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t overspeed_mm_s` | 184 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |

## wheel_loop_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `wheel_loop_config_t config` | 0 | 188 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `drv_motor_t motor` | 192 | 104 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_estimator_t estimator[2]` | 296 | 176 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_pid_t pid[2]` | 472 | 104 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_estimator_measurement_t measured[2]` | 576 | 64 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int64_t velocity_um_s[2]` | 640 | 16 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t slew_remainder[2]` | 656 | 16 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int64_t last_count[2]` | 672 | 16 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t quiet_since_us[2]` | 688 | 16 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t progress_since_us[2]` | 704 | 16 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t last_tick_us` | 720 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_quiet[2]` | 728 | 2 | 1 | false/true; preserve current ownership |
| `bool b_standstill[2]` | 730 | 2 | 1 | false/true; preserve current ownership |
| `bool b_tracking[2]` | 732 | 2 | 1 | false/true; preserve current ownership |
| `bool b_valid : 1` | None | None | 1 | false/true; preserve current ownership |
| `bool b_enabled : 1` | None | None | 1 | false/true; preserve current ownership |
| `bool b_fault : 1` | None | None | 1 | false/true; preserve current ownership |

## vehicle_config_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `uint32_t sensor_age_us` | 0 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t command_ttl_us` | 4 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t transition_timeout_us` | 8 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t remote_age_ms` | 12 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t forward_mm_s` | 16 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t correction_mm_s` | 20 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t turn_mm_s` | 24 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int32_t straight_um` | 28 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint16_t obstacle_stop_cm` | 32 | 2 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_calibrated` | 34 | 1 | 1 | false/true; preserve current ownership |
| `bool b_power_authorised` | 35 | 1 | 1 | false/true; preserve current ownership |
| `bool b_uturn_right` | 36 | 1 | 1 | false/true; preserve current ownership |
| `s5_route_config_t route` | 40 | 56 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |

## vehicle_input_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `uint64_t now_us` | 0 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t raw_now_us` | 8 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `motion_run_status_t motion` | 16 | 136 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `s1_codec_command_t remote` | 152 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `nav_event_t navigation` | 160 | 16 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t navigation_id` | 176 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t pending_navigation_id` | 180 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t line_time_us` | 184 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `line_observation_t line` | 192 | 1 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `line_state_t line_state` | 193 | 1 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `line_mode_t line_mode` | 194 | 1 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint8_t black_mask` | 195 | 1 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `imu_core_status_t imu` | 196 | 32 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `s5_obstacle_profile_t obstacle` | 228 | 20 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `s5_clearance_t clearance` | 248 | 80 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t scan_id_us` | 328 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t scan_completed_us` | 336 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_scan_held` | 344 | 1 | 1 | false/true; preserve current ownership |
| `uint16_t forward_cm` | 346 | 2 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_forward_valid` | 348 | 1 | 1 | false/true; preserve current ownership |
| `uint64_t obstacle_started_us` | 352 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t obstacle_time_us` | 360 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `int64_t encoder_count[2]` | 368 | 16 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_motion_available` | 384 | 1 | 1 | false/true; preserve current ownership |
| `bool b_line_available` | 385 | 1 | 1 | false/true; preserve current ownership |
| `bool b_line_calibrated` | 386 | 1 | 1 | false/true; preserve current ownership |
| `bool b_line_valid` | 387 | 1 | 1 | false/true; preserve current ownership |
| `bool b_imu_available` | 388 | 1 | 1 | false/true; preserve current ownership |
| `bool b_obstacle_available` | 389 | 1 | 1 | false/true; preserve current ownership |
| `bool b_counts_valid` | 390 | 1 | 1 | false/true; preserve current ownership |
| `bool b_link_up` | 391 | 1 | 1 | false/true; preserve current ownership |
| `bool b_estop` | 392 | 1 | 1 | false/true; preserve current ownership |
| `bool b_provider_fault` | 393 | 1 | 1 | false/true; preserve current ownership |

## vehicle_output_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `move_plan_command_t command` | 0 | 40 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t acknowledge_id` | 40 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t navigation_done_id` | 44 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t obstacle_recovered_us` | 48 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `line_mode_t mode` | 56 | 1 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_command` | 57 | 1 | 1 | false/true; preserve current ownership |
| `bool b_enable` | 58 | 1 | 1 | false/true; preserve current ownership |
| `bool b_stop` | 59 | 1 | 1 | false/true; preserve current ownership |
| `bool b_estop` | 60 | 1 | 1 | false/true; preserve current ownership |
| `bool b_mode` | 61 | 1 | 1 | false/true; preserve current ownership |

## vehicle_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `vehicle_config_t config` | 0 | 96 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `vehicle_state_t state` | 96 | 1 | 4 | current enum values; invalid-input checks remain required |
| `uint32_t blockers` | 100 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t next_id` | 104 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t active_id` | 108 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t navigation_id` | 112 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t last_navigation_id` | 116 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `nav_command_t navigation` | 120 | 1 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t transition_us` | 128 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t last_now_us` | 136 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_initialised` | 144 | 1 | 1 | false/true; preserve current ownership |
| `bool b_run_requested` | 145 | 1 | 1 | false/true; preserve current ownership |
| `bool b_pause_requested` | 146 | 1 | 1 | false/true; preserve current ownership |
| `bool b_turn_completed` | 147 | 1 | 1 | false/true; preserve current ownership |
| `bool b_recovery_required` | 148 | 1 | 1 | false/true; preserve current ownership |
| `bool b_pending_command` | 149 | 1 | 1 | false/true; preserve current ownership |
| `bool b_line_found` | 150 | 1 | 1 | false/true; preserve current ownership |
| `move_plan_command_t pending_command` | 152 | 40 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `s5_obstacle_plan_t route` | 192 | 64 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t route_started_us` | 256 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t route_step_raw_us` | 264 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t route_ready_raw_us` | 272 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint8_t route_index` | 280 | 1 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_obstacle_pending` | 281 | 1 | 1 | false/true; preserve current ownership |
| `bool b_route_active` | 282 | 1 | 1 | false/true; preserve current ownership |
| `bool b_route_terminal` | 283 | 1 | 1 | false/true; preserve current ownership |
| `bool b_route_found` | 284 | 1 | 1 | false/true; preserve current ownership |
| `bool b_route_finishing` | 285 | 1 | 1 | false/true; preserve current ownership |

## vehicle_odometry_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `int64_t previous_um[2]` | 0 | 16 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t previous_us` | 16 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t distance_um` | 24 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint64_t raw_timestamp_us` | 32 | 8 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint8_t half_um` | 40 | 1 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_baseline` | 41 | 1 | 1 | false/true; preserve current ownership |
| `bool b_previous_forward` | 42 | 1 | 1 | false/true; preserve current ownership |
| `bool b_tracking_forward` | 43 | 1 | 1 | false/true; preserve current ownership |

## vehicle_setup_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `vehicle_config_t mission` | 0 | 96 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `wheel_loop_config_t motion` | 96 | 188 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `s3_config_t navigation` | 284 | 84 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `s5_live_config_t obstacle` | 368 | 60 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_obstacle_requested` | 428 | 1 | 1 | false/true; preserve current ownership |

## gpio_bank_io_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `uint32_t (*p_read)(uint32_t address)` | 0 | 4 | TBD | ABI pointer/callback; interpretation clarification needed |
| `void (*p_write)(uint32_t address, uint32_t value)` | 4 | 4 | TBD | ABI pointer/callback; interpretation clarification needed |
| `uint32_t (*p_core)(void)` | 8 | 4 | TBD | ABI pointer/callback; interpretation clarification needed |
| `bool (*p_enter)(uint32_t * p_saved)` | 12 | 4 | 1 | false/true; preserve current ownership |
| `void (*p_leave)(uint32_t saved)` | 16 | 4 | TBD | ABI pointer/callback; interpretation clarification needed |
| `bool (*p_install)(void)` | 20 | 4 | 1 | false/true; preserve current ownership |
| `void (*p_fault)(void)` | 24 | 4 | TBD | ABI pointer/callback; interpretation clarification needed |

## gpio_bank_client_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `uint32_t mask` | 0 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `drv_encoder_sink_t p_sink` | 4 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `void * p_context` | 8 | 4 | TBD | ABI pointer/callback; interpretation clarification needed |
| `bool b_enabled` | 12 | 1 | 1 | false/true; preserve current ownership |

## gpio_bank_t

| Declaration | Offset | Bytes | Logical bits per element | Range basis |
|---|---:|---:|---:|---|
| `gpio_bank_io_t io` | 0 | 28 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `gpio_bank_client_t clients[GPIO_BANK_CLIENTS]` | 28 | 32 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `uint32_t allowed_mask` | 60 | 4 | TBD | review units/range/resolution/lifetime; no narrowing authorised by inventory |
| `bool b_initialised` | 64 | 1 | 1 | false/true; preserve current ownership |
| `bool b_installed` | 65 | 1 | 1 | false/true; preserve current ownership |
