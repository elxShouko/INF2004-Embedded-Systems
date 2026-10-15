/**
 * @file    test_unit_events.c
 * @brief   Unit tests: bounded event dispatcher (overflow policy, critical
 *          retention, ordering, rate limiting, notification).
 */

#include <string.h>

#include "imu/imu_events.h"
#include "test_util.h"

static imu_event_t mk(imu_event_type_t type, imu_severity_t sev)
{
    imu_event_t e;

    (void)memset(&e, 0, sizeof(e));
    e.type     = (uint8_t)type;
    e.severity = (uint8_t)sev;
    return (e);
}

static void t_seq_and_names(void)
{
    imu_event_dispatcher_t d;
    imu_event_sink_t       s = cap_sink();
    imu_event_t            e;
    int                    i;

    cap_reset(64U);
    imu_events_init(&d, &s);
    for (i = 0; i < 5; i++)
    {
        e = mk(IMU_EVT_HUMP_DETECTED, IMU_SEV_INFO);
        TEST_ASSERT_EQ(imu_events_submit(&d, &e, 100U), IMU_OK);
    }
    TEST_ASSERT_EQ(g_cap.count, 5);
    TEST_ASSERT_EQ(g_cap.ev[0].seq, 0);
    TEST_ASSERT_EQ(g_cap.ev[4].seq, 4);
    TEST_ASSERT(0 == strcmp(imu_event_type_name(IMU_EVT_IMPACT),
                            "IMPACT/COLLISION"));
    TEST_ASSERT(0 == strcmp(imu_event_type_name((imu_event_type_t)77),
                            "INVALID"));
    TEST_ASSERT(sizeof(imu_event_t) <= 40U);
}

static void t_overflow_policy(void)
{
    imu_event_dispatcher_t d;
    imu_event_sink_t       s = cap_sink();
    imu_event_t            e;

    cap_reset(2U);
    imu_events_init(&d, &s);

    e = mk(IMU_EVT_TILT, IMU_SEV_MINOR);
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_OK);
    e = mk(IMU_EVT_TURNING, IMU_SEV_INFO);
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_OK);
    e = mk(IMU_EVT_HUMP_PEAK, IMU_SEV_INFO);           /* queue full */
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_ERR_QUEUE_FULL);
    TEST_ASSERT_EQ(d.stats.dropped[IMU_EVT_HUMP_PEAK], 1);

    e = mk(IMU_EVT_IMPACT, IMU_SEV_CRITICAL);          /* critical */
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_ERR_QUEUE_FULL);
    TEST_ASSERT_EQ(imu_events_pending(&d), 1);
    TEST_ASSERT_EQ(g_cap.notified, 1);                 /* immediate */
    e = mk(IMU_EVT_IMU_FAULT, IMU_SEV_CRITICAL);
    (void)imu_events_submit(&d, &e, 0U);
    TEST_ASSERT_EQ(imu_events_pending(&d), 2);

    imu_events_service(&d);                            /* still full */
    TEST_ASSERT_EQ(imu_events_pending(&d), 2);

    cap_drain();
    imu_events_service(&d);
    TEST_ASSERT_EQ(imu_events_pending(&d), 0);
    TEST_ASSERT_EQ(g_cap.ev[2].type, IMU_EVT_IMPACT);   /* order kept */
    TEST_ASSERT_EQ(g_cap.ev[3].type, IMU_EVT_IMU_FAULT);
    TEST_ASSERT_EQ(d.stats.critical_lost, 0);
}

static void t_pending_ring_bound(void)
{
    imu_event_dispatcher_t d;
    imu_event_sink_t       s = cap_sink();
    imu_event_t            e;
    uint32_t               i;

    cap_reset(0U);   /* consumer never drains */
    imu_events_init(&d, &s);
    for (i = 0U; i < 50U; i++)
    {
        e = mk(IMU_EVT_IMU_FAULT, IMU_SEV_CRITICAL);
        e.data.fault.attempt = i;
        (void)imu_events_submit(&d, &e, i);
    }
    TEST_ASSERT_EQ(imu_events_pending(&d), IMU_EVENT_CRITICAL_PENDING);
    TEST_ASSERT_EQ(d.stats.critical_lost, 50U - IMU_EVENT_CRITICAL_PENDING);
    /* Newest critical information is the one retained. */
    g_cap.capacity = 64U;
    imu_events_service(&d);
    TEST_ASSERT_EQ(g_cap.count, IMU_EVENT_CRITICAL_PENDING);
    TEST_ASSERT_EQ(g_cap.ev[IMU_EVENT_CRITICAL_PENDING - 1U]
                       .data.fault.attempt, 49);
}

static void t_rate_limit(void)
{
    imu_event_dispatcher_t d;
    imu_event_sink_t       s = cap_sink();
    imu_event_t            e;
    uint32_t               t;
    uint32_t               ok = 0U;

    cap_reset(CAP_MAX);
    imu_events_init(&d, &s);
    imu_events_set_rate_limit(&d, IMU_EVT_MOTION, 250U);
    imu_events_set_rate_limit(&d, IMU_EVT_IMPACT, 250U);
    for (t = 0U; t < 1000U; t += 20U)
    {
        e = mk(IMU_EVT_MOTION, IMU_SEV_INFO);
        if (IMU_OK == imu_events_submit(&d, &e, t))
        {
            ok++;
        }
    }
    TEST_ASSERT_EQ(ok, 4);   /* 0, 260, 520, 780 */
    TEST_ASSERT(d.stats.rate_limited[IMU_EVT_MOTION] > 40U);

    /* Critical events are never rate-limited. */
    ok = 0U;
    for (t = 0U; t < 100U; t += 20U)
    {
        e = mk(IMU_EVT_IMPACT, IMU_SEV_CRITICAL);
        if (IMU_OK == imu_events_submit(&d, &e, t))
        {
            ok++;
        }
    }
    TEST_ASSERT_EQ(ok, 5);
}

static void t_sink_errors(void)
{
    imu_event_dispatcher_t d;
    imu_event_sink_t       s = cap_sink();
    imu_event_t            e;

    cap_reset(8U);
    g_cap.b_error = true;
    imu_events_init(&d, &s);
    e = mk(IMU_EVT_TILT, IMU_SEV_MINOR);
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_ERR_QUEUE_FULL);
    TEST_ASSERT_EQ(d.stats.sink_errors, 1);
    e = mk(IMU_EVT_IMU_FAULT, IMU_SEV_CRITICAL);       /* kept for retry */
    (void)imu_events_submit(&d, &e, 0U);
    TEST_ASSERT_EQ(imu_events_pending(&d), 1);
    g_cap.b_error = false;
    imu_events_service(&d);
    TEST_ASSERT_EQ(imu_events_pending(&d), 0);

    imu_events_init(&d, NULL);                         /* no sink */
    e = mk(IMU_EVT_TILT, IMU_SEV_MINOR);
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_ERR_QUEUE_FULL);

    e = mk((imu_event_type_t)0, IMU_SEV_INFO);
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_ERR_PARAM);
    e = mk(IMU_EVT_TYPE_COUNT, IMU_SEV_INFO);
    TEST_ASSERT_EQ(imu_events_submit(&d, &e, 0U), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_events_submit(NULL, &e, 0U), IMU_ERR_PARAM);
    TEST_ASSERT_EQ(imu_events_submit(&d, NULL, 0U), IMU_ERR_PARAM);
    imu_events_service(NULL);
    imu_events_init(NULL, NULL);
}

static void t_critical_classification(void)
{
    imu_event_t e = mk(IMU_EVT_IMPACT, IMU_SEV_MINOR);

    TEST_ASSERT(!imu_events_is_critical(&e));
    e.severity = IMU_SEV_CRITICAL;
    TEST_ASSERT(imu_events_is_critical(&e));
    e = mk(IMU_EVT_IMU_FAULT, IMU_SEV_INFO);
    TEST_ASSERT(imu_events_is_critical(&e));
    e = mk(IMU_EVT_TILT, IMU_SEV_CRITICAL);
    TEST_ASSERT(imu_events_is_critical(&e));
    e = mk(IMU_EVT_HUMP_COMPLETED, IMU_SEV_INFO);
    TEST_ASSERT(!imu_events_is_critical(&e));
    TEST_ASSERT(!imu_events_is_critical(NULL));
}

void suite_unit_events(void)
{
    test_section("UNIT (white-box): event dispatcher");
    RUN_TEST(t_seq_and_names);
    RUN_TEST(t_overflow_policy);
    RUN_TEST(t_pending_ring_bound);
    RUN_TEST(t_rate_limit);
    RUN_TEST(t_sink_errors);
    RUN_TEST(t_critical_classification);
}
