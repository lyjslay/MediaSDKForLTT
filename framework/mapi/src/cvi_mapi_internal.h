#ifndef __CVI_MAPI_INTERNAL_H__
#define __CVI_MAPI_INTERNAL_H__

// #include "sample_comm.h"
#include "cvi_vpss.h"
#define MAPI_ALIGN(value, base) (((value)+(base) - 1)/(base)*(base))

CVI_S32 Vproc_Init(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
                  VPSS_CHN_ATTR_S *pastVpssChnAttr, uint32_t *pAttachVbCnt, bool *fbOnVpss);
CVI_VOID Vproc_Deinit(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, bool *fbOnVpss);
uint32_t get_frame_size(uint32_t w, uint32_t h, PIXEL_FORMAT_E fmt);
#endif
