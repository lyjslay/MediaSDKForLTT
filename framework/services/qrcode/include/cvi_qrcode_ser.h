#ifndef _CVI_QRCODE_SER_H
#define _CVI_QRCODE_SER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <sys/select.h>
#include <inttypes.h>

#include "cvi_mapi.h"

    typedef struct cviQRCODE_PARAM_S
    {
        uint32_t w;
        uint32_t h;
        CVI_MAPI_VPROC_HANDLE_T vproc;
        uint32_t vproc_chnid;
    } CVI_QRCODE_SERVICE_PARAM_S;

    typedef void *CVI_QRCODE_SERVICE_HANDLE_T;

    int32_t CVI_QRCode_Service_Create(CVI_QRCODE_SERVICE_HANDLE_T *hdl, CVI_QRCODE_SERVICE_PARAM_S *attr);
    int32_t CVI_QRCode_Service_Destroy(CVI_QRCODE_SERVICE_HANDLE_T hdl);

#ifdef __cplusplus
}
#endif
#endif
