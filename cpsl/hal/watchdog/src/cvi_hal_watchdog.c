#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <linux/watchdog.h>

#include "cvi_appcomm.h"
#include "cvi_type.h"
#include "cvi_hal_watchdog.h"
#include "cvi_sysutils.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define HAL_FD_INITIALIZATION_VAL (-1)
#define HAL_WATCHDOG_DEV "/dev/watchdog"
#define WDT_KO_PATH CVI_KOMOD_PATH "/" CHIP_TYPE "_wdt.ko"

static int32_t s_s32HALWATCHDOGfd = HAL_FD_INITIALIZATION_VAL;

int32_t CVI_HAL_WATCHDOG_Init(int32_t s32Time_s)
{
    int32_t s32Ret = CVI_SUCCESS;
    if (s_s32HALWATCHDOGfd != HAL_FD_INITIALIZATION_VAL)
    {
        CVI_LOGE("already init");
        return CVI_FAILURE;
    }

    if (s32Time_s < 2 || s32Time_s > 1000)
    {
        CVI_LOGE("Interval time should not be less then two and bigger then 100. %d\n", s32Time_s);
        return CVI_FAILURE;
    }

    char szWdtString[CVI_APPCOMM_MAX_PATH_LEN] = {0};
    snprintf(szWdtString, CVI_APPCOMM_MAX_PATH_LEN, " default_margin=%d nodeamon=1", s32Time_s);

    s32Ret = cvi_insmod(WDT_KO_PATH, szWdtString);
    if(CVI_SUCCESS != s32Ret)
    {
        CVI_LOGE("insmod wdt.ko: failed, errno(%d)\n", errno);
        return CVI_FAILURE;
    }

    s_s32HALWATCHDOGfd = open(HAL_WATCHDOG_DEV, O_RDWR);

    if (s_s32HALWATCHDOGfd < 0)
    {
        CVI_LOGE("open [%s] failed\n",HAL_WATCHDOG_DEV);
        return CVI_FAILURE;
    }


    s32Ret = ioctl(s_s32HALWATCHDOGfd, WDIOC_KEEPALIVE);/**feed dog */
    if(-1 == s32Ret)
    {
        CVI_LOGE("WDIOC_KEEPALIVE: failed, errno(%d)\n", errno);
        return CVI_FAILURE;
    }

    return 0;
}

int32_t CVI_HAL_WATCHDOG_Feed(void)
{
    int32_t s32Ret = CVI_SUCCESS;
    s32Ret = ioctl(s_s32HALWATCHDOGfd, WDIOC_KEEPALIVE);/**feed dog */
    if(-1 == s32Ret)
    {
        CVI_LOGE("WDIOC_KEEPALIVE: failed, errno(%d)\n", errno);
        return CVI_FAILURE;
    }
    return CVI_SUCCESS;
}


int32_t CVI_HAL_WATCHDOG_Deinit(void)
{
    int32_t s32Ret;

    if (s_s32HALWATCHDOGfd == HAL_FD_INITIALIZATION_VAL)
    {
        CVI_LOGE("watchdog not initialized,no need to close\n");
        return CVI_FAILURE;
    }
    s32Ret = close(s_s32HALWATCHDOGfd);
    if (0 > s32Ret)
    {
        CVI_LOGE("wdrfd[%d] close,fail,errno(%d)\n",s_s32HALWATCHDOGfd,errno);
        return CVI_FAILURE;
    }
    s_s32HALWATCHDOGfd = HAL_FD_INITIALIZATION_VAL;
    cvi_rmmod(WDT_KO_PATH);
    return CVI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


