#include "cvi_osal.h"
#include "cvi_qrcode_ser.h"
#include "qrcode.h"

typedef struct qr_scanner_context{
    CVI_QRCODE_SERVICE_PARAM_S attr;
    int32_t exitflag;
    cvi_osal_task_handle_t task;
}qr_scanner_context_s;


static void qrcode_scanner_task(void *arg)
{
    qr_scanner_context_s *qr = (qr_scanner_context_s *)arg;
    char result[QRCODE_RESULT_MAX_CNT][QRCODE_RESULT_MAX_LEN];
    uint32_t cnt = 0;
    while(!qr->exitflag)
    {
        VIDEO_FRAME_INFO_S frame = {0};
        int32_t ret = CVI_MAPI_VPROC_GetChnFrame(qr->attr.vproc, qr->attr.vproc_chnid, &frame);
        if (ret != 0) {
            CVI_LOGE("QR[%d]: CVI_MAPI_VPROC_GetChnFrame failed", ret);
            cvi_osal_task_sleep(50 * 1000);
            continue;
        }

        if(qr->attr.w * qr->attr.h < frame.stVFrame.u32Length[0]){
            CVI_LOGE("Ydata mismatch %u < %u!!!", qr->attr.w * qr->attr.h, frame.stVFrame.u32Length[0]);
            CVI_MAPI_ReleaseFrame(&frame);
            cvi_osal_task_sleep(50 * 1000);
            continue;
        }

        if(CVI_MAPI_FrameMmap(&frame, 0) == 0){
            cnt = 0;
            qrcode_decode(frame.stVFrame.pu8VirAddr[0], result, &cnt);
            CVI_MAPI_FrameMunmap(&frame);
        }
        CVI_MAPI_ReleaseFrame(&frame);

        if(cnt > 0){
            for(uint32_t i = 0; i < cnt; i++){
                CVI_LOGI("result[%d] : %s", i, result[i]);
            }
        }

        cvi_osal_task_sleep(250 * 1000);
    }
}

int32_t CVI_QRCode_Service_Create(CVI_QRCODE_SERVICE_HANDLE_T *hdl, CVI_QRCODE_SERVICE_PARAM_S *attr)
{
    CVI_LOGI("CVI_QRCode_Service_Create start");
    int32_t rc = 0;
    qr_scanner_context_s *qr = (qr_scanner_context_s *)malloc(sizeof(qr_scanner_context_s));
    if(qr == NULL){
        CVI_LOGE("out of mem for qr %d", sizeof(qr_scanner_context_s));
        return -1;
    }
    memset(qr, 0x0, sizeof(qr_scanner_context_s));

    qrcode_attr_s qrattr;
    qrattr.w = attr->w;
    qrattr.h = attr->h;
    rc = qrcode_init(&qrattr);
    if(rc != 0){
        free(qr);
        return -1;
    }

    memcpy(&qr->attr, attr, sizeof(CVI_QRCODE_SERVICE_PARAM_S));

    cvi_osal_task_attr_t qr_ta;
    qr_ta.name = "qr";
    qr_ta.entry = qrcode_scanner_task;
    qr_ta.param = (void *)qr;
    qr_ta.priority = CVI_OSAL_PRI_NORMAL;
    qr_ta.detached = false;
    rc = cvi_osal_task_create(&qr_ta, &qr->task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("qrcode_scanner_task create failed, %d", rc);
        qrcode_deinit();
        free(qr);
        return -1;
    }
    *hdl = qr;
    CVI_LOGI("CVI_QRCode_Service_Create end");
    return 0;
}

int32_t CVI_QRCode_Service_Destroy(CVI_QRCODE_SERVICE_HANDLE_T hdl)
{
    if(hdl){
        qr_scanner_context_s *ctx = (qr_scanner_context_s *)hdl;
        ctx->exitflag = 1;
        int32_t rc = cvi_osal_task_join(ctx->task);
        if (rc != CVI_OSAL_SUCCESS) {
            CVI_LOGE("qr_scanner task join failed, %d", rc);
            return -1;
        }
        cvi_osal_task_destroy(&ctx->task);
        qrcode_deinit();
        free(ctx);
    }
    return 0;
}

