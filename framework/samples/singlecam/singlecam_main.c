#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "cvi_mapi.h"
#include "cvi_osal.h"
#include "cvi_log.h"
#include "cvi_rtsp.h"
#include "cvi_muxer.h"
#include "cvi_hal_screen.h"

#include "cvi_sys.h"

#ifndef CHECK_RET
#define CHECK_RET(express)                                                    \
    do {                                                                      \
        int rc = express;                                                     \
        if (rc != 0) {                                                        \
            printf("\nFailed at %s: %d  (rc:0x%#x!)\n",                       \
                    __FILE__, __LINE__, rc);                                  \
            return rc;                                                        \
        }                                                                     \
    } while (0)
#endif

static int save_yuv_open(char *filename, FILE **fp) {
    FILE *output = fopen(filename, "wb");
    CVI_LOG_ASSERT(output, "file open failed\n");
    *fp = output;
    return 0;
}

static int save_yuv_close(FILE *fp) {
    fclose(fp);
    return 0;
}

static int save_yuv_frame(VIDEO_FRAME_INFO_S *frm, FILE *fp)
{
    CVI_LOG_ASSERT(frm->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420,
            "Invalid frame pixel format %d\n", frm->stVFrame.enPixelFormat);
    CVI_U32 u32LumaSize, u32ChromaSize;
    u32LumaSize   =  frm->stVFrame.u32Stride[0] * frm->stVFrame.u32Height;
    u32ChromaSize =  frm->stVFrame.u32Stride[1] * frm->stVFrame.u32Height / 2;
    size_t image_size = frm->stVFrame.u32Length[0]
                      + frm->stVFrame.u32Length[1]
                      + frm->stVFrame.u32Length[2];

    void *vir_addr = CVI_SYS_Mmap(frm->stVFrame.u64PhyAddr[0], image_size);
    CVI_SYS_IonInvalidateCache(frm->stVFrame.u64PhyAddr[0], vir_addr, image_size);
    uint32_t plane_offset = 0;
    for (int i = 0; i < 3; i++) {
        frm->stVFrame.pu8VirAddr[i] = vir_addr + plane_offset;
        plane_offset += frm->stVFrame.u32Length[i];
        CVI_LOGV("plane(%d): paddr(0x%lx) vaddr(%p) stride(%d) length(%d)\n",
                i,
                frm->stVFrame.u64PhyAddr[i],
                frm->stVFrame.pu8VirAddr[i],
                frm->stVFrame.u32Stride[i],
                frm->stVFrame.u32Length[i]);
        fwrite((void *)frm->stVFrame.pu8VirAddr[i],
               (i == 0) ? u32LumaSize : u32ChromaSize, 1, fp);
    }
    CVI_SYS_Munmap(vir_addr, image_size);
    return 0;
}

static int save_yuv_single_frame(VIDEO_FRAME_INFO_S *frm, int no)
{
    #define NAME_MAX_LEN    (64)
    char name[NAME_MAX_LEN] = {0};
    snprintf(name, NAME_MAX_LEN, "test_%d", no);
    return CVI_MAPI_SaveFramePixelData(frm, name);
}

static MEDIA_VideoOutCfg videoOutCfg;

static int DispOpen(CVI_U32 width, CVI_U32 height, ROTATION_E rotate)
{
    int ret = CVI_MAPI_SUCCESS;

    MEDIA_DispCfg *dispCfg = &videoOutCfg.dispCfg[0];

    dispCfg->dispAttr.width = width;
    dispCfg->dispAttr.height = height;
    dispCfg->dispAttr.rotate = rotate;
    dispCfg->dispAttr.window_mode = false;
    dispCfg->dispAttr.stPubAttr.u32BgColor = COLOR_10_RGB_BLUE;
    dispCfg->dispAttr.stPubAttr.enIntfSync = VO_OUTPUT_USER;
    dispCfg->videoLayerAttr.u32BufLen = 3;
    dispCfg->videoLayerAttr.u32PixelFmt = PIXEL_FORMAT_YUV_PLANAR_420;

	extern CVI_HAL_SCREEN_OBJ_S stHALSCREENObj;
    CHECK_RET(CVI_HAL_SCREEN_Register(CVI_HAL_SCREEN_IDX_0, &stHALSCREENObj));
    CHECK_RET(CVI_HAL_SCREEN_Init(CVI_HAL_SCREEN_IDX_0));
    CVI_HAL_SCREEN_ATTR_S screenAttr = {0};
    CVI_HAL_SCREEN_GetAttr(CVI_HAL_SCREEN_IDX_0, &screenAttr);
    switch(screenAttr.enType) {
        case CVI_HAL_SCREEN_INTF_TYPE_MIPI:
            dispCfg->dispAttr.stPubAttr.enIntfType = VO_INTF_MIPI;
            break;
        case CVI_HAL_SCREEN_INTF_TYPE_LCD:
        default:
            CVI_LOGD("Invalid screen type\n");
            return CVI_MAPI_ERR_FAILURE;
    }

    dispCfg->dispAttr.stPubAttr.stSyncInfo.bSynm   = 1; /**<sync mode: signal */
    dispCfg->dispAttr.stPubAttr.stSyncInfo.bIop    = 1; /**<progressive display */
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16FrameRate = screenAttr.stAttr.u32Framerate;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Vact = screenAttr.stAttr.stSynAttr.u16Vact;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Vbb  = screenAttr.stAttr.stSynAttr.u16Vbb;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Vfb  = screenAttr.stAttr.stSynAttr.u16Vfb;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Hact = screenAttr.stAttr.stSynAttr.u16Hact;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Hbb  = screenAttr.stAttr.stSynAttr.u16Hbb;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Hfb  = screenAttr.stAttr.stSynAttr.u16Hfb;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Hpw  = screenAttr.stAttr.stSynAttr.u16Hpw;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Vpw  = screenAttr.stAttr.stSynAttr.u16Vpw;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.bIdv    = screenAttr.stAttr.stSynAttr.bIdv;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.bIhs    = screenAttr.stAttr.stSynAttr.bIhs;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.bIvs    = screenAttr.stAttr.stSynAttr.bIvs;

    dispCfg->dispAttr.pixel_format = dispCfg->videoLayerAttr.u32PixelFmt;

    dispCfg->videoLayerAttr.u32VLFrameRate = screenAttr.stAttr.u32Framerate;
    dispCfg->videoLayerAttr.stImageSize.u32Width  = screenAttr.stAttr.u32Width;
    dispCfg->videoLayerAttr.stImageSize.u32Height = screenAttr.stAttr.u32Height;

    CHECK_RET(CVI_MAPI_DISP_Init(&dispCfg->dispHdl, 0, &dispCfg->dispAttr));
    CHECK_RET(CVI_MAPI_DISP_Start(dispCfg->dispHdl, &dispCfg->videoLayerAttr));

    return ret;
}

static int DispClose(CVI_MAPI_DISP_HANDLE_T disp_hdl)
{
    CHECK_RET(CVI_MAPI_DISP_Stop(disp_hdl));
    CHECK_RET(CVI_MAPI_DISP_Deinit(disp_hdl));

    return CVI_MAPI_SUCCESS;
}

int main(int argc, char *argv[])
{
    bool bind_mode = false;
    bool perf_flag = false;
    bool save_yuv_flag = false;
    bool save_yuv_single = false;
    bool venc_flag = false;
    bool rtsp_flag = false;
    bool muxer_flag = false;
    int count = 300;
    const char *outputfile = NULL;
    int sns_id = 0;

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "c:ebps1o:rmi:")) != -1) {
        switch (c) {
        case 'e':
            venc_flag = true;
            break;
        case 'o':
            outputfile = optarg;
            break;
        case 'b':
            bind_mode = true;
            break;
        case 'c':
            count = atoi(optarg);
            break;
        case 'p':
            perf_flag = true;
            break;
        case 's':
            save_yuv_flag = true;
            break;
        case '1':
            save_yuv_single = true;
            break;
        case 'r':
            rtsp_flag = true;
            break;
        case 'm':
            muxer_flag = true;
            break;
        case 'i':
            sns_id = atoi(optarg);
            break;
        default:
            printf("Invalid option : %c\n", c);
            abort ();
        }
    }

#define IMAGE_WIDTH   (1920)
#define IMAGE_HEIGHT  (1080)
#define DISP_WIDTH    (1280)
#define DISP_HEIGHT   (720)
//#define DISP_FMT      PIXEL_FORMAT_RGB_888_PLANAR
#define DISP_FMT      PIXEL_FORMAT_YUV_PLANAR_420
#define RTSP_WIDTH    (1280)
#define RTSP_HEIGHT   (720)

    int vproc_chn_id_disp = 0;
    int vproc_chn_id_rtsp = 1;

    CVI_LOG_INIT();

    CVI_MAPI_MEDIA_SYS_ATTR_T sys_attr;
    sys_attr.vb_pool[0].is_frame = true;
    sys_attr.vb_pool[0].vb_blk_size.frame.width  = IMAGE_WIDTH;
    sys_attr.vb_pool[0].vb_blk_size.frame.height = IMAGE_HEIGHT;
    sys_attr.vb_pool[0].vb_blk_size.frame.fmt    = PIXEL_FORMAT_YUV_PLANAR_420;
    sys_attr.vb_pool[0].vb_blk_num = 16;
    sys_attr.vb_pool[1].is_frame = true;
    sys_attr.vb_pool[1].vb_blk_size.frame.width  = IMAGE_WIDTH;
    sys_attr.vb_pool[1].vb_blk_size.frame.height = IMAGE_HEIGHT;
    sys_attr.vb_pool[1].vb_blk_size.frame.fmt    = PIXEL_FORMAT_RGB_888_PLANAR;
    sys_attr.vb_pool[1].vb_blk_num = 4;
    sys_attr.vb_pool_num = 2;
    CHECK_RET(CVI_MAPI_Media_Init(&sys_attr));

    CVI_MAPI_VCAP_SENSOR_HANDLE_T sns;
    CVI_MAPI_VCAP_ATTR_T vcap_attr;
    CHECK_RET(CVI_MAPI_VCAP_GetGeneralVcapAttr(&vcap_attr));
    CHECK_RET(CVI_MAPI_VCAP_InitSensor(&sns, sns_id, &vcap_attr));
    CHECK_RET(CVI_MAPI_VCAP_InitISP(sns));
    CHECK_RET(CVI_MAPI_VCAP_StartISP(sns));
    CHECK_RET(CVI_MAPI_VCAP_StartDev(sns));
    CHECK_RET(CVI_MAPI_VCAP_StartPipe(sns));
    CHECK_RET(CVI_MAPI_VCAP_StartChn(sns));

    CVI_MAPI_VPROC_HANDLE_T vproc;
    CVI_MAPI_VPROC_ATTR_T vproc_attr = CVI_MAPI_VPROC_DefaultAttr_TwoChn(
            IMAGE_WIDTH, IMAGE_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420,
            DISP_WIDTH, DISP_HEIGHT, DISP_FMT,
            RTSP_WIDTH, RTSP_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420);
    CHECK_RET(CVI_MAPI_VPROC_Init(&vproc, -1, &vproc_attr));

    CHECK_RET(DispOpen(DISP_WIDTH, DISP_HEIGHT, ROTATION_90));

    CVI_MAPI_VENC_HANDLE_T venc_rec;
    CVI_MAPI_VENC_HANDLE_T venc_rtsp;
    FILE *outfp = NULL;
    CVI_MUXER_HANDLE_T muxer;
    CVI_RTSP_SESSION_HANDLE_T rtsp;
    if (venc_flag) {
        CVI_MAPI_VENC_CHN_ATTR_T venc_attr = {0};
        venc_attr.codec = CVI_MAPI_VCODEC_H264;
        venc_attr.width = IMAGE_WIDTH;
        venc_attr.height = IMAGE_HEIGHT;
        venc_attr.pixel_format = PIXEL_FORMAT_YUV_PLANAR_420;
        venc_attr.rate_ctrl_mode = 4; // FIXQP
        CHECK_RET(CVI_MAPI_VENC_InitChn(&venc_rec, &venc_attr));
        CHECK_RET(CVI_MAPI_VENC_StartRecvFrame(&venc_rec, -1));

        if (outputfile) {
            if (muxer_flag) {
                CVI_LOGI("Save MP4 file as %s\n", outputfile);
                CVI_MUXER_ATTR_S muxer_attr;
                muxer_attr.video_codec = CVI_REC_VIDEO_CODEC_H264;
                muxer_attr.width = IMAGE_WIDTH;
                muxer_attr.height = IMAGE_HEIGHT;
                muxer_attr.framerate = 25;
                CHECK_RET(CVI_MUXER_Init(&muxer, &muxer_attr));
                CHECK_RET(CVI_MUXER_StartMux(muxer, outputfile));
            } else {
                CVI_LOGI("Save H264 file as %s\n", outputfile);
                outfp = fopen(outputfile, "wb");
                if (outfp == NULL) {
                    CVI_LOGE("open file err, %s\n", outputfile);
                    exit(-1);
                }
            }
        }

        if (rtsp_flag) {
            venc_attr.codec = CVI_MAPI_VCODEC_H264;
            venc_attr.width = RTSP_WIDTH;
            venc_attr.height = RTSP_HEIGHT;
            venc_attr.pixel_format = PIXEL_FORMAT_YUV_PLANAR_420;
            venc_attr.rate_ctrl_mode = 1;   // VBR
            venc_attr.bitrate_kbps = 2000;
            CHECK_RET(CVI_MAPI_VENC_InitChn(&venc_rtsp, &venc_attr));
            CHECK_RET(CVI_MAPI_VENC_StartRecvFrame(&venc_rtsp, -1));

            CVI_LOGI("Start RTSP Streaming\n");
            CVI_MAPI_RTSP_SESSION_ATTR_T rtsp_attr;
            rtsp_attr.session_name = "cvi_cam_0";
            CHECK_RET(CVI_RTSP_CreateSession(&rtsp, &rtsp_attr));
        }
    }

    if (bind_mode) {
        CHECK_RET(CVI_MAPI_VPROC_BindVcap(vproc, sns));

        CHECK_RET(CVI_MAPI_DISP_BindVproc(videoOutCfg.dispCfg[0].dispHdl, vproc, vproc_chn_id_disp));

        printf("---------------press Enter key to exit!---------------\n");                                    \
        getchar();                                                                                             \

        CHECK_RET(CVI_MAPI_DISP_UnBindVproc(videoOutCfg.dispCfg[0].dispHdl, vproc, vproc_chn_id_disp));

        CHECK_RET(CVI_MAPI_VPROC_UnBindVcap(vproc, sns));
    } else {
        FILE *fp = NULL;
        if (save_yuv_flag && !save_yuv_single) {
            save_yuv_open("test.yuv", &fp);
        }
        while(count--) {
            VIDEO_FRAME_INFO_S vcap_frame;
            bool is_I_frame = false;

            uint64_t t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
            cvi_osal_get_boot_time(&t1);

            CHECK_RET(CVI_MAPI_VCAP_GetFrame(sns, &vcap_frame));

            cvi_osal_get_boot_time(&t2);

            if (save_yuv_flag) {
                if (save_yuv_single) {
                    save_yuv_single_frame(&vcap_frame, count);
                } else {
                    save_yuv_frame(&vcap_frame, fp);
                }
            }

            cvi_osal_get_boot_time(&t3);

            if (venc_flag) {
                CHECK_RET(CVI_MAPI_VENC_SendFrame(venc_rec, &vcap_frame));
                VENC_STREAM_S stream = {0};
                CHECK_RET(CVI_MAPI_VENC_GetStream(venc_rec, &stream));

                cvi_osal_get_boot_time(&t4);

                CHECK_RET(CVI_MAPI_VENC_GetStreamStatus(venc_rec, &stream, &is_I_frame));

                if (muxer_flag) {
                    CVI_LOG_ASSERT(stream.u32PackCount <= CVI_MUXER_FRAME_STREAM_SEGMENT_MAX_NUM,
                        "too many stream segments %d\n", stream.u32PackCount);
                    CVI_MUXER_FRAME_STREAM_T frame_stream;
                    for (unsigned i = 0; i < stream.u32PackCount; i++) {
                        VENC_PACK_S *ppack;
                        ppack = &stream.pstPack[i];
                        frame_stream.data[i] = ppack->pu8Addr + ppack->u32Offset;
                        frame_stream.len[i]  = ppack->u32Len - ppack->u32Offset;
                    }
                    frame_stream.num = stream.u32PackCount;
                    frame_stream.is_I_frame = is_I_frame;
                    static int64_t pts = 0;
                    frame_stream.pts = pts;
                    pts ++;
                    CHECK_RET(CVI_MUXER_SendFrameStream(muxer, &frame_stream));
                }

                for (uint32_t i = 0; i < stream.u32PackCount; i++) {
                    VENC_PACK_S *ppack;
                    ppack = &stream.pstPack[i];
                    if (outputfile && !muxer_flag) {
                        fwrite(ppack->pu8Addr + ppack->u32Offset,
                               ppack->u32Len - ppack->u32Offset, 1,
                               outfp);
                    }
                    //if (rtsp_flag) {
                    //    CHECK_RET(CVI_RTSP_SendStream(rtsp,
                    //            ppack->pu8Addr + ppack->u32Offset,
                    //            ppack->u32Len - ppack->u32Offset));
                    //}
                }

                CHECK_RET(CVI_MAPI_VENC_ReleaseStream(venc_rec, &stream));
            }

            cvi_osal_get_boot_time(&t5);

            CHECK_RET(CVI_MAPI_VPROC_SendFrame(vproc, &vcap_frame));
            CHECK_RET(CVI_MAPI_ReleaseFrame(&vcap_frame));

            cvi_osal_get_boot_time(&t6);

            VIDEO_FRAME_INFO_S vproc_frame_vo;
            CHECK_RET(CVI_MAPI_VPROC_GetChnFrame(vproc, vproc_chn_id_disp, &vproc_frame_vo));

            cvi_osal_get_boot_time(&t7);

            CHECK_RET(CVI_MAPI_DISP_SendFrame(videoOutCfg.dispCfg[0].dispHdl, &vproc_frame_vo));
            CHECK_RET(CVI_MAPI_ReleaseFrame(&vproc_frame_vo));

            cvi_osal_get_boot_time(&t8);

            if (venc_flag && rtsp_flag) {
                VIDEO_FRAME_INFO_S vproc_frame_rtsp;

                CHECK_RET(CVI_MAPI_VPROC_GetChnFrame(vproc, vproc_chn_id_rtsp, &vproc_frame_rtsp));

                CHECK_RET(CVI_MAPI_VENC_SendFrame(venc_rtsp, &vproc_frame_rtsp));
                CHECK_RET(CVI_MAPI_ReleaseFrame(&vproc_frame_rtsp));

                VENC_STREAM_S stream = {0};
                CHECK_RET(CVI_MAPI_VENC_GetStream(venc_rtsp, &stream));

                cvi_osal_get_boot_time(&t9);

                CHECK_RET(CVI_MAPI_VENC_GetStreamStatus(venc_rtsp, &stream, &is_I_frame));

                CVI_LOG_ASSERT(stream.u32PackCount <= CVI_MUXER_FRAME_STREAM_SEGMENT_MAX_NUM,
                        "too many stream segments %d\n", stream.u32PackCount);
                CVI_RTSP_FRAME_STREAM_T frame_stream;
                for (unsigned i = 0; i < stream.u32PackCount; i++) {
                    VENC_PACK_S *ppack;
                    ppack = &stream.pstPack[i];
                    frame_stream.data[i] = ppack->pu8Addr + ppack->u32Offset;
                    frame_stream.len[i]  = ppack->u32Len - ppack->u32Offset;
                }
                frame_stream.num = stream.u32PackCount;
                frame_stream.is_I_frame = is_I_frame;
                CHECK_RET(CVI_RTSP_SendStream(rtsp, &frame_stream));

                cvi_osal_get_boot_time(&t10);

                CHECK_RET(CVI_MAPI_VENC_ReleaseStream(venc_rtsp, &stream));
            }

            printf(".");
            if (count % 30 == 0 || is_I_frame) {
                printf("\n");
                if (perf_flag) {
                    printf("Perf: %s\n", is_I_frame?"":"I Frame");
                    printf("      Get_VI    %2.2f ms\n", (t2-t1)/1000.0);
                    printf("      Save VI   %2.2f ms\n", (t3-t2)/1000.0);
                    if (venc_flag) {
                        printf("      Venc      %2.2f ms\n", (t4-t3)/1000.0);
                        printf("      Stream    %2.2f ms\n", (t5-t4)/1000.0);
                    }
                    printf("      Send_VPSS %2.2f ms\n", (t6-t5)/1000.0);
                    printf("      Get_VPSS  %2.2f ms\n", (t7-t6)/1000.0);
                    printf("      Send_VO   %2.2f ms\n", (t8-t7)/1000.0);
                    if (venc_flag && rtsp_flag) {
                        printf("      Venc[r]   %2.2f ms\n", (t9-t8)/1000.0);
                        printf("      Stream    %2.2f ms\n", (t10-t9)/1000.0);
                    }
                }
            }
        }
        if (save_yuv_flag && !save_yuv_single) {
            save_yuv_close(fp);
        }
    }

    if (venc_flag) {
        if (rtsp_flag) {
            CHECK_RET(CVI_RTSP_DestroySession(rtsp));
            CHECK_RET(CVI_MAPI_VENC_StopRecvFrame(venc_rtsp));
            CHECK_RET(CVI_MAPI_VENC_DeinitChn(venc_rtsp));
        }
        if (outputfile) {
            if (muxer_flag) {
                CHECK_RET(CVI_MUXER_StopMux(muxer));
                CHECK_RET(CVI_MUXER_Deinit(muxer));
            } else {
                fclose(outfp);
            }
        }

        CHECK_RET(CVI_MAPI_VENC_StopRecvFrame(venc_rec));
        CHECK_RET(CVI_MAPI_VENC_DeinitChn(venc_rec));
    }

    CHECK_RET(DispClose(videoOutCfg.dispCfg[0].dispHdl));

    CHECK_RET(CVI_MAPI_VPROC_Deinit(vproc));

    CHECK_RET(CVI_MAPI_VCAP_StopChn(sns));
    CHECK_RET(CVI_MAPI_VCAP_StopDev(sns));
    CHECK_RET(CVI_MAPI_VCAP_StopPipe(sns));
    CHECK_RET(CVI_MAPI_VCAP_StopISP(sns));
    CHECK_RET(CVI_MAPI_VCAP_DeInitISP(sns));
    CHECK_RET(CVI_MAPI_VCAP_DeinitSensor(sns));

    CHECK_RET(CVI_MAPI_Media_Deinit());

    return 0;
}
