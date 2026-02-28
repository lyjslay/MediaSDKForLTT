
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include "cvi_log.h"
#include "cvi_rbuf.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct CVI_RBUF_S{
    unsigned long addr;
    uint32_t size;
    char name[64];
    unsigned long rpos;
    uint64_t rcnt;
    unsigned long wpos;
    uint64_t wcnt;
    CVI_RBUF_MALLOC_CB rbuf_malloc_cb;
    CVI_RBUF_FREE_CB rbuf_free_cb;
}CVI_RBUF_T;


void CVI_RBUF_Destroy(void *rbuf)
{
    CVI_RBUF_T *buf = (CVI_RBUF_T *)rbuf;
    if(buf == NULL)
    {
        return;
    }

    if(buf->addr) {
        buf->rbuf_free_cb((void *)buf->addr);
    }
    free((void *)buf);
}

int32_t CVI_RBUF_Create(void **rbuf, uint32_t size, const char *name, void* mallocmemcb, void* freememcb)
{
    CVI_RBUF_T *buf = (CVI_RBUF_T *)malloc(sizeof(CVI_RBUF_T));

    if(buf == NULL) {
        *rbuf = NULL;
        CVI_LOGE("out of mem size %d name %s", size, name);
        return -1;
    }
    memset(buf, 0x0, sizeof(CVI_RBUF_T));
    buf->rbuf_malloc_cb = (CVI_RBUF_MALLOC_CB)mallocmemcb;
    buf->rbuf_free_cb = (CVI_RBUF_FREE_CB)freememcb;

    int32_t s = CVI_ALIGN_UP(size, RINGBUF_ALIGN_SIZE);
    if(buf->rbuf_malloc_cb == NULL || buf->rbuf_free_cb == NULL) {
        CVI_LOGE("rbuf malloc/free callback is NULL !!!");
        return -1;
    }
    buf->addr = (unsigned long)buf->rbuf_malloc_cb(s, name);
    if(!buf->addr) {
        CVI_LOGE("rbuf create failed! the args size %d name %s", size, name);
        CVI_RBUF_Destroy((void *)buf);
    }
    buf->size = s;
    buf->wpos = buf->addr;
    buf->rpos = buf->addr;
    strncpy(buf->name, name, sizeof(buf->name) - 1);
    *rbuf = buf;
    return 0;
}

uint32_t CVI_RBUF_Get_RemainSize(void *rbuf)
{
    CVI_RBUF_T *buf = (CVI_RBUF_T *)rbuf;
    if(buf == NULL || buf->addr == 0) {
        return 0;
    }

    if(buf->wcnt != buf->rcnt && buf->wpos == buf->rpos){
        return 0;
    }

    if(buf->wpos == buf->rpos) {
        return buf->size;
    }

    if(buf->wpos > buf->rpos) {
        return buf->size - (buf->wpos - buf->rpos);
    } else {
        return buf->rpos - buf->wpos;
    }
}

void *CVI_RBUF_Req_Mem(void *rbuf, uint32_t size)
{
    CVI_RBUF_T *buf = (CVI_RBUF_T *)rbuf;
    if(buf == NULL || buf->addr == 0) {
        return NULL;
    }

    void *mem = NULL;
    if(buf->wcnt == 0 && size <= buf->size) {
        buf->wcnt++;
        mem = (void *)buf->addr;
        buf->wpos = buf->addr + size;
    }else {
        if(buf->wpos > buf->rpos) {
            if(buf->size + buf->addr - buf->wpos >= size) {
                buf->wcnt++;
                mem = (void *)buf->wpos;
                buf->wpos += size;
            } else {
                ((char *)buf->wpos)[0] = 0x00;
                if(buf->rpos > size) {
                    buf->wcnt++;
                    mem = (void *)buf->addr;
                    buf->wpos = buf->addr + size;
                }
            }
        } else if(buf->wpos < buf->rpos) {
            if(buf->rpos - buf->wpos > size) {
                buf->wcnt++;
                mem = (void *)buf->wpos;
                buf->wpos += size;
            }
        } else {

        }
    }

    return mem;
}


int32_t CVI_RBUF_Refresh_WritePos(void *rbuf, uint32_t offs)
{
    CVI_RBUF_T *buf = (CVI_RBUF_T *)rbuf;
    if(buf == NULL || buf->addr == 0) {
        return -1;
    }
    (void)offs;
    return 0;
}


