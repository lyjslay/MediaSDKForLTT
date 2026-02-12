#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cvi_storage.h"
#include "cvi_log.h"
#include "cvi_osal.h"
#include "cvi_sysutils.h"
#include "cvi_queue.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define ERROR_CNT_THRD 3

#define CVI_STORAGEDEV_QUEUE_MAXLEN  10

typedef struct __stg_context {
    stg_param_t             param;
    volatile uint8_t        shutdown;
    uint8_t                 dev_state;
    // volatile uint8_t        new_state;
    CVI_STG_HANDLE_T        stg_handle;
    CVI_QUEUE_HANDLE_T      queueHdl;
    cvi_osal_task_handle_t  stg_task;
    cvi_osal_task_handle_t  dev_task;
} stg_context_t, *stg_context_handle_t;

static uint32_t stg_state_connect(stg_context_handle_t stg)
{
    int32_t ret = 0;
    CVI_STG_STATE_E stgState = CVI_STG_STATE_IDLE;
    stg_param_handle_t p = &stg->param;
    CVI_STG_DEV_INFO_S StgDevInfo;

    stgState = CVI_STG_STATE_FS_CHECKING;
    if (p->storage_event_callback != NULL) {
        p->storage_event_callback(stgState);
    }
    ret = CVI_STG_DetectPartition(stg->stg_handle);
    if(ret != 0) {
        CVI_LOGE("Detect partition failed\n");
        stgState = CVI_STG_STATE_FS_EXCEPTION;
        goto err;
    }
    ret  = CVI_STG_GetInfo(stg->stg_handle, &StgDevInfo);
    // printf("StgDevInfo.szErrCnt = %d\n", StgDevInfo.szErrCnt);
    if(ret < 0) {
        stgState = CVI_STG_STATE_DEV_ERROR;
        goto err;
    } else if(StgDevInfo.szErrCnt > ERROR_CNT_THRD){
        stgState = CVI_STG_STATE_DEV_ERROR;
        goto err;
    } else {
        stgState = CVI_STG_STATE_MOUNTED;
    }
    ret = CVI_STG_GetFsType(stg->stg_handle);
    if(ret < 0) {
        CVI_LOGE("Get FsInfo failed\n");
        stgState = CVI_STG_STATE_FS_CHECK_FAILED;
        goto err;
    }
    if (p->devinfo.fsType_e != CVI_STG_FS_TYPE_FAT32) {
        CVI_LOGE("device file system exception\n");
        stgState = CVI_STG_STATE_FS_EXCEPTION;
        goto err;
    }
    ret = CVI_STG_RepairFAT32(stg->stg_handle);
    if(ret == -1) {
        CVI_LOGE("Fs repair failed\n");
        stgState = CVI_STG_STATE_FS_CHECK_FAILED;
        goto err;
    }
    ret = CVI_STG_Mount(stg->stg_handle);
    if (ret == 0) {
        // if ((true == CVI_STG_CheckIsReadOnly(stg->stg_handle)) &&
        //     (false == isEverPublishRDEvent)) {
        //     isEverPublishRDEvent = true;
        if (true == CVI_STG_CheckIsReadOnly(stg->stg_handle)) {
            stgState = CVI_STG_STATE_READ_ONLY;
        } else {
            ret = CVI_STG_TestPartition(stg->stg_handle);
            if (ret < 0) {
                CVI_LOGE("Test partition failed\n");
                stgState = CVI_STG_STATE_DEV_ERROR;
            } else {
                ret  = CVI_STG_GetInfo(stg->stg_handle, &StgDevInfo);
                //printf("StgDevInfo.szErrCnt = %d\n", StgDevInfo.szErrCnt);
                if(ret < 0) {
                    stgState = CVI_STG_STATE_DEV_ERROR;
                } else if(StgDevInfo.szErrCnt > ERROR_CNT_THRD){
                    stgState = CVI_STG_STATE_DEV_ERROR;
                } else {
                    stgState = CVI_STG_STATE_MOUNTED;
                }
            }
        }
    } else {
        stgState = CVI_STG_STATE_MOUNT_FAILED;
    }

err:
    if (p->storage_event_callback != NULL) {
        p->storage_event_callback(stgState);
    }

    return ret;
}

