#include <string.h>
#include "cvi_param.h"
#include "cvi_param_ini2bin.h"
#include "cvi_ini.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

extern CVI_PARAM_ACCESS g_ParamAccess;

static int32_t PARAM_LoadStgInfo(const char *file, STG_DEVINFO_S *Stg)
{
    char tmp_port[16] = {0};
    char tmp_devpath[16] = {0};
    char tmp_mntpath[16] = {0};

    CVI_INI_GetString("storage", "dev_port", "", tmp_port, 16, file);
    CVI_INI_GetString("storage", "dev_path", "", tmp_devpath, 16, file);
    CVI_INI_GetString("storage", "mnt_path", "", tmp_mntpath, 16, file);

    printf("%s %s %s\n", tmp_port, tmp_devpath, tmp_mntpath);

    memcpy(Stg->aszDevPort, tmp_port, 16);
    memcpy(Stg->aszDevPath, tmp_devpath, 16);
    memcpy(Stg->aszMntPath, tmp_mntpath, 16);

    return 0;
}

static int32_t PARAM_LoadWifiInfo(const char *file, CVI_PARAM_WIFI_S *Wifi)
{
    char tmp_ssid[32] = {0};
    char tmp_password[64] = {0};
    long int enable = 0;
    long int mode = 0;
    long int ssid_hide = 0;
    long int channel = 0;

    enable = CVI_INI_GetLong("wifi_config", "enable", 0, file);
    mode = CVI_INI_GetLong("wifi_config", "mode", 0, file);
    ssid_hide = CVI_INI_GetLong("wifi_config", "ssid_hide", 0, file);
    channel = CVI_INI_GetLong("wifi_config", "channel", 0, file);
    CVI_INI_GetString("wifi_config", "ssid", "", tmp_ssid, 16, file);
    CVI_INI_GetString("wifi_config", "password", "", tmp_password, 16, file);

    printf("%s: %ld %ld %ld %ld password:%s\n", tmp_ssid, enable, mode, ssid_hide, channel, tmp_password);

    Wifi->Enable = enable;
    Wifi->WifiCfg.enMode = mode;
    if(mode == 1) {
        Wifi->WifiCfg.unCfg.stApCfg.bHideSSID = ssid_hide;
        Wifi->WifiCfg.unCfg.stApCfg.s32Channel = channel;
        memcpy(Wifi->WifiCfg.unCfg.stApCfg.stCfg.szWiFiSSID, tmp_ssid, 32);
        memcpy(Wifi->WifiDefaultSsid, tmp_ssid, 32);
        memcpy(Wifi->WifiCfg.unCfg.stApCfg.stCfg.szWiFiPassWord, tmp_password, 64);
    } else {
        printf("now only suport ap mode");
    }

    return 0;
}

static int32_t PARAM_LoadPWMInfo(const char *file, CVI_PARAM_PWM_S *PWM)
{
    long int enable = 0;
    long int group = 0;
    long int channel = 0;
    long int period = 0;
    long int duty_cycle = 0;

    enable = CVI_INI_GetLong("pwm_config", "enable", 0, file);
    group = CVI_INI_GetLong("pwm_config", "group", 0, file);
    channel = CVI_INI_GetLong("pwm_config", "channel", 0, file);
    period = CVI_INI_GetLong("pwm_config", "period", 0, file);
    duty_cycle = CVI_INI_GetLong("pwm_config", "duty_cycle", 0, file);

    printf("PWM : group(%ld) channel(%ld) period(%ld) duty_cycle(%ld)\n", group, channel, period, duty_cycle);

    PWM->Enable = enable;
    PWM->PWMCfg.group = group;
    PWM->PWMCfg.channel = channel;
    PWM->PWMCfg.period = period;
    PWM->PWMCfg.duty_cycle = duty_cycle;

    return 0;
}

static int32_t PARAM_LoadGsensorInfo(const char *file, CVI_GSENSORMNG_CFG_S *Gsensor)
{
   int32_t  enable = 0;
    long int enSensitity = 0;
    long int sampleRate = 0;
   int32_t  level = 0;

    enable = CVI_INI_GetLong("gsensor_config", "enable", 0, file);
    enSensitity = CVI_INI_GetLong("gsensor_config", "enSensitity", 0, file);
    sampleRate = CVI_INI_GetLong("gsensor_config", "u32SampleRate", 0, file);
    level = CVI_INI_GetLong("gsensor_config", "level", 0, file);
    printf("enSensitity = %ld u32SampleRate = %ld level = %d \n", enSensitity, sampleRate, level);

    Gsensor->gsensor_enable = enable;
    Gsensor->enSensitity = enSensitity;
    Gsensor->gsensor_level = level;
    Gsensor->stAttr.u32SampleRate = sampleRate;

    return 0;
}

