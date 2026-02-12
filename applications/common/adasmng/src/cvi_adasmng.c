#include <stdio.h>
#include <string.h>
#include <math.h>
#include "cvi_adasmng.h"
#include "cvi_appcomm.h"
#include "cvi_eventhub.h"
#include "cvi_param.h"
#include "cvi_mapi.h"

int32_t CVI_ADASMNG_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_EVENTHUB_RegisterTopic(CVI_EVENT_ADASMNG_CAR_MOVING);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_ADASMNG_CAR_CLOSING);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_ADASMNG_CAR_COLLISION);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_ADASMNG_CAR_LANE);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_ADASMNG_LABEL_CAR);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_ADASMNG_LABEL_LANE);
    CVI_APPCOMM_CHECK_RETURN(s32Ret, CVI_ADASMNG_EREGISTER_EVENT);
    return s32Ret;
}

int32_t CVI_ADASMNG_VoiceCallback(int32_t index)
{
    CVI_EVENT_S stEvent;
    memset(&stEvent, 0x0, sizeof(CVI_EVENT_S));
    switch (index)
    {
    case CVI_ADASMNG_CAR_MOVING:
        stEvent.topic = CVI_EVENT_ADASMNG_CAR_MOVING;
        break;
    case CVI_ADASMNG_CAR_CLOSING:
        stEvent.topic = CVI_EVENT_ADASMNG_CAR_CLOSING;
        break;
    case CVI_ADASMNG_CAR_COLLISION:
        stEvent.topic = CVI_EVENT_ADASMNG_CAR_COLLISION;
        break;
    case CVI_ADASMNG_CAR_LANE:
        stEvent.topic = CVI_EVENT_ADASMNG_CAR_LANE;
        break;
    default:
        break;
    }
    CVI_EVENTHUB_Publish(&stEvent);
    return 0;
}

int32_t CVI_ADASMNG_LabelCallback(int32_t camid, int32_t index, uint32_t count, char* coordinates)
{
    CVI_EVENT_S stEvent;
    memset(&stEvent, 0x0, sizeof(CVI_EVENT_S));
    switch (index)
    {
    case CVI_ADASMNG_LABEL_CAR:
        stEvent.topic = CVI_EVENT_ADASMNG_LABEL_CAR;
        stEvent.arg1 = count;
        stEvent.arg2 = camid;
        memcpy(stEvent.aszPayload, coordinates, sizeof(stEvent.aszPayload));
        break;
    case CVI_ADASMNG_LABEL_LANE:
        stEvent.topic = CVI_EVENT_ADASMNG_LABEL_LANE;
        stEvent.arg1 = count;
        stEvent.arg2 = camid;
        memcpy(stEvent.aszPayload, coordinates, sizeof(stEvent.aszPayload));
        break;
    default:
        break;
    }
    CVI_EVENTHUB_Publish(&stEvent);
    return 0;
}

 /*   (0,0)                                                   w0
    *       -----------------------------------------------------
    *       |    (x,y)               w                          |
    *       |      -------------------------------------        |
    *       |      |                                    |       |
    *       |    h |           CROP_OUT                 |       |
    *       |      |                                    |       |
    *       |      -------------------------------------        |
    *       |                                                   |
    *    h0   -----------------------------------------------------
    */

static int32_t osdc_draw_transfer(int32_t camid, int32_t disp_grp, int32_t disp_chn, int32_t source_x1, int32_t source_y1, int32_t source_x2, int32_t source_y2, int32_t *dst)
{
    CVI_PARAM_ADAS_ATTR_S ADASAttr = {0};
    CVI_PARAM_GetADASConfigParam(&ADASAttr);
    int32_t bind_chn_id = ADASAttr.ChnAttrs[camid].BindVprocChnId;

    VPSS_CHN_ATTR_S stChnAttr = {0};
    int32_t s32Ret = 0;
    s32Ret = CVI_VPSS_GetChnAttr(disp_grp, disp_chn, &stChnAttr);
    if (s32Ret != 0) {
        CVI_LOGE("CVI_VPSS_GetChnCrop fail with %#x\n", s32Ret);
        return s32Ret;
    }
    long disp_width = (long)stChnAttr.u32Width;
    long disp_height = (long)stChnAttr.u32Height;


    CVI_PARAM_MEDIA_SPEC_S params;
    CVI_PARAM_GetMediaMode(camid, &params);
    uint32_t vi_width = params.VcapAttr.VcapChnAttr.u32Width;
    uint32_t vi_height = params.VcapAttr.VcapChnAttr.u32Height;
    uint32_t source_width = params.VprocAttr.VprocChnAttr[bind_chn_id].VpssChnAttr.u32Width;
    uint32_t source_height = params.VprocAttr.VprocChnAttr[bind_chn_id].VpssChnAttr.u32Height;

    int32_t vi_x1 = source_x1 * vi_width;
    int32_t vi_x2 = source_x2 * vi_width;
    int32_t vi_y1 = source_y1 * vi_height;
    int32_t vi_y2 = source_y2 * vi_height;

    VPSS_CROP_INFO_S stCropInfo = {0};
    s32Ret = CVI_VPSS_GetChnCrop(disp_grp, disp_chn, &stCropInfo);
    if (s32Ret != 0) {
        CVI_LOGE("CVI_VPSS_GetChnCrop fail with %#x\n", s32Ret);
        return s32Ret;
    }

    int32_t s32X = stCropInfo.stCropRect.s32X;
    int32_t s32Y = stCropInfo.stCropRect.s32Y;
    int32_t u32Width = stCropInfo.stCropRect.u32Width;
    int32_t u32Height = stCropInfo.stCropRect.u32Height;
    int32_t vi_min_x = s32X * source_width;
    int32_t vi_min_y = s32Y * source_height;

    int32_t new_x1 = vi_x1 - vi_min_x;
    int32_t new_x2 = vi_x2 - vi_min_x;
    int32_t new_y1 = vi_y1 - vi_min_y;
    int32_t new_y2 = vi_y2 - vi_min_y;

    dst[0] = ceil(new_x1 * disp_width/ (u32Width * source_width));
    dst[1] = ceil(new_y1 * disp_height/ (u32Height * source_height));
    dst[2] = ceil(new_x2 * disp_width/ (u32Width * source_width));
    dst[3] = ceil(new_y2 * disp_height/ (u32Height * source_height));

    return 0;
}

int32_t CVI_ADASMNG_LabelOSDCCallback(int32_t camid, uint32_t car_count, char* car_coordinates, uint32_t lane_count, char* lane_coordinates)
{
    CVI_PARAM_MEDIA_OSD_ATTR_S OsdParam;
    CVI_PARAM_GetOsdParam(&OsdParam);
    int32_t j = 0;
    uint32_t z = 0;
    for(j = 0; j < OsdParam.OsdCnt; j++){
        // CVI_MAPI_OSD_OBJECT_CONTENT_S* stObject = &OsdParam.OsdAttrs[j].stContent.stObjectContent;
        CVI_MAPI_OSD_ATTR_S osdAttr;
        if(CVI_MAPI_OSD_GetAttr(j,&osdAttr) != 0){
            CVI_LOGE("Get_attr faild\n");
        }

        CVI_MAPI_OSD_OBJECT_CONTENT_S* stObject = &osdAttr.stContent.stObjectContent;
        for(z = 0; z < OsdParam.OsdAttrs[j].u32DispNum; z++){
            if ((OsdParam.OsdAttrs[j].astDispAttr[z].u32Batch == (uint32_t)camid) && (OsdParam.OsdAttrs[j].astDispAttr[z].bShow == 1) &&
                OsdParam.OsdAttrs[j].stContent.enType == CVI_MAPI_OSD_TYPE_OBJECT){
                stObject->objectInfo.rec_cnt = car_count;
                for (uint32_t k = 0; k < car_count; ++k) {
                    int32_t offset = k << 4;
                    int32_t x1 = *((int32_t*)(car_coordinates + 0 + offset));
                    int32_t y1 = *((int32_t*)(car_coordinates + 4 + offset));
                    int32_t x2 = *((int32_t*)(car_coordinates + 8 + offset));
                    int32_t y2 = *((int32_t*)(car_coordinates + 12 + offset));
                    int32_t dst[4];
                    if (osdc_draw_transfer(camid, OsdParam.OsdAttrs[j].astDispAttr[z].ModHdl, OsdParam.OsdAttrs[j].astDispAttr[z].ChnHdl, x1, y1, x2, y2, dst) != 0)
                        continue;

                    (stObject->objectInfo.rec_coordinates)[4*k] = dst[0];
                    (stObject->objectInfo.rec_coordinates)[4*k+1] = dst[1];
                    (stObject->objectInfo.rec_coordinates)[4*k+2] = dst[2];
                    (stObject->objectInfo.rec_coordinates)[4*k+3] = dst[3];
                }
                stObject->objectInfo.line_cnt = lane_count;
                for (uint32_t k = 0; k < lane_count; ++k) {
                    int32_t offset = k << 4;
                    int32_t x1 = *((int32_t*)(lane_coordinates + 0 + offset));
                    int32_t y1 = *((int32_t*)(lane_coordinates + 4 + offset));
                    int32_t x2 = *((int32_t*)(lane_coordinates + 8 + offset));
                    int32_t y2 = *((int32_t*)(lane_coordinates + 12 + offset));
                    int32_t dst[4];
                    if (osdc_draw_transfer(camid, OsdParam.OsdAttrs[j].astDispAttr[z].ModHdl, OsdParam.OsdAttrs[j].astDispAttr[z].ChnHdl, x1, y1, x2, y2, dst) != 0)
                        continue;

                    (stObject->objectInfo.line_coordinates)[4*k] = dst[0];
                    (stObject->objectInfo.line_coordinates)[4*k+1] = dst[1];
                    (stObject->objectInfo.line_coordinates)[4*k+2] = dst[2];
                    (stObject->objectInfo.line_coordinates)[4*k+3] = dst[3];
                }

                if(CVI_MAPI_OSD_SetAttr(j, &osdAttr) != 0){
                    CVI_LOGE("set_attr faild\n");
                }
            }
        }
    }

    return 0;
}