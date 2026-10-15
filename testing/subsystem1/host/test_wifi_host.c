/* PC test of the real cyw43_utk_task() with a scripted CYW43 driver.
 * It checks the join and rejoin logic (feedback item 1, FR15, FR19) without a
 * radio: a fake clock, a fake driver and a script that decides what each join
 * request and each link status poll returns. */
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cyw43.h"
#include "cyw43_utk.h"
#include "tm/tmonitor.h"

#define MAX_JOINS 16
#define POLL_MS 2u
#define TASK_NAME_UNUSED 0

/* ---- Fake driver and clock ------------------------------------------- */
cyw43_t cyw43_state;
void  (*cyw43_poll) (void) = 0;
volatile uint32_t cyw43_utk_poll_requested = 0u;

static uint64_t g_now_us;
static uint32_t g_max_delay_ms;
static jmp_buf  g_done;
static int      g_checks;
static int      g_failures;

/* Scenario controls, set by each test before running the task. */
static int      g_join_result[MAX_JOINS]; /* what the n-th join request returns */
static int      g_join_count;
static uint32_t g_join_at_ms[MAX_JOINS];
static int      g_link_status;
static int      g_bus_ok;                  /* set_up succeeds */
static int      g_drop_itf_on_join_fail;   /* interface disappears after a failed request */
static int      g_mqtt_polls;
static int      g_mqtt_polls_up;
static uint32_t g_end_ms;                  /* stop the task at this time */
static void   (*g_on_tick) (uint32_t now_ms);

uint64_t time_us_64 (void) { return g_now_us; }

int32_t
tk_dly_tsk (uint32_t delay_ms)
{
    if (delay_ms > g_max_delay_ms && delay_ms <= POLL_MS) { g_max_delay_ms = delay_ms; }
    if (delay_ms > POLL_MS) { g_max_delay_ms = delay_ms; }
    g_now_us += (uint64_t) delay_ms * 1000u;
    if (0 != g_on_tick) { g_on_tick((uint32_t) (g_now_us / 1000u)); }
    if ((uint32_t) (g_now_us / 1000u) >= g_end_ms) { longjmp(g_done, 1); }
    return 0;
}

int32_t tk_get_prc (void) { return 1; }
void    mp_memory_barrir (void) {}
void    lwip_init (void) {}
void    sys_check_timeouts (void) {}
void    cyw43_init (cyw43_t * p) { (void) p; }
void    cyw43_thread_enter (void) {}
void    cyw43_thread_exit (void) {}
int     cyw43_wifi_get_mac (cyw43_t * p, int i, uint8_t m[6]) { (void) p; (void) i; memset(m, 1, 6); return 0; }

void
cyw43_wifi_set_up (cyw43_t * p, int itf, bool b_up, uint32_t country)
{
    (void) b_up; (void) country;
    p->itf_state = g_bus_ok ? (1u << itf) : 0u;
}

int
cyw43_wifi_join (cyw43_t * p, size_t sl, const uint8_t * s, size_t kl,
                 const uint8_t * k, uint32_t a, const uint8_t * b, uint32_t c)
{
    int result = (g_join_count < MAX_JOINS) ? g_join_result[g_join_count] : -1;
    (void) sl; (void) s; (void) kl; (void) k; (void) a; (void) b; (void) c;
    if (g_join_count < MAX_JOINS) { g_join_at_ms[g_join_count] = (uint32_t) (g_now_us / 1000u); }
    g_join_count++;
    if ((0 != result) && g_drop_itf_on_join_fail) { p->itf_state = 0u; }
    return result;
}

int cyw43_tcpip_link_status (cyw43_t * p, int itf) { (void) p; (void) itf; return g_link_status; }

int
tm_printf (const UB * p_format, ...)
{
    (void) p_format;
    return 0; /* silent; the checks below print their own result */
}

/* ---- The Wi-Fi task calls these (mqtt_utk is replaced by a recorder) --- */
void
mqtt_utk_poll (bool b_link_up)
{
    g_mqtt_polls++;
    if (b_link_up) { g_mqtt_polls_up++; }
}

/* ---- Helpers ------------------------------------------------------------ */
#define CHECK(cond, ...) do { g_checks++; if (!(cond)) { g_failures++; printf("  FAIL: "); printf(__VA_ARGS__); printf(" (line %d)\n", __LINE__); } else { printf("  ok:   "); printf(__VA_ARGS__); printf("\n"); } } while (0)

extern void cyw43_utk_task (int32_t stacd, void * exinf);

static void
reset (const char * p_title)
{
    memset(&cyw43_state, 0, sizeof(cyw43_state));
    memset(g_join_result, 0, sizeof(g_join_result));
    memset(g_join_at_ms, 0, sizeof(g_join_at_ms));
    g_now_us = 0u; g_max_delay_ms = 0u; g_join_count = 0; g_link_status = CYW43_LINK_DOWN;
    g_bus_ok = 1; g_drop_itf_on_join_fail = 0; g_mqtt_polls = 0; g_mqtt_polls_up = 0;
    g_on_tick = 0; g_end_ms = 60000u;
    cyw43_poll = 0;
    printf("\n%s\n", p_title);
}

static void
run_task (void)
{
    if (0 == setjmp(g_done)) { cyw43_utk_task(0, 0); }
}

static int
gap (int a, int b) { return (int) (g_join_at_ms[b] - g_join_at_ms[a]); }