static void stg_task_entry(void *arg)
{
    stg_context_handle_t stg = (stg_context_handle_t)arg;
    stg_param_handle_t p = &stg->param;
    int32_t ret = 0;
    CVI_STG_STATE_E stgState = CVI_STG_STATE_IDLE;
    CVI_STG_DEV_STATE_E DevState = CVI_STG_DEV_STATE_IDLE;
    CVI_STG_DEV_STATE_E OldDevState = CVI_STG_DEV_STATE_IDLE;
    while (!stg->shutdown) {
        if (CVI_QUEUE_GetLen(stg->queueHdl) != 0) {
            ret = CVI_QUEUE_Pop(stg->queueHdl, &DevState);
            if (0 != ret) {
                CVI_LOGE("CVI_QUEUE_Pop failed\n");
                continue;
            }
            if (DevState == OldDevState) {
                CVI_LOGI("Same dev state Pass !!!!!!!! \n");
                continue;
            }
            switch(DevState) {
                case CVI_STG_DEV_STATE_CONNECTING:
                    CVI_LOGI("CVI_STG_DEV_STATE_CONNECTING \n");
                    stgState = CVI_STG_STATE_DEV_CONNECTING;
                    if (p->storage_event_callback != NULL) {
                        p->storage_event_callback(stgState);
                    }
                    break;
                case CVI_STG_DEV_STATE_UNPLUGGED:
                    CVI_LOGI("CVI_STG_DEV_STATE_UNPLUGGED \n");
                    ret = CVI_STG_Umount(stg->stg_handle);
                    if (ret != 0) {
                        CVI_LOGE("CVI_STG_Umount Failed !\n");
                    }
                    stgState = CVI_STG_STATE_DEV_UNPLUGGED;
                    if (p->storage_event_callback != NULL) {
                        p->storage_event_callback(stgState);
                    }
                    break;
                case CVI_STG_DEV_STATE_CONNECTED:
                    CVI_LOGI("CVI_STG_DEV_STATE_CONNECTED \n");
                    stg_state_connect(stg);
                    break;
                default:
                    CVI_LOGE("STG_DEV_STATE error! \n");
                    break;
            }
        }
        OldDevState = DevState;
        cvi_osal_task_sleep(100 * 1000); //100 ms
    }
}

static void dev_task_entry(void *arg)
{
    stg_context_handle_t stg = (stg_context_handle_t)arg;
    stg_param_handle_t p = &stg->param;
    uint8_t count = 0;
    int32_t ret = 0;
    while (!stg->shutdown) {
        ret = CVI_STG_GetSDInfo(stg->stg_handle);
        if (ret == 0) {
            if (stg->dev_state != p->devinfo.devState) {
                stg->dev_state = p->devinfo.devState;
                if(CVI_QUEUE_GetLen(stg->queueHdl) >= CVI_STORAGEDEV_QUEUE_MAXLEN) {
                    CVI_LOGW("over queue maxlen , pop first node!\n");
                    CVI_QUEUE_Pop(stg->queueHdl, NULL);
                }
                ret = CVI_QUEUE_Push((CVI_QUEUE_HANDLE_T)stg->queueHdl, &(stg->dev_state));
                if (ret != 0) {
                    CVI_LOGE("Push stg queue failed ! \n");
                }
            }
        }
        if (stg->dev_state == CVI_STG_DEV_STATE_CONNECTING) {
            if(count < 30) {
                count++;
                if (count == 30 && p->storage_event_callback != NULL) {
                    CVI_LOGE("connecting timeout\n");
                    p->storage_event_callback(CVI_STG_STATE_DEV_ERROR);
                }
            }
        } else {
            count = 0;
        }
        cvi_osal_task_sleep(100 * 1000); //100 ms
    }
}

static int32_t stg_start_task(stg_context_handle_t stg)
{
    cvi_osal_task_attr_t ta;
    ta.name = "stg";
    ta.entry = stg_task_entry;
    ta.param = (void *)stg;
    ta.priority = CVI_OSAL_PRI_NORMAL;
    ta.detached = false;
    int32_t rc = cvi_osal_task_create(&ta, &stg->stg_task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("stg task create failed, %d\n", rc);
        return -1;
    }

    return 0;
}

int32_t stg_stop_task(stg_context_handle_t stg) {

    int32_t rc = cvi_osal_task_join(stg->stg_task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("stg task join failed, %d\n", rc);
        return -1;
    }
    cvi_osal_task_destroy(&stg->stg_task);
    return 0;
}

static int32_t dev_start_task(stg_context_handle_t stg)
{
    cvi_osal_task_attr_t ta;
    ta.name = "dev_stg";
    ta.entry = dev_task_entry;
    ta.param = (void *)stg;
    ta.priority = CVI_OSAL_PRI_NORMAL;
    ta.detached = false;
    int32_t rc = cvi_osal_task_create(&ta, &stg->dev_task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("dev task create failed, %d\n", rc);
        return -1;
    }

    return 0;
}

int32_t dev_stop_task(stg_context_handle_t stg) {

    int32_t rc = cvi_osal_task_join(stg->dev_task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("dev task join failed, %d\n", rc);
        return -1;
    }
    cvi_osal_task_destroy(&stg->dev_task);

    return 0;
}

int32_t CVI_STORAGE_SERVICE_Create(CVI_STORAGE_SERVICE_HANDLE_T *hdl, CVI_STORAGE_SERVICE_PARAM_S *params)
{
    stg_context_handle_t stg = (stg_context_handle_t)calloc(sizeof(stg_context_t), 1);
    stg->param = *params;
    stg_param_handle_t p = &stg->param;

    CVI_STG_Init(&(p->devinfo), (CVI_STG_HANDLE_T *)&(stg->stg_handle));

    stg->queueHdl = CVI_QUEUE_Create(sizeof(uint8_t), CVI_STORAGEDEV_QUEUE_MAXLEN);
    if (stg->queueHdl == NULL) {
        CVI_LOGE("Create stg queue failed !\n");
        return -1;
    }

    stg->dev_state = CVI_STG_DEV_STATE_IDLE;

    dev_start_task(stg);

    stg_start_task(stg);

    *hdl = (CVI_STORAGE_SERVICE_HANDLE_T)stg;

    return 0;
}

