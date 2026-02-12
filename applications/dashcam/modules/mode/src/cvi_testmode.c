#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "cvi_modetest.h"
#include "cvi_mapi.h"
#include "cvi_osal.h"
#include "cvi_mode.h"
#include "cvi_param.h"
#include "cvi_media_dump.h"
#include "cvi_media_init.h"
#include "cvi_modeinner.h"
#include "cvi_media_sensor_test.h"
// #include "tkc/mem.h"

typedef struct __mt_context {
    mt_param_t                  param;
    volatile uint32_t           shutdown;
    volatile bool               curwndmode;
    volatile bool               newwndmode;
    pthread_mutex_t             mt_mutex;
    cvi_osal_task_handle_t      mt_task;

    // event task
    cvi_osal_task_handle_t      event_task;
    CVI_MQ_ENDPOINT_HANDLE_t    mq_ep;
} mt_context_t, *mt_context_handle_t;

typedef int32_t (*mt_cmd_cb_t)(CVI_MQ_MSG_S *msg, void *userdate);

typedef struct _mt_cmd_desc {
    mt_cmd_cb_t cb;
    uint32_t flags;
} mt_cmd_desc_t;

uint32_t get_sd_card_avaible(void)
{
    return CVI_MODEMNG_GetCardState();
}

static int32_t mt_cmd_cb_start_rec(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    // tk_mem_dump();
    // return 0;
    if (get_sd_card_avaible()) {
        CVI_MESSAGE_S Msg = {0};
        Msg.topic = CVI_EVENT_MODEMNG_START_REC;
        CVI_MODEMNG_SendMessage(&Msg);
    } else {
        CVI_LOGI("the card is not exist\n");
    }
    return 0;
}

static int32_t mt_cmd_cb_stop_rec(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    uint32_t u32ModeState = 0;
    CVI_MODEMNG_GetModeState(&u32ModeState);
    if ((u32ModeState == CVI_MEDIA_MOVIE_STATE_REC) ||
        (u32ModeState == CVI_MEDIA_MOVIE_STATE_LAPSE_REC)) {
        CVI_MESSAGE_S Msg = {0};
        Msg.topic = CVI_EVENT_MODEMNG_STOP_REC;
        CVI_MODEMNG_SendMessage(&Msg);
    } else {
        CVI_LOGI("the record is not working\n");
    }

    return 0;
}

static int32_t mt_cmd_cb_start_event_rec(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);

    uint32_t u32ModeState = 0;
    CVI_MODEMNG_GetModeState(&u32ModeState);
    if (u32ModeState != CVI_MEDIA_MOVIE_STATE_LAPSE_REC) {
        if (get_sd_card_avaible()) {
            CVI_MESSAGE_S Msg = {0};
            Msg.topic = CVI_EVENT_MODEMNG_START_EMRREC;
            CVI_MODEMNG_SendMessage(&Msg);
        } else {
            CVI_LOGI("the card is not exist\n");
        }
    }

    return 0;
}

static int32_t mt_cmd_cb_start_piv(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    if (get_sd_card_avaible()) {
        if (msg->arg2 < 101 && msg->arg2 >0) {
            for(int32_t i=0; i<msg->arg2; i++) {
                CVI_MESSAGE_S Msg = {0};
                Msg.topic = CVI_EVENT_MODEMNG_START_PIV;
                CVI_MODEMNG_SendMessage(&Msg);
                cvi_osal_task_sleep(500 * 1000);
            }
        } else {
            printf("error start pri parm\n");
        }
    } else {
        CVI_LOGI("the card is not exist\n");
    }

    return 0;
}

#define MAX_PATH 255
static int32_t mt_cmd_cb_remove_piv(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    char filename[MAX_PATH];
    static uint32_t pu32FileObjCnt = 0;
    CVI_DTCF_DIR_E aenDirs = DTCF_DIR_PHOTO_FRONT;
    CVI_FILEMNG_SetSearchScope(&aenDirs, 1, &pu32FileObjCnt);

    if (get_sd_card_avaible()) {
        if (msg->arg2 < 100 || msg->arg2 > 0) {
            for (int32_t i=0; i<msg->arg2; i++) {
                CVI_FILEMNG_GetFileByIndex(pu32FileObjCnt - 1, filename, MAX_PATH);
                CVI_LOGE("delete cur filename = %s\n", filename);
                if (1 < pu32FileObjCnt) {
                    CVI_FILEMNG_RemoveFile(filename);
                    cvi_async();
                    pu32FileObjCnt--;
                } else {
                    pu32FileObjCnt = 0;
                    CVI_FILEMNG_RemoveFile(filename);
                    cvi_async();
                    return 0;
                }
            }
        } else {
            printf("error start pri parm\n");
        }
    } else {
        CVI_LOGI("the card is not exist\n");
    }

    return 0;
}

static void set_option_itemcnt2videosize(uint32_t *viedosize, int32_t itemcnt)
{
    if (itemcnt == 0) {
        *viedosize = CVI_MEDIA_VIDEO_SIZE_2560X1440P25;
    } else if (itemcnt == 1){
        *viedosize = CVI_MEDIA_VIDEO_SIZE_1920X1080P25;
    } else {
        CVI_LOGE("media video size faile !\n");
    }
}

static int32_t mt_cmd_cb_set_media_size(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    uint32_t videosize = 0;
    if (msg->arg2 > 2 ||  msg->arg2 < 0) {
        return 0;
    }
    set_option_itemcnt2videosize(&videosize, msg->arg2);
    CVI_MESSAGE_S Msg = {0};
    Msg.topic = CVI_EVENT_MODEMNG_SETTING;
    Msg.arg1 = CVI_PARAM_MENU_VIDEO_SIZE;
    if (0 == msg->arg2) {
        Msg.arg2 = CVI_MEDIA_VIDEO_SIZE_2560X1440P25;
    } else {
        Msg.arg2 = CVI_MEDIA_VIDEO_SIZE_1920X1080P25;
    }
    CVI_MODEMNG_SendMessage(&Msg);

    // CVI_MODE_SetMediaVideoSize(videosize);
    return 0;
}

static int32_t mt_cmd_cb_set_media_audio(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    CVI_MESSAGE_S Msg = {0};

    Msg.topic = CVI_EVENT_MODEMNG_SETTING;
    Msg.arg1 = CVI_PARAM_MENU_AUDIO_STATUS;
    Msg.arg2 = msg->arg2;
    CVI_MODEMNG_SendMessage(&Msg);

    return 0;
}

static int32_t mt_cmd_cb_dump_data(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    #ifdef ENABLE_ISP_PQ_TOOL
    if (CVI_CARD_STATE_AVAILABLE == CVI_MODEMNG_GetCardState()) {
        CVI_MEDIA_DUMP_SetDumpRawAttr(0);
        CVI_MEDIA_DUMP_DumpRaw(0);
        CVI_MEDIA_DUMP_DumpYuv(0);
        cvi_system("sync");
    }
    #else
    CVI_LOGE("PQ Tool Disable!\n");
    #endif
    return 0;
}

static int32_t mt_cmd_cb_set_media_venc(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    CVI_MODEMNG_SetMediaVencFormat(msg->arg2);
    return 0;
}

static int32_t dec_flag = 0;
static int32_t mt_cmd_cb_start_dec(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    if (get_sd_card_avaible() && (dec_flag == 0)) {
        CVI_MESSAGE_S Msg;
        Msg.topic = CVI_EVENT_MODEMNG_MODESWITCH;
        Msg.arg1 = CVI_WORK_MODE_PLAYBACK;
        CVI_MODEMNG_SendMessage(&Msg);
        dec_flag = 1;
    }
    return 0;
}

static int32_t mt_cmd_cb_stop_dec(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    if (get_sd_card_avaible() && (dec_flag == 1)) {
        int32_t s32Ret = 0;
        uint32_t u32Index = 0;
        uint32_t u32DirCount = 0;
        CVI_PARAM_FILEMNG_S stCfg = {};
        CVI_DTCF_DIR_E aenDirs[DTCF_DIR_BUTT];

        s32Ret = CVI_PARAM_GetFileMngParam(&stCfg);
        if (s32Ret != 0) {
            CVI_LOGW("get file system param fialed\n");
        }
        for(u32Index = 0; u32Index < DTCF_DIR_BUTT; u32Index++) {
            if ( 0 < strnlen(stCfg.FileMngDtcf.aszDirNames[u32Index], CVI_DIR_LEN_MAX)) {
                aenDirs[u32DirCount++] = u32Index;
            }
        }

        uint32_t u32Temp = 0;
        s32Ret = CVI_FILEMNG_SetSearchScope(aenDirs, u32DirCount, &u32Temp);
        if (s32Ret != 0) {
            CVI_LOGW("search scope all dir fialed\n");
        }

        CVI_MESSAGE_S Msg;
        Msg.topic = CVI_EVENT_MODEMNG_MODESWITCH;
        Msg.arg1 = CVI_WORK_MODE_MOVIE;
        CVI_MODEMNG_SendMessage(&Msg);
        dec_flag = 0;
    }
    return 0;
}

static int32_t mt_cmd_cb_switch_mode(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    if (get_sd_card_avaible()) {
        CVI_MESSAGE_S event;
        event.topic = CVI_EVENT_MODEMNG_MODESWITCH;
        event.arg1 = msg->arg2;
        CVI_MODEMNG_SendMessage(&event);
    }
    return 0;
}

static int32_t mt_cmd_cb_start_record(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODETEST_START_RECORD;
    CVI_EVENTHUB_Publish(&stEvent);
    return 0;
}

static int32_t mt_cmd_cb_stop_record(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODETEST_STOP_RECORD;
    CVI_EVENTHUB_Publish(&stEvent);
    return 0;
}

static int32_t mt_cmd_cb_play_record(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODETEST_PLAY_RECORD;
    stEvent.arg1 = msg->arg2;
    CVI_EVENTHUB_Publish(&stEvent);
    return 0;
}

static int32_t mt_cmd_cb_dump_sendvo(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    UNUSED(userdata);
    CVI_MAPI_DISP_SetDumpStatus(true);

    return 0;
}

static int32_t mt_cmd_cb_start_sensor_test(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    CVI_LOGD("msg->payload: %s, msg->arg1 = 0x%x\n", msg->payload, msg->arg1);
    sensor_test(msg->payload);
    return 0;
}

static int32_t mt_cmd_cb_update_ota(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    if (CVI_MODEMNG_GetCurWorkMode() == CVI_WORK_MODE_UPDATE) {
        if (get_sd_card_avaible()) {
            CVI_MESSAGE_S event;
            event.topic = CVI_EVENT_MODEMNG_START_UPFILE;
            event.arg1 = msg->arg2;
            char mode_version[] = "generic_0.1.8";
            memcpy(event.aszPayload, mode_version, strlen(mode_version)+1);
            CVI_MODEMNG_SendMessage(&event);
        } else {
            CVI_LOGE("the card is not exist\n");
        }
    } else {
        CVI_LOGE("please first switchmode to updatemode\n");
    }

    return 0;
}

static int32_t mt_cmd_cb_adjust_focus(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    if (CVI_MODEMNG_GetCurWorkMode() == CVI_WORK_MODE_MOVIE || CVI_MODEMNG_GetCurWorkMode() == CVI_WORK_MODE_PHOTO) {
        CVI_MESSAGE_S event;
        event.topic = CVI_EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS;
        event.arg1 = msg->arg2;
        snprintf((char *)event.aszPayload , sizeof(msg->payload) , "%s" , msg->payload);
        CVI_MODEMNG_SendMessage(&event);
    } else {
        CVI_LOGE("please first switchmode to movie or photo mode\n");
    }

    return 0;
}

static mt_cmd_desc_t mt_cmd_tbl[] = {
    {mt_cmd_cb_start_rec,    0},                    /* 0x00    START REC */
    {mt_cmd_cb_stop_rec,    0},                     /* 0x01    STOP REC */
    {mt_cmd_cb_start_event_rec,    0},              /* 0x02    EVNET REC */
    {mt_cmd_cb_start_piv,    0},                    /* 0x03    SET PIV */
    {mt_cmd_cb_set_media_size,    0},               /* 0x04    MEDIA SIZE */
    {mt_cmd_cb_set_media_audio,    0},              /* 0x05    SET AUDO */
    {mt_cmd_cb_dump_data,    0},                    /* 0x06    DWMP DATA */
    {mt_cmd_cb_set_media_venc,    0},               /* 0x06    SET VENC */
    {mt_cmd_cb_start_dec,    0},                    /* 0x07    START DEC */
    {mt_cmd_cb_stop_dec,    0},                     /* 0x08    STOP DEC */
    {mt_cmd_cb_remove_piv,    0},                   /* 0x09    REMOVE PHOTO*/
    {mt_cmd_cb_switch_mode,    0},                  /* 0x0A    SWITCH MODE*/
    {mt_cmd_cb_start_record,    0},                 /* 0x0B    START RECORD*/
    {mt_cmd_cb_stop_record,    0},                  /* 0x0C    STop RECORD*/
    {mt_cmd_cb_play_record,    0},                  /* 0x0D    PLAY RECORD*/
    {mt_cmd_cb_dump_sendvo,    0},                  /* 0x0E    DUMP SENDVO*/
    {mt_cmd_cb_start_sensor_test,    0},            /* 0x0F    SENSOR TEST*/
    {mt_cmd_cb_update_ota,    0},                   /* 0x11    start OTA*/
    {mt_cmd_cb_adjust_focus,    0},                 /* 0x12    adjust focus*/
};

