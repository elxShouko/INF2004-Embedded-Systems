/* Host fake of kernel calls, exercising the actual RTOS adapter owner logic.
 * Compile with -I pointing to the overlaid template's subsystem3 directory.
 * APP_H suppresses hardware headers; the included file still resolves in the
 * template tree so the delivered application source is tested unchanged.
 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define APP_H
#define TK_SUPPORT_SMP 1
#ifndef CNF_TIMER_PERIOD
#define CNF_TIMER_PERIOD 1 /* Logical suite only; production suite uses 10. */
#endif
typedef int32_t ER;
typedef int32_t ID;
typedef int32_t INT;
typedef int32_t TMO;
typedef uint32_t UW;
typedef void (*FP)(INT, void *);
#define E_OK 0
#define E_PAR (-17)
#define E_OBJ (-41)
#define E_TMOUT (-50)
#define TMO_POL 0
#define TMO_FEVR (-1)
#define TA_INHERIT 1U
#define TA_TFIFO 0U
#define TA_HLNG 1U
#define TA_RNG3 2U
#define TA_ASSPRC 4U
#define TP_PRC1 1U
#define IMPORT extern
typedef struct {uint32_t mbfatr, bufsz; INT maxmsz;} T_CMBF;
typedef struct {uint32_t mtxatr;} T_CMTX;
typedef struct {FP task; INT itskpri; uint32_t stksz, tskatr, assprc;} T_CTSK;
ER tk_snd_mbf(ID id, const void *message, INT size, TMO timeout);
INT tk_rcv_mbf(ID id, void *message, TMO timeout);
ER tk_loc_mtx(ID id, TMO timeout);
ER tk_unl_mtx(ID id);
ID tk_cre_mtx(const T_CMTX *p_create);
ID tk_cre_mbf(const T_CMBF *p_create);
ID tk_cre_tsk(const T_CTSK *p_create);
ER tk_sta_tsk(ID id, INT code);
ER tk_del_tsk(ID id);
ER tk_del_mtx(ID id);
ER tk_del_mbf(ID id);
ER tk_dly_tsk(uint32_t delay);

#include "member3_navigation.c"

static s3_output_t g_outputs[256];
static unsigned int g_count;
static bool g_full;
static ID g_next_id = 1;

ER tk_snd_mbf(ID id, const void *message, INT size, TMO timeout)
{
    (void)id; (void)timeout;
    if (g_full) return E_TMOUT;
    if (size == sizeof(s3_output_t))
    {
        assert(g_count < 256U);
        memcpy(&g_outputs[g_count++], message, sizeof(s3_output_t));
    }
    return E_OK;
}
#ifndef S3_HOST_PRODUCTION
INT tk_rcv_mbf(ID id, void *message, TMO timeout)
{ (void)id; (void)message; (void)timeout; return E_TMOUT; }
#endif
ER tk_loc_mtx(ID id, TMO timeout) { (void)id; (void)timeout; return E_OK; }
ER tk_unl_mtx(ID id) { (void)id; return E_OK; }
ID tk_cre_mtx(const T_CMTX *p_create) { (void)p_create; return g_next_id++; }
ID tk_cre_mbf(const T_CMBF *p_create) { assert(p_create->bufsz > 0U); return g_next_id++; }
ID tk_cre_tsk(const T_CTSK *p_create) { assert(p_create->itskpri == 17); return g_next_id++; }
ER tk_sta_tsk(ID id, INT code) { (void)id; (void)code; return E_OK; }
ER tk_del_tsk(ID id) { (void)id; return E_OK; }
ER tk_del_mtx(ID id) { (void)id; return E_OK; }
ER tk_del_mbf(ID id) { (void)id; return E_OK; }
#ifndef S3_HOST_PRODUCTION
ER tk_dly_tsk(uint32_t delay) { (void)delay; return E_OK; }
ER s3_ir_init(void) { return E_OK; }
void s3_ir_read(ir_sample_t *p_sample) { memset(p_sample, 0, sizeof(*p_sample)); }
uint64_t s3_clock_us(void) { return 0U; }
#endif

static void init_context(s3_context_t *p_context)
{
    memset(p_context, 0, sizeof(*p_context));
    p_context->status.mode = LINE_MODE_PAUSED;
    p_context->status.state = LINE_STATE_CALIBRATING;
    nav_map_init(&p_context->map);
    s3_config_t config = {0};
    config.calibration = (ir_calibration_t){7U, 7U};
    config.period_ms = 1U;
    config.line_stable_us = 1000U;
    config.line_lost_us = 3000U;
    config.line_found_us = 3000U;
    config.recovery_timeout_us = 50000U;
    config.barcode_quiet_us = 10000U;
    config.barcode_timeout_us = 1000000U;
    config.max_sample_gap_us = 5000U;
    config.odometry_stale_us = 10000U;
    config.barcode_rearm_us = 50000U;
    assert(configure(p_context, &config));
    s3_input_t request = {0};
    request.kind = S3_INPUT_MODE;
    request.value.mode = LINE_MODE_TRACKING;
    input(p_context, &request, 0U);
    assert(p_context->status.mode == LINE_MODE_TRACKING);
    g_count = 0U;
}

static void tick(s3_context_t *p_context, uint8_t mask)
{
    ir_sample_t sample = {p_context->status.raw.timestamp_us + 1000U, mask};
    process(p_context, &sample);
}

static unsigned int events(nav_event_kind_t kind)
{
    unsigned int count = 0U;
    for (unsigned int i = 0U; i < g_count; ++i)
    {
        if (g_outputs[i].kind == S3_OUTPUT_NAVIGATION &&
            g_outputs[i].navigation.kind == kind) ++count;
    }
    return count;
}

int main(void)
{
    assert(s3_navigation_init() == E_OK);
    assert(s3_navigation_init() == E_OBJ);
    assert(s3_navigation_start() == E_OK);
    assert(s3_navigation_start() == E_OBJ);
    s3_context_t context;
    init_context(&context);
    for (unsigned int i = 0U; i < 10U; ++i) tick(&context, 3U);
    assert(context.status.line == LINE_OBSERVATION_CENTRED);
    assert(events(NAV_EVENT_LINE_LOST) == 0U);
    for (unsigned int i = 0U; i < 6U; ++i) tick(&context, 0U);
    assert(events(NAV_EVENT_LINE_LOST) == 1U);
    assert(context.status.state == LINE_STATE_RECOVERY);
    for (unsigned int i = 0U; i < 6U; ++i) tick(&context, 3U);
    assert(events(NAV_EVENT_LINE_FOUND) == 1U);
    assert(context.status.mode == LINE_MODE_PAUSED);

    init_context(&context);
    for (unsigned int i = 0U; i < 70U; ++i) tick(&context, 0U);
    assert(context.status.fault == S3_FAULT_RECOVERY_TIMEOUT);
    assert(events(NAV_EVENT_FAULT) == 1U);

    init_context(&context);
    s3_input_t request = {0};
    request.kind = S3_INPUT_MODE;
    request.value.mode = LINE_MODE_BYPASS;
    input(&context, &request, 0U);
    for (unsigned int i = 0U; i < 70U; ++i) tick(&context, 0U);
    assert(events(NAV_EVENT_LINE_LOST) == 0U);
    assert(context.status.fault == S3_FAULT_NONE);

    init_context(&context);
    request.kind = S3_INPUT_MAPPING;
    request.request_id = 42U;
    strcpy(request.value.mapping, "MAP A=RIGHT");
    input(&context, &request, 0U);
    assert(g_outputs[g_count - 1U].accepted);
    assert(g_outputs[g_count - 1U].request_id == 42U);
    strcpy(request.value.mapping, "MAP A=LEFT Z=INVALID");
    input(&context, &request, 0U);
    assert(!g_outputs[g_count - 1U].accepted);
    assert(nav_map_lookup(&context.map, 'A') == NAV_COMMAND_RIGHT);
    /* Feed *A* edges at realistic per-sample intervals, then quiet time. */
    for (unsigned int i = 0U; i < 12U; ++i) tick(&context, 3U);
    const char *symbols[] = {"nWnnWnWnn", "WnnnnWnnW", "nWnnWnWnn"};
    bool black = true;
    for (unsigned int symbol_index = 0U; symbol_index < 3U; ++symbol_index)
    {
        for (unsigned int element = 0U; element < 9U; ++element)
        {
            unsigned int ticks = symbols[symbol_index][element] == 'W' ? 3U : 1U;
            for (unsigned int i = 0U; i < ticks; ++i) tick(&context, black ? 7U : 3U);
            black = !black;
        }
        if (symbol_index < 2U)
        {
            tick(&context, 3U);
            black = true;
        }
    }
    for (unsigned int i = 0U; i < 12U; ++i) tick(&context, 3U);
    assert(events(NAV_EVENT_BARCODE_VALID) == 1U);
    assert(context.status.state == LINE_STATE_AWAITING_MANOEUVRE);
    uint32_t pending = context.status.pending_manoeuvre_event_id;
    assert(pending != 0U);
    for (unsigned int i = 0U; i < g_count; ++i)
    {
        if (g_outputs[i].navigation.kind == NAV_EVENT_BARCODE_VALID)
            assert(g_outputs[i].navigation.command == NAV_COMMAND_RIGHT);
    }
    strcpy(request.value.mapping, "MAP A=LEFT");
    input(&context, &request, context.status.raw.timestamp_us);
    assert(context.status.pending_manoeuvre_event_id == pending);
    request.kind = S3_INPUT_MANOEUVRE_DONE;
    request.value.manoeuvre_event_id = pending + 1U;
    input(&context, &request, context.status.raw.timestamp_us);
    assert(!g_outputs[g_count - 1U].accepted);
    request.value.manoeuvre_event_id = pending;
    input(&context, &request, context.status.raw.timestamp_us);
    assert(context.status.state == LINE_STATE_RECOVERY);

    init_context(&context);
    tick(&context, 3U);
    ir_sample_t stale = {100000U, 3U};
    process(&context, &stale);
    assert(context.status.fault == S3_FAULT_SAMPLE_GAP);
    init_context(&context);
    g_full = true;
    tick(&context, 3U);
    tick(&context, 3U);
    assert(context.status.fault == S3_FAULT_OUTPUT_FULL);
    assert(context.status.mode == LINE_MODE_PAUSED);
    g_full = false;
    puts("Member 3 RTOS owner tests passed: mapping, barcode-to-command, recovery, modes, faults.");
    return 0;
}
