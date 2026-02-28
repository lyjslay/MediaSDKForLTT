#include "cvi_mapi.h"
#include "cvi_log.h"
#include "cvi_player_service.h"
#include "cvi_comm_vdec.h"
#include "cvi_vdec.h"
#ifdef CONFIG_SCREEN_ON
#include "cvi_hal_screen.h"
#endif
#include "ps_context.h"
#include "ps_event.h"
#include "ps_param.h"

#define MAX_FILE_PATH_LENGTH 256
#define CVI_ALIGN_UP(x, a) ((x + a - 1) & (~(a - 1)))
#define ALIGN_DOWN(x, a) ((x) & ~((a)-1))

static const int32_t FRAME_PLANAR_NUM = 2;
bool videostreamflage = true;
bool find_first_sps = false;
bool VideoCustom = true;
static bool audio_enable = true;
CVI_MAPI_VPROC_HANDLE_T * g_vpsshal = NULL;
bool AdditionalVb = false;
CVI_PLAYER_MEDIA_INFO_S fileinfo = {};
static int32_t deinit_vproc(CVI_PLAYER_SERVICE_HANDLE_T handle);
static int32_t init_vproc(CVI_PLAYER_SERVICE_HANDLE_T handle,
        uint32_t video_width, uint32_t video_height);

static CVI_MAPI_VCODEC_E get_payload_type_from_name(const char *codec_name)
{
    CVI_MAPI_VCODEC_E type = CVI_MAPI_VCODEC_H264;
    if (strcmp(codec_name, "hevc") == 0) {
        type = CVI_MAPI_VCODEC_H265;
    } else if (strcmp(codec_name, "mjpeg") == 0) {
        type = CVI_MAPI_VCODEC_JPG;
    } else if(strcmp(codec_name, "h264") == 0) {
        type = CVI_MAPI_VCODEC_H264;
    }

    return type;
}

static inline bool is_h26x(CVI_MAPI_VCODEC_E codec) {
    return (codec == CVI_MAPI_VCODEC_H264) || (codec == CVI_MAPI_VCODEC_H265);
}

static inline bool is_jpeg(CVI_MAPI_VCODEC_E codec) {
    return (codec == CVI_MAPI_VCODEC_JPG);
}

static int32_t vpss_attr_reset(void *arg, VIDEO_FRAME_INFO_S *video_frame)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)arg;
    int32_t s32Ret = CVI_SUCCESS;
    bool is_changed = false;
    VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
    if ((s32Ret=CVI_MAPI_VPROC_GetGrpAttr(ps->vproc, &stVpssGrpAttr)) != CVI_SUCCESS) {
        CVI_LOGE("CVI_MAPI_VPROC_GetGrpAttr failed, s32Ret:%x", s32Ret);
        return s32Ret;
    }

    CVI_LOGD("stVpssGrpAttr.enPixelFormat:%d, video_frame.stVFrame.enPixelFormat:%d", stVpssGrpAttr.enPixelFormat, video_frame->stVFrame.enPixelFormat);
    if (stVpssGrpAttr.enPixelFormat != video_frame->stVFrame.enPixelFormat) {
        stVpssGrpAttr.enPixelFormat = video_frame->stVFrame.enPixelFormat;
        is_changed = true;
    }
    if (stVpssGrpAttr.u32MaxW != video_frame->stVFrame.u32Width) {
        stVpssGrpAttr.u32MaxW = video_frame->stVFrame.u32Width;
        is_changed = true;
    }
    if (stVpssGrpAttr.u32MaxH != video_frame->stVFrame.u32Height) {
        stVpssGrpAttr.u32MaxH = video_frame->stVFrame.u32Height;
        is_changed = true;
    }

    if (is_changed) {
        if ((s32Ret=CVI_MAPI_VPROC_SetGrpAttr(ps->vproc, &stVpssGrpAttr)) != CVI_SUCCESS) {
            CVI_LOGE("CVI_MAPI_VPROC_SetGrpAttr failed, s32Ret:%x", s32Ret);
            return s32Ret;
        }
    }

    return CVI_SUCCESS;
}

/*rescale to 64 bit alignment, use Nearest Neighbor Interpolation*/
static void frame_align64(VIDEO_FRAME_INFO_S *rescale_frame, CVI_PLAYER_FRAME_S *video_frame)
{
    if (rescale_frame == NULL) {
        CVI_LOGE("rescale_frame is null");
        return;
    }

    int32_t dst_width = rescale_frame->stVFrame.u32Width;
    int32_t dst_height = rescale_frame->stVFrame.u32Height;
    register int32_t scale_x = (video_frame->width<<8) / dst_width;
    register int32_t scale_y = (video_frame->height<<8) / dst_height;

    int32_t* src_x = (int32_t*)malloc(sizeof(int32_t) * dst_width);
    if (src_x == NULL) {
        CVI_LOGE("malloc fail");
        return;
    }

    for (register int32_t x = 0; x < dst_width; x += 2) {
        int32_t x1 = x + 1;
        src_x[x] = (x * scale_x)>>8;
        src_x[x1] = (x1 * scale_x)>>8;
    }

    for (register int32_t y = 0; y < dst_height; y +=2) {
        int32_t y0 = y;
        int32_t y1 = y + 1;
        int32_t src_y0 = (y0 * scale_y)>>8;
        int32_t src_y1 = (y1* scale_y)>>8;
        uint8_t* src_p_y0 = video_frame->data[0] + src_y0 * video_frame->linesize[0];
        uint8_t* src_p_y1 = video_frame->data[0] + src_y1 * video_frame->linesize[0];
        uint8_t* src_p_u1 = video_frame->data[1] + (src_y1 >> 1) * video_frame->linesize[1];
        uint8_t* src_p_v1 = video_frame->data[2] + (src_y1 >> 1) * video_frame->linesize[2];

        uint8_t* dst_p_y0 = rescale_frame->stVFrame.pu8VirAddr[0] + y0 * rescale_frame->stVFrame.u32Stride[0];
        uint8_t* dst_p_y1 = rescale_frame->stVFrame.pu8VirAddr[0] + y1 * rescale_frame->stVFrame.u32Stride[0];
        uint8_t* dst_p_u1 = rescale_frame->stVFrame.pu8VirAddr[1] + (y1 >> 1) * rescale_frame->stVFrame.u32Stride[1];
        uint8_t* dst_p_v1 = rescale_frame->stVFrame.pu8VirAddr[2] + (y1 >> 1) * rescale_frame->stVFrame.u32Stride[2];

        for (register int32_t x = 0; x < dst_width ; x += 4) {
            int32_t x0 = x;
            int32_t x1 = x + 1;
            int32_t x2 = x + 2;
            int32_t x3 = x + 3;
            int32_t src_x0 = src_x[x0];
            int32_t src_x1 = src_x[x1];

            uint8_t* dst_p_y_00 = x0 + dst_p_y0;
            uint8_t* dst_p_y_10 = x1 + dst_p_y0;
            *dst_p_y_00 = *(src_x0 + src_p_y0);
            *dst_p_y_10 = *(src_x1 + src_p_y0);
            uint8_t* dst_p_y_01 = x0 + dst_p_y1;
            uint8_t* dst_p_y_11 = x1 + dst_p_y1;
            *dst_p_y_01 = *(src_x0 + src_p_y1);
            *dst_p_y_11 = *(src_x1 + src_p_y1);

            int32_t src_x2 = src_x[x2];
            int32_t src_x3 = src_x[x3];
            uint8_t* dst_p_y_20 = x2 + dst_p_y0;
            uint8_t* dst_p_y_30 = x3 + dst_p_y0;
            *dst_p_y_20 = *(src_x2 + src_p_y0);
            *dst_p_y_30 = *(src_x3 + src_p_y0);
            uint8_t* dst_p_y_21 = x2 + dst_p_y1;
            uint8_t* dst_p_y_31 = x3 + dst_p_y1;
            *dst_p_y_21 = *(src_x2 + src_p_y1);
            *dst_p_y_31 = *(src_x3 + src_p_y1);

            uint8_t* dst_p_u_11 = (x1 >> 1) + dst_p_u1;
            uint8_t* dst_p_v_11 = (x1 >> 1) + dst_p_v1;
            *dst_p_u_11 = *((src_x1 >> 1) + src_p_u1);
            *dst_p_v_11 = *((src_x1 >> 1) +src_p_v1);
            uint8_t* dst_p_u_31 = (x3 >> 1) + dst_p_u1;
            uint8_t* dst_p_v_31 = (x3 >> 1) + dst_p_v1;
            *dst_p_u_31 = *((src_x3 >> 1) + src_p_u1);
            *dst_p_v_31 = *((src_x3 >> 1) +src_p_v1);
        }
    }
    free(src_x);
}

void vo_cb(void *arg, CVI_PLAYER_FRAME_S *video_frame)
{
    if (video_frame == NULL) {
        CVI_LOGW("Frame is null");
        return;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)arg;
    VIDEO_FRAME_INFO_S vo_frame = {};
    // harddecoding
    if (VideoCustom) {
        if (CVI_MAPI_AllocateFrame(&vo_frame, video_frame->width,
                video_frame->height, PIXEL_FORMAT_NV21) != 0) {
            CVI_LOGE("MAPI allocate frame failed");
            return;
        }

        CVI_MAPI_FrameMmap(&vo_frame, true);
        for (int32_t i = 0; i < FRAME_PLANAR_NUM; ++i) {
            memcpy((void *)vo_frame.stVFrame.pu8VirAddr[i],
                (const void *)video_frame->data[i], vo_frame.stVFrame.u32Length[i]);
            vo_frame.stVFrame.u32Stride[i] = video_frame->linesize[i];
        }
        CVI_MAPI_FrameFlushCache(&vo_frame);
        CVI_MAPI_FrameMunmap(&vo_frame);
    } else {
        /*softdecoding: width need 64 bit alignment, but height dont need */
        int32_t dst_width = CVI_ALIGN_UP(video_frame->width, 64);
        if (((dst_width / 2) % 64) > 0) {
            dst_width += 64;
        }
        int32_t dst_height = video_frame->height;
        VIDEO_FRAME_INFO_S rescale_frame = {};
        if (CVI_MAPI_AllocateFrame(&rescale_frame, dst_width, dst_height, PIXEL_FORMAT_YUV_PLANAR_420) != 0) {
                CVI_LOGE("MAPI allocate frame failed, try create vb pool");
                CVI_MAPI_CreateVbPool(PIXEL_FORMAT_YUV_PLANAR_420, dst_width, dst_height);
                AdditionalVb = true;
                return;
        }
        CVI_MAPI_FrameMmap(&rescale_frame, true);

        if (((video_frame->width & 63) != 0) || (((video_frame->width / 2 ) & 63) != 0)) {
            rescale_frame.stVFrame.u32Stride[0] = dst_width;
            rescale_frame.stVFrame.u32Stride[1] = dst_width>>1;
            rescale_frame.stVFrame.u32Stride[2] = dst_width>>1;
            frame_align64(&rescale_frame, video_frame);
        } else {
            for (int32_t i = 0; i < 3; ++i) {
                memcpy((void *)rescale_frame.stVFrame.pu8VirAddr[i],
                    (const void *)video_frame->data[i], rescale_frame.stVFrame.u32Length[i]);
                rescale_frame.stVFrame.u32Stride[i] = video_frame->linesize[i];
            }
        }
        CVI_MAPI_FrameFlushCache(&rescale_frame);
        CVI_MAPI_FrameMunmap(&rescale_frame);

        if (vpss_attr_reset(arg, &rescale_frame) != 0) {
            CVI_LOGE("vpss_attr_reset failed");
            CVI_MAPI_ReleaseFrame(&rescale_frame);
            return;
        }

        if (CVI_MAPI_VPROC_SendFrame(ps->vproc, &rescale_frame) != 0) {
                CVI_LOGE("MAPI vproc send frame failed");
                CVI_MAPI_ReleaseFrame(&rescale_frame);
                return;
        }
        CVI_MAPI_ReleaseFrame(&rescale_frame);

        if (CVI_MAPI_VPROC_GetChnFrame(ps->vproc, 0, &vo_frame) != 0) {
            CVI_LOGE("MAPI vproc get frame failed");
            return;
        }
    }
    // end softdecoding

    //CVI_MAPI_SaveFramePixelData(&vo_frame, "/mnt/sd/revpss.yuv");

    if (CVI_MAPI_DISP_SendFrame(ps->disp, &vo_frame) != 0){
        CVI_LOGE("MAPI disp send frame failed");
        CVI_MAPI_ReleaseFrame(&vo_frame);
        return;
    }
    // first send vo have no picture
    if (ps->send_vo_again) {
        if (CVI_MAPI_DISP_SendFrame(ps->disp, &vo_frame) != 0){
            CVI_LOGE("MAPI disp send frame failed");
            CVI_MAPI_ReleaseFrame(&vo_frame);
            return;
        }
        ps->send_vo_again = false;
    }
    CVI_MAPI_ReleaseFrame(&vo_frame);
}

static void ao_cb(void *arg, CVI_PLAYER_FRAME_S *frame)
{
    int32_t backwardstatus = true;
    if ((frame == NULL) || (frame->packet_size <= 0)) {
        return;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)arg;
    static AUDIO_FRAME_S audio_frame = {};
    audio_frame.u64VirAddr[0] = frame->data[0];
    audio_frame.u32Len = (frame->packet_size / 2) / ps->param.AudioChannel;

    audio_frame.enBitwidth = AUDIO_BIT_WIDTH_16;
    if (ps->param.AudioChannel == 2) {
        audio_frame.enSoundmode = 1;
    }
    backwardstatus = CVI_PLAYER_GetForWardBackWardStatus(ps);
    if (backwardstatus == true && audio_enable == true) {
        if (CVI_MAPI_AO_SendFrame(ps->ao, ps->ao_channel, (const AUDIO_FRAME_S *)&audio_frame, 1000) != 0) {
            CVI_LOGE("MAPI ao send frame failed");
            return;
        }
    }
}

static void player_event_cb(void *arg, CVI_PLAYER_EVENT_S *event)
{
    if (event == NULL) {
        CVI_LOGE("event is null");
        return;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)arg;
    PS_PARAM_HANDLE param = &ps->param;
    CVI_PLAYER_SERVICE_EVENT_S ps_event = {
        .type = CVI_PLAYER_SERVICE_EVENT_UNKNOWN
    };

    switch(event->type) {
    case CVI_PLAYER_EVENT_PLAY:
        ps_event.type = CVI_PLAYER_SERVICE_EVENT_PLAY;
        break;
    case CVI_PLAYER_EVENT_PLAY_FINISHED:
        if ((!param->repeat) || (is_jpeg(ps->codec_type))) {
            CVI_LOGI("Player play finish");
            CVI_SIGNAL_Emit(ps->signals.finish);
        } else {
            CVI_LOGI("Play repeat");
            if (CVI_PLAYER_SERVICE_Seek(ps, 0) != 0) {
                CVI_LOGE("Player service repeat failed");
                return;
            }
        }
        ps_event.type = CVI_PLAYER_SERVICE_EVENT_PLAY_FINISHED;
        break;
    case CVI_PLAYER_EVENT_PAUSE:
        CVI_LOGI("Player pause");
        CVI_SIGNAL_Emit(ps->signals.pause);
        ps_event.type =  CVI_PLAYER_SERVICE_EVENT_PAUSE;
        break;
    case CVI_PLAYER_EVENT_RESUME:
        CVI_LOGI("Player resume");
        CVI_SIGNAL_Emit(ps->signals.resume);
        ps_event.type =  CVI_PLAYER_SERVICE_EVENT_RESUME;
        break;
    case CVI_PLAYER_EVENT_PLAY_PROGRESS:
        ps_event.type =  CVI_PLAYER_SERVICE_EVENT_PLAY_PROGRESS;
        break;
    default:
        break;
    }

    if (ps->event_handler != NULL) {
        ps->event_handler(ps, &ps_event);
    }
}

void file_recover_event_cb(void *arg, CVI_FILE_RECOVER_EVENT_S *event)
{
    if (event == NULL) {
        CVI_LOGE("event is null");
        return;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)arg;
    CVI_PLAYER_SERVICE_EVENT_S ps_event = {
        .type = CVI_PLAYER_SERVICE_EVENT_UNKNOWN,
        .value = event->value
    };

    switch(event->type) {
    case CVI_FILE_RECOVER_EVENT_OPEN_FAILED:
        CVI_LOGI("File open failed");
        ps_event.type = CVI_PLAYER_SERVICE_EVENT_OPEN_FAILED;
        break;
    case CVI_FILE_RECOVER_EVENT_RECOVER_START:
        ps_event.type = CVI_PLAYER_SERVICE_EVENT_RECOVER_START;
        break;
    case CVI_FILE_RECOVER_EVENT_RECOVER_PROGRESS:
        ps_event.type = CVI_PLAYER_SERVICE_EVENT_RECOVER_PROGRESS;
        break;
    case CVI_FILE_RECOVER_EVENT_RECOVER_FAILED:
        CVI_LOGI("File recover failed");
        ps_event.type = CVI_PLAYER_SERVICE_EVENT_RECOVER_FAILED;
        break;
    case CVI_FILE_RECOVER_EVENT_RECOVER_FINISHED:
        ps_event.type = CVI_PLAYER_SERVICE_EVENT_RECOVER_FINISHED;
        break;
    default:
        break;
    }

    if (ps->event_handler != NULL) {
        ps->event_handler(ps, &ps_event);
    }
}

static inline int32_t send_packet_to_decoder(CVI_MAPI_VDEC_HANDLE_T vdec, CVI_PLAYER_PACKET_S *packet)
{
    static VDEC_STREAM_S stStream = {};
    stStream.u64PTS = packet->pts;
    stStream.pu8Addr = packet->data;
    stStream.u32Len = packet->size;
    stStream.bEndOfFrame = CVI_TRUE;
    stStream.bEndOfStream = CVI_FALSE;
    stStream.bDisplay = 1;

    if (CVI_MAPI_VDEC_SendStream(vdec, &stStream) != 0) {
        CVI_LOGE("MAPI vdec send stream failed");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

static int32_t decode_packet_cb(void *arg, CVI_PLAYER_PACKET_S *packet)
{
    int32_t S32Ret = 0;
    if ((packet == NULL) || (packet->data == NULL)) {
        CVI_LOGW("Packet is null");
        return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)arg;

    if (is_h26x(ps->codec_type)) {
        // find first sps
        if (!find_first_sps) {
            if (CVI_PLAYER_PacketContainSps(ps->player, packet)) {
                find_first_sps = true;
            } else {
                CVI_PLAYER_PACKET_S extra_packet;
                S32Ret = CVI_PLAYER_GetMediumVideoExtraPacket(ps->player, &extra_packet);
                if (S32Ret != 0) {
                    CVI_LOGE("player get video extra packet failed");
                    return CVI_FAILURE;
                }

                if (CVI_PLAYER_PacketContainSps(ps->player, &extra_packet)) {
                    find_first_sps = true;

                    send_packet_to_decoder(ps->vdec, &extra_packet);
                }
            }
        }
        // video decoder need send sps at the beginning for h26x
        if (find_first_sps) {
            if (send_packet_to_decoder(ps->vdec, packet) != 0) {
                CVI_LOGE("Decode packet failed");
                return CVI_FAILURE;
            }
        }
    } else {
        if (send_packet_to_decoder(ps->vdec, packet) != 0) {
            CVI_LOGE("Decode packet failed");
            return CVI_FAILURE;
        }
    }

    return CVI_SUCCESS;
}

static int32_t clone_frame(VIDEO_FRAME_INFO_S *target_frame, VIDEO_FRAME_INFO_S *source_frame)
{
    if (CVI_MAPI_AllocateFrame(target_frame, source_frame->stVFrame.u32Width,
            source_frame->stVFrame.u32Height, PIXEL_FORMAT_NV21) != 0) {
        CVI_LOGE("MAPI allocate frame failed");
        return CVI_FAILURE;
    }
    if (CVI_MAPI_FrameMmap(target_frame, true) != 0) {
        CVI_LOGE("MAPI frame mmap failed");
        CVI_MAPI_ReleaseFrame(target_frame);
        return CVI_FAILURE;
    }
    // copy frame data to target frame
    for (int32_t i = 0; i < FRAME_PLANAR_NUM; ++i) {
        memcpy(target_frame->stVFrame.pu8VirAddr[i],
            source_frame->stVFrame.pu8VirAddr[i],
            target_frame->stVFrame.u32Length[i]);
    }
    if (CVI_MAPI_FrameFlushCache(target_frame) != 0) {
        CVI_LOGE("MAPI flush cache failed");
        CVI_MAPI_ReleaseFrame(target_frame);
        return CVI_FAILURE;
    }
    if (CVI_MAPI_FrameMunmap(target_frame) != 0) {
        CVI_LOGE("MAPI frame munmap failed");
        CVI_MAPI_ReleaseFrame(target_frame);
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

static int32_t get_frame_cb(void *arg, CVI_PLAYER_FRAME_S *video_frame)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)arg;

    VIDEO_FRAME_INFO_S vdec_frame = {};
    if (CVI_MAPI_VDEC_GetFrame(ps->vdec, &vdec_frame) != CVI_MAPI_SUCCESS) {
        // this is normal, meaning waiting for new stream
        return CVI_FAILURE;
    }

    if ((fileinfo.width != (int32_t)vdec_frame.stVFrame.u32Width) || (fileinfo.height != (int32_t)vdec_frame.stVFrame.u32Height)) {
        deinit_vproc(ps);
        if (init_vproc(ps, vdec_frame.stVFrame.u32Width, vdec_frame.stVFrame.u32Height) != 0) {
            CVI_LOGE("recreate width:%d, height:%d , fail!", vdec_frame.stVFrame.u32Width, vdec_frame.stVFrame.u32Height);
            return CVI_FAILURE;
        }
        fileinfo.width = vdec_frame.stVFrame.u32Width;
        fileinfo.height = vdec_frame.stVFrame.u32Height;
    }

    // if (is_jpeg(ps->codec_type)) {
    if (0) {
        VIDEO_FRAME_INFO_S vdec_buffer_frame = {};
        // jpeg decoded frame can't send to vproc directly
        if (clone_frame(&vdec_buffer_frame, &vdec_frame) != CVI_SUCCESS) {
            CVI_LOGE("Clone frame failed");
            CVI_MAPI_VDEC_ReleaseFrame(ps->vdec, &vdec_frame);
            return CVI_FAILURE;
        }
        CVI_MAPI_VDEC_ReleaseFrame(ps->vdec, &vdec_frame);

#ifdef SERVICES_CARPLAY_ON
        if (vpss_attr_reset(arg, &vdec_buffer_frame) != CVI_SUCCESS) {
            CVI_LOGE("vpss_attr_reset failed");
            CVI_MAPI_ReleaseFrame(&vdec_buffer_frame);
            return CVI_FAILURE;
        }
#endif
        // send decoded frame to vproc for resize
        if (CVI_MAPI_VPROC_SendFrame(ps->vproc, &vdec_buffer_frame) != 0) {
            CVI_LOGE("MAPI vproc send frame failed");
            CVI_MAPI_ReleaseFrame(&vdec_buffer_frame);
            return CVI_FAILURE;
        }
        CVI_MAPI_ReleaseFrame(&vdec_buffer_frame);
    } else {
#ifdef SERVICES_CARPLAY_ON
        if (vpss_attr_reset(arg, &vdec_frame) != CVI_SUCCESS) {
            CVI_LOGE("vpss_attr_reset failed");
            CVI_MAPI_VDEC_ReleaseFrame(ps->vdec, &vdec_frame);
            return CVI_FAILURE;
        }
#endif
        // send decoded frame to vproc for resize
        if (CVI_MAPI_VPROC_SendFrame(ps->vproc, &vdec_frame) != 0) {
            CVI_LOGE("MAPI vproc send frame failed");
            CVI_MAPI_VDEC_ReleaseFrame(ps->vdec, &vdec_frame);
            return CVI_FAILURE;
        }
        CVI_MAPI_VDEC_ReleaseFrame(ps->vdec, &vdec_frame);
    }

    VIDEO_FRAME_INFO_S resize_frame = {};
    if (CVI_MAPI_VPROC_GetChnFrame(ps->vproc, 0, &resize_frame) != 0) {
        CVI_LOGE("MAPI vproc get frame failed");
        return CVI_FAILURE;
    }
    // copy resized frame to result video frame
    CVI_MAPI_FrameMmap(&resize_frame, true);
    for (int32_t i = 0; i < FRAME_PLANAR_NUM; ++i) {
        memcpy((void *)video_frame->data[i], (const void *)resize_frame.stVFrame.pu8VirAddr[i],
                resize_frame.stVFrame.u32Length[i]);
        video_frame->linesize[i] = resize_frame.stVFrame.u32Stride[i];
    }
    CVI_MAPI_FrameFlushCache(&resize_frame);
    CVI_MAPI_FrameMunmap(&resize_frame);

    video_frame->width = resize_frame.stVFrame.u32Width;
    video_frame->height = resize_frame.stVFrame.u32Height;

    CVI_MAPI_ReleaseFrame(&resize_frame);

    return CVI_SUCCESS;
}


static int32_t init_signals(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    CVI_PLAYER_SERVICE_SIGNALS_S *signals = &ps->signals;

    CVI_SIGNAL_InitByType(&signals->play, CVI_SIGNAL_SLOT_TYPE_VOID);
    CVI_SIGNAL_InitByType(&signals->finish, CVI_SIGNAL_SLOT_TYPE_NONE);
    CVI_SIGNAL_InitByType(&signals->pause, CVI_SIGNAL_SLOT_TYPE_NONE);
    CVI_SIGNAL_InitByType(&signals->resume, CVI_SIGNAL_SLOT_TYPE_NONE);

    return CVI_SUCCESS;
}

static int32_t deinit_signals(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    CVI_PLAYER_SERVICE_SIGNALS_S *signals = &ps->signals;

    CVI_SIGNAL_Deinit(&signals->play);
    CVI_SIGNAL_Deinit(&signals->finish);
    CVI_SIGNAL_Deinit(&signals->pause);
    CVI_SIGNAL_Deinit(&signals->resume);

    return CVI_SUCCESS;
}

static int32_t init_slots(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    CVI_PLAYER_SERVICE_SLOTS_S *slots = &ps->slots;

    CVI_SLOT_Init(&slots->set_input, ps, (CVI_INT_SLOT_STRING_HANLDER)CVI_PLAYER_SERVICE_SetInput);
    CVI_SLOT_Init(&slots->play, ps, CVI_PLAYER_SERVICE_Play);
    CVI_SLOT_Init(&slots->play, ps, CVI_PLAYER_SERVICE_PlayerSeep);
    CVI_SLOT_Init(&slots->play, ps, CVI_PLAYER_SERVICE_PlayerSeepBack);
    CVI_SLOT_Init(&slots->play, ps, CVI_PLAYER_SERVICE_PlayerAndSeek);
    CVI_SLOT_Init(&slots->stop, ps, CVI_PLAYER_SERVICE_Stop);
    CVI_SLOT_Init(&slots->pause, ps, CVI_PLAYER_SERVICE_Pause);
    CVI_SLOT_Init(&slots->seek, ps, CVI_PLAYER_SERVICE_Seek);
    CVI_SLOT_Init(&slots->play, ps, CVI_PLAYER_SERVICE_SeekPause);
    CVI_SLOT_Init(&slots->play, ps, CVI_PLAYER_SERVICE_TouchSeekPause);
    CVI_SLOT_Init(&slots->resize, ps, CVI_PLAYER_SERVICE_Resize);
    CVI_SLOT_Init(&slots->toggle_fullscreen, ps, CVI_PLAYER_SERVICE_ToggleFullscreen);
    CVI_SLOT_Init(&slots->get_play_info, ps, (CVI_INT_SLOT_VOID_HANLDER)CVI_PLAYER_SERVICE_GetPlayInfo);

    return CVI_SUCCESS;
}

static int32_t deinit_slots(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    CVI_PLAYER_SERVICE_SLOTS_S *slots = &ps->slots;

    CVI_SLOT_Deinit(&slots->set_input);
    CVI_SLOT_Deinit(&slots->play);
    CVI_SLOT_Deinit(&slots->stop);
    CVI_SLOT_Deinit(&slots->seek);
    CVI_SLOT_Deinit(&slots->pause);
    CVI_SLOT_Deinit(&slots->resize);
    CVI_SLOT_Deinit(&slots->toggle_fullscreen);
    CVI_SLOT_Deinit(&slots->get_play_info);

    return CVI_SUCCESS;
}

static int32_t init_vdec(CVI_PLAYER_SERVICE_HANDLE_T handle,
        uint32_t video_width, uint32_t video_height)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    CVI_MAPI_VDEC_CHN_ATTR_T vdec_attr = {};
    vdec_attr.codec = ps->codec_type;
    vdec_attr.max_width = video_width;
    vdec_attr.max_height = video_height;
    if (CVI_MAPI_VDEC_InitChn(&ps->vdec, &vdec_attr) != 0) {
        CVI_LOGE("MAPI vdec init failed");
        return CVI_FAILURE;
    }

    VDEC_CHN_ATTR_S attr = {};
    if (CVI_VDEC_GetChnAttr(CVI_MAPI_VDEC_GetChn(ps->vdec), &attr) == 0) {
        ps->vdec_max_buffer_size = attr.u32StreamBufSize;
    }

    return CVI_SUCCESS;
}

static int32_t deinit_vdec(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    CVI_MAPI_VDEC_DeinitChn(ps->vdec);

    return CVI_SUCCESS;
}

static uint32_t get_adjust_side(uint32_t src_value, uint32_t max_value)
{
    if (max_value == 0) {
        return src_value;
    }

    uint32_t dest_value = src_value;
    if ((src_value == 0) || (src_value > max_value)) {
        dest_value = max_value;
    }

    return dest_value;
}

static uint32_t get_adjust_pos(uint32_t src_value, uint32_t side_length, uint32_t max_value)
{
    if (max_value == 0) {
        return src_value;
    }

    uint32_t dest_value = src_value;
    if ((src_value + side_length) > max_value) {
        dest_value = max_value - side_length;
    }

    return dest_value;
}

static void init_disp_params(void *handle)
{
#ifdef CONFIG_SCREEN_ON
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    PS_PARAM_HANDLE param = &ps->param;

    CVI_HAL_SCREEN_ATTR_S screen_attr = {};
    CVI_HAL_SCREEN_GetAttr(CVI_HAL_SCREEN_IDX_0, &screen_attr);
    if ((param->disp_rotate == ROTATION_90) ||
        (param->disp_rotate == ROTATION_270)) {
        ps->screen_width = screen_attr.stAttr.u32Height;
        ps->screen_height = screen_attr.stAttr.u32Width;
    } else {
        ps->screen_width = screen_attr.stAttr.u32Width;
        ps->screen_height = screen_attr.stAttr.u32Height;
    }
    // adjust size and pos
    param->width = get_adjust_side(param->width, ps->screen_width);
    param->height = get_adjust_side(param->height, ps->screen_height);
    param->x = get_adjust_pos(param->x, param->width, ps->screen_width);
    param->y = get_adjust_pos(param->y, param->height, ps->screen_height);
#else
    (void)handle;
#endif
}

static int32_t init_display(void *handle)
{
#ifdef CONFIG_SCREEN_ON
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    PS_PARAM_HANDLE param = &ps->param;

    CVI_MAPI_DISP_ATTR_T disp_attr = {};
    CVI_MAPI_DISP_VIDEOLAYER_ATTR_S layer_attr = {};
    disp_attr.rotate = param->disp_rotate;
    disp_attr.window_mode = false;
    disp_attr.stPubAttr.u32BgColor = COLOR_10_RGB_BLUE;
    disp_attr.stPubAttr.enIntfSync = VO_OUTPUT_USER;
    layer_attr.u32BufLen = 3;
    layer_attr.u32PixelFmt = param->disp_fmt;
    extern CVI_HAL_SCREEN_OBJ_S stHALSCREENObj;
    CVI_HAL_SCREEN_Register(CVI_HAL_SCREEN_IDX_0, &stHALSCREENObj);
    CVI_HAL_SCREEN_Init(CVI_HAL_SCREEN_IDX_0);

    CVI_HAL_SCREEN_ATTR_S screen_attr = {};
    CVI_HAL_SCREEN_GetAttr(CVI_HAL_SCREEN_IDX_0, &screen_attr);
    switch(screen_attr.enType) {
        case CVI_HAL_SCREEN_INTF_TYPE_MIPI:
            disp_attr.stPubAttr.enIntfType = VO_INTF_MIPI;
            break;
        case CVI_HAL_SCREEN_INTF_TYPE_LCD:
            break;
        default:
            CVI_LOGE("Invalid screen type");
    }

    init_disp_params(ps);
    disp_attr.width = ps->screen_width;
    disp_attr.height = ps->screen_height;
    disp_attr.stPubAttr.stSyncInfo.bSynm = 1;
    disp_attr.stPubAttr.stSyncInfo.bIop = 1;
    disp_attr.stPubAttr.stSyncInfo.u16FrameRate = screen_attr.stAttr.u32Framerate;
    disp_attr.stPubAttr.stSyncInfo.u16Vact = screen_attr.stAttr.stSynAttr.u16Vact;
    disp_attr.stPubAttr.stSyncInfo.u16Vbb = screen_attr.stAttr.stSynAttr.u16Vbb;
    disp_attr.stPubAttr.stSyncInfo.u16Vfb = screen_attr.stAttr.stSynAttr.u16Vfb;
    disp_attr.stPubAttr.stSyncInfo.u16Hact = screen_attr.stAttr.stSynAttr.u16Hact;
    disp_attr.stPubAttr.stSyncInfo.u16Hbb = screen_attr.stAttr.stSynAttr.u16Hbb;
    disp_attr.stPubAttr.stSyncInfo.u16Hfb = screen_attr.stAttr.stSynAttr.u16Hfb;
    disp_attr.stPubAttr.stSyncInfo.u16Hpw = screen_attr.stAttr.stSynAttr.u16Hpw;
    disp_attr.stPubAttr.stSyncInfo.u16Vpw = screen_attr.stAttr.stSynAttr.u16Vpw;
    disp_attr.stPubAttr.stSyncInfo.bIdv = screen_attr.stAttr.stSynAttr.bIdv;
    disp_attr.stPubAttr.stSyncInfo.bIhs = screen_attr.stAttr.stSynAttr.bIhs;
    disp_attr.stPubAttr.stSyncInfo.bIvs = screen_attr.stAttr.stSynAttr.bIvs;

    disp_attr.pixel_format = (PIXEL_FORMAT_E)layer_attr.u32PixelFmt;

    layer_attr.u32VLFrameRate = screen_attr.stAttr.u32Framerate;
    layer_attr.stImageSize.u32Width  = screen_attr.stAttr.u32Width;
    layer_attr.stImageSize.u32Height = screen_attr.stAttr.u32Height;

    if (CVI_MAPI_DISP_Init(&ps->disp, param->disp_id, &disp_attr) != 0) {
        CVI_LOGE("MAPI disp init failed");
        return CVI_FAILURE;
    }
    if (CVI_MAPI_DISP_Start(ps->disp, &layer_attr) != 0) {
        CVI_LOGE("MAPI disp start failed");
        return CVI_FAILURE;
    }
#else
    (void)handle;
#endif
    return CVI_SUCCESS;
}

static int32_t deinit_display(CVI_MAPI_DISP_HANDLE_T disp_handle)
{
    CVI_MAPI_DISP_Stop(disp_handle);
    CVI_MAPI_DISP_Deinit(disp_handle);
    /// TODO: call screen deinit will let screen unusual
    // CVI_HAL_SCREEN_Deinit(CVI_HAL_SCREEN_IDX_0);

    return CVI_SUCCESS;
}

static int32_t init_vproc(CVI_PLAYER_SERVICE_HANDLE_T handle,
        uint32_t video_width, uint32_t video_height)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    PS_PARAM_HANDLE param = &ps->param;
    CVI_MAPI_VPROC_ATTR_T vproc_attr = CVI_MAPI_VPROC_DefaultAttr_OneChn(
        video_width, video_height, PIXEL_FORMAT_YUV_PLANAR_420,
        ps->screen_width, ps->screen_height, param->disp_fmt);
    vproc_attr.attr_chn[0].stAspectRatio.enMode = param->disp_aspect_ratio;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Width = get_adjust_side(param->width, ps->screen_width);
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Height = get_adjust_side(param->height, ps->screen_height);
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32X = get_adjust_pos(param->x,
        vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Width, ps->screen_width);
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32Y = get_adjust_pos(param->y,
        vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Height, ps->screen_height);
    vproc_attr.attr_chn[0].stAspectRatio.bEnableBgColor = CVI_TRUE;
    vproc_attr.attr_chn[0].stAspectRatio.u32BgColor = 0x0;
    vproc_attr.attr_inp.u8VpssDev = 1;
    //vproc_attr.attr_inp.u8VpssDev = 0; // note: previous val is 1, for rec+play, use dev0 for vdec-vpss-vo, dev1 for sensor
    if (CVI_MAPI_VPROC_Init(&ps->vproc, -1, &vproc_attr) != 0) {
        CVI_LOGE("MAPI vproc init failed");
        return CVI_FAILURE;
    }

    CVI_PLAYER_SERVICE_Resize(handle, param->width, param->height);

    return CVI_SUCCESS;
}

static int32_t deinit_vproc(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    CVI_MAPI_VPROC_Deinit(ps->vproc);
    return CVI_SUCCESS;
}

static int32_t init_display_if_need(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    PS_PARAM_HANDLE param = &ps->param;
    // init display or use param passed handle
    if (param->disp == NULL) {
        if (init_display(ps) != 0) {
            CVI_LOGE("Init vo failed");
            return CVI_FAILURE;
        }
    } else {
        ps->disp = param->disp;
        init_disp_params(ps);
    }

    return CVI_SUCCESS;
}

static int32_t deinit_display_if_need(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    if (ps->param.disp == NULL) {
        deinit_display(ps->disp);
    }

    return CVI_SUCCESS;
}

static int32_t init_ao_if_need(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    PS_PARAM_HANDLE param = &ps->param;

    if (param->ao == NULL) {
        if ((CVI_MAPI_AO_GetHandle(&ps->ao) != 0)) {
            CVI_MAPI_AO_ATTR_S ao_attr = {};
            ao_attr.enSampleRate = AUDIO_SAMPLE_RATE_16000;
            ao_attr.channels = 1;
            ao_attr.u32PtNumPerFrm = 640;
            if (CVI_MAPI_AO_Init(&ps->ao, &ao_attr) != 0) {
                CVI_LOGE("MAPI ao init failed");
                return CVI_FAILURE;
            }
        }
    } else {
        ps->ao = param->ao;
    }

    return CVI_SUCCESS;
}

static int32_t init_file_recover(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
#if defined(COMPONENTS_FILE_RECOVER_ON) && !defined(FLUSH_MOOV_STREAM_ON)
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    CVI_FILE_RECOVER_Create(&ps->file_recover);
    CVI_FILE_RECOVER_SetCustomArgEventHandler(ps->file_recover, file_recover_event_cb, (void*)ps);
#else
    (void)handle;
#endif

    return CVI_SUCCESS;
}

static int32_t deinit_file_recover(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
#if defined(COMPONENTS_FILE_RECOVER_ON) && !defined(FLUSH_MOOV_STREAM_ON)
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    CVI_FILE_RECOVER_Destroy(&ps->file_recover);
#else
    (void)handle;
#endif

    return CVI_SUCCESS;
}

static int32_t recover_player_source(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
#if defined(COMPONENTS_FILE_RECOVER_ON) && !defined(FLUSH_MOOV_STREAM_ON)
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;

    char data_source[MAX_FILE_PATH_LENGTH] = "";
    if (CVI_PLAYER_GetDataSource(ps->player, data_source) != 0) {
        CVI_LOGE("Player get data source failed");
        return CVI_FAILURE;
    }
    CVI_LOGI("Start recover file %s", data_source);
    if (CVI_FILE_RECOVER_Open(ps->file_recover, data_source) != 0) {
        CVI_LOGE("File recover open file %s failed", data_source);
        return CVI_FAILURE;
    }
    if (CVI_FILE_RECOVER_Check(ps->file_recover) == 0) {
        CVI_LOGI("File recover check %s is not broken", data_source);
        return CVI_SUCCESS;
    }
    if (CVI_FILE_RECOVER_RecoverAsync(ps->file_recover, data_source, "", false) != 0) {
        CVI_LOGE("File recover %s failed", data_source);
        return CVI_FAILURE;
    }
    if (CVI_FILE_RECOVER_RecoverJoin(ps->file_recover) != 0) {
	    CVI_LOGE("File recover join %s failed", data_source);
	    return CVI_FAILURE;
    }
    CVI_FILE_RECOVER_Close(ps->file_recover);
#else
    (void)handle;
#endif
    return CVI_SUCCESS;
}

static int32_t start_ao(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    if (CVI_MAPI_AO_Start(ps->ao, ps->ao_channel) != 0) {
        CVI_LOGE("MAPI ao start failed");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

static int32_t stop_ao(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    CVI_MAPI_AO_Stop(ps->ao, ps->ao_channel);

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_GetDefaultParam(CVI_PLAYER_SERVICE_PARAM_S *param)
{
    if (param == NULL) {
       CVI_LOGE("Param is null");
       return CVI_FAILURE;
    }

    ps_load_default_params(param);

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_Create(CVI_PLAYER_SERVICE_HANDLE_T *handle,
        PS_PARAM_HANDLE param)
{
    CVI_LOGI("Player service create begin");

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)calloc(1, sizeof(PS_CONTEXT_T));
    if (param == NULL) {
        ps_load_default_params(&ps->param);
    } else {
        ps->param = *param;
    }
    ps->shutdown = false;
    ps->send_vo_again = false;
    // init signals and slots
    if (init_signals(ps) != 0) {
        CVI_LOGE("Init signals failed");
    }
    if (init_slots(ps) != 0) {
        CVI_LOGE("Init slots failed");
    }
    // init related components
    if (init_file_recover(ps) != 0) {
        CVI_LOGE("Init file recover failed");
    }
    // init mutex
    cvi_osal_mutex_attr_t mutex_attr = {
        .name = "ps_play_mutex"
    };
    if (cvi_osal_mutex_create(&mutex_attr, &ps->play_mutex) != CVI_OSAL_SUCCESS) {
        CVI_LOGE("Mutex create failed");
        return CVI_FAILURE;
    }
    // init hardware
    if (init_display_if_need(ps) != 0) {
        CVI_LOGE("Init display failed");
        return CVI_FAILURE;
    }
    if (init_ao_if_need(ps) != 0) {
        CVI_LOGE("Init ao failed");
        deinit_display_if_need(ps);
        return CVI_FAILURE;
    }

    CVI_PLAYER_Init();
    CVI_PLAYER_Create(&ps->player);
    CVI_PLAYER_SetCustomArgVOHandler(ps->player, vo_cb, (void *)ps);
    CVI_PLAYER_SetCustomArgEventHandler(ps->player, player_event_cb, (void *)ps);
    CVI_PLAYER_SetCustomArgAOHandler(ps->player, ao_cb, (void *)ps);
    //CVI_PLAYER_SetVideoCustomArgDecodeHandler(ps->player,
    //    (CVI_PLAYER_CUSTOM_ARG_DECODE_HANDLER_S) {
    //        .get_frame = get_frame_cb,
    //        .decode_packet = decode_packet_cb
    //    }, (void *)ps);
    // start event task
    if (ps_start_event_task(ps) != 0) {
        CVI_LOGE("Start event task failed");
    }

    *handle = (CVI_PLAYER_SERVICE_HANDLE_T)ps;

    CVI_LOGI("Player Service on channel %d created", ps->param.chn_id);

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_Destroy(CVI_PLAYER_SERVICE_HANDLE_T *handle)
{
    if ((handle == NULL) || (*handle == NULL)) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)(*handle);

    CVI_LOGI("Player Service on channel %d destroy", ps->param.chn_id);

    ps->shutdown = true;
    CVI_PLAYER_SERVICE_Stop(ps);
    deinit_signals(ps);
    deinit_slots(ps);
    deinit_file_recover(ps);
    ps_stop_event_task(ps);
    // cleanup player
    CVI_PLAYER_Destroy(&ps->player);
    // deinit hardware
    deinit_display_if_need(ps);
    // cleanup mutex
    cvi_osal_mutex_destroy(ps->play_mutex);

    free(ps);
    *handle = NULL;

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_SetInput(CVI_PLAYER_SERVICE_HANDLE_T handle, const char *input)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    if (CVI_PLAYER_SetDataSource(ps->player, input) != 0) {
        CVI_LOGE("Player set data source failed");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_GetMediaInfo(CVI_PLAYER_SERVICE_HANDLE_T handle, CVI_PLAYER_MEDIA_INFO_S *info)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    if (CVI_PLAYER_GetMediaInfo(ps->player, info) != 0) {
        CVI_LOGE("Player get media info failed");

        if (recover_player_source(ps) != 0) {
            CVI_LOGE("File recover failed");
            return CVI_FAILURE;
        }
        if (CVI_PLAYER_GetMediaInfo(ps->player, info) != 0) {
            CVI_LOGE("Player get media info still failed");
            return CVI_FAILURE;
        }
    }

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_GetFileMediaInfo(char *filepatch)
{
	void* player = NULL;
    int32_t ret = 0;
    if (filepatch == NULL) {
        ret = -1;
        CVI_LOGE("Player get file media info failed, file patch is null");
        return ret;
    }
    CVI_PLAYER_Create(&player);
    CVI_PLAYER_SetDataSource(player, filepatch);
    CVI_PLAYER_MEDIA_INFO_S info = {};

    ret = CVI_PLAYER_GetMediaInfo(player, &info);
/*    if (ret != 0) {
        CVI_LOGE("Player get file media info failed");
    }
*/
    CVI_PLAYER_Destroy(&player);

    return ret;
}

int32_t CVI_PLAYER_SERVICE_GetPlayInfo(CVI_PLAYER_SERVICE_HANDLE_T handle, CVI_PLAYER_PLAY_INFO *info)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    if (CVI_PLAYER_GetPlayInfo(ps->player, info) != 0) {
        CVI_LOGE("Player get play info failed");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_PlayerAndSeek(CVI_PLAYER_SERVICE_HANDLE_T handle, int64_t seektime)
{
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    cvi_osal_mutex_lock(ps->play_mutex, CVI_OSAL_WAIT_FOREVER);
    CVI_PLAYER_SERVICE_Play(handle);
    CVI_PLAYER_MEDIA_INFO_S info;
    CVI_PLAYER_SERVICE_GetMediaInfo(ps, &info);
    if (seektime < (info.duration_sec * 1000)) {
        CVI_PLAYER_SERVICE_Seek(handle, seektime);
    }
    cvi_osal_mutex_unlock(ps->play_mutex);

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_PlayerSeep(CVI_PLAYER_SERVICE_HANDLE_T handle, int32_t speeds)
{
    int32_t speed = speeds;
    int32_t backforward = 1;
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    cvi_osal_mutex_lock(ps->play_mutex, CVI_OSAL_WAIT_FOREVER);
    if ((speed == 1) || (speed == 2) || (speed == 4) || (speed == 8)) {
        CVI_PLAYER_PlayerSeep(ps, speed, backforward);
    } else if ((speed > 8) || (speed < 0)) {
        CVI_LOGE("Speed doubling parameter error, current speed doubling parameter is (%d)", speeds);
        cvi_osal_mutex_unlock(ps->play_mutex);
        return CVI_FAILURE;
    }
    cvi_osal_mutex_unlock(ps->play_mutex);
    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_PlayerSeepBack(CVI_PLAYER_SERVICE_HANDLE_T handle, int32_t speeds)
{
    int32_t speed = speeds;
    int32_t backforward = 0;
    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    cvi_osal_mutex_lock(ps->play_mutex, CVI_OSAL_WAIT_FOREVER);
    if ((speed == 1) || (speed == 2) || (speed == 4) || (speed == 8)) {
        CVI_PLAYER_PlayerSeep(ps, speed, backforward);
    } else if ((speed > 8) || (speed < 0)) {
        CVI_LOGE("Speed back doubling parameter error, current speed doubling parameter is (%d)", speeds);
        cvi_osal_mutex_unlock(ps->play_mutex);
        return CVI_FAILURE;
    }
    cvi_osal_mutex_unlock(ps->play_mutex);
    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_Play(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    find_first_sps = false;
    int32_t ret = 0;
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    PS_PARAM_HANDLE param = &ps->param;
    cvi_osal_mutex_lock(ps->play_mutex, CVI_OSAL_WAIT_FOREVER);
    if (ps->playing) {
        if (CVI_PLAYER_Resume(ps->player) != 0) {
            CVI_LOGE("Player resume failed");
            cvi_osal_mutex_unlock(ps->play_mutex);
            return CVI_FAILURE;
        }
        cvi_osal_mutex_unlock(ps->play_mutex);
        return CVI_SUCCESS;
    }

    ret = CVI_PLAYER_LightOpen(ps->player);
    if (ret != 0) {
        CVI_LOGE("Light open failed");
        if (1 == ret) {
            CVI_LOGE("file failed");
            cvi_osal_mutex_unlock(ps->play_mutex);
            return CVI_FAILURE;
        } else {
            if (is_jpeg(ps->codec_type)) {
                cvi_osal_mutex_unlock(ps->play_mutex);
                return CVI_FAILURE;
            } else {
                if (recover_player_source(ps) != 0) {
                    CVI_LOGE("File recover failed");
                    cvi_osal_mutex_unlock(ps->play_mutex);
                    return CVI_FAILURE;
                }
                if (CVI_PLAYER_LightOpen(ps->player) != 0) {
                    CVI_LOGE("Light open still failed");
                    cvi_osal_mutex_unlock(ps->play_mutex);
                    return CVI_FAILURE;
                }
            }
        }
    }

    // CVI_PLAYER_MEDIA_INFO_S info = {};
    memset(&fileinfo, 0, sizeof(fileinfo));
    if (CVI_PLAYER_SERVICE_GetMediaInfo(ps, &fileinfo) != CVI_SUCCESS) {
        cvi_osal_mutex_unlock(ps->play_mutex);
        CVI_PLAYER_Stop(ps->player);
        return CVI_FAILURE;
    }
    if ((fileinfo.width == 0) || (fileinfo.height == 0)) {
        videostreamflage = false;
    } else {
        videostreamflage = true;
    }

    CVI_LOGI("ps fileinfo.video_codec === %s\n", fileinfo.video_codec);
    if ((strcmp(fileinfo.video_codec, "h264") == 0) || (strcmp(fileinfo.video_codec, "mjpeg") == 0)) {
        if ((strcmp(fileinfo.video_codec, "mjpeg") == 0)) {
            if ((strcmp(fileinfo.audio_codec, "") == 0)) {
                VideoCustom = true;
            } else {
                VideoCustom = false;
            }
        } else {
            VideoCustom = true;
        }
    } else {
        VideoCustom = false;
    }

    if (VideoCustom) {
        CVI_PLAYER_SetVideoCustomArgDecodeHandler(ps->player,
            (CVI_PLAYER_CUSTOM_ARG_DECODE_HANDLER_S) {
                .get_frame = get_frame_cb,
                .decode_packet = decode_packet_cb
            }, (void *)ps);
    } else {
        CVI_PLAYER_SetVideoCustomArgDecodeHandler(ps->player,
            (CVI_PLAYER_CUSTOM_ARG_DECODE_HANDLER_S) {
                .get_frame = NULL,
                .decode_packet = NULL
            }, (void *)ps);
    }

    printf("### fileinfo.width = %d, fileinfo.height = %d, cust = %d\n", fileinfo.width, fileinfo.height, VideoCustom);

    if (videostreamflage) {
        if (!VideoCustom) {
            fileinfo.width = CVI_ALIGN_UP(fileinfo.width, 64);
            if (((fileinfo.width / 2) % 64) > 0) {
                fileinfo.width += 64;
            }
            //info.height = CVI_ALIGN_UP(info.height, 64);

            if ((int32_t)(param->width * param->height) < (fileinfo.width * fileinfo.height)) {
                CVI_MAPI_CreateVbPool(PIXEL_FORMAT_YUV_PLANAR_420, fileinfo.width, fileinfo.height);
                AdditionalVb = true;
            }
        }

        ps->codec_type = get_payload_type_from_name(fileinfo.video_codec);
        if (is_jpeg(ps->codec_type)) {
            CVI_MAPI_VDEC_SetVBMode(VB_SOURCE_COMMON, 1);
        } else {
            CVI_MAPI_VDEC_SetVBMode(VB_SOURCE_COMMON, 5);
        }
        // init hardware
        if (init_vdec(ps, fileinfo.width, fileinfo.height) != 0) {
            CVI_LOGE("Init vdec failed");
            cvi_osal_mutex_unlock(ps->play_mutex);
            return CVI_FAILURE;
        }
        if (init_vproc(ps, fileinfo.width, fileinfo.height) != 0) {
            CVI_LOGE("Init vproc failed");
            cvi_osal_mutex_unlock(ps->play_mutex);
            deinit_vdec(ps);
            return CVI_FAILURE;
        }
    }
    if (start_ao(ps) != 0) {
        CVI_LOGE("Start ao failed");
        cvi_osal_mutex_unlock(ps->play_mutex);
        deinit_vdec(ps);
        deinit_vproc(ps);
        return CVI_FAILURE;
    }
    // jpeg don't need buffer size limit, 0 means no limit
    if (videostreamflage) {
        if (is_jpeg(ps->codec_type)) {
            ps->vdec_max_buffer_size = 0;
            if (CVI_MAPI_VPROC_EnableTileMode() != 0) {
                CVI_LOGE("CVI_MAPI_VPROC_EanbleTileMode failed\n");
                return CVI_FAILURE;
            }
        }
        CVI_PLAYER_SetVideoParameters(ps->player, (CVI_PLAYER_VIDEO_PARAMETERS) {
            .output_width = ALIGN(ps->screen_width, DEFAULT_ALIGN),
            .output_height = ps->screen_height,
            .max_packet_size = ps->vdec_max_buffer_size
        });
    }

    CVI_PLAYER_AUDIO_PARAMETERS AudioParameters;
    memset(&AudioParameters, 0, sizeof(AudioParameters));
    AudioParameters.sample_rate = param->SampleRate;
    AudioParameters.channel = param->AudioChannel;

	CVI_PLAYER_SetAudioParameters(ps->player, AudioParameters);

    if (CVI_PLAYER_Play(ps->player) != 0) {
        CVI_LOGE("Player play failed");
        cvi_osal_mutex_unlock(ps->play_mutex);
        deinit_vdec(ps);
        deinit_vproc(ps);
        stop_ao(ps);
        return CVI_FAILURE;
    }
    CVI_LOGI("Player start, file %s", fileinfo.file_name);
    CVI_LOGI("- Format      : %s", fileinfo.format);
    CVI_LOGI("- Video codec : %s", fileinfo.video_codec);
    CVI_LOGI("- Audio codec : %s", fileinfo.audio_codec);
    CVI_LOGI("- Resolution  : w = %d, h = %d", fileinfo.width, fileinfo.height);
    CVI_LOGI("- Duration    : %.2f", fileinfo.duration_sec);
    CVI_LOGI("- Frame rate  : %.2f", fileinfo.frame_rate);

    ps->playing = true;
    cvi_osal_mutex_unlock(ps->play_mutex);

    CVI_SIGNAL_Emit(ps->signals.play, &fileinfo);

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_Stop(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    cvi_osal_mutex_lock(ps->play_mutex, CVI_OSAL_WAIT_FOREVER);
    CVI_PLAYER_Stop(ps->player);
    if (!ps->playing) {
        CVI_LOGW("Player is not playing");
        cvi_osal_mutex_unlock(ps->play_mutex);
        return CVI_SUCCESS;
    }

    if (is_jpeg(ps->codec_type)) {
        if (CVI_MAPI_VPROC_DisableTileMode() != 0) {
            CVI_LOGE("CVI_MAPI_VPROC_DisableTileMode failed\n");
            return CVI_FAILURE;
        }
    }

    stop_ao(ps);
    if (videostreamflage) {
        deinit_vproc(ps);
        deinit_vdec(ps);
        if (AdditionalVb) {
            CVI_MAPI_DestroyVbPool();
            AdditionalVb = false;
        }
    }

    ps->playing = false;
    cvi_osal_mutex_unlock(ps->play_mutex);

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_Pause(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    if (!ps->playing) {
        CVI_LOGW("Player is not playing");
        return CVI_SUCCESS;
    }
    if (CVI_PLAYER_Pause(ps->player) != 0) {
        CVI_LOGE("Player pause failed");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_Seek(CVI_PLAYER_SERVICE_HANDLE_T handle, int64_t time_in_ms)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    if (CVI_PLAYER_Seek(ps->player, time_in_ms) != 0) {
        CVI_LOGE("Player seek failed");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_SeekPause(CVI_PLAYER_SERVICE_HANDLE_T handle, int64_t time_in_ms)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    CVI_PLAYER_MEDIA_INFO_S info;
    cvi_osal_mutex_lock(ps->play_mutex, CVI_OSAL_WAIT_FOREVER);

    CVI_PLAYER_SERVICE_Play(ps);
    CVI_PLAYER_SERVICE_GetMediaInfo(ps, &info);

    if (time_in_ms <= (info.duration_sec * 1000)) {
        if (CVI_PLAYER_SeekPause(ps->player, time_in_ms) != 0) {
            CVI_LOGE("Player seek pause failed");
            cvi_osal_mutex_unlock(ps->play_mutex);
            return CVI_FAILURE;
        }
    }
    cvi_osal_mutex_unlock(ps->play_mutex);

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_TouchSeekPause(CVI_PLAYER_SERVICE_HANDLE_T handle, int64_t time_in_ms)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    CVI_PLAYER_MEDIA_INFO_S info;
    cvi_osal_mutex_lock(ps->play_mutex, CVI_OSAL_WAIT_FOREVER);
    CVI_PLAYER_SERVICE_GetMediaInfo(ps, &info);

    if (time_in_ms <= (info.duration_sec * 1000)) {
        if (CVI_PLAYER_SeekPause(ps->player, time_in_ms) != 0) {
            CVI_LOGE("Player touch seek pause failed");
            cvi_osal_mutex_unlock(ps->play_mutex);
            return CVI_FAILURE;
        }
    }
    cvi_osal_mutex_unlock(ps->play_mutex);

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_SeekFlage()
{
    return CVI_PLAYER_SeekFlage();
}

int32_t CVI_PLAYER_SERVICE_SeekTime()
{
    return CVI_PLAYER_SeekTime();
}

int32_t CVI_PLAYER_SERVICE_SetEventHandler(CVI_PLAYER_SERVICE_HANDLE_T handle,
    CVI_PLAYER_SERVICE_EVENT_HANDLER handler)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    ps->event_handler = handler;

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_Resize(CVI_PLAYER_SERVICE_HANDLE_T handle, uint32_t width, uint32_t height)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    VPSS_CHN_ATTR_S vpss_attr = {};
    PS_PARAM_HANDLE param = &ps->param;
    if (CVI_MAPI_VPROC_GetChnAttr(ps->vproc, 0, &vpss_attr) != 0) {
        CVI_LOGE("MAPI get vpss attr failed");
        return CVI_FAILURE;
    }
    param->width = get_adjust_side(width, ps->screen_width);
    param->height = get_adjust_side(height, ps->screen_height);
    param->x = get_adjust_pos(vpss_attr.stAspectRatio.stVideoRect.s32X,
        vpss_attr.stAspectRatio.stVideoRect.u32Width, ps->screen_width);
    param->y = get_adjust_pos(vpss_attr.stAspectRatio.stVideoRect.s32Y,
        vpss_attr.stAspectRatio.stVideoRect.u32Height, ps->screen_height);
    vpss_attr.stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
    vpss_attr.stAspectRatio.stVideoRect.u32Width = param->width;
    vpss_attr.stAspectRatio.stVideoRect.u32Height = param->height;
    vpss_attr.stAspectRatio.stVideoRect.s32X = param->x;
    vpss_attr.stAspectRatio.stVideoRect.s32Y = param->y;
    if (CVI_MAPI_VPROC_SetChnAttr(ps->vproc, 0, &vpss_attr) != 0) {
        CVI_LOGE("MAPI set vpss attr failed");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_MoveTo(CVI_PLAYER_SERVICE_HANDLE_T handle, uint32_t x, uint32_t y)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    PS_PARAM_HANDLE param = &ps->param;
    if ((x > ps->screen_width) || (y > ps->screen_height)) {
        CVI_LOGE("Move position x or y is invalid");
        return CVI_FAILURE;
    }

    VPSS_CHN_ATTR_S vpss_attr = {};
    if (CVI_MAPI_VPROC_GetChnAttr(ps->vproc, 0, &vpss_attr) != 0) {
        CVI_LOGE("MAPI get vpss attr failed");
        return CVI_FAILURE;
    }
    vpss_attr.stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
    if ((x + vpss_attr.stAspectRatio.stVideoRect.u32Width) > ps->screen_width) {
        vpss_attr.stAspectRatio.stVideoRect.u32Width = ps->screen_width - x;
        param->width = ps->screen_width - x;
    }
    if ((y + vpss_attr.stAspectRatio.stVideoRect.u32Height) > ps->screen_height) {
        vpss_attr.stAspectRatio.stVideoRect.u32Height = ps->screen_height - y;
        param->height = ps->screen_height - y;
    }
    param->x = get_adjust_pos(x,
        vpss_attr.stAspectRatio.stVideoRect.u32Width, ps->screen_width);
    param->y = get_adjust_pos(y,
        vpss_attr.stAspectRatio.stVideoRect.u32Height, ps->screen_height);
    vpss_attr.stAspectRatio.stVideoRect.s32X = param->x;
    vpss_attr.stAspectRatio.stVideoRect.s32Y = param->y;
    if (CVI_MAPI_VPROC_SetChnAttr(ps->vproc, 0, &vpss_attr) != 0) {
        CVI_LOGE("MAPI set vpss attr failed");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_ToggleFullscreen(CVI_PLAYER_SERVICE_HANDLE_T handle)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    VPSS_CHN_ATTR_S vpss_attr = {};
    if (CVI_MAPI_VPROC_GetChnAttr(ps->vproc, 0, &vpss_attr) != 0) {
        CVI_LOGE("MAPI get vpss attr failed");
        return CVI_FAILURE;
    }
    vpss_attr.stAspectRatio.enMode = (vpss_attr.stAspectRatio.enMode == ASPECT_RATIO_AUTO) ?
        ASPECT_RATIO_MANUAL : ASPECT_RATIO_AUTO;
    if (CVI_MAPI_VPROC_SetChnAttr(ps->vproc, 0, &vpss_attr) != 0) {
        CVI_LOGE("MAPI set vpss attr failed");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_GetSignals(CVI_PLAYER_SERVICE_HANDLE_T handle, CVI_PLAYER_SERVICE_SIGNALS_S **signals)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    *signals = &ps->signals;

    return CVI_SUCCESS;
}

int32_t CVI_PLAYER_SERVICE_GetSlots(CVI_PLAYER_SERVICE_HANDLE_T handle, CVI_PLAYER_SERVICE_SLOTS_S **slots)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return CVI_FAILURE;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    *slots = &ps->slots;

    return CVI_SUCCESS;
}

void CVI_PLAYER_SERVICE_SetPlaySubStreamFlag(CVI_PLAYER_SERVICE_HANDLE_T handle, bool subflag)
{
    if (handle == NULL) {
       CVI_LOGE("Handle is null");
       return;
    }

    PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)handle;
    CVI_PLAYER_SetPlaySubStreamFlag(ps->player, subflag);
}
