#ifndef __CVI_QUEUE_H__
#define __CVI_QUEUE_H__

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef void * CVI_QUEUE_HANDLE_T;

CVI_QUEUE_HANDLE_T CVI_QUEUE_Create(uint32_t nodeSize, uint32_t maxLen);

void CVI_QUEUE_Destroy(CVI_QUEUE_HANDLE_T queueHdl);

void CVI_QUEUE_Clear(CVI_QUEUE_HANDLE_T queueHdl);

int32_t CVI_QUEUE_GetLen(CVI_QUEUE_HANDLE_T queueHdl);

int32_t CVI_QUEUE_Push(CVI_QUEUE_HANDLE_T queueHdl, const void *node);

int32_t CVI_QUEUE_Pop(CVI_QUEUE_HANDLE_T queueHdl, void *node);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  // __CVI_QUEUE_H__
