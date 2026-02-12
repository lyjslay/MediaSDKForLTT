
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "cvi_queue.h"
#include "cvi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct tagQUEUE_S {
    pthread_mutex_t mutex;
    int32_t frontIdx;
    int32_t rearIdx;
    int32_t curLen;
    int32_t maxLen;
    int32_t nodeSize;
    void **node;
} QUEUE_S;

/*create queue and malloc memory*/
CVI_QUEUE_HANDLE_T CVI_QUEUE_Create(uint32_t nodeSize, uint32_t maxLen)
{
    QUEUE_S *queue = malloc(sizeof(QUEUE_S));

    if (!queue) {
        return 0;
    }

    int32_t i = 0;
    queue->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    queue->frontIdx = 0;
    queue->rearIdx = 0;
    queue->curLen = 0;
    queue->maxLen = maxLen;
    queue->nodeSize = nodeSize;
    queue->node = (void **)malloc(sizeof(void *) * maxLen);

    for (i = 0; i < queue->maxLen; i++) {
        queue->node[i] = (void *)malloc(nodeSize);
    }

    return (CVI_QUEUE_HANDLE_T)queue;
}

void CVI_QUEUE_Destroy(CVI_QUEUE_HANDLE_T queueHdl)
{
    if (queueHdl == 0) {
        printf("queueHdl is NULL!\n");
        return;
    }

    QUEUE_S *queue = (QUEUE_S *)queueHdl;
    CVI_MUTEX_LOCK(queue->mutex);
    int32_t i = 0;

    for (i = 0; i < queue->maxLen; i++) {
        CVI_APPCOMM_SAFE_FREE(queue->node[i]);
    }

    CVI_APPCOMM_SAFE_FREE(queue->node);
    CVI_MUTEX_UNLOCK(queue->mutex);
    CVI_MUTEX_DESTROY(queue->mutex);
    CVI_APPCOMM_SAFE_FREE(queue);
}

void CVI_QUEUE_Clear(CVI_QUEUE_HANDLE_T queueHdl)
{
    if (queueHdl == 0) {
        printf("queueHdl is NULL!\n");
        return;
    }

    QUEUE_S *queue = (QUEUE_S *)queueHdl;
    CVI_MUTEX_LOCK(queue->mutex);
    queue->curLen = 0;
    queue->frontIdx = 0;
    queue->rearIdx = 0;
    CVI_MUTEX_UNLOCK(queue->mutex);
}

int32_t CVI_QUEUE_GetLen(CVI_QUEUE_HANDLE_T queueHdl)
{
    if (queueHdl == 0) {
        printf("queueHdl is NULL!\n");
        return -1;
    }

    QUEUE_S *queue = (QUEUE_S *)queueHdl;
    return (int32_t)queue->curLen;
}

int32_t CVI_QUEUE_Push(CVI_QUEUE_HANDLE_T queueHdl, const void *node)
{
    if (queueHdl == 0) {
        printf("queueHdl is NULL!\n");
        return CVI_EINVAL;
    }

    QUEUE_S *queue = (QUEUE_S *)queueHdl;
    CVI_MUTEX_LOCK(queue->mutex);

    if (queue->curLen >= queue->maxLen) {
        CVI_MUTEX_UNLOCK(queue->mutex);
        printf("queue is full!\n");
        return CVI_EFULL;
    }

    if (node) {
        memcpy(queue->node[queue->rearIdx], node, queue->nodeSize);
    }

    queue->curLen++;
    queue->rearIdx = (queue->rearIdx + 1) % queue->maxLen;
    CVI_MUTEX_UNLOCK(queue->mutex);
    return 0;
}

int32_t CVI_QUEUE_Pop(CVI_QUEUE_HANDLE_T queueHdl, void *node)
{
    if (queueHdl == 0) {
        printf("queueHdl is NULL!\n");
        return CVI_EINTER;
    }

    QUEUE_S *queue = (QUEUE_S *)queueHdl;
    CVI_MUTEX_LOCK(queue->mutex);

    if (queue->curLen == 0) {
        CVI_MUTEX_UNLOCK(queue->mutex);
        printf("queue is empity!\n");
        return CVI_EEMPTY;
    }

    if (node) {
        memcpy(node, queue->node[queue->frontIdx], queue->nodeSize);
    }

    queue->curLen--;
    queue->frontIdx = (queue->frontIdx + 1) % queue->maxLen;
    CVI_MUTEX_UNLOCK(queue->mutex);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
