#include <inttypes.h>

#include "rs_event.h"
#include "rs_master.h"
#include <cvi_dtcf.h>

#include "cvi_command_record.h"

typedef int32_t (*rs_cmd_cb_t)(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *userdate);

typedef struct _rs_cmd_desc {
    rs_cmd_cb_t cb;
    uint32_t flags;
} rs_cmd_desc_t;

static int32_t not_implemented(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(ep);
    UNUSED(userdata);
    CVI_LOGE("cmd %x not implemented yet\n", msg->arg1);
    return RS_ERR_INVALID;
}

static int32_t rs_cmd_cb_shutdown(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(ep);
    UNUSED(msg);

    return CVI_RECORD_SERVICE_Destroy(userdata);
}

static int32_t rs_cmd_cb_debug(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(ep);
    UNUSED(msg);
    rs_context_handle_t rs = (rs_context_handle_t)userdata;

    int32_t debug_flag = msg->arg2;
    CVI_LOGI("set debug flag to 0x%08x\n", debug_flag);
    if (debug_flag == 0) {
        rs_disable_state(rs, RS_STATE_PERF_EN | RS_STATE_DEBUG_EN);
    } else {
        rs_enable_state(rs, RS_STATE_PERF_EN | RS_STATE_DEBUG_EN);
    }

    // TODO: send ACK

    return RS_SUCCESS;
}

static int32_t rs_cmd_cb_rec_start(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(ep);
    UNUSED(msg);

    return CVI_RECORD_SERVICE_StartRecord(userdata);
}

static int32_t rs_cmd_cb_rec_stop(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(ep);
    UNUSED(msg);

    return CVI_RECORD_SERVICE_StopRecord(userdata);
}

static int32_t rs_cmd_cb_event_rec(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(ep);
    UNUSED(msg);

    return CVI_RECORD_SERVICE_EventRecord(userdata);
}

static int32_t rs_cmd_cb_timelapse_rec_start(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(ep);
    UNUSED(msg);

    return CVI_RECORD_SERVICE_StartTimelapseRecord(userdata);
}

static int32_t rs_cmd_cb_timelapse_rec_stop(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(ep);
    UNUSED(msg);

    return CVI_RECORD_SERVICE_StopTimelapseRecord(userdata);
}

static int32_t rs_cmd_cb_mute(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(ep);
    UNUSED(msg);

    return CVI_RECORD_SERVICE_StartMute(userdata);
}

static int32_t rs_cmd_cb_disable_mute(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(ep);
    UNUSED(msg);

    return CVI_RECORD_SERVICE_StopMute(userdata);
}

static int32_t rs_cmd_cb_piv_capture(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *userdata) {
    if (CVI_RECORD_SERVICE_PivCapture(userdata, NULL) != 0) {
        return -1;
    }

    if (msg->needack) {
        MSG_ACK_t msg_ack = {0};
        rs_context_handle_t rs = (rs_context_handle_t)userdata;
        snprintf(msg_ack.ackmsg, sizeof(msg_ack.ackmsg), "%s", rs->piv_filename);

        msg_ack.result_len = strlen(msg_ack.ackmsg);
        msg_ack.status = msg_ack.result_len == 0? -1: 0;

        return CVI_MQ_SendAck(ep, &msg_ack, msg->client_id);
    }

    return 0;
}

static rs_cmd_desc_t rs_cmd_tbl[] = {
    {NULL, 0},                            /* 0x00    INVALID */
    {rs_cmd_cb_shutdown, 0},              /* 0x01    SHUTDOWN */
    {rs_cmd_cb_debug, 0},                 /* 0x02    DEBUG */
    {not_implemented, 0},                 /* 0x03    GET_STATUS */
    {NULL, 0},                            /* 0x04    RESERVED */
    {NULL, 0},                            /* 0x05    */
    {NULL, 0},                            /* 0x06    */
    {rs_cmd_cb_rec_start, 0},             /* 0x07    REC_START */
    {rs_cmd_cb_rec_stop, 0},              /* 0x08    REC_STOP */
    {rs_cmd_cb_piv_capture, 0},           /* 0x09    PIV_CAPTURE */
    {not_implemented, 0},                 /* 0x0a    LOCK_REC_START */
    {not_implemented, 0},                 /* 0x0b    LOCK_REC_STOP */
    {rs_cmd_cb_event_rec, 0},             /* 0x0c    EVENT_REC */
    {NULL, 0},                            /* 0x0d    */
    {NULL, 0},                            /* 0x0e    */
    {NULL, 0},                            /* 0x0f    */
    {NULL, 0},                            /* 0x10    */
    {NULL, 0},                            /* 0x11    */
    {NULL, 0},                            /* 0x12    */
    {NULL, 0},                            /* 0x13    */
    {NULL, 0},                            /* 0x14    */
    {rs_cmd_cb_timelapse_rec_start, 0},   /* 0x15    TIMELAPSE_REC_START */
    {rs_cmd_cb_timelapse_rec_stop, 0},    /* 0x16    TIMELAPSE_REC_STOP */
    {rs_cmd_cb_mute, 0},                  /* 0x17    MUTE */
    {rs_cmd_cb_disable_mute, 0},          /* 0x18    DISABLE_MUTE */
};