/* Joins succeed on the 2nd request; link comes up once a request is accepted. */
static void
tick_link_follows_join (uint32_t now_ms)
{
    (void) now_ms;
    if ((g_join_count >= 1) && (0 == g_join_result[g_join_count - 1])) { g_link_status = CYW43_LINK_UP; }
}

static void
test_first_join_fails_then_succeeds (void)
{
    T_CYW43_UTK_STATUS st;
    reset("W1: first join request fails immediately, second succeeds");
    g_join_result[0] = -1; g_join_result[1] = 0;
    g_on_tick = tick_link_follows_join; g_end_ms = 5000u;
    run_task();
    cyw43_utk_get_status(&st);
    CHECK(g_join_count == 2, "two join requests were made (got %d)", g_join_count);
    CHECK(gap(0, 1) >= 1000 && gap(0, 1) <= 1010, "retry came after the 1 s backoff (%d ms)", gap(0, 1));
    CHECK(1 == st.ready && 0 == st.init_result, "radio reported ready, init_result 0");
    CHECK(st.poll_count > 100u, "task entered the poll loop (%u polls)", (unsigned) st.poll_count);
    CHECK(CYW43_LINK_UP == st.link_status && g_mqtt_polls_up > 0, "link up, MQTT polled with link up");
    CHECK(g_max_delay_ms <= POLL_MS, "no delay longer than %u ms in the task (max %u)", POLL_MS, (unsigned) g_max_delay_ms);
}

static void
test_repeated_failures_backoff (void)
{
    int i;
    int want[] = {1000, 2000, 4000, 8000, 8000, 8000};
    int b_ok   = 1;
    reset("W2: every join request fails immediately -> 1,2,4,8,8,8 s");
    for (i = 0; i < MAX_JOINS; i++) { g_join_result[i] = -1; }
    g_end_ms = 40000u;
    run_task();
    CHECK(g_join_count >= 7, "at least 7 requests in 40 s (got %d)", g_join_count);
    for (i = 0; i < 6; i++) { int g = gap(i, i + 1); if (g < want[i] || g > want[i] + 10) { b_ok = 0; } printf("        gap %d: %d ms (want %d)\n", i, g, want[i]); }
    CHECK(b_ok, "gaps follow 1,2,4,8,8,8 s (FR19)");
    CHECK(g_max_delay_ms <= POLL_MS, "never blocks longer than %u ms (max %u)", POLL_MS, (unsigned) g_max_delay_ms);
}

static uint32_t g_lost_at_ms;
static void
tick_lose_link (uint32_t now_ms)
{
    if ((g_join_count >= 3) && (0u == g_lost_at_ms))
    {
        g_link_status = CYW43_LINK_UP;               /* 3rd request accepted */
        if (now_ms >= (g_join_at_ms[2] + 100u)) { g_lost_at_ms = now_ms; g_link_status = CYW43_LINK_DOWN; }
    }
    else if (g_join_count >= 4)
    {
        g_link_status = CYW43_LINK_UP;               /* the rejoin worked */
    }
}

static void
test_established_loss_recovers (void)
{
    reset("W3: link established, then lost -> rejoin after 1 s again");
    g_lost_at_ms = 0u;
    g_join_result[0] = -1; g_join_result[1] = -1; g_join_result[2] = 0;   /* UP after 2 failures (1 s + 2 s) */
    g_join_result[3] = 0;                                                    /* the rejoin after the loss succeeds */
    g_on_tick = tick_lose_link; g_end_ms = 12000u;
    run_task();
    CHECK(g_join_count >= 4, "a rejoin request followed the loss (%d requests)", g_join_count);
    CHECK(0u != g_lost_at_ms, "the link was lost at %u ms", (unsigned) g_lost_at_ms);
    CHECK((int) (g_join_at_ms[3] - g_lost_at_ms) >= 1000 && (int) (g_join_at_ms[3] - g_lost_at_ms) <= 1010,
          "rejoin came 1 s after the loss, not at the old 4 s step (%d ms)", (int) (g_join_at_ms[3] - g_lost_at_ms));
}

static void
test_radio_init_failure_is_permanent (void)
{
    T_CYW43_UTK_STATUS st;
    reset("W4: radio/interface bring-up failure is NOT retried");
    g_bus_ok = 0; g_end_ms = 20000u;
    run_task();
    cyw43_utk_get_status(&st);
    CHECK(0 == g_join_count, "no join request when the interface never came up");
    CHECK(0 == st.ready && 0 != st.init_result, "radio not ready, init_result %d", (int) st.init_result);
    CHECK(0 == g_mqtt_polls, "network service is not polled");
}

static void
test_interface_lost_stops_retries (void)
{
    T_CYW43_UTK_STATUS st;
    reset("W5: interface disappears during a retry -> stop, report not ready");
    g_join_result[0] = -1; g_join_result[1] = -1; g_drop_itf_on_join_fail = 1;
    g_end_ms = 20000u;
    run_task();
    cyw43_utk_get_status(&st);
    CHECK(2 == g_join_count, "exactly two requests, then no more (got %d)", g_join_count);
    CHECK(0 == st.ready && 0 != st.init_result, "radio reported not ready after the interface was lost");
}

int
main (void)
{
    test_first_join_fails_then_succeeds();
    test_repeated_failures_backoff();
    test_established_loss_recovers();
    test_radio_init_failure_is_permanent();
    test_interface_lost_stops_retries();
    printf("\n%d checks, %d failed\n", g_checks, g_failures);
    return (0 == g_failures) ? 0 : 1;
}
