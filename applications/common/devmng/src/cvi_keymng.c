/**
* @file    cvi_keymng.c
* @brief   product keymng function
*/

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "cvi_keymng.h"
#include "cvi_hal_key.h"
#include "cvi_eventhub.h"

//#include "cvi_appcomm_util.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */


#define KEYMNG_CHECK_INTERVAL  (50) /**< key check interval, unit ms */

#define KEYMNG_LONG_CLICK_MIN_TIME  (1000) /**< long click key min time, unit ms */

static bool s_bKEYMNGInitState = 0;
static pthread_mutex_t s_KEYMNGMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t s_KEYMNGCheckId;
static bool s_bKEYMNGCheckRun;

/** keymng information */
typedef struct tagKEYMNG_INFO_S
{
    CVI_HAL_KEY_IDX_E enKeyIdx;
    CVI_HAL_KEY_STATE_E  enKeyState;
    uint32_t u32KeyDownCnt;
    bool bMultiKeyFound;
    CVI_KEYMNG_KEY_ATTR_S stKeyAttr;
} KEYMNG_INFO_S;

/** keymng information */
typedef struct tagKEYMNG_INFO_SET_S
{
    uint32_t u32KeyCnt;
    KEYMNG_INFO_S astKeyInfo[CVI_KEYMNG_KEY_IDX_BUTT];
    CVI_KEYMNG_GRP_KEY_CFG_S stGrpKeyCfg;
} KEYMNG_INFO_SET_S;


static KEYMNG_INFO_SET_S s_stKEYMNGInfoSet;/**< keymng info set*/


static int32_t KEYMNG_InParmValidChck(const CVI_KEYMNG_CFG_S* pstCfg)
{
    uint32_t i = 0, j = 0;
    /**parm check*/
    CVI_APPCOMM_CHECK_POINTER(pstCfg, -1);
    CVI_APPCOMM_CHECK_EXPR(pstCfg->stKeyCfg.u32KeyCnt <= CVI_KEYMNG_KEY_IDX_BUTT, -1);

    if (pstCfg->stGrpKeyCfg.bEnable == 1) {
        CVI_APPCOMM_CHECK_EXPR(pstCfg->stKeyCfg.u32KeyCnt >= CVI_KEYMNG_KEY_NUM_EACH_GRP, -1);

        for (i = 0; i < CVI_KEYMNG_KEY_NUM_EACH_GRP; i++)
        {
            CVI_APPCOMM_CHECK_EXPR(pstCfg->stGrpKeyCfg.au32GrpKeyIdx[i] < pstCfg->stKeyCfg.u32KeyCnt, -1);
            CVI_APPCOMM_CHECK_EXPR(pstCfg->stKeyCfg.astKeyAttr[pstCfg->stGrpKeyCfg.au32GrpKeyIdx[i]].enType == CVI_KEYMNG_KEY_TYPE_CLICK, -1);

            for (j = i + 1; j < CVI_KEYMNG_KEY_NUM_EACH_GRP; j++)
            {
                CVI_APPCOMM_CHECK_EXPR(pstCfg->stGrpKeyCfg.au32GrpKeyIdx[i] != pstCfg->stGrpKeyCfg.au32GrpKeyIdx[j], -1);
            }
        }
    }

    for (i = 0; i < pstCfg->stKeyCfg.u32KeyCnt; i++)
    {
        if ((pstCfg->stKeyCfg.astKeyAttr[i].enType == CVI_KEYMNG_KEY_TYPE_CLICK) && (pstCfg->stKeyCfg.astKeyAttr[i].unAttr.stClickKeyAttr.bLongClickEnable == 1)) {
            CVI_APPCOMM_CHECK_EXPR(pstCfg->stKeyCfg.astKeyAttr[i].unAttr.stClickKeyAttr.u32LongClickTime_msec >= KEYMNG_LONG_CLICK_MIN_TIME, -1);
        }
    }

    return 0;
}

static int32_t KEYMNG_InternalParmInit(const CVI_KEYMNG_CFG_S* pstCfg)
{
    uint32_t i = 0;
    CVI_APPCOMM_CHECK_POINTER(pstCfg, -1);
    s_stKEYMNGInfoSet.u32KeyCnt = pstCfg->stKeyCfg.u32KeyCnt;
    memcpy(&s_stKEYMNGInfoSet.stGrpKeyCfg, &pstCfg->stGrpKeyCfg, sizeof(CVI_KEYMNG_GRP_KEY_CFG_S));

    for (i = 0; i < pstCfg->stKeyCfg.u32KeyCnt; i++)
    {
        memcpy(&s_stKEYMNGInfoSet.astKeyInfo[i].stKeyAttr, &pstCfg->stKeyCfg.astKeyAttr[i], sizeof(CVI_KEYMNG_KEY_ATTR_S));
        s_stKEYMNGInfoSet.astKeyInfo[i].enKeyIdx = (CVI_HAL_KEY_IDX_E)i;
        s_stKEYMNGInfoSet.astKeyInfo[i].enKeyState = CVI_HAL_KEY_STATE_UP;
        s_stKEYMNGInfoSet.astKeyInfo[i].u32KeyDownCnt = 0;
        s_stKEYMNGInfoSet.astKeyInfo[i].bMultiKeyFound = 0;
    }

    return 0;
}

static int32_t  KEYMNG_GroupKeyCheck(KEYMNG_INFO_SET_S* pstKeySetInfo)
{
    uint32_t i = 0;
    uint32_t u32KeyIndex;
    CVI_EVENT_S stEvent;
    CVI_APPCOMM_CHECK_POINTER(pstKeySetInfo, -1);
    CVI_APPCOMM_CHECK_EXPR(pstKeySetInfo->stGrpKeyCfg.bEnable == 1, -1);

    for (i = 0; i < CVI_KEYMNG_KEY_NUM_EACH_GRP; i++)
    {
        u32KeyIndex = pstKeySetInfo->stGrpKeyCfg.au32GrpKeyIdx[i];

        if (pstKeySetInfo->astKeyInfo[u32KeyIndex].bMultiKeyFound == 1) {
            return 0;
        }

        if (pstKeySetInfo->astKeyInfo[u32KeyIndex].enKeyState == CVI_HAL_KEY_STATE_UP) {
            return 0;
        }
    }

    memset(&stEvent, '\0', sizeof(stEvent));
    stEvent.topic = CVI_EVENT_KEYMNG_GROUP;

    for (i = 0; i < CVI_KEYMNG_KEY_NUM_EACH_GRP; i++)
    {
        u32KeyIndex = pstKeySetInfo->stGrpKeyCfg.au32GrpKeyIdx[i];
        pstKeySetInfo->astKeyInfo[u32KeyIndex].bMultiKeyFound = 1;
        stEvent.aszPayload[i] = u32KeyIndex;
    }

    CVI_EVENTHUB_Publish(&stEvent);
    CVI_LOGD("group key event\n");
    return 0;
}


static int32_t  KEYMNG_HoldCheck (KEYMNG_INFO_S* pstHoldKeyInfo)
{

    CVI_HAL_KEY_STATE_E enKeyState = CVI_HAL_KEY_STATE_UP;
    CVI_EVENT_S stEvent;

    CVI_APPCOMM_CHECK_POINTER(pstHoldKeyInfo, -1);
    CVI_APPCOMM_CHECK_EXPR(pstHoldKeyInfo->stKeyAttr.enType == CVI_KEYMNG_KEY_TYPE_HOLD, -1);

    CVI_HAL_KEY_GetState(pstHoldKeyInfo->enKeyIdx, &enKeyState);

    if (CVI_HAL_KEY_STATE_DOWN == enKeyState) {
        if (CVI_HAL_KEY_STATE_UP == pstHoldKeyInfo->enKeyState) {
            pstHoldKeyInfo->enKeyState = CVI_HAL_KEY_STATE_DOWN;
            memset(&stEvent, '\0', sizeof(stEvent));
            stEvent.topic = CVI_EVENT_KEYMNG_HOLD_DOWN;
            stEvent.arg1 = pstHoldKeyInfo->enKeyIdx;      /**<KEYID 0/1/2*/
            CVI_EVENTHUB_Publish(&stEvent);
            CVI_LOGD("key down event[%u]\n", pstHoldKeyInfo->enKeyIdx);
        }
    } else {
        if (CVI_HAL_KEY_STATE_DOWN == pstHoldKeyInfo->enKeyState) {
            pstHoldKeyInfo->enKeyState = CVI_HAL_KEY_STATE_UP;

            memset(&stEvent, '\0', sizeof(stEvent));
            stEvent.topic = CVI_EVENT_KEYMNG_HOLD_UP;
            stEvent.arg1 = pstHoldKeyInfo->enKeyIdx;      /**<KEYID 0/1/2*/
            CVI_EVENTHUB_Publish(&stEvent);
            CVI_LOGD("key up event[%u]\n", pstHoldKeyInfo->enKeyIdx);
        }
    }

    return 0;
}


static int32_t  KEYMNG_ClickCheck (KEYMNG_INFO_S* pstClickKeyInfo)
{
    CVI_HAL_KEY_STATE_E enKeyState = CVI_HAL_KEY_STATE_UP;
    CVI_EVENT_S stEvent;

    CVI_APPCOMM_CHECK_POINTER(pstClickKeyInfo, -1);
    CVI_APPCOMM_CHECK_EXPR(pstClickKeyInfo->stKeyAttr.enType == CVI_KEYMNG_KEY_TYPE_CLICK, -1);
    CVI_HAL_KEY_GetState(pstClickKeyInfo->enKeyIdx, &enKeyState);

    /** Check Key Event */
    if (CVI_HAL_KEY_STATE_DOWN == enKeyState) {
        if (CVI_HAL_KEY_STATE_UP == pstClickKeyInfo->enKeyState) {
            pstClickKeyInfo->enKeyState = CVI_HAL_KEY_STATE_DOWN;
        }

        if ((pstClickKeyInfo->u32KeyDownCnt < (pstClickKeyInfo->stKeyAttr.unAttr.stClickKeyAttr.u32LongClickTime_msec / KEYMNG_CHECK_INTERVAL))) {
            pstClickKeyInfo->u32KeyDownCnt++;

            if (pstClickKeyInfo->u32KeyDownCnt >= (pstClickKeyInfo->stKeyAttr.unAttr.stClickKeyAttr.u32LongClickTime_msec / KEYMNG_CHECK_INTERVAL)) {
                if (pstClickKeyInfo->bMultiKeyFound == 0) {
                    memset(&stEvent, '\0', sizeof(stEvent));
                    stEvent.topic = CVI_EVENT_KEYMNG_LONG_CLICK;
                    stEvent.arg1 = pstClickKeyInfo->enKeyIdx;      /**<KEYID 0/1/2*/
                    CVI_EVENTHUB_Publish(&stEvent);
                    CVI_LOGD("long click event[%u]\n", pstClickKeyInfo->enKeyIdx);
                }
            }
        }
    } else {
        if (CVI_HAL_KEY_STATE_DOWN == pstClickKeyInfo->enKeyState) {
            pstClickKeyInfo->enKeyState = CVI_HAL_KEY_STATE_UP;

            if (pstClickKeyInfo->u32KeyDownCnt < (pstClickKeyInfo->stKeyAttr.unAttr.stClickKeyAttr.u32LongClickTime_msec / KEYMNG_CHECK_INTERVAL)) {
                if (pstClickKeyInfo->bMultiKeyFound == 0) {
                    memset(&stEvent, '\0', sizeof(stEvent));
                    stEvent.topic = CVI_EVENT_KEYMNG_SHORT_CLICK;
                    stEvent.arg1 = pstClickKeyInfo->enKeyIdx;      /**<KEYID 0/1/2*/
                    CVI_EVENTHUB_Publish(&stEvent);
                    CVI_LOGD("short click event[%u]\n", pstClickKeyInfo->enKeyIdx);
                }
            }

            pstClickKeyInfo->u32KeyDownCnt = 0;
            pstClickKeyInfo->bMultiKeyFound = 0;
        }
    }

    return 0;
}

static int32_t  KEYMNG_KeyCheck(KEYMNG_INFO_SET_S* pstKeySetInfo)
{
    uint32_t i = 0;

    CVI_APPCOMM_CHECK_POINTER(pstKeySetInfo, -1);

    for (i = 0; i < pstKeySetInfo->u32KeyCnt; i++)
    {
        if (pstKeySetInfo->astKeyInfo[i].stKeyAttr.enType == CVI_KEYMNG_KEY_TYPE_CLICK) {
            KEYMNG_ClickCheck(&pstKeySetInfo->astKeyInfo[i]);
        } else {
            KEYMNG_HoldCheck(&pstKeySetInfo->astKeyInfo[i]);
        }
    }

    if (pstKeySetInfo->stGrpKeyCfg.bEnable == 1) {
        KEYMNG_GroupKeyCheck(pstKeySetInfo);
    }

    return 0;
}


static void*  KEYMNG_CheckThread(void* pData)
{

    CVI_LOGD("thread KEY_CHECK enter\n");
    prctl(PR_SET_NAME, "KEY_CHECK", 0, 0, 0); /**< Set Task Name */

    while (s_bKEYMNGCheckRun)
    {

        KEYMNG_KeyCheck(&s_stKEYMNGInfoSet);
        usleep(KEYMNG_CHECK_INTERVAL * 1000);
    }

    CVI_LOGD("thread KEY_CHECK exit\n");
    return NULL;
}

int32_t CVI_KEYMNG_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret=CVI_EVENTHUB_RegisterTopic(CVI_EVENT_KEYMNG_SHORT_CLICK);
    if(0 != s32Ret) {
        CVI_LOGE("Register short click key event fail\n");
        return CVI_KEYMNG_EREGISTEREVENT;
    }
    s32Ret=CVI_EVENTHUB_RegisterTopic(CVI_EVENT_KEYMNG_LONG_CLICK);
    if(0 != s32Ret) {
        CVI_LOGE("Register long click key event fail\n");
        return CVI_KEYMNG_EREGISTEREVENT;
    }
    s32Ret=CVI_EVENTHUB_RegisterTopic(CVI_EVENT_KEYMNG_HOLD_DOWN);
    if(0 != s32Ret) {
        CVI_LOGE("Register hold down key event fail\n");
        return CVI_KEYMNG_EREGISTEREVENT;
    }
    s32Ret=CVI_EVENTHUB_RegisterTopic(CVI_EVENT_KEYMNG_HOLD_UP);
    if(0 != s32Ret) {
        CVI_LOGE("Register hold up key event fail\n");
        return CVI_KEYMNG_EREGISTEREVENT;
    }
    s32Ret=CVI_EVENTHUB_RegisterTopic(CVI_EVENT_KEYMNG_GROUP);
    if(0 != s32Ret) {
        CVI_LOGE("Register group key event fail\n");
        return CVI_KEYMNG_EREGISTEREVENT;
    }
    return 0;
}

int32_t CVI_KEYMNG_Init(CVI_KEYMNG_CFG_S stKeyCfg)
{
    int32_t s32Ret;

    /** keymng init */
    CVI_MUTEX_LOCK(s_KEYMNGMutex);

    if (s_bKEYMNGInitState  == 1) {
        CVI_LOGE("keymng has already been started\n");
        CVI_MUTEX_UNLOCK(s_KEYMNGMutex);
        return CVI_KEYMNG_EINITIALIZED;
    }

    s32Ret = CVI_HAL_KEY_Init();
    if (s32Ret != 0) {
        CVI_LOGE("CVI_HAL_KEY_Init Failed\n");
        CVI_MUTEX_UNLOCK(s_KEYMNGMutex);
        return CVI_KEYMNG_EINTER;
    }

    CVI_KEYMNG_RegisterEvent();
    KEYMNG_InternalParmInit(&stKeyCfg);

    /** Create Key Check Task */
    s_bKEYMNGCheckRun = 1;
    s32Ret = pthread_create(&s_KEYMNGCheckId, NULL, KEYMNG_CheckThread, NULL);
    if (s32Ret != 0) {
        CVI_LOGE("Create KeyCheck Thread Fail!\n");
        CVI_HAL_KEY_Deinit();
        CVI_MUTEX_UNLOCK(s_KEYMNGMutex);
        return CVI_KEYMNG_ETHREAD;
    }

    s_bKEYMNGInitState = 1;
    CVI_MUTEX_UNLOCK(s_KEYMNGMutex);
    return 0;

}

int32_t CVI_KEYMNG_DeInit(void)
{
    int32_t s32Ret;

    CVI_MUTEX_LOCK(s_KEYMNGMutex);

    if (s_bKEYMNGInitState  == 0) {
        CVI_LOGE("keymng no init\n");
        CVI_MUTEX_UNLOCK(s_KEYMNGMutex);
        return CVI_KEYMNG_ENOINIT;
    }

    /** Destory Key Check Task */
    s_bKEYMNGCheckRun = 0;

    s32Ret = pthread_join(s_KEYMNGCheckId, NULL);
    if (s32Ret != 0) {
        CVI_LOGE("Join KeyCheck Thread Fail!\n");
        CVI_MUTEX_UNLOCK(s_KEYMNGMutex);
        return CVI_KEYMNG_ETHREAD;
    }

    /** Close Key */
    s32Ret = CVI_HAL_KEY_Deinit();
    if (s32Ret != 0) {
        CVI_LOGE("CVI_HAL_KEY_Deinit Fail!\n");
        CVI_MUTEX_UNLOCK(s_KEYMNGMutex);
        return CVI_KEYMNG_EINTER;
    }

    s_bKEYMNGInitState = 0;
    CVI_MUTEX_UNLOCK(s_KEYMNGMutex);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */
