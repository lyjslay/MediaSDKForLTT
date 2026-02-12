#include "cvi_mapi.h"
#include "cvi_buffer.h"
#include "cvi_mapi_internal.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_vpss.h"
#include "cvi_vo.h"
#include "cvi_isp.h"
#include "cvi_venc.h"
#include "cvi_vdec.h"
#include "cvi_gdc.h"
#include "cvi_region.h"
#include "cvi_bin.h"

CVI_S32 Vproc_Init(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
                  VPSS_CHN_ATTR_S *pastVpssChnAttr, uint32_t *pAttachVbCnt, bool *fbOnVpss)
{
    VPSS_CHN VpssChn = 0;
    CVI_S32 s32Ret;

    s32Ret = CVI_VPSS_CreateGrp(VpssGrp, pstVpssGrpAttr);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_VPSS_ResetGrp(VpssGrp);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_ResetGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
        goto exit1;
    }

    for (unsigned j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++) {
        if (pabChnEnable[j]) {
            VpssChn = j;

            if (fbOnVpss[VpssChn]) {
                CVI_LOGI("############ vpss bind fb %d %d", VpssGrp, VpssChn);
                s32Ret = CVI_VPSS_BindFb(VpssGrp, VpssChn);
                if (s32Ret != CVI_SUCCESS) {
                    CVI_LOGE("CVI_VPSS_BindFb failed. rc: 0x%x !\n", s32Ret);
                    goto exit2;
                }
            }

            s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &pastVpssChnAttr[VpssChn]);
            if (s32Ret != CVI_SUCCESS) {
                CVI_LOGE("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
                goto exit2;
            }

            s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
            if (s32Ret != CVI_SUCCESS) {
                CVI_LOGE("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
                goto exit2;
            }
            if ((pAttachVbCnt != NULL) && (pAttachVbCnt[j] > 0)) {
                VB_POOL_CONFIG_S stVbPoolCfg;
                VB_POOL chnVbPool;
                CVI_U32 u32BlkSize = 0;

                u32BlkSize = COMMON_GetPicBufferSize(pastVpssChnAttr[j].u32Width, pastVpssChnAttr[j].u32Height, pastVpssChnAttr[j].enPixelFormat,
                    DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

                memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
                stVbPoolCfg.u32BlkSize	= u32BlkSize;
                stVbPoolCfg.u32BlkCnt	= pAttachVbCnt[j];
                stVbPoolCfg.enRemapMode = VB_REMAP_MODE_CACHED;
                chnVbPool = CVI_VB_CreatePool(&stVbPoolCfg);
                if (chnVbPool == VB_INVALID_POOLID) {
                    CVI_LOGE("CVI_VB_CreatePool failed.\n");
                } else {
                    CVI_VPSS_AttachVbPool(VpssGrp, j, chnVbPool);
                }
            }
        }
    }

    s32Ret = CVI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
        goto exit2;
    }
    return CVI_SUCCESS;

exit2:
    for(signed j = 0; j < VpssChn; j++){
        if (CVI_VPSS_DisableChn(VpssGrp, j) != CVI_SUCCESS) {
            CVI_LOGE("CVI_VPSS_DisableChn failed!\n");
        }
    }
exit1:
    if (CVI_VPSS_DestroyGrp(VpssGrp) != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_DestroyGrp(grp:%d) failed!\n", VpssGrp);
    }

    return s32Ret;
}

CVI_VOID Vproc_Deinit(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, bool *fbOnVpss)
{
    CVI_S32 j;
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPSS_CHN VpssChn;

    for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++) {
        if (pabChnEnable[j]) {
            VpssChn = j;
            s32Ret = CVI_VPSS_DisableChn(VpssGrp, VpssChn);
            if (s32Ret != CVI_SUCCESS) {
                CVI_LOGE("failed with %#x!\n", s32Ret);
            }
        }

        if(fbOnVpss[j]){
            s32Ret = CVI_VPSS_UnbindFb(VpssGrp, j);
			if (s32Ret != CVI_SUCCESS) {
				CVI_LOGE("CVI_VPSS_UnbindFb failed. rc: 0x%x !\n", s32Ret);
			}
        }
    }
    s32Ret = CVI_VPSS_StopGrp(VpssGrp);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("failed with %#x!\n", s32Ret);
    }
    s32Ret = CVI_VPSS_DestroyGrp(VpssGrp);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("failed with %#x!\n", s32Ret);
    }
}

uint32_t get_frame_size(uint32_t w, uint32_t h, PIXEL_FORMAT_E fmt) {
    // try rotate and non-rotate, choose the larger one
    uint32_t sz_0 = COMMON_GetPicBufferSize(w, h, fmt,
        DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    // uint32_t sz_1 = COMMON_GetPicBufferSize(h, w, fmt,
    //     DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    // return (sz_0 > sz_1) ? sz_0 : sz_1;
    return sz_0;
}