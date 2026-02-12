#include <sys/statfs.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "cvi_net.h"
#include "cvi_netctrl.h"
#include "cvi_netctrlinner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static int32_t CVI_NETCTRL_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_EVENTHUB_RegisterTopic(CVI_EVENT_NETCTRL_CONNECT);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_NETCTRL_UIUPDATE);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_NETCTRL_APPCONNECT_SUCCESS);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_NETCTRL_APPDISCONNECT);
    if (0 != s32Ret) {
        CVI_LOGE("Register NET event fail\n");
        return -1;
    }

    return 0;
}

int32_t CVI_NETCTRL_Init(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_NETCTRLINNER_InitTimer();
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "CVI_NETCTRLINNER_InitTimer");
    s32Ret = CVI_NETCTRL_RegisterEvent();
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "CVI_NETCTRL_RegisterEvent");
    s32Ret = CVI_NETCTRLINNER_SubscribeEvents();
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "CVI_NETCTRLINNER_SubscribeEvents");
    CVI_NETCTRLINNER_CMDRegister();
    s32Ret = CVI_NET_Init();
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "CVI_NET_Init");
    return s32Ret;
}

int32_t CVI_NETCTRL_DeInit(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_NET_DeInit();
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "CVI_NET_DeInit");
    s32Ret = CVI_NETCTRLINNER_DeInitTimer();
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "CVI_NETCTRLINNER_DeInitTimer");
    return s32Ret;
}

int32_t CVI_NETCTRL_NetToUiConnectState(void)
{
    return CVI_NETCTRLINNER_APPConnetState();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif