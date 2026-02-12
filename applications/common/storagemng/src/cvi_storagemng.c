#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "cvi_storage.h"
#include "cvi_storagemng.h"
#include "cvi_sysutils.h"
#include "cvi_eventhub.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static CVI_STORAGE_SERVICE_HANDLE_T            StgHdl;
static CVI_STORAGE_SERVICE_PARAM_S     StgParam;

static int32_t CVI_STG_Monitor_StatusNotify(CVI_STG_STATE_E enState)
{
    /* Publish Event */
    CVI_EVENT_S stEvent;
    switch (enState) {
        case CVI_STG_STATE_DEV_UNPLUGGED:
            stEvent.topic = CVI_EVENT_STORAGEMNG_DEV_UNPLUGED;
            break;
        case CVI_STG_STATE_DEV_CONNECTING:
            stEvent.topic = CVI_EVENT_STORAGEMNG_DEV_CONNECTING;
            break;
        case CVI_STG_STATE_DEV_ERROR:
            stEvent.topic = CVI_EVENT_STORAGEMNG_DEV_ERROR;
            break;
        case CVI_STG_STATE_FS_CHECKING:
            stEvent.topic = CVI_EVENT_STORAGEMNG_FS_CHECKING;
            break;
        case CVI_STG_STATE_FS_CHECK_FAILED:
            stEvent.topic = CVI_EVENT_STORAGEMNG_FS_CHECK_FAILED;
            break;
        case CVI_STG_STATE_FS_EXCEPTION:
            stEvent.topic = CVI_EVENT_STORAGEMNG_FS_EXCEPTION;
            break;
        case CVI_STG_STATE_MOUNTED:
            stEvent.topic = CVI_EVENT_STORAGEMNG_MOUNTED;
            break;
        case CVI_STG_STATE_MOUNT_FAILED:
            stEvent.topic = CVI_EVENT_STORAGEMNG_MOUNT_FAILED;
            break;
        case CVI_STG_STATE_READ_ONLY:
            stEvent.topic = CVI_EVENT_STORAGEMNG_MOUNT_READ_ONLY;
            break;
        case CVI_STG_STATE_IDLE:
            CVI_LOGD("Idle State, igore\n");
            return -1;
        default:
            CVI_LOGW("Invalid State[%d]\n", enState);
            return -1;
    }
    CVI_EVENTHUB_Publish(&stEvent);
    return 0;
}

static int32_t CVI_STORAGEMNG_CallBack(CVI_STG_STATE_E state)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_STG_Monitor_StatusNotify(state);
    CVI_APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

    return s32Ret;
}

int32_t CVI_STORAGEMNG_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_EVENTHUB_RegisterTopic(CVI_EVENT_STORAGEMNG_DEV_UNPLUGED);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_STORAGEMNG_DEV_CONNECTING);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_STORAGEMNG_DEV_ERROR);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_STORAGEMNG_FS_CHECKING);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_STORAGEMNG_FS_CHECK_FAILED);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_STORAGEMNG_FS_EXCEPTION);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_STORAGEMNG_MOUNTED);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_STORAGEMNG_MOUNT_FAILED);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_STORAGEMNG_MOUNT_READ_ONLY);
    CVI_APPCOMM_CHECK_RETURN(s32Ret, CVI_STORAGEMNG_EREGISTER_EVENT);
    return 0;
}

int32_t CVI_STORAGEMNG_Create(STG_DEVINFO_S *stg_attr)
{
    CVI_APPCOMM_CHECK_POINTER(stg_attr, CVI_STORAGEMNG_EINVAL);
    int32_t s32Ret = 0;
    memcpy(&StgParam.devinfo, stg_attr, sizeof(STG_DEVINFO_S));
    StgParam.storage_event_callback = CVI_STORAGEMNG_CallBack;

    s32Ret = CVI_STORAGE_SERVICE_Create(&StgHdl, &StgParam);
    CVI_APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

    return s32Ret;
}

int32_t CVI_STORAGEMNG_Destroy(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_STORAGE_SERVICE_Destroy(StgHdl);
    CVI_APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

    return s32Ret;
}

int32_t CVI_STORAGEMNG_GetFSInfo(CVI_STG_FS_INFO_S *pstInfo)
{
    CVI_APPCOMM_CHECK_POINTER(pstInfo, CVI_STORAGEMNG_EINVAL);
    int32_t s32Ret = 0;
    s32Ret = CVI_STG_GetFsInfo(StgHdl, pstInfo);
    CVI_APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

    return s32Ret;
}

int32_t CVI_STORAGEMNG_GetInfo(CVI_STG_DEV_INFO_S *pstInfo)
{
    CVI_APPCOMM_CHECK_POINTER(pstInfo, CVI_STORAGEMNG_EINVAL);
    int32_t s32Ret = 0;
    s32Ret = CVI_STORAGE_SERVICE_GetDevInfo(StgHdl, pstInfo);
    CVI_APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

    return s32Ret;
}

int32_t CVI_STORAGEMNG_Format(char *labelname)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_STORAGE_SERVICE_Format(StgHdl, labelname);
    CVI_APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

    return s32Ret;
}

int32_t CVI_STORAGEMNG_Mount(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_STORAGE_SERVICE_Mount(StgHdl);
    CVI_APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

    return s32Ret;
}

int32_t CVI_STORAGEMNG_Umount(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_STORAGE_SERVICE_Umount(StgHdl);
    CVI_APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

