/* Minimal PC emulation of the micro T-Kernel calls that subsystem 1 uses, so
 * the on-target tests can be checked on a PC. Only what is needed: message
 * buffers, one event flag, the clock and no-op task/cyclic calls. */
/* NOTE: No host libc header is included here. The kernel headers define their
 * own size_t, which would clash with the host one, so the few libc functions
 * that are needed are declared by hand. */
#include <tk/tkernel.h>
#include <tm/tmonitor.h>

extern void * memcpy (void * to, const void * from, unsigned long count);
extern void * memset (void * to, int value, unsigned long count);
extern int    vprintf (const char * format, __builtin_va_list arguments);
extern int    printf (const char * format, ...);

#define MAX_BUFFERS 4
#define MAX_MESSAGES 64
#define MAX_MESSAGE_SIZE 64

typedef struct { unsigned char data[MAX_MESSAGE_SIZE]; INT size; } message_t;
typedef struct { message_t queue[MAX_MESSAGES]; int head, count; INT bufsz, maxmsz; int used_bytes; } buffer_t;

static buffer_t g_buffers[MAX_BUFFERS];
static int g_buffer_count = 0;
static UINT g_flag_pattern = 0u;
static UW g_time_ms = 1000u;

ID tk_cre_mbf (const T_CMBF * p) { buffer_t * b = &g_buffers[g_buffer_count]; memset (b, 0, sizeof (*b)); b->bufsz = p->bufsz; b->maxmsz = p->maxmsz; g_buffer_count++; return g_buffer_count; }
ER tk_snd_mbf (ID id, const void * msg, INT size, TMO tmout)
{
    buffer_t * b = &g_buffers[id - 1]; (void) tmout;
    if (size > b->maxmsz) { return E_PAR; }
    if ((b->used_bytes + size + 8) > b->bufsz) { return E_TMOUT; }   /* same overhead rule as the target code */
    b->queue[(b->head + b->count) % MAX_MESSAGES].size = size;
    memcpy (b->queue[(b->head + b->count) % MAX_MESSAGES].data, msg, (size_t) size);
    b->count++; b->used_bytes += size + 8; return E_OK;
}
INT tk_rcv_mbf (ID id, void * msg, TMO tmout)
{
    buffer_t * b = &g_buffers[id - 1]; message_t * m; (void) tmout;
    if (0 == b->count) { return E_TMOUT; }
    m = &b->queue[b->head]; memcpy (msg, m->data, (size_t) m->size);
    b->head = (b->head + 1) % MAX_MESSAGES; b->count--; b->used_bytes -= m->size + 8; return m->size;
}
ID tk_cre_flg (const T_CFLG * p) { (void) p; return 1; }
ER tk_set_flg (ID id, UINT p) { (void) id; g_flag_pattern |= p; return E_OK; }
ER tk_wai_flg (ID id, UINT wai, UINT mode, UINT * flg, TMO tmout)
{
    (void) id; (void) mode; (void) tmout;
    if (0u == (g_flag_pattern & wai)) { return E_TMOUT; }
    *flg = g_flag_pattern; g_flag_pattern &= ~(g_flag_pattern & wai); return E_OK;   /* TWF_BITCLR */
}
ER tk_get_otm (SYSTIM * t) { t->lo = g_time_ms; t->hi = 0; return E_OK; }
ER tk_dly_tsk (RELTIM d) { g_time_ms += d; return E_OK; }
ID tk_cre_cyc (const T_CCYC * p) { (void) p; return 1; }
ER tk_sta_cyc (ID id) { (void) id; return E_OK; }
ER tk_wup_tsk (ID id) { (void) id; return E_OK; }
ER tk_slp_tsk (TMO t) { (void) t; return E_OK; }
ID app_start_task (FP entry, PRI pri, UW prc, const char * name) { (void) entry; (void) pri; (void) prc; (void) name; return 1; }
INT tm_printf (const UB * fmt, ...) { __builtin_va_list a; __builtin_va_start (a, fmt); vprintf ((const char *) fmt, a); __builtin_va_end (a); return 0; }
INT tm_putstring (const UB * s) { return printf ("%s", (const char *) s); }