static int32_t PARAM_Load_KeyInfo(const char *file, CVI_KEYMNG_CFG_S *key)
{
    long int key_cnt = 0;
    char typebuffer[64] = {0};
    char idbuffer[64] = {0};
    char enablebuffer[64] = {0};
    char timebuffer[64] = {0};
    char giridbuffer[64] = {0};

    key_cnt = CVI_INI_GetLong("keymng.key", "key_cnt", 0, file);
    key->stKeyCfg.u32KeyCnt = key_cnt;
    for (int32_t  i = 0; i < key_cnt; i++) {
        memset(typebuffer, 0, sizeof(typebuffer));
        memset(idbuffer, 0, sizeof(idbuffer));
        memset(enablebuffer, 0, sizeof(enablebuffer));
        memset(timebuffer, 0, sizeof(timebuffer));
        snprintf(typebuffer, sizeof(typebuffer), "key_type%d", i);
        snprintf(idbuffer, sizeof(idbuffer), "key_id%d", i);
        snprintf(enablebuffer, sizeof(enablebuffer), "longkey_enable%d", i);
        snprintf(timebuffer, sizeof(timebuffer), "longkey_time%d", i);

        key->stKeyCfg.astKeyAttr[i].enType = CVI_INI_GetLong("keymng.key", typebuffer, 0, file);
        key->stKeyCfg.astKeyAttr[i].s32Id = CVI_INI_GetLong("keymng.key", idbuffer, 0, file);
        key->stKeyCfg.astKeyAttr[i].unAttr.stClickKeyAttr.bLongClickEnable = CVI_INI_GetLong("keymng.key", enablebuffer, 0, file);
        key->stKeyCfg.astKeyAttr[i].unAttr.stClickKeyAttr.u32LongClickTime_msec = CVI_INI_GetLong("keymng.key", timebuffer, 0, file);
    }

    /* Key Configure */
    key->stGrpKeyCfg.bEnable = CVI_INI_GetLong("keymng.grpkey", "enable", 0, file);
    for (int32_t  i = 0; i < CVI_KEYMNG_KEY_NUM_EACH_GRP; i++) {
        memset(giridbuffer, 0, sizeof(giridbuffer));
        snprintf(giridbuffer, sizeof(giridbuffer), "key_idx%d", i);
        key->stGrpKeyCfg.au32GrpKeyIdx[i] = CVI_INI_GetLong("keymng.grpkey", giridbuffer, 0, file);
    }

    return 0;
}

static int32_t PARAM_LoadGaugeInfo(const char *file, CVI_GAUGEMNG_CFG_S *Gauge)
{
    long int LowLevel = 0;
    long int UltraLowLevel = 0;

    LowLevel = CVI_INI_GetLong("gaugemng", "LowLevel", 0, file);
    UltraLowLevel = CVI_INI_GetLong("gaugemng", "UltraLowLevel", 0, file);
    printf("LowLevel = %ld UltraLowLevel = %ld\n", LowLevel, UltraLowLevel);

    Gauge->s32LowLevel = LowLevel;
    Gauge->s32UltraLowLevel = UltraLowLevel;

    return 0;
}

static int32_t PARAM_LoadDevmng(const char *file, CVI_PARAM_DEVMNG_S *DevMng)
{
   int32_t  s32Ret = 0;

    s32Ret = PARAM_LoadStgInfo(file, &DevMng->Stg);
    s32Ret = PARAM_LoadWifiInfo(file, &DevMng->Wifi);
    s32Ret = PARAM_LoadPWMInfo(file, &DevMng->PWM);
    s32Ret = PARAM_Load_KeyInfo(file, &DevMng->stkeyMngCfg);
    s32Ret = PARAM_LoadGsensorInfo(file, &DevMng->Gsensor);
    s32Ret = PARAM_LoadGaugeInfo(file, &DevMng->GaugeCfg);

    return s32Ret;
}

int32_t  CVI_INI_PARAM_LoadDevmngCfg(CVI_PARAM_DEVMNG_S *DevMng)
{
    printf("\n---enter: %s\n", __func__);

    uint32_t i = 0;
    char filepath[CVI_PARAM_MODULE_NAME_LEN] = {0};

    for (i = 0; i < g_ParamAccess.module_num; i++) {
        if (strstr(g_ParamAccess.modules[i].name, "config_devmng")) {
            memset(filepath, 0, sizeof(filepath));
            snprintf(filepath, sizeof(filepath), "%s%s",
                g_ParamAccess.modules[i].path, g_ParamAccess.modules[i].name);
            // find a media comm file
            PARAM_LoadDevmng(filepath, DevMng);
            break;
        }
    }

    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */