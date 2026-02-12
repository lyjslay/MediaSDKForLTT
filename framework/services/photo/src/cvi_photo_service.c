#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

#include "cvi_dtcf.h"

#include "cvi_photo_service.h"
#include "cvi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct thumbnail_buf_s{
    void *buf;
    uint32_t size;
    uint32_t actsize;
}thumbnail_buf_t;
static thumbnail_buf_t g_photo_thumbnail_buf[MAX_CONTEXT_CNT];
static thumbnail_buf_t g_snap_thumbnail_buf[MAX_CONTEXT_CNT];
static thumbnail_buf_t g_snap_buf[MAX_CONTEXT_CNT];

static phs_context_t gstPhsCtx[MAX_CONTEXT_CNT];

static CVI_PHOTO_SERVICE_ATTR_S gst_cvi_phs_attr[MAX_CONTEXT_CNT];

static int32_t phs_get_venc_stream(CVI_MAPI_VENC_HANDLE_T vhdl, VENC_STREAM_S *stream){
    int32_t ret = CVI_MAPI_VENC_GetStreamTimeWait(vhdl, stream, 1000);
    if (ret != 0) {
        CVI_LOGE("[%p]: CVI_MAPI_VENC_GetStreamTimeWait failed", vhdl);
        return -1;
    }

    if(stream->u32PackCount <= 0 || stream->u32PackCount > CVI_FRAME_STREAM_SEGMENT_MAX_NUM){
        CVI_MAPI_VENC_ReleaseStream(vhdl, stream);
        return -1;
    }

    return 0;
}

static int32_t cvi_phs_get_id(CVI_PHOTO_SERVICE_HANDLE_T phs)
{
    for(int32_t i = 0; i < MAX_CONTEXT_CNT; i++)
    {
        if(gst_cvi_phs_attr[i].phs == phs)
        {
            return i;
        }
    }
    return MAX_CONTEXT_CNT;
}

static void CVI_APPATTR_2_PHOTOATTR(CVI_PHOTO_SERVICE_PARAM_S *param, CVI_PHOTO_SERVICE_ATTR_S *attr)
{
    attr->handles.photo_venc_hdl = param->photo_venc_hdl;
    attr->handles.photo_bufsize = param->photo_bufsize;
    if(attr->handles.photo_bufsize <= 0){
        attr->handles.photo_bufsize = 1024 * 1024;
    }

    attr->handles.thumbnail_vproc = param->thumbnail_vproc;
    attr->handles.vproc_chn_id_thumbnail = param->vproc_chn_id_thumbnail;
    attr->handles.thumbnail_venc_hdl = param->thumbnail_venc_hdl;
    attr->handles.thumbnail_bufsize = param->thumbnail_bufsize;
    if(attr->handles.thumbnail_bufsize <= 0){
        attr->handles.thumbnail_bufsize = 128 * 1024;
    }

    attr->handles.src_vproc = param->src_vproc;
    attr->handles.src_vproc_chn_id = param->src_vproc_chn_id;

    attr->handles.scale_vproc = param->scale_vproc;
    attr->handles.scale_vproc_chn_id = param->scale_vproc_chn_id;

    attr->stCallback.pfnNormalPhsCb = param->cont_photo_event_cb;

    attr->s32SnapPresize = param->prealloclen;
}