static int32_t rs_mq_cb(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *ep_arg) {
    UNUSED(ep);

#if 1
    printf("rs_mq_cb: rx, target_id = %08x, len = %d, ep_arg = %p\n", msg->target_id, msg->len, ep_arg);
    printf("rs_mq_cb:     arg1 = 0x%08x, arg2 = 0x%08x\n", msg->arg1, msg->arg2);
    printf("rs_mq_cb:     seq_no = 0x%04x, time = %"PRIu64"\n", msg->seq_no, msg->crete_time);
    if (msg->len > (int32_t)CVI_MQ_MSG_HEADER_LEN + 4) {
        printf("rs_mq_cb:     payload [%02x %02x %02x %02x]\n", msg->payload[0], msg->payload[1],
               msg->payload[2], msg->payload[3]);
    }
#endif

    int32_t cmd_id = msg->arg1;
    CVI_LOG_ASSERT(cmd_id >= 0 && cmd_id < (int32_t)(sizeof(rs_cmd_tbl) / sizeof(rs_cmd_desc_t)),
                   "cmd_id %d out of range\n", cmd_id);

    if (rs_cmd_tbl[cmd_id].cb == NULL) {
        CVI_LOGE("cmd_id %d not handled\n", cmd_id);
        return RS_ERR_INVALID;
    }

    return rs_cmd_tbl[cmd_id].cb(ep, msg, ep_arg);
}

#if 0  // testing 2
static void CVI_CMDMNG_SendMqCmd(int32_t sns_id, int32_t cmd_id) {
  static int32_t i = 0;

  // test CVI_MQ_SendRAW
  CVI_MQ_MSG_S msg;
  msg.target_id = CVI_MQ_ID(CVI_CMD_CLIENT_ID_RSC,
                            CVI_CMD_CHANNEL_ID_CAMERA(sns_id));
  msg.arg1 = cmd_id;
  msg.arg2 = 0xfacedead;
  msg.seq_no = i++;
  msg.len = (int32_t)CVI_MQ_MSG_HEADER_LEN;
  uint64_t boot_time;
  cvi_osal_get_boot_time(&boot_time);
  msg.crete_time = boot_time;
  CVI_MQ_SendRAW(&msg);
}
#endif

static void rs_event_task_entry(void *arg) {
    rs_context_handle_t rs = (rs_context_handle_t)arg;

    UNUSED(rs);

    // start mq
    CVI_MQ_ENDPOINT_CONFIG_S mq_config;
    mq_config.name = "rs_mq";
    mq_config.id = CVI_MQ_ID(CVI_CMD_CLIENT_ID_RECORD, CVI_CMD_CHANNEL_ID_RECORD(rs->id));
    mq_config.recv_cb = rs_mq_cb;
    mq_config.recv_cb_arg = (void *)rs;
    int32_t rc = CVI_MQ_CreateEndpoint(&mq_config, &rs->mq_ep);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("CVI_MQ_CreateEndpoint failed\n");
        exit(-1);
    }

    while (!rs->shutdown) {
        cvi_osal_task_sleep(10000);  // 10 ms
    }

    // cleanup mq
    CVI_MQ_DestroyEndpoint(rs->mq_ep);
}

int32_t rs_start_event_task(rs_context_handle_t rs) {
    // rs_param_handle_t p = &rs->param;

    cvi_osal_task_attr_t ta;
    ta.name = "rs_event";
    ta.entry = rs_event_task_entry;
    ta.param = (void *)rs;
    ta.priority = CVI_OSAL_PRI_NORMAL;
    ta.detached = false;
    int32_t rc = cvi_osal_task_create(&ta, &rs->event_task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("rs_event task create failed, %d\n", rc);
        return RS_ERR_FAILURE;
    }

    return RS_SUCCESS;
}

int32_t rs_stop_event_task(rs_context_handle_t rs) {
    // rs_param_handle_t p = &rs->param;

    int32_t rc = cvi_osal_task_join(rs->event_task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("rs_event task join failed, %d\n", rc);
        return RS_ERR_FAILURE;
    }
    cvi_osal_task_destroy(&rs->event_task);

    return RS_SUCCESS;
}
