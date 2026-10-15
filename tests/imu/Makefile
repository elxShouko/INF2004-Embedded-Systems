# Host-side test build for Buddy 4 (no Pico SDK, no uT-Kernel, no hardware).
#
#   make            build + run all tests
#   make replay-demo   build the offline replay tool and run it on a
#                      synthetic RAW log
#   make asan       build + run with AddressSanitizer + UBSan
#   make coverage   build + run with gcov, print line coverage
#   make tidy       clang-tidy static analysis of the production sources
#   make clean

CC      ?= gcc
ROOT    := ../..
SRC     := $(ROOT)/src/imu
INC     := -I$(ROOT)/include -I. -Ifake_tk
CSTD    := -std=c11
WARN    := -Wall -Wextra -Wpedantic -Wshadow -Wdouble-promotion \
           -Wfloat-equal -Wmissing-prototypes -Werror
DEFS    := -DIMU_STATE_TRACE_LEN=64U

PROD    := $(SRC)/imu_params.c $(SRC)/imu_math.c $(SRC)/imu_lsm303dlhc.c \
           $(SRC)/imu_calibration.c $(SRC)/imu_processing.c \
           $(SRC)/imu_hump.c $(SRC)/imu_events.c $(SRC)/imu_terrain.c \
           $(SRC)/imu_rtos.c
TESTS   := sim_lsm303.c scenario.c test_util.c fake_tk/fake_tk.c \
           test_unit_driver.c test_unit_math_cal.c test_unit_processing.c \
           test_unit_hump.c test_unit_events.c test_blackbox.c \
           test_robustness.c test_rtos.c test_main.c

all: run

buddy4_tests: $(PROD) $(TESTS) $(wildcard $(ROOT)/include/imu/*.h) *.h
	$(CC) $(CSTD) -O2 -g $(WARN) $(DEFS) $(INC) $(PROD) $(TESTS) -lm -o $@

run: buddy4_tests
	./buddy4_tests

# Offline replay of RAW logs captured with the bring-up firmware.
REPLAY_SRC := sim_lsm303.c scenario.c test_util.c replay_main.c
buddy4_replay: $(PROD) $(REPLAY_SRC)
	$(CC) $(CSTD) -O2 -g $(WARN) $(DEFS) $(INC) \
	    $(filter-out $(SRC)/imu_rtos.c,$(PROD)) $(REPLAY_SRC) -lm -o $@

replay-demo: buddy4_replay
	./buddy4_replay --gen > synthetic_log.txt
	./buddy4_replay synthetic_log.txt | tail -25

asan: $(PROD) $(TESTS)
	$(CC) $(CSTD) -O1 -g -fsanitize=address,undefined \
	    -fno-sanitize-recover=all -fno-omit-frame-pointer $(WARN) $(DEFS) \
	    $(INC) $(PROD) $(TESTS) -lm -o buddy4_tests_asan
	./buddy4_tests_asan

coverage: $(PROD) $(TESTS)
	rm -rf cov && mkdir -p cov
	$(CC) $(CSTD) -O0 -g --coverage $(DEFS) $(INC) $(PROD) $(TESTS) -lm \
	    -o cov/buddy4_tests_cov
	cd cov && ./buddy4_tests_cov > /dev/null
	cd cov && for f in params math lsm303dlhc calibration processing hump \
	    events terrain rtos; do \
	    printf "%-22s" "imu_$$f.c"; \
	    gcov -n -b buddy4_tests_cov-imu_$$f.gcno 2>/dev/null \
	    | grep -A3 "File '.*imu_$$f.c'" | grep -E "Lines|Taken" \
	    | tr '\n' ' '; echo; done

tidy:
	clang-tidy -checks='-*,clang-analyzer-*,bugprone-*,cert-*,-bugprone-easily-swappable-parameters,-cert-err33-c,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling' \
	    $(PROD) -- $(CSTD) $(DEFS) $(INC) 2>/dev/null \
	    | grep -E "warning:|error:" || echo "clang-tidy: no findings" 

clean:
	rm -rf buddy4_tests buddy4_tests_asan buddy4_replay synthetic_log.txt \
	    cov *.gcda *.gcno

.PHONY: all run asan coverage tidy clean replay-demo
