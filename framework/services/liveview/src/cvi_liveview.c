#include <stdio.h>
#include <stdlib.h>

#include "cvi_liveview.h"
#include "cvi_log.h"
#include "cvi_mapi.h"
#include "cvi_osal.h"
#include "cvi_vpss.h"

typedef struct __lv_condext {
    lv_param_t                  param;

    volatile uint32_t           shutdown;

    CVI_MAPI_WND_HANDLE_T       wnd[CVI_DISP_MAX_WND_NUM];
    pthread_mutex_t             lv_mutex;
    pthread_cond_t              lv_cond;
    cvi_osal_task_handle_t      lv_task;

    // event task
    cvi_osal_task_handle_t      event_task;
    CVI_MQ_ENDPOINT_HANDLE_t    mq_ep;
} lv_context_t, *lv_context_handle_t;

typedef int32_t (*lv_cmd_cb_t)(CVI_MQ_MSG_S *msg, void *userdate);

typedef struct _lv_cmd_desc {
    lv_cmd_cb_t cb;
    uint32_t flags;
} lv_cmd_desc_t;

static uint8_t move_up_flag = 0;
static uint8_t move_down_flag = 0;

static void lv_cmd_movewndpost(lv_context_handle_t lv, int32_t wnd, bool add)
{
    lv_param_handle_t param = &lv->param;
    if (param->LiveviewService[wnd].wnd_attr.UsedCrop == true) {
        VPSS_CROP_INFO_S attr;
        VPSS_GRP_ATTR_S stGrpAttr;
        CHECK_RET(CVI_VPSS_GetGrpAttr(param->LiveviewService[wnd].wnd_attr.BindVprocId, &stGrpAttr));
        CHECK_RET(CVI_VPSS_GetChnCrop(param->LiveviewService[wnd].wnd_attr.BindVprocId, param->LiveviewService[wnd].wnd_attr.BindVprocChnId, &attr));
        if (add == true) {
            attr.stCropRect.s32Y = attr.stCropRect.s32Y + (param->LiveviewService[wnd].wnd_attr.OneStep);
        } else {
            attr.stCropRect.s32Y = attr.stCropRect.s32Y - (param->LiveviewService[wnd].wnd_attr.OneStep);
        }

        if ((attr.stCropRect.s32Y >= 0) && ((attr.stCropRect.s32Y + attr.stCropRect.u32Height) <= stGrpAttr.u32MaxH)) {
            if (add == true) {
                param->LiveviewService[wnd].wnd_attr.yStep++;
            } else {
                param->LiveviewService[wnd].wnd_attr.yStep--;
            }
        }
    }
}

static void lv_cmd_switchwndmod(cvi_cmd_liveview_t cmdid, lv_context_handle_t lv, uint32_t idset)
{
    lv_param_handle_t param = &lv->param;
    switch(cmdid) {
        case CVI_CMD_LIVEVIEW_SWITCH:
            for(uint32_t i = 0; i < param->WndCnt * 2; i+=2){
                uint32_t smallEn = (0x1 << i) & idset;
                uint32_t wndEn = (0x1 << (i+1)) & idset;
                param->LiveviewService[i/2].wnd_attr.SmallWndEnable = smallEn>0?true:false;
                param->LiveviewService[i/2].wnd_attr.WndEnable = wndEn>0?true:false;
            }
            break;
        case CVI_CMD_LIVEVIEW_MOVEUP:
            if (param->LiveviewService[idset].wnd_attr.SmallWndEnable == false){
                lv_cmd_movewndpost(lv, idset, false);
                move_up_flag = 1;
            }
            break;
        case CVI_CMD_LIVEVIEW_MOVEDOWN:
            if (param->LiveviewService[idset].wnd_attr.SmallWndEnable == false) {
                lv_cmd_movewndpost(lv, idset, true);
                move_down_flag = 1;
            }
            break;
        default:
            CVI_LOGE("cmdid %d is illegal\n", cmdid);
            break;
    }

}
static void lv_cmd_adjustwndfocus(lv_context_handle_t lv ,int32_t wnd , char* ratio){
    lv_param_handle_t param = &lv->param;

    if (param->LiveviewService[wnd].wnd_attr.UsedCrop == true) {
        param->LiveviewService[wnd].wnd_attr.ratio = atof(ratio);
        CVI_LOGD("focus ratio:%f",param->LiveviewService[wnd].wnd_attr.ratio);
    }
}