static int32_t mt_mq_cb(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *ep_arg) {
    UNUSED(ep);

#if 1
    printf("mt_mq_cb: rx, target_id = %08x, len = %d, ep_arg = %p\n", msg->target_id, msg->len, ep_arg);
    printf("mt_mq_cb:     arg1 = 0x%08x, arg2 = 0x%08x\n", msg->arg1, msg->arg2);
    printf("mt_mq_cb:     seq_no = 0x%04x, time = %"PRIu64"\n", msg->seq_no, msg->crete_time);
    if (msg->len > (int32_t)CVI_MQ_MSG_HEADER_LEN + 4) {
        printf("mt_mq_cb:     payload [%02x %02x %02x %02x]\n", msg->payload[0], msg->payload[1],
               msg->payload[2], msg->payload[3]);
    }
#endif
    int32_t cmd_id = msg->arg1;
    CVI_LOG_ASSERT(cmd_id >= 0 && cmd_id < (int32_t)(sizeof(mt_cmd_tbl) / sizeof(mt_cmd_desc_t)),
                   "cmd_id %d out of range\n", cmd_id);

    if (mt_cmd_tbl[cmd_id].cb == NULL) {
        CVI_LOGE("cmd_id %d not handled\n", cmd_id);
        return -1;
    }

    return mt_cmd_tbl[cmd_id].cb(msg, ep_arg);
}

static void mt_event_task_entry(void *arg)
{
    mt_context_handle_t mt = (mt_context_handle_t)arg;
    UNUSED(mt);

    // start mq
    CVI_MQ_ENDPOINT_CONFIG_S mq_config;
    mq_config.name = "mt_mq";
    mq_config.id = CVI_MQ_ID(CVI_CMD_CLIENT_ID_MT_TOOL, CVI_CMD_CHANNEL_ID_MT(0));
    mq_config.recv_cb = mt_mq_cb;
    mq_config.recv_cb_arg = (void *)mt;
    int32_t rc = CVI_MQ_CreateEndpoint(&mq_config, &mt->mq_ep);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("CVI_MQ_CreateEndpoint failed\n");
        exit(-1);
    }
    while (!mt->shutdown) {
        cvi_osal_task_sleep(10000);  // 10 ms
    }
    // cleanup mq
    CVI_MQ_DestroyEndpoint(mt->mq_ep);
}

static int32_t mt_start_event_task(mt_context_handle_t mt)
{
    cvi_osal_task_attr_t ta;
    ta.name = "mt_event";
    ta.entry = mt_event_task_entry;
    ta.param = (void *)mt;
    ta.priority = CVI_OSAL_PRI_NORMAL;
    ta.detached = false;
    /*create event_task parm*/
    int32_t rc = cvi_osal_task_create(&ta, &mt->event_task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("lv_event task create failed, %d\n", rc);
        return -1;
    }
    return 0;
}

static int32_t mt_stop_event_task(mt_context_handle_t mt)
{
    int32_t rc = cvi_osal_task_join(mt->event_task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("lv_event task join failed, %d\n", rc);
        return -1;
    }
    cvi_osal_task_destroy(&mt->event_task);
    return 0;
}

int32_t CVI_MODE_Test_Create(CVI_MT_HANDLE_T *hdl, CVI_MT_SERVICE_PARAM_T *params)
{
    mt_context_handle_t mt = (mt_context_handle_t)calloc(sizeof(mt_context_t), 1);
    mt->param = *params;
    mt->curwndmode = true;

    pthread_mutex_init(&mt->mt_mutex, NULL);
    mt_start_event_task(mt);
    *hdl = (CVI_MT_HANDLE_T)mt;
    return 0;
}

int32_t CVI_MODE_Test_Destroy(CVI_MT_HANDLE_T hdl)
{
    mt_context_handle_t mt = (mt_context_handle_t)hdl;
    // send shutdown to self
    mt->shutdown = 1;
    int32_t s32Ret = 0;

    // wait for exit
    while (!mt->shutdown) {
        cvi_osal_task_sleep(20000);
    }

    CVI_LOGI("MODE_Test Service destroy\n");
    pthread_mutex_destroy(&mt->mt_mutex);

    s32Ret = mt_stop_event_task(mt);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "mt_stop_event_task");

    free(mt);
    return 0;
}

int32_t CVI_MODEMNG_TEST_MAIN_Create(void)
{
    CVI_MT_HANDLE_T *pmtHdl                 =   &CVI_MEDIA_GetCtx()->SysServices.MtHdl;
    CVI_MT_SERVICE_PARAM_T *pmtParams       =   &CVI_MEDIA_GetCtx()->SysServices.MtParam;
    int32_t s32Ret = 0;
    s32Ret = CVI_MODE_Test_Create(pmtHdl, pmtParams);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MODE_Test_Create");
    return 0;
}

int32_t CVI_MODEMNG_TEST_MAIN_Destroy(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_MODE_Test_Destroy(CVI_MEDIA_GetCtx()->SysServices.MtHdl);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MODE_Test_Destroy");
    return 0;
}