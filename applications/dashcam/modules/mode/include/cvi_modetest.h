#ifndef __CVI_CC_TOOL_H__
#define __CVI_CC_TOOL_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "cvi_mapi.h"
#include "cvi_mq.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define CVI_CMD_CLIENT_ID_MT_TOOL (CVI_MQ_CLIENT_ID_USER_0)
#define CVI_CMD_CHANNEL_ID_MT(mt_id) (0x00 + (mt_id))

typedef struct CVI_MT_SERVICE_PARAM_S {
    uint32_t    parm;
} CVI_MT_SERVICE_PARAM_T;

typedef void * CVI_MT_HANDLE_T;
typedef CVI_MT_SERVICE_PARAM_T mt_param_t, *mt_param_handle_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
