/** @file subsystem3_test.c
 * @brief Member 3 subsystem3 test module.
 * @note See docs/buddy3/BARR_COMPLIANCE.md for approved exceptions.
 */

/*
 *    subsystem3_test.c
 *    On-target tests for subsystem 3.  Built only with: make TEST=1
 *    Record unit results in testing/subsystem3/TEST_LOG.md and the
 *    INT- result in testing/integration/TEST_LOG.md.
 */

#include "subsystem3_test.h"
#include "../test_report.h"

#if APP_TEST

/**
 * @brief Read retained demo statistics under their shared mutex.
 */
LOCAL APP_STATS read_stats (void)
{
    APP_STATS statistics;

    tk_loc_mtx(mtx_stats, TMO_FEVR);
    statistics = app_stats;
    tk_unl_mtx(mtx_stats);

    return statistics;
}

/** @brief Read kernel buffers on an idle, unconfigured test image. */
static bool check_queue_storage (void)
{
    T_RMBF input_status = {0};
    T_RMBF output_status = {0};
    const ER input_result = tk_ref_mbf(mbf_s3_input, &input_status);
    const ER output_result = tk_ref_mbf(mbf_s3_output, &output_status);

    tm_printf((UB *)"[S3-MEM] input_er=%d free=%u max=%d next=%d\n",
              input_result, (UINT)input_status.frbufsz,
              input_status.maxmsz, input_status.msgsz);
    tm_printf((UB *)"[S3-MEM] output_er=%d free=%u max=%d next=%d\n",
              output_result, (UINT)output_status.frbufsz,
              output_status.maxmsz, output_status.msgsz);
    return (E_OK == input_result) && (E_OK == output_result) &&
        (0 == input_status.msgsz) && (0 == output_status.msgsz) &&
        (336U == input_status.frbufsz) && (384U == output_status.frbufsz) &&
        (79 == input_status.maxmsz) && (19 == output_status.maxmsz);
}

/**
 * @brief Run the professor-provided demonstration checks on target.
 */
EXPORT void subsystem3_test (void)
{
    APP_STATS before;
    APP_STATS after;

    TEST_INIT();
    TEST_CHECK("S3-U37", check_queue_storage(),
               "idle S3 kernel buffers match compact storage budget");

    /* INT-01 (integration, IF-01): samples from subsystem 2 arrive.
       At one sample per 300 ms, 2 statistics should bring at least 5. */
    before = read_stats();
    tk_dly_tsk(2000);
    after = read_stats();
    TEST_CHECK("INT-01", after.received >= before.received + 5U,
               "samples flow from subsystem 2 to subsystem 3");

    /* S3-U01: the statistics are self-consistent */
    TEST_CHECK("S3-U01",
               (after.received > 0U) && (after.min <= after.max) &&
                   (after.min >= 0) && (after.max <= 100),
               "min <= max, both within 0..100");

    TEST_SUMMARY("subsystem3");
}

#endif /* APP_TEST */

/*** End of file. ***/