int32_t CVI_STORAGE_SERVICE_Destroy(CVI_STORAGE_SERVICE_HANDLE_T hdl)
{
    stg_context_handle_t stg = (stg_context_handle_t)hdl;

    // stg_param_handle_t p = &stg->param;
    // send shutdown to self
    stg->shutdown = 1;

    // wait for exit
    while (!stg->shutdown) {
        cvi_osal_task_sleep(20000);
    }

    // stop stg task
    CHECK_RET(dev_stop_task(stg));

    // stop stg task
    CHECK_RET(stg_stop_task(stg));

    CVI_QUEUE_Destroy(stg->queueHdl);

    CVI_STG_DeInit((CVI_STG_HANDLE_T *)&(stg->stg_handle));

    free(stg);
    CVI_LOGI("Storage Service destroy\n");
    return 0;
}

int32_t CVI_STORAGE_SERVICE_GetFsInfo(CVI_STORAGE_SERVICE_HANDLE_T hdl, CVI_STG_FS_INFO_S *pstInfo)
{
    stg_context_handle_t stg = (stg_context_handle_t)hdl;
    int32_t s32Ret = 0;
    if (stg->shutdown == 0) {
        s32Ret = CVI_STG_GetFsInfo(stg->stg_handle, pstInfo);
        if (s32Ret < 0) {
            CVI_LOGE("CVI_STG_GetFsInfo failed!\n");
        }
    } else {
        CVI_LOGE("Stg servers not init\n");
        return -1;
    }
    return s32Ret;
}

int32_t CVI_STORAGE_SERVICE_GetDevInfo(CVI_STORAGE_SERVICE_HANDLE_T hdl, CVI_STG_DEV_INFO_S *pstInfo)
{
    stg_context_handle_t stg = (stg_context_handle_t)hdl;
    int32_t s32Ret = 0;
    if (stg->shutdown == 0) {
        s32Ret  = CVI_STG_GetInfo(stg->stg_handle, pstInfo);
        // s32Ret |= CVI_STG_GetFsType(stg->stg_handle);
        if (s32Ret < 0) {
            CVI_LOGE("CVI_STG_GetInfo failed!\n");
        }
    } else {
        CVI_LOGE("Stg servers not init\n");
        return -1;
    }
    return s32Ret;
}

static int32_t CVI_STORAGE_SERVICE_Formating(void *arg)
{

    int32_t s32Ret = 0;
    stg_context_handle_t stg = (stg_context_handle_t)arg;
    stg_param_handle_t p = &stg->param;

    if (strlen(p->labelname) == 0) {
        s32Ret = CVI_STG_Format(stg->stg_handle);
    } else {
        s32Ret = CVI_STG_FormatWithLabel(stg->stg_handle, p->labelname);
    }

    return s32Ret;
}

int32_t CVI_STORAGE_SERVICE_Format(CVI_STORAGE_SERVICE_HANDLE_T hdl, char *labelname)
{
    stg_context_handle_t stg = (stg_context_handle_t)hdl;
    stg_param_handle_t p = &stg->param;

    if(labelname != NULL) {
        memset(p->labelname, 0, STORAGE_LABEL_LEN);
        snprintf(p->labelname, STORAGE_LABEL_LEN, "%s", labelname);
        // memcpy(p->labelname, labelname, STORAGE_LABEL_LEN);
    }

    int32_t s32Ret = 0;
    if (stg->shutdown == 0) {
        s32Ret = CVI_STORAGE_SERVICE_Formating(stg);
        if (s32Ret != 0) {
            CVI_LOGE("CVI_STORAGE_SERVICE_Formating failed!\n");
            return s32Ret;
        }
    } else {
        CVI_LOGE("Stg servers not init\n");
        return -1;
    }
    return s32Ret;
}

int32_t CVI_STORAGE_SERVICE_Mount(CVI_STORAGE_SERVICE_HANDLE_T hdl)
{
    stg_context_handle_t stg = (stg_context_handle_t)hdl;
    int32_t s32Ret = 0;
    if (stg->shutdown == 0) {
        s32Ret = CVI_STG_Mount(stg->stg_handle);
        if (s32Ret < 0) {
            CVI_LOGE("CVI_STG_GetInfo failed!\n");
        }
    } else {
        CVI_LOGE("Stg servers not init\n");
        return -1;
    }
    return s32Ret;
}

int32_t CVI_STORAGE_SERVICE_Umount(CVI_STORAGE_SERVICE_HANDLE_T hdl)
{
    stg_context_handle_t stg = (stg_context_handle_t)hdl;
    int32_t s32Ret = 0;
    if (stg->shutdown == 0) {
        s32Ret = CVI_STG_Umount(stg->stg_handle);
        if (s32Ret < 0) {
            CVI_LOGE("CVI_STG_GetInfo failed!\n");
        }
    } else {
        CVI_LOGE("Stg servers not init\n");
        return -1;
    }
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */