/** @file task_consumer.c
 * @brief Member 3 task consumer module.
 * @note See docs/buddy3/BARR_COMPLIANCE.md for approved exceptions.
 */

/*
 *    task_consumer.c
 *    Subsystem 3 -- the consumer task.
 *
 *    APIs shown: tk_rcv_mbx, tk_rel_mpf, tk_sig_sem, tk_loc_mtx,
 *                tk_unl_mtx, tk_snd_mbf, tk_set_flg
 */

#include "task_consumer.h"
#include "../subsystem2/subsystem2.h" /* IF-01/02: SAMPLE_MSG, pool, slots */
#include "../subsystem4/subsystem4.h" /* IF-03: mbf_summary, flg_events */

/**
 * @brief Consume demo samples while retaining the S2/S4 interfaces.
 * @note The mailbox pointer and sample header share the documented S2 message
 * layout. UB is the port character-byte type; void casts discard task
 * arguments.
 */
EXPORT void consumer_task (INT stacd, void * p_exinf)
{
    SAMPLE_MSG * p_message = NULL;
    UB           text[S4_SUMMARY_MAX]; /* tm_sprintf adds a '\0' */
    INT          length;
    UW           count;
    ER           result;

    (void)stacd;
    (void)p_exinf;

    for (;;)
    {
        /* 1. Wait for the next sample (TMO_FEVR = wait forever) */
        result = tk_rcv_mbx(mbx_samples, (T_MSG **)&p_message, TMO_FEVR);

        if (result < E_OK)
        {
            CHECK(result, "tk_rcv_mbx");
            continue;
        }

        /* 2. Update the shared statistics.  Subsystem 4 reads them
              too, so lock the mutex around every access (IF-04). */
        tk_loc_mtx(mtx_stats, TMO_FEVR);

        if ((0U == app_stats.received) || (p_message->value < app_stats.min))
        {
            app_stats.min = p_message->value;
        }

        if ((0U == app_stats.received) || (p_message->value > app_stats.max))
        {
            app_stats.max = p_message->value;
        }

        app_stats.sum += p_message->value;
        count = ++app_stats.received;
        tk_unl_mtx(mtx_stats);

        /* 3. Every S3_BATCH_SIZE samples, send a text line to subsystem
              4 through its message buffer (the text is copied), then
              set FLG_BATCH to wake it up (IF-03). */

        if (0U == (count % S3_BATCH_SIZE))
        {
            /* tm_sprintf: like sprintf, returns the length.
               The longest line here is ~35 bytes < S4_SUMMARY_MAX. */
            length = tm_sprintf(text, (const UB *)"n=%u last=%d from core %d",
                                count, p_message->value, p_message->prc - 1);

            /* TMO_POL: if the buffer is full, drop the line rather
               than block the data path. */
            result = tk_snd_mbf(mbf_summary, text, length, TMO_POL);

            if (E_OK == result)
            {
                tk_set_flg(flg_events, FLG_BATCH);
            }
        }

        /* 4. Done with the sample (IF-02): return its memory block to
              the pool, then the slot so the producer may go on. */
        tk_rel_mpf(mpf_samples, p_message);
        tk_sig_sem(sem_slots, 1);
    }
}

/*** End of file. ***/

