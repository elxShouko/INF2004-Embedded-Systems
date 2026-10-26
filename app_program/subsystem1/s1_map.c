/**
 * @file s1_map.c
 * @brief Buddy 1: submit mapping updates to Buddy 3 and report its answer.
 */

#include "s1_map.h"

typedef struct
{
    bool b_pending; /* Submitted, reply not yet seen. */
    uint32_t pending_seq;
    uint32_t submitted_ms;
    bool b_unreported; /* A result is waiting for the status mutex. */
    uint32_t report_seq;
    s1_codec_map_result_t report_result;
    uint32_t report_revision;
} s1_map_state_t;

static s1_map_state_t g_s1_map = {0};

static void
map_report(uint32_t seq, s1_codec_map_result_t result, uint32_t revision)
{
    g_s1_map.report_seq = seq;
    g_s1_map.report_result = result;
    g_s1_map.report_revision = revision;
    g_s1_map.b_unreported = (E_OK != s1_net_map_report(seq, result, revision));
}

void s1_map_pump(uint32_t now_ms)
{
    s1_net_map_request_t request = {{0u, {0u}}, 0u, 0u};
    s3_input_t input = {0};
    ER result = E_OK;

    if (g_s1_map.b_unreported)
    {
        map_report(g_s1_map.report_seq, g_s1_map.report_result,
                   g_s1_map.report_revision);
    }
    if (g_s1_map.b_pending)
    {
        if ((now_ms - g_s1_map.submitted_ms) > S1_MAP_REPLY_TIMEOUT_MS)
        {
            g_s1_map.b_pending = false;
            map_report(g_s1_map.pending_seq, S1_MAP_RESULT_REJECTED, 0u);
        }
    }
    else if (!g_s1_map.b_unreported &&
             (E_OK == s1_net_map_receive(&request, TMO_POL)))
    {
        input.kind = S3_INPUT_MAPPING;
        input.request_id = S1_MAP_REQUEST_ID_FLAG | request.seq;
        input.value.mapping = request.update;
        result = s3_submit(&input, TMO_POL);
        if (E_OK == result)
        {
            /* Queued only. Not validated, not applied. */
            g_s1_map.b_pending = true;
            g_s1_map.pending_seq = request.seq;
            g_s1_map.submitted_ms = now_ms;
            map_report(request.seq, S1_MAP_RESULT_QUEUED, 0u);
        }
        else
        {
            map_report(request.seq, S1_MAP_RESULT_REJECTED, 0u);
        }
    }
}

bool s1_map_on_reply(const s3_output_t *p_reply)
{
    bool b_mine = false;

    if ((NULL != p_reply) && g_s1_map.b_pending &&
        (S3_OUTPUT_REPLY == (s3_output_kind_t)p_reply->kind) &&
        ((S1_MAP_REQUEST_ID_FLAG | g_s1_map.pending_seq) ==
         p_reply->request_id))
    {
        b_mine = true;
        g_s1_map.b_pending = false;
        map_report(g_s1_map.pending_seq,
                   (0u != p_reply->accepted) ? S1_MAP_RESULT_ACCEPTED
                                             : S1_MAP_RESULT_REJECTED,
                   p_reply->map_revision);
    }
    return b_mine;
}

/*** end of file ***/