static int32_t lv_cmd_cb_adjustfocus(CVI_MQ_MSG_S *msg, void *userdata) {
    lv_context_handle_t lv = (lv_context_handle_t)userdata;
    lv_cmd_adjustwndfocus(lv, msg->arg2 , msg->payload);
    return 0;
}



// static int32_t lv_set_vprocattr(CVI_MAPI_VPROC_HANDLE_T vproc_hdl, CVI_LIVEVIEW_SERVICE_WNDATTR_S wnd_attr)
// {
//     int32_t ret = 0;

//     VPSS_CHN_ATTR_S stChnAttr;
//     ret = CVI_MAPI_VPROC_GetChnAttr(vproc_hdl, wnd_attr.BindVprocChnId, &stChnAttr);
//     if (ret != 0) {
//         CVI_LOGE("CVI_MAPI_VPROC_GetChnAttr failed\n");
//         return ret;
//     }
//     stChnAttr.bMirror = wnd_attr.WndMirror;
//     stChnAttr.bFlip = wnd_attr.WndFilp;

//     ret = CVI_MAPI_VPROC_SetChnAttr(vproc_hdl, wnd_attr.BindVprocChnId, &stChnAttr);
//     if (ret != 0) {
//         CVI_LOGE("CVI_MAPI_VPROC_SetChnAttr failed\n");
//         return ret;
//     }
//     return ret;
// }

static void lv_set_wndattrmirror(lv_context_handle_t lv, int32_t id, int32_t en)
{
    lv->param.LiveviewService[id].wnd_attr.WndMirror = en;
}

static void lv_set_wndattrfilp(lv_context_handle_t lv, int32_t id, int32_t en)
{
    lv->param.LiveviewService[id].wnd_attr.WndFilp = en;
}

static void lv_cmd_wnd_mirrorfilp(cvi_cmd_liveview_t cmdid, uint32_t attr_en, lv_context_handle_t lv)
{
    lv_param_handle_t param = &lv->param;
    uint32_t wndIndex = attr_en >> 1;
    uint32_t val = attr_en & 0x1;
    switch(cmdid) {
        case CVI_CMD_LIVEVIEW_MIRROR:
            param->LiveviewService[wndIndex].wnd_attr.WndMirror = val;
            lv_set_wndattrmirror(lv, wndIndex, val);
            break;
        case CVI_CMD_LIVEVIEW_FILP:
            param->LiveviewService[wndIndex].wnd_attr.WndFilp = val;
            lv_set_wndattrfilp(lv, wndIndex, val);
            break;
        default:
            CVI_LOGE("cmdid %d is illegal\n", cmdid);
            break;
    }
}

static int32_t lv_cmd_cb_shutdown(CVI_MQ_MSG_S *msg, void *userdata) {
    UNUSED(msg);
    lv_context_handle_t lv = (lv_context_handle_t)userdata;
    lv->shutdown = 1;
    // TODO: send ACK
    return 0;
}

static int32_t lv_cmd_cb_switchwndmod(CVI_MQ_MSG_S *msg, void *userdata) {
    lv_context_handle_t lv = (lv_context_handle_t)userdata;
    int32_t cmd_id = msg->arg1;
    lv_cmd_switchwndmod(cmd_id, lv, msg->arg2);
    // TODO: send ACK
    return 0;
}

static int32_t lv_cmd_cb_movewndup(CVI_MQ_MSG_S *msg, void *userdata) {
    lv_context_handle_t lv = (lv_context_handle_t)userdata;
    int32_t cmd_id = msg->arg1;
    lv_cmd_switchwndmod(cmd_id, lv, msg->arg2);
    // TODO: send ACK
    return 0;
}

static int32_t lv_cmd_cb_movewnddown(CVI_MQ_MSG_S *msg, void *userdata) {
    lv_context_handle_t lv = (lv_context_handle_t)userdata;
    int32_t cmd_id = msg->arg1;
    lv_cmd_switchwndmod(cmd_id, lv, msg->arg2);
    // TODO: send ACK
    return 0;
}

static int32_t lv_cmd_cb_mirror(CVI_MQ_MSG_S *msg, void *userdata) {
    lv_context_handle_t lv = (lv_context_handle_t)userdata;
    int32_t cmd_id = msg->arg1;
    int32_t en = msg->arg2;
    lv_cmd_wnd_mirrorfilp(cmd_id, en, lv);
    // TODO: send ACK
    return 0;
}


static int32_t lv_cmd_cb_filp(CVI_MQ_MSG_S *msg, void *userdata) {
    lv_context_handle_t lv = (lv_context_handle_t)userdata;
    int32_t cmd_id = msg->arg1;
    int32_t en = msg->arg2;
    lv_cmd_wnd_mirrorfilp(cmd_id, en, lv);
    // TODO: send ACK
    return 0;
}


static lv_cmd_desc_t lv_cmd_tbl[] = {
    {NULL,   0},                            /* 0x00    INVALID */
    {lv_cmd_cb_shutdown,   0},              /* 0x01    SHUTDOWN */
    {lv_cmd_cb_switchwndmod,  0},              /* 0x02    switch WNDMOD */
    {lv_cmd_cb_movewndup,   0},              /* 0x03    moveup */
    {lv_cmd_cb_movewnddown, 0},              /* 0x04    movedown WNDMOD */
    {lv_cmd_cb_mirror,  0},              /* 0x05    WND MIRROR */
    {lv_cmd_cb_filp,    0},              /* 0x06    WND FILP */
    {lv_cmd_cb_adjustfocus,    0},              /* 0x07    ADJUST FOCUS */
};

static int32_t lv_mq_cb(CVI_MQ_ENDPOINT_HANDLE_t ep, CVI_MQ_MSG_S *msg, void *ep_arg) {
    UNUSED(ep);
    int32_t ret = 0;
    lv_context_handle_t lv = (lv_context_handle_t)ep_arg;
    pthread_mutex_lock(&lv->lv_mutex);
#if 0
    printf("lv_mq_cb: rx, target_id = %08x, len = %d, ep_arg = %p\n", msg->target_id, msg->len, ep_arg);
    printf("lv_mq_cb:     arg1 = 0x%08x, arg2 = 0x%08x\n", msg->arg1, msg->arg2);
    printf("lv_mq_cb:     seq_no = 0x%04x, time = %lu\n", msg->seq_no, msg->crete_time);
    if (msg->len > (int32_t)CVI_MQ_MSG_HEADER_LEN + 4) {
        printf("lv_mq_cb:     payload [%02x %02x %02x %02x]\n", msg->payload[0], msg->payload[1],
               msg->payload[2], msg->payload[3]);
    }
#endif

    int32_t cmd_id = msg->arg1;
    CVI_LOG_ASSERT(cmd_id >= 0 && cmd_id < (int32_t)(sizeof(lv_cmd_tbl) / sizeof(lv_cmd_desc_t)),
                   "cmd_id %d out of range\n", cmd_id);

    if (lv_cmd_tbl[cmd_id].cb == NULL) {
        CVI_LOGE("cmd_id %d not handled\n", cmd_id);
        pthread_cond_signal(&lv->lv_cond);
        pthread_mutex_unlock(&lv->lv_mutex);
        return -1;
    }

    ret = lv_cmd_tbl[cmd_id].cb(msg, ep_arg);
    if (ret != 0) {
        CVI_LOGE("cb %d failed! \n", cmd_id);
    }
    pthread_cond_signal(&lv->lv_cond);
    pthread_mutex_unlock(&lv->lv_mutex);

    return ret;
}

static void lv_event_task_entry(void *arg)
{
    lv_context_handle_t lv = (lv_context_handle_t)arg;
    lv_param_handle_t p = &lv->param;

    UNUSED(lv);
    UNUSED(p);

    // start mq
    CVI_MQ_ENDPOINT_CONFIG_S mq_config;
    mq_config.name = "lv_mq";
    mq_config.id = CVI_MQ_ID(CVI_CMD_CLIENT_ID_LIVEVIEW, 0);
    mq_config.recv_cb = lv_mq_cb;
    mq_config.recv_cb_arg = (void *)lv;
    int32_t rc = CVI_MQ_CreateEndpoint(&mq_config, &lv->mq_ep);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("CVI_MQ_CreateEndpoint failed\n");
        exit(-1);
    }

    while (!lv->shutdown) {
        cvi_osal_task_sleep(10000);  // 10 ms
    }

    // cleanup mq
    CVI_MQ_DestroyEndpoint(lv->mq_ep);
}

int32_t lv_start_event_task(lv_context_handle_t lv)
{
    cvi_osal_task_attr_t ta;
    ta.name = "lv_event";
    ta.entry = lv_event_task_entry;
    ta.param = (void *)lv;
    ta.priority = CVI_OSAL_PRI_NORMAL;
    ta.detached = false;
    int32_t rc = cvi_osal_task_create(&ta, &lv->event_task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("lv_event task create failed, %d\n", rc);
        return -1;
    }

    return 0;
}

int32_t lv_stop_event_task(lv_context_handle_t lv)
{
    int32_t rc = cvi_osal_task_join(lv->event_task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("lv_event task join failed, %d\n", rc);
        return -1;
    }
    cvi_osal_task_destroy(&lv->event_task);

    return 0;
}


static void lv_set_toVoVprocChnAttr(lv_context_handle_t lv)
{
    lv_param_handle_t p = &lv->param;
    uint32_t i = 0;
    CVI_STITCH_ATTR_S stStitchAttr = {0};
    CVI_VPSS_GetStitchAttr(p->vproc_id, &stStitchAttr);
    for (i = 0; i < p->WndCnt; i++) {
		stStitchAttr.astStitchChn[i].stStitchSrc.VpssGrp = -1;
		stStitchAttr.astStitchChn[i].stStitchSrc.VpssChn = -1;
	}
	CVI_VPSS_SetStitchAttr(p->vproc_id, &stStitchAttr);
    for (i = 0; i < p->WndCnt; i++) {
        if (p->LiveviewService[i].wnd_attr.WndEnable == true) {
            VPSS_CHN_ATTR_S stChnAttr;
            CHECK_RET(CVI_MAPI_VPROC_GetChnAttr(p->LiveviewService[i].vproc_hdl, p->LiveviewService[i].wnd_attr.BindVprocChnId, &stChnAttr));
            VPSS_GRP_ATTR_S pstGrpAttr = {0};
            CHECK_RET(CVI_VPSS_GetGrpAttr(p->LiveviewService[i].wnd_attr.BindVprocId, &pstGrpAttr));
            if (p->LiveviewService[i].wnd_attr.SmallWndEnable == false) {
                stChnAttr.u32Width = p->LiveviewService[i].wnd_attr.WndWidth;
                stChnAttr.u32Height = p->LiveviewService[i].wnd_attr.WndHeight;
                stStitchAttr.astStitchChn[i].stDstRect.s32X = p->LiveviewService[i].wnd_attr.WndX;
                stStitchAttr.astStitchChn[i].stDstRect.s32Y = p->LiveviewService[i].wnd_attr.WndY;
                stStitchAttr.astStitchChn[i].stDstRect.u32Width = p->LiveviewService[i].wnd_attr.WndWidth;
                stStitchAttr.astStitchChn[i].stDstRect.u32Height = p->LiveviewService[i].wnd_attr.WndHeight;
            } else {
                stChnAttr.u32Width = p->LiveviewService[i].wnd_attr.WndsWidth;
                stChnAttr.u32Height = p->LiveviewService[i].wnd_attr.WndsHeight;
                stStitchAttr.astStitchChn[i].stDstRect.s32X = p->LiveviewService[i].wnd_attr.WndsX;
                stStitchAttr.astStitchChn[i].stDstRect.s32Y = p->LiveviewService[i].wnd_attr.WndsY;
                stStitchAttr.astStitchChn[i].stDstRect.u32Width = p->LiveviewService[i].wnd_attr.WndsWidth;
                stStitchAttr.astStitchChn[i].stDstRect.u32Height = p->LiveviewService[i].wnd_attr.WndsHeight;
            }
			stStitchAttr.astStitchChn[i].stStitchSrc.VpssGrp = p->LiveviewService[i].wnd_attr.BindVprocId;
			stStitchAttr.astStitchChn[i].stStitchSrc.VpssChn = p->LiveviewService[i].wnd_attr.BindVprocChnId;
            stChnAttr.bFlip = p->LiveviewService[i].wnd_attr.WndFilp;
            stChnAttr.bMirror = p->LiveviewService[i].wnd_attr.WndMirror;
            // CVI_LOGE(" w = %d  h = %d  w = %d  h = %d \n", stChnAttr.u32Width, stChnAttr.u32Height, p->LiveviewService[i].wnd_attr.WndWidth, p->LiveviewService[i].wnd_attr.WndHeight);
            CHECK_RET(CVI_MAPI_VPROC_SetChnAttr(p->LiveviewService[i].vproc_hdl,
                p->LiveviewService[i].wnd_attr.BindVprocChnId, &stChnAttr));
#define CROP_ALIGN_UP(s, n) (((s) + (n) - 1) & (~((n) - 1)))
            if (p->LiveviewService[i].wnd_attr.UsedCrop == true) {
                VPSS_CROP_INFO_S pAttr = {0};
                CHECK_RET(CVI_VPSS_GetChnCrop(p->LiveviewService[i].wnd_attr.BindVprocId, p->LiveviewService[i].wnd_attr.BindVprocChnId, &pAttr));
                int32_t w = 0, h = 0, offset = 0, wndw = 0, wndh = 0;
                if (p->LiveviewService[i].wnd_attr.SmallWndEnable == false) {
                    wndw = p->LiveviewService[i].wnd_attr.WndWidth;
                    wndh = p->LiveviewService[i].wnd_attr.WndHeight;
                } else {
                    wndw = p->LiveviewService[i].wnd_attr.WndsWidth;
                    wndh = p->LiveviewService[i].wnd_attr.WndsHeight;
                }
                float ratio = (float)wndw / (float)wndh;
                CVI_LOGD("srceen ratio:%f \n",ratio);
                CVI_LOGD("p->LiveviewService[i].wnd_attr.ratio:%f \n",p->LiveviewService[i].wnd_attr.ratio);
                w = (int32_t)((float)pstGrpAttr.u32MaxH * ratio);
                w = CROP_ALIGN_UP(w, 16);
                if(w <= (int32_t)pstGrpAttr.u32MaxW){
                    h = pstGrpAttr.u32MaxH;
                    p->LiveviewService[i].wnd_attr.Yoffset = (float)h / 2 - (float)h / (p->LiveviewService[i].wnd_attr.ratio * 2);
                    p->LiveviewService[i].wnd_attr.Xoffset = (float)w / 2 - (float)w / (p->LiveviewService[i].wnd_attr.ratio * 2);
                    pAttr.stCropRect.s32X = (pstGrpAttr.u32MaxW - w) / 2 +  p->LiveviewService[i].wnd_attr.Xoffset;
                }else{
                    w = pstGrpAttr.u32MaxW;
                    h = (int32_t)((float)pstGrpAttr.u32MaxW / ratio);
                    h = CROP_ALIGN_UP(h, 16);
                    p->LiveviewService[i].wnd_attr.Yoffset = (float)h / 2 - (float)h / (p->LiveviewService[i].wnd_attr.ratio * 2);
                    p->LiveviewService[i].wnd_attr.Xoffset = (float)w / 2 - (float)w / (p->LiveviewService[i].wnd_attr.ratio * 2);
                    pAttr.stCropRect.s32X = p->LiveviewService[i].wnd_attr.Xoffset;
                }
                CVI_LOGD(" p->LiveviewService[i].wnd_attr.Yoffset:%d \n", p->LiveviewService[i].wnd_attr.Yoffset);
                CVI_LOGD(" p->LiveviewService[i].wnd_attr.Xoffset:%d \n", p->LiveviewService[i].wnd_attr.Xoffset);
                if(p->LiveviewService[i].wnd_attr.OneStep > 0 && move_up_flag == 1) { // 窄屏适应,移动屏幕
                    pAttr.stCropRect.s32Y = pAttr.stCropRect.s32Y - p->LiveviewService[i].wnd_attr.OneStep;
                    move_up_flag = 0;
                    if(pAttr.stCropRect.s32Y < 0){
                        pAttr.stCropRect.s32Y = 0;
                    }else if(pAttr.stCropRect.s32Y > (int32_t)(pstGrpAttr.u32MaxH - h + p->LiveviewService[i].wnd_attr.Yoffset)){
                        pAttr.stCropRect.s32Y = pstGrpAttr.u32MaxH - h + p->LiveviewService[i].wnd_attr.Yoffset;
                    }
                } else if(p->LiveviewService[i].wnd_attr.OneStep > 0 && move_down_flag == 1) { // 窄屏适应,移动屏幕
                    pAttr.stCropRect.s32Y = pAttr.stCropRect.s32Y + p->LiveviewService[i].wnd_attr.OneStep;
                    move_down_flag = 0;
                    if(pAttr.stCropRect.s32Y < 0){
                        pAttr.stCropRect.s32Y = 0;
                    }else if(pAttr.stCropRect.s32Y > (int32_t)(pstGrpAttr.u32MaxH - h + p->LiveviewService[i].wnd_attr.Yoffset)){
                        pAttr.stCropRect.s32Y = pstGrpAttr.u32MaxH - h + p->LiveviewService[i].wnd_attr.Yoffset;
                    }
                } else if(p->LiveviewService[i].wnd_attr.OneStep > 0){ // 窄屏适应,调整focus
                    offset = p->LiveviewService[i].wnd_attr.yStep * p->LiveviewService[i].wnd_attr.OneStep;
                    int32_t temp_Y = (pstGrpAttr.u32MaxH - h) / 2 + offset;
                    pAttr.stCropRect.s32Y = temp_Y + p->LiveviewService[i].wnd_attr.Yoffset;
                    if(pAttr.stCropRect.s32Y < 0){
                        pAttr.stCropRect.s32Y = 0;
                    }else if(pAttr.stCropRect.s32Y > (int32_t)(pstGrpAttr.u32MaxH - h + p->LiveviewService[i].wnd_attr.Yoffset)){
                        pAttr.stCropRect.s32Y = pstGrpAttr.u32MaxH - h + p->LiveviewService[i].wnd_attr.Yoffset;
                    }
                } else { // 宽屏适应,调整focus
                    pAttr.stCropRect.s32Y = p->LiveviewService[i].wnd_attr.Yoffset;
                }
                pAttr.bEnable = true;
                pAttr.enCropCoordinate = VPSS_CROP_ABS_COOR;
                pAttr.stCropRect.u32Width = w - 2 *  p->LiveviewService[i].wnd_attr.Xoffset;
                pAttr.stCropRect.u32Height = h - 2 *  p->LiveviewService[i].wnd_attr.Yoffset;
                CVI_LOGD("pAttr.stCropRect.s32Y:%d \n",pAttr.stCropRect.s32Y);
                CVI_LOGD("pAttr.stCropRect.s32X:%d \n",pAttr.stCropRect.s32X);
                CVI_LOGD("pAttr.stCropRect.u32Width:%d \n",pAttr.stCropRect.u32Width);
                CVI_LOGD("pAttr.stCropRect.u32Height:%d \n",pAttr.stCropRect.u32Height);
                CHECK_RET(CVI_VPSS_SetChnCrop(p->LiveviewService[i].wnd_attr.BindVprocId, p->LiveviewService[i].wnd_attr.BindVprocChnId, &pAttr));
            }
        } else {
			stStitchAttr.astStitchChn[i].stStitchSrc.VpssGrp = -1;
			stStitchAttr.astStitchChn[i].stStitchSrc.VpssChn = -1;
        }
    }

	CVI_VPSS_SetStitchAttr(p->vproc_id, &stStitchAttr);
}

