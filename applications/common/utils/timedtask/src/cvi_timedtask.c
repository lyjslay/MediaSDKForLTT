#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "cvi_timedtask.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define INVALID_TIMER_GROUP (-1)

/** time task max number */
#define TIMEDTASK_MAX_NUM (16)

/** check interval in usec */
#define TASK_CHECK_INTERVAL (300000)

/** timed-task check manage struct */
typedef struct tagTIMEDTASK_MNG_S {
    bool bUsed;
    CVI_TIMER_HANDLE_T timerHdl;
    pthread_mutex_t mutex;
    CVI_TIMEDTASK_CFG_S stCfg;
} TIMEDTASK_MNG_S;
static TIMEDTASK_MNG_S s_stTIMEDTSKMng[TIMEDTASK_MAX_NUM];

/** timer group id */
static int32_t g_timerGroup = INVALID_TIMER_GROUP;

void TIMEDTASK_Proc(void *data, struct timespec *now)
{
    TIMEDTASK_MNG_S *ctx = (TIMEDTASK_MNG_S *)data;
    CVI_MUTEX_LOCK(ctx->mutex);
    /* no-periodic timer auto destroy */
    if (ctx->stCfg.stAttr.periodic == false) {
        ctx->timerHdl = NULL;
    }
    CVI_MUTEX_UNLOCK(ctx->mutex);
    ctx->stCfg.timerProc(ctx->stCfg.pvPrivData, now);
}

int32_t CVI_TIMEDTASK_Init(void)
{
    CVI_APPCOMM_CHECK_EXPR(g_timerGroup == INVALID_TIMER_GROUP, CVI_EINITIALIZED);
    g_timerGroup = CVI_Timer_Init(false);
    if (g_timerGroup == -1) {
        CVI_LOGE("Create Timer Group Failed\n");
        return -1;
    }
    CVI_Timer_SetTickValue(g_timerGroup, TASK_CHECK_INTERVAL);
    uint32_t s32Idx = 0;
    for (s32Idx = 0; s32Idx < TIMEDTASK_MAX_NUM; ++s32Idx) {
        CVI_MUTEX_INIT_LOCK(s_stTIMEDTSKMng[s32Idx].mutex);
        CVI_MUTEX_LOCK(s_stTIMEDTSKMng[s32Idx].mutex);
        s_stTIMEDTSKMng[s32Idx].bUsed = false;
        s_stTIMEDTSKMng[s32Idx].timerHdl = NULL;
        memset(&s_stTIMEDTSKMng[s32Idx].stCfg, 0, sizeof(CVI_TIMEDTASK_CFG_S));
        CVI_MUTEX_UNLOCK(s_stTIMEDTSKMng[s32Idx].mutex);
    }
    return 0;
}

int32_t CVI_TIMEDTASK_DeInit(void)
{
    CVI_APPCOMM_CHECK_EXPR(g_timerGroup != INVALID_TIMER_GROUP, CVI_ENOINIT);
    CVI_Timer_DeInit(g_timerGroup);
    g_timerGroup = INVALID_TIMER_GROUP;
    uint32_t s32Idx = 0;
    for (s32Idx = 0; s32Idx < TIMEDTASK_MAX_NUM; ++s32Idx) {
        CVI_MUTEX_DESTROY(s_stTIMEDTSKMng[s32Idx].mutex);
    }
    return 0;
}

int32_t CVI_TIMEDTASK_Create(const CVI_TIMEDTASK_CFG_S *pstTimeTskCfg, uint32_t *pTimeTskid)
{
    uint32_t s32Idx = 0;
    CVI_APPCOMM_CHECK_EXPR(g_timerGroup != INVALID_TIMER_GROUP, CVI_ENOINIT);
    CVI_APPCOMM_CHECK_POINTER(pstTimeTskCfg, CVI_EINVAL);
    CVI_APPCOMM_CHECK_POINTER(pTimeTskid, CVI_EINVAL);
    if (pstTimeTskCfg->stAttr.bEnable) {
        CVI_APPCOMM_CHECK_EXPR(pstTimeTskCfg->stAttr.u32Time_sec != 0, CVI_EINVAL);
    }
    CVI_APPCOMM_CHECK_EXPR(pstTimeTskCfg->timerProc != 0, CVI_EINVAL);
    for (s32Idx = 0; s32Idx < TIMEDTASK_MAX_NUM; ++s32Idx) {
        CVI_MUTEX_LOCK(s_stTIMEDTSKMng[s32Idx].mutex);
        if (false == s_stTIMEDTSKMng[s32Idx].bUsed) {
            if (pstTimeTskCfg->stAttr.bEnable == true) {
                CVI_TIMER_S timerConf;
                timerConf.now = NULL;
                timerConf.interval_ms = pstTimeTskCfg->stAttr.u32Time_sec * 1000;
                timerConf.periodic = pstTimeTskCfg->stAttr.periodic;
                if (timerConf.periodic == true) {
                    timerConf.timer_proc = pstTimeTskCfg->timerProc;
                    timerConf.clientData = pstTimeTskCfg->pvPrivData;
                } else {
                    timerConf.timer_proc = TIMEDTASK_Proc;
                    timerConf.clientData = &s_stTIMEDTSKMng[s32Idx];
                }
                s_stTIMEDTSKMng[s32Idx].timerHdl = CVI_Timer_Create(g_timerGroup, &timerConf);
                if (s_stTIMEDTSKMng[s32Idx].timerHdl == NULL) {
                    CVI_MUTEX_UNLOCK(s_stTIMEDTSKMng[s32Idx].mutex);
                    CVI_LOGE("Create Timer Task Failed\n");
                    return -1;
                }
            } else {
                s_stTIMEDTSKMng[s32Idx].timerHdl = NULL;
            }
            memcpy(&s_stTIMEDTSKMng[s32Idx].stCfg, pstTimeTskCfg, sizeof(CVI_TIMEDTASK_CFG_S));
            s_stTIMEDTSKMng[s32Idx].bUsed = true;
            *pTimeTskid = s32Idx;
            CVI_MUTEX_UNLOCK(s_stTIMEDTSKMng[s32Idx].mutex);
            return 0;
        }
        CVI_MUTEX_UNLOCK(s_stTIMEDTSKMng[s32Idx].mutex);
    }
    return CVI_ENORES;
}

int32_t CVI_TIMEDTASK_Destroy(uint32_t TimeTskid)
{
    CVI_APPCOMM_CHECK_EXPR(g_timerGroup != INVALID_TIMER_GROUP, CVI_ENOINIT);
    CVI_APPCOMM_CHECK_EXPR(TimeTskid < TIMEDTASK_MAX_NUM, CVI_EINVAL);
    CVI_MUTEX_LOCK(s_stTIMEDTSKMng[TimeTskid].mutex);
    if (false == s_stTIMEDTSKMng[TimeTskid].bUsed) {
        CVI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskid].mutex);
        return CVI_EINVAL;
    }
    if (s_stTIMEDTSKMng[TimeTskid].timerHdl != NULL) {
        CVI_Timer_Destroy(g_timerGroup, s_stTIMEDTSKMng[TimeTskid].timerHdl);
        s_stTIMEDTSKMng[TimeTskid].timerHdl = NULL;
    }
    s_stTIMEDTSKMng[TimeTskid].bUsed = false;
    memset(&s_stTIMEDTSKMng[TimeTskid].stCfg.stAttr, 0, sizeof(CVI_TIMEDTASK_ATTR_S));
    CVI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskid].mutex);
    return 0;
}

int32_t CVI_TIMEDTASK_GetAttr(uint32_t TimeTskid, CVI_TIMEDTASK_ATTR_S *pstTimeTskAttr)
{
    CVI_APPCOMM_CHECK_EXPR(g_timerGroup != INVALID_TIMER_GROUP, CVI_ENOINIT);
    CVI_APPCOMM_CHECK_POINTER(pstTimeTskAttr, CVI_EINVAL);
    CVI_APPCOMM_CHECK_EXPR(TimeTskid < TIMEDTASK_MAX_NUM, CVI_EINVAL);
    CVI_MUTEX_LOCK(s_stTIMEDTSKMng[TimeTskid].mutex);
    if (false == s_stTIMEDTSKMng[TimeTskid].bUsed) {
        CVI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskid].mutex);
        return CVI_EINVAL;
    }
    memcpy(pstTimeTskAttr, &s_stTIMEDTSKMng[TimeTskid].stCfg.stAttr, sizeof(CVI_TIMEDTASK_ATTR_S));
    CVI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskid].mutex);
    return 0;
}

int32_t CVI_TIMEDTASK_SetAttr(uint32_t TimeTskid, const CVI_TIMEDTASK_ATTR_S *pstTimeTskAttr)
{
    CVI_APPCOMM_CHECK_EXPR(g_timerGroup != INVALID_TIMER_GROUP, CVI_ENOINIT);
    CVI_APPCOMM_CHECK_POINTER(pstTimeTskAttr, CVI_EINVAL);
    if (pstTimeTskAttr->bEnable) {
        CVI_APPCOMM_CHECK_EXPR(pstTimeTskAttr->u32Time_sec != 0, CVI_EINVAL);
    }
    CVI_APPCOMM_CHECK_EXPR(TimeTskid < TIMEDTASK_MAX_NUM, CVI_EINVAL);
    CVI_MUTEX_LOCK(s_stTIMEDTSKMng[TimeTskid].mutex);
    if (false == s_stTIMEDTSKMng[TimeTskid].bUsed) {
        CVI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskid].mutex);
        return CVI_EINVAL;
    }
    if (s_stTIMEDTSKMng[TimeTskid].timerHdl != NULL) {
        if (pstTimeTskAttr->bEnable) {
            struct timespec now = { 0, 0 };
            clock_gettime(CLOCK_MONOTONIC, &now);
            CVI_Timer_Reset(g_timerGroup, s_stTIMEDTSKMng[TimeTskid].timerHdl, &now, (pstTimeTskAttr->u32Time_sec * 1000));
        } else {
            CVI_Timer_Destroy(g_timerGroup, s_stTIMEDTSKMng[TimeTskid].timerHdl);
            s_stTIMEDTSKMng[TimeTskid].timerHdl = NULL;
        }
    } else {
        if (pstTimeTskAttr->bEnable) {
            CVI_TIMER_S timerConf;
            timerConf.now = NULL;
            timerConf.interval_ms = pstTimeTskAttr->u32Time_sec * 1000;
            timerConf.periodic = pstTimeTskAttr->periodic;
            if (timerConf.periodic == true) {
                timerConf.timer_proc = s_stTIMEDTSKMng[TimeTskid].stCfg.timerProc;
                timerConf.clientData = s_stTIMEDTSKMng[TimeTskid].stCfg.pvPrivData;
            } else {
                timerConf.timer_proc = TIMEDTASK_Proc;
                timerConf.clientData = &s_stTIMEDTSKMng[TimeTskid];
            }
            s_stTIMEDTSKMng[TimeTskid].timerHdl = CVI_Timer_Create(g_timerGroup, &timerConf);
            if (s_stTIMEDTSKMng[TimeTskid].timerHdl == NULL) {
                CVI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskid].mutex);
                CVI_LOGE("Create Timer Task Failed\n");
                return -1;
            }
        }
    }
    memcpy(&s_stTIMEDTSKMng[TimeTskid].stCfg.stAttr, pstTimeTskAttr, sizeof(CVI_TIMEDTASK_ATTR_S));
    CVI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskid].mutex);
    return 0;
}

int32_t CVI_TIMEDTASK_ResetTime(uint32_t TimeTskid)
{
    CVI_APPCOMM_CHECK_EXPR(g_timerGroup != INVALID_TIMER_GROUP, CVI_ENOINIT);
    CVI_APPCOMM_CHECK_EXPR(TimeTskid < TIMEDTASK_MAX_NUM, CVI_EINVAL);
    CVI_MUTEX_LOCK(s_stTIMEDTSKMng[TimeTskid].mutex);
    if (false == s_stTIMEDTSKMng[TimeTskid].bUsed) {
        CVI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskid].mutex);
        return CVI_EINVAL;
    }
    if (s_stTIMEDTSKMng[TimeTskid].timerHdl != NULL) {
        struct timespec now = { 0, 0 };
        clock_gettime(CLOCK_MONOTONIC, &now);
        CVI_Timer_Reset(g_timerGroup, s_stTIMEDTSKMng[TimeTskid].timerHdl, &now,
                       (s_stTIMEDTSKMng[TimeTskid].stCfg.stAttr.u32Time_sec * 1000));
    }
    CVI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskid].mutex);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

