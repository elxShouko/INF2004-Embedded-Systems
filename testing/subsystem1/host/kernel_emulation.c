/* PC emulation of the micro T-Kernel calls that subsystem 1 uses, so the
 * on-target tests can run on a PC: message buffers, event flags, mutexes,
 * tasks, the clock and no-op cyclic calls.
 *
 * NOTE: No libc header is included here. The kernel headers define their
 * own size_t, which clashes with the host one (MinGW in particular), so every
 * libc call goes through host_sync.c with plain int/void* signatures. */
#include <tk/tkernel.h>
#include <tm/tmonitor.h>

extern void host_copy (void * p_to, const void * p_from, int count);
extern void host_fill (void * p_to, int value, int count);
extern void host_vprint (const char * p_format, __builtin_va_list arguments);
extern void host_puts (const char * p_text);
extern void host_mutex_create (int id);
extern void host_mutex_lock (int id);
extern void host_mutex_unlock (int id);
extern int  host_mutex_trylock (int id);
extern void host_global_lock (void);
extern void host_global_unlock (void);
extern void host_sleep_ms (int ms);
extern void host_thread_exit (void);
extern int  host_thread_start (void (*entry) (int, void *));

#define MAX_BUFFERS 4
#define MAX_FLAGS 8
#define MAX_MUTEXES 8
#define MAX_MESSAGES 64
#define MAX_MESSAGE_SIZE 128
#define WAIT_STEP_MS 1

typedef struct { unsigned char data[MAX_MESSAGE_SIZE]; INT size; } message_t;
typedef struct { message_t queue[MAX_MESSAGES]; int head, count; INT bufsz, maxmsz; int used_bytes; } buffer_t;

static buffer_t g_buffers[MAX_BUFFERS];
static int      g_buffer_count = 0;
static UINT     g_flag_pattern[MAX_FLAGS];
static int      g_flag_count   = 0;
static int      g_mutex_count  = 0;
static UW       g_time_ms      = 1000u;

ID tk_cre_mbf (const T_CMBF * p) { buffer_t * b = &g_buffers[g_buffer_count]; host_fill (b, 0, (int) sizeof (*b)); b->bufsz = p->bufsz; b->maxmsz = p->maxmsz; g_buffer_count++; return g_buffer_count; }
ER tk_snd_mbf (ID id, const void * msg, INT size, TMO tmout)
{
    buffer_t * b = &g_buffers[id - 1]; (void) tmout;
    if (size > b->maxmsz) { return E_PAR; }
    if ((b->used_bytes + size + 8) > b->bufsz) { return E_TMOUT; }   /* same overhead rule as the target code */
    b->queue[(b->head + b->count) % MAX_MESSAGES].size = size;
    host_copy (b->queue[(b->head + b->count) % MAX_MESSAGES].data, msg, (int) size);
    b->count++; b->used_bytes += size + 8; return E_OK;
}
INT tk_rcv_mbf (ID id, void * msg, TMO tmout)
{
    buffer_t * b = &g_buffers[id - 1]; message_t * m; (void) tmout;
    if (0 == b->count) { return E_TMOUT; }
    m = &b->queue[b->head]; host_copy (msg, m->data, (int) m->size);
    b->head = (b->head + 1) % MAX_MESSAGES; b->count--; b->used_bytes -= m->size + 8; return m->size;
}

ID tk_cre_flg (const T_CFLG * p) { (void) p; g_flag_pattern[g_flag_count] = 0u; g_flag_count++; return g_flag_count; }
/* Fault injection for the tests: the next N tk_set_flg calls fail. */
static int g_fail_set_flg = 0;
void kernel_emulation_fail_set_flg (int count) { g_fail_set_flg = count; }

ER tk_set_flg (ID id, UINT p)
{
    host_global_lock ();
    if (g_fail_set_flg > 0) { g_fail_set_flg--; host_global_unlock (); return E_SYS; }
    g_flag_pattern[id - 1] |= p; host_global_unlock (); return E_OK;
}
ER tk_clr_flg (ID id, UINT p) { host_global_lock (); g_flag_pattern[id - 1] &= p; host_global_unlock (); return E_OK; }
ER tk_ref_flg (ID id, T_RFLG * r) { host_global_lock (); r->flgptn = g_flag_pattern[id - 1]; r->wtsk = 0; host_global_unlock (); return E_OK; }

/* Honours TWF_ANDW / TWF_ORW, TWF_CLR / TWF_BITCLR, and a timeout in ms. */
ER tk_wai_flg (ID id, UINT wai, UINT mode, UINT * flg, TMO tmout)
{
    long waited = 0;
    for (;;)
    {
        UINT now; int b_met;
        host_global_lock ();
        now   = g_flag_pattern[id - 1];
        b_met = (0u == (mode & TWF_ORW)) ? ((now & wai) == wai) : (0u != (now & wai));
        if (b_met)
        {
            *flg = now;
            if (0u != (mode & TWF_BITCLR)) { g_flag_pattern[id - 1] &= ~(now & wai); }
            else if (0u != (mode & TWF_CLR)) { g_flag_pattern[id - 1] = 0u; }
            host_global_unlock ();
            return E_OK;
        }
        host_global_unlock ();
        if ((TMO_POL == tmout) || ((TMO_FEVR != tmout) && (waited >= tmout))) { return E_TMOUT; }
        host_sleep_ms (WAIT_STEP_MS); waited += WAIT_STEP_MS;
    }
}

ID tk_cre_mtx (const T_CMTX * p) { (void) p; host_mutex_create (g_mutex_count); g_mutex_count++; return g_mutex_count; }
/* TMO_FEVR waits; TMO_POL and a timeout in ms try, then give up with E_TMOUT. */
ER tk_loc_mtx (ID id, TMO tmout)
{
    long waited = 0;
    if (TMO_FEVR == tmout) { host_mutex_lock (id - 1); return E_OK; }
    for (;;)
    {
        if (host_mutex_trylock (id - 1)) { return E_OK; }
        if ((TMO_POL == tmout) || (waited >= tmout)) { return E_TMOUT; }
        host_sleep_ms (WAIT_STEP_MS); waited += WAIT_STEP_MS;
    }
}
ER tk_unl_mtx (ID id) { host_mutex_unlock (id - 1); return E_OK; }

ER tk_get_otm (SYSTIM * t) { t->lo = g_time_ms; t->hi = 0; return E_OK; }
ER tk_dly_tsk (RELTIM d) { g_time_ms += d; return E_OK; }
ID tk_cre_cyc (const T_CCYC * p) { (void) p; return 1; }
ER tk_sta_cyc (ID id) { (void) id; return E_OK; }
ER tk_wup_tsk (ID id) { (void) id; return E_OK; }
ER tk_slp_tsk (TMO t) { (void) t; return E_OK; }
void tk_ext_tsk (void) { host_thread_exit (); }
ID app_start_task (FP entry, PRI pri, UW prc, const char * name)
{
    (void) pri; (void) prc; (void) name;
    /* A cyclic-driven task (the simulator) is never started in the host build:
     * S1_SIMULATE_TELEMETRY is 0. Only the bounded test tasks run. */
    return host_thread_start ((void (*) (int, void *)) entry);
}
INT tm_printf (const UB * fmt, ...) { __builtin_va_list a; __builtin_va_start (a, fmt); host_vprint ((const char *) fmt, a); __builtin_va_end (a); return 0; }
INT tm_putstring (const UB * s) { host_puts ((const char *) s); return 0; }