static void lv_video_task_entry(void *arg)
{
    lv_context_handle_t lv = (lv_context_handle_t)arg;

    while (!lv->shutdown) {
        pthread_mutex_lock(&lv->lv_mutex);
        lv_set_toVoVprocChnAttr(lv);
        pthread_cond_wait(&lv->lv_cond, &lv->lv_mutex);
        pthread_mutex_unlock(&lv->lv_mutex);
    }
}

static int32_t lv_start_video_task(lv_context_handle_t lv)
{
    cvi_osal_task_attr_t ta;
    ta.name = "liveview";
    ta.entry = lv_video_task_entry;
    ta.param = (void *)lv;
    ta.priority = CVI_OSAL_PRI_NORMAL;
    ta.detached = false;
    int32_t rc = cvi_osal_task_create(&ta, &lv->lv_task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("lv_video task create failed, %d\n", rc);
        return -1;
    }

    return 0;
}

int32_t lv_stop_video_task(lv_context_handle_t lv) {

    int32_t rc = cvi_osal_task_join(lv->lv_task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("lv_video task join failed, %d\n", rc);
        return -1;
    }
    cvi_osal_task_destroy(&lv->lv_task);

    return 0;
}


int32_t CVI_LIVEVIEW_SERVICE_GetStepY(CVI_LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId,int32_t* lastStep)
{
    (void)hdl;
    (void)wndId;
    (void)lastStep;
    // if (hdl == NULL) {
    //     return -1;
    // }
    // CVI_LIVEVIEW_SERVICE_WNDATTR_S *WndParam;
    // WndParam = (CVI_LIVEVIEW_SERVICE_WNDATTR_S *)malloc(sizeof(CVI_LIVEVIEW_SERVICE_WNDATTR_S));
    // if(CVI_LIVEVIEW_SERVICE_GetParam(hdl, wndId, WndParam) != 0) {
    //     free(WndParam);
    //     return -1;
    // } else {
    //     lastStep = (int32_t*)WndParam->yStep;
    // }

    // CVI_LOGD("CVI_LIVEVIEW_SERVICE_GetStepY lastStep = (%d)\n", (int32_t)lastStep);

    // free(WndParam);
    return 0;
}