static int32_t phs_write_snap_file(char *filename, uint8_t *pivdata, uint32_t pivlen, uint8_t *thmbdata, uint32_t thmblen, uint32_t alignsize){
    char JPEG_SOI[2] = {0xFF, 0xD8};
    int32_t fd = open(filename, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0) {
        CVI_LOGE("open file fail, %s", filename);
        return -1;
    }

    if (0 > write(fd, JPEG_SOI, sizeof(JPEG_SOI))) {
        CVI_LOGE("write error");
        close(fd);
        return -1;
    }

    if (thmblen < 0xFFFF) {
        char JFXX_header[10] = { // 10 = 2+2+5+1
            0xFF, 0xE0, //APP0 marker
            (thmblen + 8) >> 8, (thmblen + 8) & 0xFF, // Length of segment excluding APP0 marker
            0x4A, 0x46,0x58,0x58, 0x00, // Identifier,
            0x10 // Thumbnail format, 1 means jpeg
        };

        if (0 > write(fd, JFXX_header, sizeof(JFXX_header))) {
            CVI_LOGE("write JFXX_header error");
            close(fd);
            return -1;
        }

        if (0 > write(fd, thmbdata, thmblen)) {
            CVI_LOGE("write thumbnail_data error");
            close(fd);
            return -1;
        }
    } else {
        CVI_LOGE("piv thumbnail size exceed: %u", thmblen);
        close(fd);
        return -1;
    }

    CVI_LOGD("%d %X\n", pivlen, (pivlen >> 24) & 0xFF);
    unsigned char JPEG_LEN[8] = {0xFF, 0xE2, 0x00, 0x06, \
                        (pivlen >> 24) & 0xFF, (pivlen >> 16) & 0xFF, \
                        (pivlen >> 8) & 0xFF,  (pivlen >> 0) & 0xFF };
    if (0 > write(fd, JPEG_LEN, sizeof(JPEG_LEN))) {
        CVI_LOGE("write piv data end error");
        close(fd);
        return -1;
    }

    // skip SOI, 0xFF, 0xD8
    if (0 > write(fd, pivdata + 2, pivlen - 2)) {
        CVI_LOGE("write piv data error");
        close(fd);
        return -1;
    }

    // add useless msg in file
    char JPEG_END[2] = {0xFF, 0xD9};
    if (0 > write(fd, JPEG_END, sizeof(JPEG_END))) {
        CVI_LOGE("write piv data end error");
        close(fd);
        return -1;
    }

    if (pivlen + 8 < alignsize) {
        ftruncate(fd, alignsize);
    }
    close(fd);
    return 0;
}

static void phs_thumb_task_entry(void *arg){
    phs_context_handle_t phs = (phs_context_handle_t)arg;
    CVI_PHOTO_SERVICE_ATTR_S *p = (CVI_PHOTO_SERVICE_ATTR_S *)phs->attr;
    VIDEO_FRAME_INFO_S frame = {0};
    VENC_STREAM_S stream = {0};
    uint8_t *thumbdata = NULL;
    uint32_t *thmbsize = 0;
    uint32_t bufsize = 0;
    int32_t  thumbnail_flag = 0;
    while (!phs->shutdown) {
        if(phs->need_thumbnail == 0){
            cvi_osal_task_sleep(10 * 1000);
            continue;
        }

        pthread_mutex_lock(&phs->thumbnail_mutex);
        if((phs->need_thumbnail & 0x1) == 1){
            thumbdata = (uint8_t *)g_snap_thumbnail_buf[phs->id].buf;
            thmbsize = &g_snap_thumbnail_buf[phs->id].actsize;
            bufsize = g_snap_thumbnail_buf[phs->id].size;
            thumbnail_flag = 0;
        }
        *thmbsize = 0;

        CVI_MAPI_VENC_StartRecvFrame(p->handles.thumbnail_venc_hdl, -1);

        int32_t ret = CVI_MAPI_VPROC_GetChnFrame(p->handles.thumbnail_vproc, p->handles.vproc_chn_id_thumbnail, &frame);
        if (ret != 0) {
            CVI_LOGE("PHS[%d]: CVI_MAPI_VPROC_GetChnFrame failed", phs->id);
            goto END1;
        }

        ret = CVI_MAPI_VENC_SendFrame(p->handles.thumbnail_venc_hdl, &frame);
        if (ret != 0) {
            CVI_LOGE("PHS[%d]: CVI_MAPI_VENC_SendFrame failed", phs->id);
            goto END;
        }

        ret = CVI_MAPI_VENC_GetStream(p->handles.thumbnail_venc_hdl, &stream);
        if (ret != 0) {
            CVI_LOGE("PHS[%d]: CVI_MAPI_VENC_GetStream failed", phs->id);
            goto END;
        }

        CVI_LOGD("PHS[%d]: thumbnail buf size %u thmbsize %u\n", phs->id, bufsize, stream.pstPack[0].u32Len);
        if(bufsize < stream.pstPack[0].u32Len){
            LOG_PHET(CVI_MAPI_VENC_ReleaseStream(p->handles.thumbnail_venc_hdl, &stream));
            goto END;
        }

        memcpy(thumbdata, stream.pstPack[0].pu8Addr, stream.pstPack[0].u32Len);
        *thmbsize = stream.pstPack[0].u32Len;
        LOG_PHET(CVI_MAPI_VENC_ReleaseStream(p->handles.thumbnail_venc_hdl, &stream));
END:
        LOG_PHET(CVI_MAPI_ReleaseFrame(&frame));
END1:
        CVI_MAPI_VENC_StopRecvFrame(p->handles.thumbnail_venc_hdl);
        if(thumbnail_flag == 0){
            phs->need_thumbnail &= (~0x1);
        }else if(thumbnail_flag == 1){
            phs->need_thumbnail &= (~0x2);
        }
        CVI_LOGD("PHS[%d] catch thumbnail %s", phs->id, (*thmbsize > 0)?("success"):("failed"));
        pthread_mutex_unlock(&phs->thumbnail_mutex);
        cvi_osal_task_sleep(10 * 1000);
    }
    CVI_LOGD("PHS[%d] exit", phs->id);
}

static void  phs_snap_task_entry(void *arg) {
    phs_context_handle_t phs = (phs_context_handle_t)arg;
    CVI_PHOTO_SERVICE_ATTR_S *p = (CVI_PHOTO_SERVICE_ATTR_S *)phs->attr;

    uint8_t *thumbnail_data = (uint8_t *)g_snap_thumbnail_buf[phs->id].buf;
    uint32_t *thumbnail_len = &g_snap_thumbnail_buf[phs->id].actsize;
    VIDEO_FRAME_INFO_S src_frame;
    VIDEO_FRAME_INFO_S scale_frame;

    VENC_STREAM_S stream = {0};

    while (!phs->shutdown) {
        cvi_osal_task_sleep(50 * 1000);
        pthread_mutex_lock(&phs->piv_mutex);
        pthread_cond_wait(&phs->piv_cond, &phs->piv_mutex);
        if (phs->shutdown == 1) {
            phs->piv_finish = 1;
            pthread_mutex_unlock(&phs->piv_mutex);
            break;
        }

        if (p->stCallback.pfnNormalPhsCb) {
            ((CVI_PHOTO_SERVICE_EVENT_CALLBACK)p->stCallback.pfnNormalPhsCb)(CVI_PHOTO_SERVICE_EVENT_PIV_START, phs->piv_filename, (void*)(&phs->id));
        }

        if (p->handles.photo_venc_hdl == NULL) {
            CVI_LOGE("null snapshot venc hdl");
            phs->piv_finish = 1;
            pthread_mutex_unlock(&phs->piv_mutex);
            continue;
        }
        *thumbnail_len = 0;
        phs->need_thumbnail |= 0x1;
        int32_t timeout_cnt = 0;
        while(*thumbnail_len == 0){
            if(phs->shutdown == 1 || timeout_cnt++ > 20){
                phs->piv_finish = 1;
                pthread_mutex_unlock(&phs->piv_mutex);
                break;
            }
            cvi_osal_task_sleep(10 * 1000);
        }

        if(*thumbnail_len == 0){
            phs->piv_finish = 1;
            pthread_mutex_unlock(&phs->piv_mutex);
            continue;
        }

        if (0 != CVI_MAPI_VPROC_GetChnFrame(p->handles.src_vproc, p->handles.src_vproc_chn_id, &src_frame)) {
            CVI_LOGE("PHS[%d]: CVI_MAPI_VPROC_GetChnFrame failed", phs->id);
            phs->piv_finish = 1;
            *thumbnail_len = 0;
            pthread_mutex_unlock(&phs->piv_mutex);
            continue;
        }

        if (0 != CVI_MAPI_VPROC_SendFrame(p->handles.scale_vproc, &src_frame)) {
            CVI_LOGE("PHS[%d]: CVI_MAPI_VPROC_SendFrame failed", phs->id);
            CVI_MAPI_ReleaseFrame(&src_frame);
            phs->piv_finish = 1;
            *thumbnail_len = 0;
            pthread_mutex_unlock(&phs->piv_mutex);
            continue;
        }
        CVI_MAPI_ReleaseFrame(&src_frame);

        CVI_MAPI_VENC_StartRecvFrame(p->handles.photo_venc_hdl, -1);
        if (0 != CVI_MAPI_VPROC_GetChnFrame(p->handles.scale_vproc, p->handles.scale_vproc_chn_id, &scale_frame)) {
            CVI_LOGE("PHS[%d]: CVI_MAPI_VPROC_GetChnFrame 2 failed", phs->id);
            CVI_MAPI_VENC_StopRecvFrame(p->handles.photo_venc_hdl);
            phs->piv_finish = 1;
            *thumbnail_len = 0;
            pthread_mutex_unlock(&phs->piv_mutex);
            continue;
        }

        if (0 > CVI_MAPI_VENC_SendFrame(p->handles.photo_venc_hdl, &scale_frame)) {
            CVI_LOGE("PHS[%d]: snapshot venc send frame fail", phs->id);
            CVI_MAPI_ReleaseFrame(&scale_frame);
            CVI_MAPI_VENC_StopRecvFrame(p->handles.photo_venc_hdl);
            phs->piv_finish = 1;
            *thumbnail_len = 0;
            pthread_mutex_unlock(&phs->piv_mutex);
            continue;
        }
        CVI_MAPI_ReleaseFrame(&scale_frame);


        if (0 > phs_get_venc_stream(p->handles.photo_venc_hdl, &stream)) {
            CVI_LOGE("PHS[%d]: snapshot get venc stream fail", phs->id);
            CVI_MAPI_VENC_StopRecvFrame(p->handles.photo_venc_hdl);
            phs->piv_finish = 1;
            *thumbnail_len = 0;
            pthread_mutex_unlock(&phs->piv_mutex);
            continue;
        }

        uint32_t len = stream.pstPack[0].u32Len;

        if(len > g_snap_buf[phs->id].size){
            CVI_LOGE("PHS[%d]snapshot size too big %u %u", phs->id, len, g_snap_buf[phs->id].size);
            CVI_MAPI_VENC_ReleaseStream(p->handles.photo_venc_hdl, &stream);
            CVI_MAPI_VENC_StopRecvFrame(p->handles.photo_venc_hdl);
            phs->piv_finish = 1;
            *thumbnail_len = 0;
            pthread_mutex_unlock(&phs->piv_mutex);
            continue;
        }
        memcpy(g_snap_buf[phs->id].buf, stream.pstPack[0].pu8Addr, len);

        CVI_MAPI_VENC_ReleaseStream(p->handles.photo_venc_hdl, &stream);
        CVI_MAPI_VENC_StopRecvFrame(p->handles.photo_venc_hdl);

        if(phs_write_snap_file(phs->piv_filename, g_snap_buf[phs->id].buf, len, thumbnail_data, *thumbnail_len, phs->piv_prealloclen) < 0){
            phs->piv_finish = 1;
            *thumbnail_len = 0;
            pthread_mutex_unlock(&phs->piv_mutex);
            continue;
        }

        if (p->stCallback.pfnNormalPhsCb) {
            ((CVI_PHOTO_SERVICE_EVENT_CALLBACK)p->stCallback.pfnNormalPhsCb)(CVI_PHOTO_SERVICE_EVENT_PIV_END, phs->piv_filename, (void*)(&phs->id));
        }
        CVI_LOGD("PHS[%d] snap success", phs->id);
        CVI_FILESYNC_Push(phs->piv_filename, NULL);
        phs->piv_finish = 1;
        *thumbnail_len = 0;
        pthread_mutex_unlock(&phs->piv_mutex);
    }

    CVI_LOGD("PHS[%d] exit", phs->id);

}

static void cvi_phs_state_task_entry(void *arg){

    phs_context_handle_t phs = (phs_context_handle_t)arg;
    while (!phs->shutdown) {
        // handle transition
        pthread_mutex_lock(&phs->state_mutex);
        if (phs->new_state != phs->cur_state) {
            phs->cur_state = phs->new_state;
            CVI_LOGI("PHS[%d]: PHS_STATE: change to 0x%x done", phs->id, phs->cur_state);
        }
        pthread_mutex_unlock(&phs->state_mutex);
        cvi_osal_task_sleep(20 * 1000);
    }
    CVI_LOGD("PHS[%d] exit", phs->id);
}

static int32_t cvi_phs_start_task(phs_context_handle_t phs) {
    cvi_osal_task_attr_t piv_ta;
    static char piv_name[16] = {0};
    snprintf(piv_name, sizeof(piv_name), "phs_piv_%d", phs->id);
    piv_ta.name = piv_name;
    piv_ta.entry = phs_snap_task_entry;
    piv_ta.param = (void *)phs;
    piv_ta.priority = CVI_OSAL_PRI_RT_MID;
    piv_ta.detached = false;
    int32_t pc = cvi_osal_task_create(&piv_ta, &phs->piv_task);
    if (pc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("phs_snap task create failed, %d", pc);
        return PHS_ERR_FAILURE;
    }

    cvi_osal_task_attr_t thumb_ta;
    static char thumb_name[16] = {0};
    snprintf(thumb_name, sizeof(thumb_name), "phs_thumb_%d", phs->id);
    thumb_ta.name = thumb_name;
    thumb_ta.entry = phs_thumb_task_entry;
    thumb_ta.param = (void *)phs;
    thumb_ta.priority = CVI_OSAL_PRI_NORMAL;
    thumb_ta.detached = false;
    pc = cvi_osal_task_create(&thumb_ta, &phs->thumb_task);
    if (pc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("phs_thumb task create failed, %d", pc);
        return PHS_ERR_FAILURE;
    }

    static char state_name[16] = {0};
    snprintf(state_name, sizeof(state_name), "phs_state_%d", phs->id);
    cvi_osal_task_attr_t ta;
    ta.name = state_name;
    ta.entry = cvi_phs_state_task_entry;
    ta.param = (void *)phs;
    ta.priority = CVI_OSAL_PRI_RT_HIGH;
    ta.detached = false;
    pc = cvi_osal_task_create(&ta, &phs->state_task);
    if (pc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("phs_state task create failed, %d", pc);
        return PHS_ERR_FAILURE;
    }

    return PHS_SUCCESS;
}

static void *cvi_phs_create(int32_t id, CVI_PHOTO_SERVICE_ATTR_S *attr) {
    if(id >= MAX_CONTEXT_CNT){
        return 0;
    }
    memset(&gstPhsCtx[id], 0x0, sizeof(gstPhsCtx[id]));
    phs_context_handle_t handle = &gstPhsCtx[id];
    handle->attr = attr;
    handle->id = id;
    if(g_photo_thumbnail_buf[id].size == 0){
        g_photo_thumbnail_buf[id].buf = malloc(attr->handles.thumbnail_bufsize);
        if(g_photo_thumbnail_buf[id].buf == NULL){
            CVI_LOGE("g_photo_thumbnail_buf %d %u malloc failed, OOM!!!", id, attr->handles.thumbnail_bufsize);
            return NULL;
        }
        g_photo_thumbnail_buf[id].size = attr->handles.thumbnail_bufsize;
        CVI_LOGD("photo thumbnail size %u", g_photo_thumbnail_buf[id].size);
    }

    if(g_snap_thumbnail_buf[id].size == 0){
        g_snap_thumbnail_buf[id].buf = malloc(attr->handles.thumbnail_bufsize);
        if(g_snap_thumbnail_buf[id].buf == NULL){
            CVI_LOGE("snap thumbnail %d %u malloc failed, OOM!!!", id, attr->handles.thumbnail_bufsize);
            return NULL;
        }
        g_snap_thumbnail_buf[id].size = attr->handles.thumbnail_bufsize;
        CVI_LOGD("snap thumbnail size %u", g_snap_thumbnail_buf[id].size);
    }

    if(g_snap_buf[id].size == 0){
        g_snap_buf[id].buf = malloc(attr->handles.photo_bufsize);
        if(g_snap_buf[id].buf == NULL){
            CVI_LOGE("snap buf %d %u malloc failed, OOM!!!", id, attr->handles.photo_bufsize);
            return NULL;
        }
        g_snap_buf[id].size = attr->handles.photo_bufsize;
        CVI_LOGD("snap buf size %u", g_snap_buf[id].size);
    }

    pthread_mutex_init(&handle->state_mutex, NULL);
    pthread_mutex_init(&handle->piv_mutex, NULL);
    pthread_condattr_t piv_condattr;
    pthread_condattr_init(&piv_condattr);
    pthread_condattr_setclock(&piv_condattr, CLOCK_MONOTONIC);
    pthread_cond_init(&handle->piv_cond, &piv_condattr);
    pthread_condattr_destroy(&piv_condattr);

    pthread_mutex_init(&handle->thumbnail_mutex, NULL);
    handle->cur_state = PHS_STATE_IDLE;
    handle->piv_prealloclen = attr->s32SnapPresize;

    cvi_phs_start_task(handle);

    uint32_t enable_states = 0;
    enable_states |= PHS_STATE_PHOTO_CREATE_EN;

    phs_enable_state(handle, enable_states);
    return (void *)handle;
}

int32_t CVI_PHOTO_SERVICE_Create(CVI_PHOTO_SERVICE_HANDLE_T *hdl, CVI_PHOTO_SERVICE_PARAM_S *param) {
    if(param->photo_id >= MAX_CONTEXT_CNT){
        return -1;
    }
    CVI_PHOTO_SERVICE_ATTR_S *attr = &gst_cvi_phs_attr[param->photo_id];
    CVI_APPATTR_2_PHOTOATTR(param, attr);
    attr->phs = cvi_phs_create(param->photo_id, attr);
    *hdl = attr->phs;
    return (*hdl != NULL) ? 0 : -1;
}

static int32_t cvi_phs_destroy(int32_t id) {
    if(id >= MAX_CONTEXT_CNT){
        return 0;
    }

    phs_context_handle_t handle = &gstPhsCtx[id];

    CVI_LOGD("PHS[%d] destroy start", id);
    phs_change_state(handle, PHS_STATE_IDLE);
    handle->shutdown = 1;
    int32_t pc = cvi_osal_task_join(handle->state_task);
    if (pc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("phs_state task join failed, %d", pc);
        return PHS_ERR_FAILURE;
    }
    cvi_osal_task_destroy(&handle->state_task);
    pc = cvi_osal_task_join(handle->thumb_task);
    if (pc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("thumbnail task join failed, %d", pc);
        return PHS_ERR_FAILURE;
    }
    cvi_osal_task_destroy(&handle->thumb_task);

    pthread_mutex_lock(&handle->piv_mutex);
    pthread_cond_signal(&handle->piv_cond);
    pthread_mutex_unlock(&handle->piv_mutex);
    pc = cvi_osal_task_join(handle->piv_task);
    if (pc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("phs_piv task join failed, %d", pc);
        return PHS_ERR_FAILURE;
    }
    cvi_osal_task_destroy(&handle->piv_task);
    CVI_LOGD("PHS[%d] destroy start 1", id);
    pthread_mutex_destroy(&handle->state_mutex);
    pthread_mutex_destroy(&handle->piv_mutex);
    pthread_cond_destroy(&handle->piv_cond);
    pthread_mutex_destroy(&handle->thumbnail_mutex);

    if(g_photo_thumbnail_buf[id].buf){
        free(g_photo_thumbnail_buf[id].buf);
        g_photo_thumbnail_buf[id].buf = NULL;
        CVI_LOGD("## free g_photo_thumbnail_buf %d %u", id, g_photo_thumbnail_buf[id].size);
    }
    g_photo_thumbnail_buf[id].actsize = 0;
    g_photo_thumbnail_buf[id].size = 0;

    if(g_snap_thumbnail_buf[id].buf){
        free(g_snap_thumbnail_buf[id].buf);
        g_snap_thumbnail_buf[id].buf = NULL;
        CVI_LOGD("## free g_snap_thumbnail_buf %d %u", id, g_snap_thumbnail_buf[id].size);
    }
    g_snap_thumbnail_buf[id].actsize = 0;
    g_snap_thumbnail_buf[id].size = 0;

    if(g_snap_buf[id].buf){
        free(g_snap_buf[id].buf);
        g_snap_buf[id].buf = NULL;
        CVI_LOGD("## free g_snap_buf %d %u", id, g_snap_buf[id].size);
    }
    g_snap_buf[id].actsize = 0;
    g_snap_buf[id].size = 0;

    CVI_LOGE("PHS[%d]end", id);

    return pc;
}

int32_t CVI_PHOTO_SERVICE_Destroy(CVI_PHOTO_SERVICE_HANDLE_T hdl) {
    return cvi_phs_destroy(cvi_phs_get_id(hdl));
}

static int32_t cvi_phs_snap(int32_t id, char *file_name) {
    int32_t ret = 0;
    if(id >= MAX_CONTEXT_CNT){
        return 0;
    }
    phs_context_handle_t handle = &gstPhsCtx[id];

    pthread_mutex_lock(&handle->piv_mutex);
    memset(handle->piv_filename, 0, sizeof(handle->piv_filename));
    snprintf(handle->piv_filename, sizeof(handle->piv_filename), "%s", file_name);

    if (0 == strlen(handle->piv_filename)) {
        CVI_LOGE("get snap filename failed! \n");
        ret = -1;
    }
    handle->piv_finish = 0;
    pthread_mutex_unlock(&handle->piv_mutex);
    pthread_cond_signal(&handle->piv_cond);
    CVI_LOGD("%d snap_finish", id);
    return ret;
}

int32_t CVI_PHOTO_SERVICE_PivCapture(CVI_PHOTO_SERVICE_HANDLE_T hdl, char *file_name)
{
    if (!hdl) {
        return -1;
    }
    return cvi_phs_snap(cvi_phs_get_id(hdl), file_name);
}
static void cvi_phs_waitsnap_finish(int32_t id)
{
    if(id >= MAX_CONTEXT_CNT){
        CVI_LOGE("PHOTO ID of fix photomode is error\n");
        return;
    }
    phs_context_handle_t handle = &gstPhsCtx[id];
    CVI_LOGD("%d waitsnap_start", id);
    while(!handle->piv_finish){
        cvi_osal_task_sleep(20 * 1000);
    }
    CVI_LOGD("%d waitsnap_finish", id);
}

void CVI_PHOTO_SERVICE_WaitPivFinish(CVI_PHOTO_SERVICE_HANDLE_T hdl)
{
    if (!hdl) {
        CVI_LOGE("photo service handle is null !\n");
        return;
    }
    cvi_phs_waitsnap_finish(cvi_phs_get_id(hdl));
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */