#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/prctl.h>
#include <inttypes.h>

#include "cvi_eventhub.h"
#include "cvi_filemng_dtcf.h"
#include "cvi_storagemng.h"
#include "filemng_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define SPACEMONITOR_DEFAULT_INVL (5)
#define FILEMNG_INVAILD_SPACE_THRESHOLD (50)

// static pthread_t s_SMThread = 0;
cvi_osal_task_handle_t s_SMThread;
static bool s_SMThreadStart = false;
static pthread_mutex_t g_SMCheckMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_SMExitMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_SMCheckCond;
static pthread_cond_t g_SMExitCond;
static struct timespec s_SMLasttime = { .tv_sec = 0 };
static SPACEMONITOR_CFG_S s_stSMCfg;
/**<0x000:space enough;0x001:total space full;0x010:movie space full;0x100:emr movie space full */
static int32_t s_s32SMFullFlag = 0;

static uint32_t s_u32SMMovieSpace = 0;
static uint32_t s_u32SMEmrSpace = 0;

static void FILEMNG_SPACEMONITOR_CheckMovie(uint32_t u32TotalSize_MB)
{
    if ((s_u32SMMovieSpace) >= (s_stSMCfg.u8SharePercent * u32TotalSize_MB / 100)) {
        s_s32SMFullFlag |= SPACEMONITOR_MASK_MOVIEFULL;
        CVI_LOGW("movie space full(used:%u warning:%u ratio:%u)\n", s_u32SMMovieSpace, s_stSMCfg.u32WarningStage,
              (s_stSMCfg.u8SharePercent * u32TotalSize_MB / 100));
    } else {
        s_s32SMFullFlag = s_s32SMFullFlag & ~SPACEMONITOR_MASK_MOVIEFULL;
    }
}

static void FILEMNG_SPACEMONITOR_CheckEmr(uint32_t u32TotalSize_MB)
{
    if ((s_u32SMEmrSpace) >= (s_stSMCfg.u32WarningStage)) {
        s_s32SMFullFlag |= SPACEMONITOR_MASK_EMRFULL;
        CVI_LOGW("emr movie space full(used:%u warning:%u ratio:%u)\n", s_u32SMEmrSpace, s_stSMCfg.u32WarningStage,
              ((100 - s_stSMCfg.u8SharePercent) * u32TotalSize_MB / 100));
    } else {
        s_s32SMFullFlag = s_s32SMFullFlag & ~SPACEMONITOR_MASK_EMRFULL;
    }
}

static void FILEMNG_SPACEMONITOR_CheckTotal(uint32_t u32AvailableSize_MB, uint32_t u32TotalSize_MB)
{
    if (u32AvailableSize_MB < ((100 - s_stSMCfg.u8SharePercent) * u32TotalSize_MB / 100)) {
        s_s32SMFullFlag |= SPACEMONITOR_MASK_TOTALFULL;
        CVI_LOGW("total space full(warning:%u free:%u)\n", s_stSMCfg.u32WarningStage, u32AvailableSize_MB);
    } else {
        s_s32SMFullFlag = s_s32SMFullFlag & ~SPACEMONITOR_MASK_TOTALFULL;
    }
}

static void FILEMNG_SPACEMONITOR_SpaceCheckThread(void *pData)
{
    int32_t s32Ret = 0;
    // prctl(PR_SET_NAME, "SpaceMonitor", 0, 0, 0);
    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_FILEMNG_BUTT;
    CVI_STG_FS_INFO_S stFSInfo;
    uint32_t u32CheckDelay = s_stSMCfg.u32MaxCheckDelay;

    while (s_stSMCfg.u32Interval) {
        CVI_MUTEX_LOCK(g_SMExitMutex);
        s32Ret = CVI_STORAGEMNG_GetFSInfo(&stFSInfo);
        uint32_t u32AvailableSize_MB = stFSInfo.u64AvailableSize >> 20;
        uint32_t u32TotalSize_MB = stFSInfo.u64TotalSize >> 20;
        if ((0 == s32Ret) && (s_stSMCfg.bStopCover == true)) {
            if (0 != s_stSMCfg.u8SharePercent && NULL != s_stSMCfg.pfnGetRatioSpace) {
                /* ratio mode */
                s32Ret = s_stSMCfg.pfnGetRatioSpace(&s_u32SMMovieSpace, &s_u32SMEmrSpace);
                if (0 == s32Ret) {
                    FILEMNG_SPACEMONITOR_CheckMovie(u32TotalSize_MB);
                    FILEMNG_SPACEMONITOR_CheckEmr(u32TotalSize_MB);
                }
            }
            FILEMNG_SPACEMONITOR_CheckTotal(u32AvailableSize_MB, u32TotalSize_MB);
            if (SPACEMONITOR_MASK_ENOUGH != s_s32SMFullFlag) {
                if (CVI_EVENT_FILEMNG_SPACE_FULL != stEvent.topic) {
                    stEvent.topic = CVI_EVENT_FILEMNG_SPACE_FULL;
                    CVI_EVENTHUB_Publish(&stEvent);
                }

                if (s_stSMCfg.u32GuaranteedStage >= s_stSMCfg.u32WarningStage && NULL != s_stSMCfg.pfnCoverCB) {
                    s32Ret = s_stSMCfg.pfnCoverCB(s_s32SMFullFlag);
                    if (0 == s32Ret) {
                        u32CheckDelay = s_stSMCfg.u32MaxCheckDelay;
                        CVI_MUTEX_UNLOCK(g_SMExitMutex);
                        continue;
                    } else if (-1 == s32Ret) {
                        u32CheckDelay = 0;
                    }
                }
            } else {
                if (CVI_EVENT_FILEMNG_SPACE_ENOUGH != stEvent.topic) {
                    stEvent.topic = CVI_EVENT_FILEMNG_SPACE_ENOUGH;
                    CVI_EVENTHUB_Publish(&stEvent);
                }
            }
        }

        CVI_MUTEX_LOCK(g_SMCheckMutex);
        CVI_COND_WAIT(g_SMCheckCond, g_SMCheckMutex);
        CVI_MUTEX_UNLOCK(g_SMCheckMutex);

        CVI_COND_TIMEDWAIT(g_SMExitCond, g_SMExitMutex, u32CheckDelay * 1000 * 1000);
        CVI_MUTEX_UNLOCK(g_SMExitMutex);
    }

    CVI_LOGD("SpaceMonitor thread exit\n");
    return ;
}

int32_t FILEMNG_SPACEMONITOR_Create(const SPACEMONITOR_CFG_S *pstConfig)
{
    int32_t s32Ret = 0;
    memset(&s_stSMCfg, 0, sizeof(SPACEMONITOR_CFG_S));
    if (0 == pstConfig->u32WarningStage) {
        return 0;
    }
    if (pstConfig->u8SharePercent >= 100) {
        return CVI_FILEMNG_EINVAL;
    }
    if (s_SMThreadStart == false) {
        if (clock_gettime(CLOCK_MONOTONIC, &s_SMLasttime) < 0) {
            CVI_LOGE("clock_gettime error:%s \n", strerror(errno));
            return CVI_FILEMNG_EINTER;
        }

        /* init cond */
        CVI_COND_INIT(g_SMCheckCond);
        CVI_COND_INIT(g_SMExitCond);
        memcpy(&s_stSMCfg, pstConfig, sizeof(SPACEMONITOR_CFG_S));

        if (0 == s_stSMCfg.u32Interval) {
            s_stSMCfg.u32Interval = SPACEMONITOR_DEFAULT_INVL;
        }

        s_stSMCfg.bStopCover = true;

        cvi_osal_task_attr_t fs_ta;
        fs_ta.name = "SpaceMonitor";
        fs_ta.entry = FILEMNG_SPACEMONITOR_SpaceCheckThread;
        fs_ta.param = (void *)&s_stSMCfg;
        fs_ta.priority = CVI_OSAL_PRI_NORMAL;
        fs_ta.detached = false;
        s32Ret = cvi_osal_task_create(&fs_ta, &s_SMThread);

        // s32Ret = pthread_create(&s_SMThread, NULL, FILEMNG_SPACEMONITOR_SpaceCheckThread, &s_stSMCfg);
        if (0 != s32Ret) {
            CVI_LOGE("pthread_create error:%s \n", strerror(errno));
            CVI_COND_DESTROY(g_SMCheckCond);
            CVI_COND_DESTROY(g_SMExitCond);
            return CVI_FILEMNG_EINTER;
        }
        s_SMThreadStart = true;
    }
    return 0;
}

int32_t FILEMNG_SPACEMONITOR_Destroy(void)
{
    if (true == s_SMThreadStart) {
        s_stSMCfg.u32Interval = 0;
        CVI_MUTEX_LOCK(g_SMCheckMutex);
        CVI_COND_SIGNAL(g_SMCheckCond);
        CVI_MUTEX_UNLOCK(g_SMCheckMutex);
        CVI_MUTEX_LOCK(g_SMExitMutex);
        CVI_COND_SIGNAL(g_SMExitCond);
        CVI_MUTEX_UNLOCK(g_SMExitMutex);
        // pthread_join(s_SMThread, NULL);
        cvi_osal_task_join(s_SMThread);
        CVI_COND_DESTROY(g_SMCheckCond);
        CVI_COND_DESTROY(g_SMExitCond);
        cvi_osal_task_destroy(&s_SMThread);
        // s_SMThread = 0;
        s_SMThread = NULL;
        s_SMThreadStart = false;
    }

    memset(&s_stSMCfg, 0, sizeof(SPACEMONITOR_CFG_S));
    return 0;
}

int32_t CVI_FILEMNG_SpacemonitorCheckSpace(void)
{
    static struct timespec s_Curtime;

    if (clock_gettime(CLOCK_MONOTONIC, &s_Curtime) < 0) {
        CVI_LOGE("clock_gettime error:%s \n", strerror(errno));
        return CVI_FILEMNG_EINTER;
    }

    if ((0 < s_stSMCfg.u32Interval && (uint32_t)(s_Curtime.tv_sec - s_SMLasttime.tv_sec) > s_stSMCfg.u32Interval)
        || (0 == s_SMLasttime.tv_sec)) {
        CVI_MUTEX_LOCK(g_SMCheckMutex);
        CVI_COND_SIGNAL(g_SMCheckCond);
        CVI_MUTEX_UNLOCK(g_SMCheckMutex);
        memcpy(&s_SMLasttime, &s_Curtime, sizeof(struct timespec));
    }
    return 0;
}

int32_t FILEMNG_SPACEMONITOR_JudgeStage(uint64_t u64RealUsedSize_MB)
{
    int32_t s32Ret = 0;
    CVI_EVENT_S stEvent;
    CVI_STG_FS_INFO_S stFSInfo;

    if (0 < s_stSMCfg.u32WarningStage) {
        s32Ret = CVI_STORAGEMNG_GetFSInfo(&stFSInfo);
        if (0 != s32Ret) {
            return CVI_FILEMNG_EINTER;
        }

        if ((stFSInfo.u64AvailableSize >> 20) <= s_stSMCfg.u32WarningStage) {
            s32Ret = CVI_FILEMNG_EFULL;
            stEvent.topic = CVI_EVENT_FILEMNG_SPACE_FULL;
            CVI_EVENTHUB_Publish(&stEvent);
            CVI_LOGW("space is NOT enough!\n");
        }

        /*invalid file check*/
        s32Ret = CVI_STORAGEMNG_GetFSInfo(&stFSInfo);
        uint64_t u32AvailableSize_MB = stFSInfo.u64AvailableSize >> 20;
        uint64_t u32TotalSize_MB = stFSInfo.u64TotalSize >> 20;
        if((u32TotalSize_MB - u32AvailableSize_MB - u64RealUsedSize_MB) * 100 / u32TotalSize_MB  > FILEMNG_INVAILD_SPACE_THRESHOLD) {
            CVI_LOGW("Unrecognized file space_ratio over threshold: %"PRIu64" > %d \n", \
                                    (u32TotalSize_MB - u32AvailableSize_MB - u64RealUsedSize_MB) * 100 / u32TotalSize_MB , \
                                    FILEMNG_INVAILD_SPACE_THRESHOLD);
            stEvent.topic = CVI_EVENT_FILEMNG_UNIDENTIFICATION;
            s32Ret = CVI_FILEMNG_EUNIDENTIFICATION;
            CVI_EVENTHUB_Publish(&stEvent);
        }
    }

    return s32Ret;
}

int32_t FILEMNG_SPACEMONITOR_SetCoverStatus(bool en)
{
    s_stSMCfg.bStopCover = en;

    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