void *CVI_RBUF_ReadData(void *rbuf, CVI_RBUF_RECORD_TYPE_E type)
{
    (void)type;
    CVI_RBUF_T *buf = (CVI_RBUF_T *)rbuf;
    if(buf == NULL || buf->addr == 0) {
        return NULL;
    }

    if(buf->wcnt == buf->rcnt) {
        return NULL;
    }

    void *mem = NULL;
    if(((unsigned char *)buf->rpos)[0] != 0x5a) {
        buf->rpos = buf->addr;
        mem = (void *)buf->addr;
    } else {
        mem = (void *)buf->rpos;
    }

    return mem;
}

int32_t CVI_RBUF_Refresh_ReadPos(void *rbuf, uint32_t offs, CVI_RBUF_RECORD_TYPE_E type)
{
    (void)type;
    CVI_RBUF_T *buf = (CVI_RBUF_T *)rbuf;
    if(buf == NULL || buf->addr == 0) {
        return -1;
    }

    if(buf->rpos + offs > buf->addr + buf->size) {
        buf->rpos = buf->addr + offs;
    } else {
        buf->rpos += offs;
    }
    buf->rcnt++;

    return 0;
}

uint64_t CVI_RBUF_Get_DataCnt(void *rbuf)
{
    CVI_RBUF_T *buf = (CVI_RBUF_T *)rbuf;
    if(buf == NULL || buf->addr == 0) {
        return 0;
    }
    return buf->wcnt - buf->rcnt;
}

void CVI_RBUF_ShowMeminfo(void *rbuf)
{
    CVI_RBUF_T *buf = (CVI_RBUF_T *)rbuf;
    if(buf == NULL || buf->addr == 0) {
        return;
    }
    uint32_t rmsize = CVI_RBUF_Get_RemainSize(rbuf);
    CVI_LOGD("%p wcnt %"PRIu64", rcnt %"PRIu64", wpos %#lx, rpos %#lx, bsize %u, rsize %u", rbuf, buf->wcnt, buf->rcnt, buf->wpos, buf->rpos, buf->size, rmsize);
}

#define RINGBUF_OUTPTR_CNT 4

typedef struct _CVI_RBUF_INFO_S{
    int32_t size;
    uint64_t in;
    uint64_t inCnt;
    uint64_t out[RINGBUF_OUTPTR_CNT];
    uint64_t outTmp[RINGBUF_OUTPTR_CNT];
    uint64_t outCnt[RINGBUF_OUTPTR_CNT];
    int32_t used;
    void *data;
    char name[64];
    CVI_RBUF_MALLOC_CB rbuf_malloc_cb;
    CVI_RBUF_FREE_CB rbuf_free_cb;
}CVI_RBUF_INFO_S;

void CVI_RBUF_ShowLog(void *rbuf)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)rbuf;
    if(buf){
        char log[256] = {0};
        size_t len = snprintf(log, sizeof(log), "%p %08d %016"PRIu64" %08"PRIu64"", rbuf, buf->size, buf->in, buf->inCnt);
        for(int32_t i = 0; i < buf->used; i++){
            len += snprintf(log + len, sizeof(log) - len, " %016"PRIu64" %08"PRIu64"", buf->out[i], buf->outCnt[i]);
        }
        CVI_LOGI("%s", log);
    }
}

int32_t CVI_RBUF_Reset(void *rbuf)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)rbuf;
    if(buf) {
        buf->in = 0;
        buf->inCnt = 0;
        for(int32_t i = 0; i < RINGBUF_OUTPTR_CNT; i++){
            buf->out[i] = 0;
            buf->outTmp[i] = 0;
            buf->outCnt[i] = 0;
        }
    }
    return 0;
}

int32_t CVI_RBUF_Init(void **rbuf, int32_t size, const char *name, int32_t outcnt, void * mallocmemcb, void * freememcb)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)malloc(sizeof(CVI_RBUF_INFO_S));
    if(buf == NULL) {
        *rbuf = NULL;
        CVI_LOGE("out of mem size %d name %s", size, name);
        return -1;
    }
    memset(buf, 0x0, sizeof(CVI_RBUF_INFO_S));
    buf->rbuf_malloc_cb = (CVI_RBUF_MALLOC_CB)mallocmemcb;
    buf->rbuf_free_cb = (CVI_RBUF_FREE_CB)freememcb;

    int32_t s = CVI_ALIGN_UP(size, RINGBUF_ALIGN_SIZE);
    if(buf->rbuf_malloc_cb == NULL || buf->rbuf_free_cb == NULL) {
        CVI_LOGE("rbuf malloc/free callback is NULL !!!");
        return -1;
    }
    buf->data = buf->rbuf_malloc_cb(s,name);
    if(!buf->data) {
        CVI_LOGE("rbuf create failed! the args size %d name %s", size, name);
        CVI_RBUF_DeInit((void *)buf);
        return -1;
    }
    strncpy(buf->name, name, sizeof(buf->name) - 1);
    buf->used = outcnt;
    CVI_RBUF_Reset((void *)buf);
    buf->size = s;
    *rbuf = buf;
    CVI_LOGD("rbuf %s is created, size %d", buf->name, buf->size);
    return 0;
}

void CVI_RBUF_DeInit(void *rbuf)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)rbuf;
    if(buf) {
        if(buf->data) {
            if(buf->rbuf_free_cb != NULL) {
                buf->rbuf_free_cb((void *)buf->data);
            } else {
                CVI_LOGE("rbuf free callback is NULL !!!");
            }

        }
        CVI_LOGD("rbuf %s is free, size %d", buf->name, buf->size);
        free(buf);
    }
}

uint32_t CVI_RBUF_Unused(void *rbuf)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)rbuf;
    if(buf) {
        uint64_t min = buf->out[0];
        for(int32_t i = 0; i < buf->used; i++){
            if(min > buf->out[i]){
                min = buf->out[i];
            }
        }
        return buf->size - (buf->in - min);
    }
    return 0;
}

int32_t CVI_RBUF_Get_Totalsize(void *rbuf)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)rbuf;
    if(buf) {
        return buf->size;
    }
    return 0;
}

uint32_t CVI_RBUF_DataCnt(void *rbuf, int32_t type)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)rbuf;
    if(!buf || type >= buf->used){
        return 0;
    }
    return buf->inCnt - buf->outCnt[type];
}

uint64_t CVI_RBUF_Get_InSize(void *rbuf)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)rbuf;
    if(buf) {
        return buf->in;
    }
    return 0;
}

int32_t CVI_RBUF_Copy_In(void *rbuf, void *src, int32_t len, int32_t off)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)rbuf;
    if(buf && src) {
        if(CVI_RBUF_Unused(rbuf) < (uint32_t)len) {
            return -1;
        }
        uint32_t ll = (buf->in + off) % buf->size;
        int32_t l = ((buf->size - ll) < (uint32_t)len) ? (buf->size - ll) : (uint32_t)len;
        memcpy((char *)buf->data + ll, src, l);
        memcpy(buf->data, (char *)src + l, len - l);
        return 0;
    }
    return -1;
}

void CVI_RBUF_Refresh_In(void *rbuf, int32_t off)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)rbuf;
    if(buf) {
        buf->in += off;
        buf->inCnt++;
    }
}

void CVI_RBUF_Refresh_Out(void *rbuf, int32_t off, int32_t inx)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)rbuf;
    if(buf && inx < buf->used) {
        buf->out[inx] += off;
        buf->outTmp[inx] = buf->out[inx];
        buf->outCnt[inx]++;
    }
}


int32_t CVI_RBUF_Copy_Out(void *rbuf, void *dst, int32_t len, int32_t off, int32_t inx)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)rbuf;
    if(buf && dst && inx < buf->used) {
        if(buf->in - buf->out[inx] - off < (uint32_t)len) {
            return -1;
        }
        uint32_t ll = (buf->out[inx] + off) % buf->size;
        int32_t l = ((buf->size - ll) < (uint32_t)len) ? (buf->size - ll) : (uint32_t)len;
        memcpy(dst, (char *)buf->data + ll, l);
        memcpy((char *)dst + l, buf->data, len - l);
        return 0;
    }
    CVI_LOGE("invalid argument %p %p", buf, dst);
    return -1;
}

int32_t CVI_RBUF_Copy_OutTmp(void *rbuf, void *dst, int32_t len, int32_t off, int32_t inx)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)rbuf;
    if(buf && dst && inx < buf->used){
        if(buf->in - buf->outTmp[inx] - off < (uint32_t)len) {
            return -1;
        }

        uint32_t ll = (buf->outTmp[inx] + off) % buf->size;
        int32_t l = ((buf->size - ll) < (uint32_t)len) ? (buf->size - ll) : (uint32_t)len;
        memcpy(dst, (char *)buf->data + ll, l);
        memcpy((char *)dst + l, buf->data, len - l);
        return 0;
    }
    return -1;
}

void CVI_RBUF_Refresh_OutTmp(void *rbuf, int32_t off, int32_t inx)
{
    CVI_RBUF_INFO_S *buf = (CVI_RBUF_INFO_S *)rbuf;
    if(buf && inx < buf->used) {
        buf->outTmp[inx] += off;
    }
}

#ifdef __cplusplus
}
#endif