int32_t CVI_LIVEVIEW_SERVICE_SetStepY(CVI_LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId,int32_t step,int32_t* lastStep)
{
    (void)hdl;
    (void)wndId;
    (void)lastStep;
    (void)step;
    // if (hdl == NULL) {
    //     return -1;
    // }

    // CVI_PARAM_WND_ATTR_S WndParam;
    // CVI_PARAM_GetWndParam(&WndParam);
    // WndParam.Wnds[wndId].yStep = step;
    // lastStep = (int32_t*)step;
    // CVI_PARAM_SetWndParam(&WndParam);

    // CVI_LOGD("CVI_LIVEVIEW_SERVICE_SetStepY lastStep = (%d)\n", (int32_t)lastStep);
    return 0;
}

int32_t CVI_LIVEVIEW_SERVICE_AddStepY(CVI_LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId, int32_t step,int32_t* lastStep)
{
    (void)hdl;
    (void)wndId;
    (void)lastStep;
    (void)step;
    // if(hdl == NULL) {
    //     return -1;
    // }

    // CVI_PARAM_WND_ATTR_S WndParam;
    // CVI_PARAM_GetWndParam(&WndParam);
    // WndParam.Wnds[wndId].yStep = (WndParam.Wnds[wndId].yStep + step);
    // lastStep = (int32_t*)WndParam.Wnds[wndId].yStep;
    // CVI_PARAM_SetWndParam(&WndParam);
    // CVI_LOGD("CVI_LIVEVIEW_SERVICE_AddStepY lastStep = (%d)\n", (int32_t)lastStep);

	return 0;
}

int32_t CVI_LIVEVIEW_SERVICE_Create(CVI_LIVEVIEW_SERVICE_HANDLE_T *hdl, CVI_LIVEVIEW_SERVICE_PARAM_S *params)
{
    lv_context_handle_t lv = (lv_context_handle_t)calloc(sizeof(lv_context_t), 1);
    lv->param = *params;
    lv_param_handle_t p = &lv->param;
    uint32_t i = 0;

    CVI_STITCH_ATTR_S stStitchAttr;
    memset(&stStitchAttr, 0x0, sizeof(stStitchAttr));
    stStitchAttr.u8ChnNum = p->WndCnt;
    for (i = 0; i < p->WndCnt; i++) {
        p->LiveviewService[i].wnd_attr.ratio = 1; // 初始化focus ratio为1
        if (p->LiveviewService[i].wnd_attr.SmallWndEnable == false) {
            stStitchAttr.astStitchChn[i].stDstRect.s32X = p->LiveviewService[i].wnd_attr.WndX;
            stStitchAttr.astStitchChn[i].stDstRect.s32Y = p->LiveviewService[i].wnd_attr.WndY;
            stStitchAttr.astStitchChn[i].stDstRect.u32Width = p->LiveviewService[i].wnd_attr.WndWidth;
            stStitchAttr.astStitchChn[i].stDstRect.u32Height = p->LiveviewService[i].wnd_attr.WndHeight;
        } else {
            stStitchAttr.astStitchChn[i].stDstRect.s32X = p->LiveviewService[i].wnd_attr.WndsX;
            stStitchAttr.astStitchChn[i].stDstRect.s32Y = p->LiveviewService[i].wnd_attr.WndsY;
            stStitchAttr.astStitchChn[i].stDstRect.u32Width = p->LiveviewService[i].wnd_attr.WndsWidth;
            stStitchAttr.astStitchChn[i].stDstRect.u32Height = p->LiveviewService[i].wnd_attr.WndsHeight;
        }
        stStitchAttr.astStitchChn[i].stStitchSrc.VpssGrp = p->LiveviewService[i].wnd_attr.BindVprocId;
        stStitchAttr.astStitchChn[i].stStitchSrc.VpssChn = p->LiveviewService[i].wnd_attr.BindVprocChnId;
    }
    CVI_MAPI_DISP_CreateWindow(0, &stStitchAttr);
    p->hVbPool = stStitchAttr.hVbPool;
    p->vproc_id = CVI_VPSS_GetAvailableGrp();
    if (p->vproc_id < 0) {
        CVI_LOGE("CVI_VPSS_GetAvailableGrp failed grpid = %d!", p->vproc_id);
        return -1;
    }
    CVI_LOGD("VpssGrp ==== %d\n", p->vproc_id);

    CVI_VPSS_CreateStitch(p->vproc_id, &stStitchAttr);
    CVI_VPSS_StartStitch(p->vproc_id);
    pthread_mutex_init(&lv->lv_mutex, NULL);
    pthread_condattr_t condattr;
    pthread_condattr_init(&condattr);
    pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
    pthread_cond_init(&lv->lv_cond, &condattr);
    pthread_condattr_destroy(&condattr);

    //start send frame to wnd grp
    lv_start_video_task(lv);

    //start recvie cmd
    lv_start_event_task(lv);

    *hdl = (CVI_LIVEVIEW_SERVICE_HANDLE_T)lv;

    return 0;
}

int32_t CVI_LIVEVIEW_SERVICE_Destroy(CVI_LIVEVIEW_SERVICE_HANDLE_T hdl)
{
    lv_context_handle_t lv = (lv_context_handle_t)hdl;
    lv_param_handle_t p = &lv->param;
    // uint32_t i = 0;
    // send shutdown to self
    lv->shutdown = 1;

    // wait for exit
    while (!lv->shutdown) {
        cvi_osal_task_sleep(20000);
    }
    pthread_mutex_lock(&lv->lv_mutex);
    pthread_cond_signal(&lv->lv_cond);
    pthread_mutex_unlock(&lv->lv_mutex);
    CVI_LOGI("LiveView Service destroy\n");

    // stop event task
    CHECK_RET(lv_stop_event_task(lv));

    // stop video task
    CHECK_RET(lv_stop_video_task(lv));

    CVI_VPSS_StopStitch(p->vproc_id);
    CVI_VPSS_DestroyStitch(p->vproc_id);
    if(p->hVbPool != VB_INVALID_POOLID){
        CVI_STITCH_ATTR_S stStitchAttr = {0};
        stStitchAttr.hVbPool = p->hVbPool;
        CVI_MAPI_DISP_DestroyWindow(0, &stStitchAttr);
        p->hVbPool = VB_INVALID_POOLID;
    }
    pthread_cond_destroy(&lv->lv_cond);
    pthread_mutex_destroy(&lv->lv_mutex);
    CVI_LOGI("LiveView Service destroy end\n");
    free(lv);
    return 0;
}

int32_t CVI_LIVEVIEW_SERVICE_GetParam(CVI_LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId, CVI_LIVEVIEW_SERVICE_WNDATTR_S *WndParam)
{
    lv_context_handle_t lv = (lv_context_handle_t)hdl;

    lv_param_handle_t p = &lv->param;

    memcpy(WndParam, &p->LiveviewService[wndId].wnd_attr, sizeof(CVI_LIVEVIEW_SERVICE_WNDATTR_S));

    return 0;
}
