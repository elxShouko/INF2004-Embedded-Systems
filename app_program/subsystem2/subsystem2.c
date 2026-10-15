/*
 *	subsystem2.c
 *	Subsystem 2 -- Producer: creates its objects and starts its task.
 *
 *	APIs shown: tk_cre_sem, tk_cre_mpf
 */

#include "subsystem2.h"
#include "task_motion.h"

EXPORT ID	sem_slots;	/* IF-02, see subsystem2.h */
EXPORT ID	mpf_samples;

static bool g_subsystem2_b_ready;
static bool g_subsystem2_b_start_attempted;

EXPORT ER subsystem2_init(void)
{
	T_CSEM	csem = {0};
	T_CMPF	cmpf = {0};
    ER error;

    /* Only retry before publication, after complete owned-object rollback. */
    if (g_subsystem2_b_ready || (sem_slots > 0) || (mpf_samples > 0))
    {
        return (E_OBJ);
    }

	/* Semaphore: a counter of free "slots".  The producer takes one
	   (tk_wai_sem) before each sample; the consumer gives one back
	   (tk_sig_sem).  It never exceeds the number of memory blocks, so
	   the producer can never run out of memory. */
	csem.sematr  = TA_TFIFO;	/* waiting tasks queue in FIFO order */
	csem.isemcnt = S2_POOL_COUNT;	/* initial count */
	csem.maxsem  = S2_POOL_COUNT;	/* maximum count */
	sem_slots = tk_cre_sem(&csem);
	if (sem_slots < E_OK) {
		return sem_slots;
	}

	/* Fixed-size memory pool: S2_POOL_COUNT blocks, each exactly one
	   SAMPLE_MSG.  Faster and more predictable than malloc(). */
	cmpf.mpfatr = TA_TFIFO | TA_RNG3;
	cmpf.mpfcnt = S2_POOL_COUNT;		/* number of blocks */
	cmpf.blfsz  = sizeof(SAMPLE_MSG);	/* size of one block */
	mpf_samples = tk_cre_mpf(&cmpf);
    error = (mpf_samples < E_OK) ? mpf_samples : task_motion_init();
    if (E_OK == error)
    {
        g_subsystem2_b_ready = true;
    }
    else
    {
        /* No subsystem task has started, so these objects are still ours. */
        if ((mpf_samples > 0) && (E_OK == tk_del_mpf(mpf_samples)))
        {
            mpf_samples = 0;
        }
        if ((sem_slots > 0) && (E_OK == tk_del_sem(sem_slots)))
        {
            sem_slots = 0;
        }
        /* Preserve the first failure; retained positive IDs block retry. */
    }
    return (error);
}

EXPORT ER subsystem2_start(void)
{
    if (!g_subsystem2_b_ready || g_subsystem2_b_start_attempted)
    {
        return (E_OBJ);
    }
    /* A failed start can already leave the producer running. Never duplicate
     * it or delete objects another buddy may now use. Reboot recovery only.
     */
    g_subsystem2_b_start_attempted = true;
	/* Pinned to core 1 (TP_PRC2): samples cross to subsystem 3 on core 0 */
	ID tid = app_start_task(producer_task, S2_PRI_PRODUCER, TP_PRC2,
				"producer_task");
	return (tid < E_OK) ? tid : task_motion_start();
}
