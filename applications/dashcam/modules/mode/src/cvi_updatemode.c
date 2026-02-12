#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cvi_modeinner.h"
#include "cvi_upgrade.h"
#include "cvi_param.h"

int32_t CVI_MODEMNG_OtaUpFile(CVI_MESSAGE_S* pstMsg)
{
    char upmode[64] = {0};
	char upver[64] = {0};
    char uppath[64] = {0};
    char tmppath[64] = {0};
    int32_t result = 0;
    CVI_UPGRADE_DEV_INFO_S tDev;
    char str[128];
    memset(str, '\0', 128);
    memcpy(str, pstMsg->aszPayload, 128);

	// Read from define, or change to read from file-system
	memcpy(upmode, (pstMsg->aszPayload), (strrchr((str), '_') - (str)));
	memcpy(upver, (strrchr((str), '_') + 1), 64);
	snprintf(tDev.szSoftVersion, CVI_COMM_STR_LEN, "%s", upver);
	snprintf(tDev.szModel, CVI_COMM_STR_LEN, "%s", upmode);

    CVI_UPGRADE_Init();
    STG_DEVINFO_S SDParam = {0};
    CVI_PARAM_GetStgInfoParam(&SDParam);
    strcpy(tmppath, SDParam.aszMntPath);
    strcat(tmppath, "upgrade_%s.bin");
    snprintf(uppath, CVI_COMM_STR_LEN, tmppath, upver);
    if (CVI_UPGRADE_CheckPkg(uppath, &tDev, false) == CVI_SUCCESS) {
		printf("There is a new package!!\n");
		if(pstMsg->arg2 == 1){
            CVI_LOGD("enter SD_updata");
            char mount_path[64] = {0};
            CVI_PARAM_FILEMNG_S FileMng;
            CVI_PARAM_GetFileMngParam(&FileMng);
            strcpy(mount_path, FileMng.FileMngComm.szMntPath);
            result = CVI_UPGRADE_DoUpgradeViaSD(uppath, mount_path);
        }else{
            result = CVI_UPGRADE_DoUpgrade(uppath);
        }
	} else {
        result = -1;
    }
    if (NULL != uppath) {
        char rmpath[72] = {0};
        snprintf(rmpath, 72, "rm -rf %s", uppath);
        system(rmpath);
    }

    return result;
}

int32_t CVI_MODEMNG_OpenUpDateMode(void)
{
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = CVI_WORK_MODE_UPDATE;
    CVI_EVENTHUB_Publish(&stEvent);

    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_UPDATE;

    //set open amplifiler
#ifdef SERVICES_LIVEVIEW_ON
    CVI_VOICEPLAY_SetAmplifier(false);
#endif

    return 0;
}

int32_t CVI_MODEMNG_CloseUpDateMode(void)
{
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_BUTT;


    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_MODECLOSE;
    stEvent.arg1 = CVI_WORK_MODE_UPDATE;
    CVI_EVENTHUB_Publish(&stEvent);

    //set close amplifiler
#ifdef SERVICES_LIVEVIEW_ON
    CVI_VOICEPLAY_SetAmplifier(true);
#endif

    return 0;
}

int32_t CVI_MODEMNG_UpDateModeMsgProc(CVI_MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
{
    // int32_t s32Ret = 0;
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    if (pstModeMngCtx->bSysPowerOff == true) {
        CVI_LOGI("power off ignore msg id: %x\n", pstMsg->topic);
        return CVI_PROCESS_MSG_RESULTE_OK;
    }

    /** check parameters */
    MODEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg, pstModeMngCtx->bInProgress);

    CVI_STATE_S* pstStateAttr = (CVI_STATE_S*)pvArg;
    CVI_LOGD("current mode(%s)\n\n", pstStateAttr->name);
    CVI_LOGD(" will process message topic(%x) \n\n", pstMsg->topic);
    switch (pstMsg->topic)
    {
        case CVI_EVENT_MODEMNG_START_UPFILE:
        {
            int32_t ret = 0;
            ret = CVI_MODEMNG_OtaUpFile(pstMsg);
#if 0
            CVI_EVENT_S stEvent = {0};
            if (1 == ret) {
                stEvent.topic = CVI_EVENT_MODEMNG_UPFILE_FAIL;
            } else if (0 == ret) {
                stEvent.topic = CVI_EVENT_MODEMNG_UPFILE_SUCCESSED;
            } else {
                stEvent.topic = CVI_EVENT_MODEMNG_UPFILE_FAIL_FILE_ERROR;
            }
            CVI_MODEMNG_UPDATESTATUS(&stEvent, true, false);
#endif
            sleep(3);
            if (0 == ret) {
                system("reboot -f");
            }

            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        default:
            return CVI_PROCESS_MSG_UNHANDLER;
    }

    return CVI_PROCESS_MSG_RESULTE_OK;
}

int32_t CVI_MODEMNG_UpDateStatesInit(const CVI_STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static CVI_STATE_S stUpdateState =
    {
        CVI_WORK_MODE_UPDATE,
        MODEEMNG_STATE_UPDATE,
        CVI_MODEMNG_OpenUpDateMode,
        CVI_MODEMNG_CloseUpDateMode,
        CVI_MODEMNG_UpDateModeMsgProc,
        NULL
    };
    stUpdateState.argv = &stUpdateState;
    s32Ret = CVI_HFSM_AddState(CVI_MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stUpdateState,
                              (CVI_STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "HFSM add NormalRec state");

    return s32Ret;
}

/** deinit update mode */
int32_t CVI_MODEMNG_UpDateStatesDeinit(void)
{
    int32_t s32Ret = 0;
    CVI_MODEMNG_CloseUpDateMode();
    return s32Ret;
}