/* Host-only helpers for kernel_emulation.c: everything that needs a libc
 * header lives here, so the kernel headers (which define their own size_t)
 * and libc are never included in the same file. That keeps the harness
 * building on Linux, macOS and Windows hosts (MinGW, where size_t is 64-bit
 * but the kernel's size_t is not). Plain int/void* signatures only.
 *
 * Build with -DHOST_THREADS=1 -pthread for real concurrent threads. Without
 * it, a started "task" runs to completion inside the start call (logic only,
 * no concurrency). */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if defined(HOST_THREADS) && HOST_THREADS
#include <pthread.h>
#include <time.h>
#endif

#define MAX_MUTEXES 8

static int g_test_pass;
static int g_test_fail;

void host_copy (void * p_to, const void * p_from, int count) { memcpy(p_to, p_from, (size_t) count); }
void host_fill (void * p_to, int value, int count) { memset(p_to, value, (size_t) count); }

/* Print like tm_printf and count the [TEST] PASS / FAIL lines, so the
 * runner can exit non-zero when any check fails. */
void
host_vprint (const char * p_format, va_list arguments)
{
    if (0 == strncmp(p_format, "[TEST]", 6))
    {
        if (NULL != strstr(p_format, " FAIL ")) { g_test_fail++; }
        else if (NULL != strstr(p_format, " PASS ")) { g_test_pass++; }
        else { /* summary or banner line */ }
    }
    vprintf(p_format, arguments);
}

void host_puts (const char * p_text) { fputs(p_text, stdout); }

int host_test_pass_count (void) { return g_test_pass; }
int host_test_fail_count (void) { return g_test_fail; }

#if defined(HOST_THREADS) && HOST_THREADS
static pthread_mutex_t g_mutex[MAX_MUTEXES];
static pthread_mutex_t g_global = PTHREAD_MUTEX_INITIALIZER;
typedef void (*host_entry_t) (int, void *);

typedef struct { host_entry_t entry; } thread_arg_t;
static thread_arg_t g_args[8];
static int          g_arg_count;

static void *
thread_main (void * p_arg)
{
    thread_arg_t * p_thread = p_arg;
    p_thread->entry(0, NULL);
    return NULL;
}

int host_has_threads (void) { return 1; }
void host_mutex_create (int id) { pthread_mutex_init(&g_mutex[id % MAX_MUTEXES], NULL); }
void host_mutex_lock (int id) { pthread_mutex_lock(&g_mutex[id % MAX_MUTEXES]); }
void host_mutex_unlock (int id) { pthread_mutex_unlock(&g_mutex[id % MAX_MUTEXES]); }
int host_mutex_trylock (int id) { return (0 == pthread_mutex_trylock(&g_mutex[id % MAX_MUTEXES])); }
void host_global_lock (void) { pthread_mutex_lock(&g_global); }
void host_global_unlock (void) { pthread_mutex_unlock(&g_global); }
void host_sleep_ms (int ms) { struct timespec t = {0, ms * 1000000L}; nanosleep(&t, NULL); }
void host_thread_exit (void) { pthread_exit(NULL); }
int
host_thread_start (void (*entry) (int, void *))
{
    pthread_t thread;
    g_args[g_arg_count % 8].entry = entry;
    pthread_create(&thread, NULL, thread_main, &g_args[g_arg_count % 8]);
    pthread_detach(thread);
    g_arg_count++;
    return g_arg_count;
}
#else
int host_has_threads (void) { return 0; }
void host_mutex_create (int id) { (void) id; }
void host_mutex_lock (int id) { (void) id; }
void host_mutex_unlock (int id) { (void) id; }
int host_mutex_trylock (int id) { (void) id; return 1; }
void host_global_lock (void) {}
void host_global_unlock (void) {}
void host_sleep_ms (int ms) { (void) ms; }
void host_thread_exit (void) {}
int
host_thread_start (void (*entry) (int, void *))
{
    entry(0, NULL); /* runs to completion; tk_ext_tsk() is a no-op here */
    return 1;
}
#endif
