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

#define CVI_PARAM_LoadValueSet(file, valueset)                  \
    do {                                                        \
        return PARAM_LoadValueSet(file, valueset, #valueset);   \
    } while (0)


static int32_t PARAM_LoadValueSet(const char *file, void *param, const char *section)
{
    long int i = 0;
    long int num = 0;
    long int cur = 0;
    long int value = 0;
    char tmp_desc[CVI_PARAM_MENU_ITEM_DESC_LEN] = {0};
    char tmp_key_desc[32] = {0};
    char tmp_key_value[32] = {0};

    CVI_PARAM_VALUESET_S *ValueSet = param;
    printf("file: %s, section：%s\n", file, section);

    num = CVI_INI_GetLong(section, "num", 0, file);
    cur = CVI_INI_GetLong(section, "current", 0, file);
    printf("%s: %ld %ld\n", __func__, num, cur);

    ValueSet->ItemCnt = num;
    ValueSet->Current = cur;

    for (i = 0; i < num; i++) {
        memset(tmp_desc, 0, sizeof(tmp_desc));
        memset(tmp_key_desc, 0, sizeof(tmp_key_desc));
        memset(tmp_key_value, 0, sizeof(tmp_key_value));

        snprintf(tmp_key_desc, sizeof(tmp_key_desc), "description%ld", i);
        snprintf(tmp_key_value, sizeof(tmp_key_value), "value%ld", i);

        CVI_INI_GetString(section, tmp_key_desc, "", tmp_desc, CVI_PARAM_MENU_ITEM_DESC_LEN, file);
        value = CVI_INI_GetLong(section, tmp_key_value, 0, file);
        printf("%s: %ld\n", tmp_desc, value);

        ValueSet->Items[i].Value = value;
        memcpy(ValueSet->Items[i].Desc, tmp_desc, CVI_PARAM_MENU_ITEM_DESC_LEN);
    }

    return 0;
}


static int32_t CVI_PARAM_LoadUserDataSet(const char *file, void *param, const char *section)
{
    long int bBootFirst = 0;
    unsigned char num = false;

    CVI_PARAM_USER_MENU_S *ValueSet = param;
    printf("file: %s, section：%s\n", file, section);

    bBootFirst = CVI_INI_GetLong(section, "bBootFirst", 0, file);
    num = (unsigned char)bBootFirst;
    printf("%s: %ld: %d\n", __func__, bBootFirst, num);

    ValueSet->bBootFirst = num;


    return 0;
}

static int32_t PARAM_LoadVideoSize(const char *file, CVI_PARAM_VALUESET_S *video_size)
{
    CVI_PARAM_LoadValueSet(file, video_size);
    return 0;
}

static int32_t PARAM_LoadVideoLoop(const char *file, CVI_PARAM_VALUESET_S *video_loop)
{
    CVI_PARAM_LoadValueSet(file, video_loop);
    return 0;
}

static int32_t PARAM_LoadVideoCodec(const char *file, CVI_PARAM_VALUESET_S *video_codec)
{
    CVI_PARAM_LoadValueSet(file, video_codec);
    return 0;
}

static int32_t PARAM_LoadLapseTime(const char *file, CVI_PARAM_VALUESET_S *lapse_time)
{
    CVI_PARAM_LoadValueSet(file, lapse_time);
    return 0;
}

static int32_t PARAM_LoadAudioEnable(const char *file, CVI_PARAM_VALUESET_S *audio_enable)
{
    CVI_PARAM_LoadValueSet(file, audio_enable);
    return 0;
}

static int32_t PARAM_LoadOsdEnable(const char *file, CVI_PARAM_VALUESET_S *osd_enable)
{
    CVI_PARAM_LoadValueSet(file, osd_enable);
    return 0;
}

static int32_t PARAM_LoadScreenDormant(const char *file, CVI_PARAM_VALUESET_S *screen_dormant)
{
    CVI_PARAM_LoadValueSet(file, screen_dormant);
    return 0;
}

static int32_t PARAM_LoadKeyToneEnable(const char *file, CVI_PARAM_VALUESET_S *key_tone)
{
    CVI_PARAM_LoadValueSet(file, key_tone);
    return 0;
}

static int32_t PARAM_LoadFatigueEnable(const char *file, CVI_PARAM_VALUESET_S *fatigue_driving)
{
    CVI_PARAM_LoadValueSet(file, fatigue_driving);
    return 0;
}

static int32_t PARAM_LoadSpeedStampEnable(const char *file, CVI_PARAM_VALUESET_S *speed_stamp)
{
    CVI_PARAM_LoadValueSet(file, speed_stamp);
    return 0;
}

static int32_t PARAM_LoadGPSStampEnable(const char *file, CVI_PARAM_VALUESET_S *GPS_stamp)
{
    CVI_PARAM_LoadValueSet(file, GPS_stamp);
    return 0;
}

static int32_t PARAM_LoadSpeedUnitEnable(const char *file, CVI_PARAM_VALUESET_S *Speed_Unit)
{
    CVI_PARAM_LoadValueSet(file, Speed_Unit);
    return 0;
}
static int32_t PARAM_LoadRearCamMirrorEnable(const char *file, CVI_PARAM_VALUESET_S *RearCam_Mirror)
{
    CVI_PARAM_LoadValueSet(file, RearCam_Mirror);
    return 0;
}

static int32_t PARAM_LoadLanguageEnable(const char *file, CVI_PARAM_VALUESET_S *Language)
{
    CVI_PARAM_LoadValueSet(file, Language);
    return 0;
}

static int32_t PARAM_LoadTimeFormatEnable(const char *file, CVI_PARAM_VALUESET_S *Time_Format)
{
    CVI_PARAM_LoadValueSet(file, Time_Format);
    return 0;
}
static int32_t PARAM_LoadTimeZoneEnable(const char *file, CVI_PARAM_VALUESET_S *Time_Zone)
{
    CVI_PARAM_LoadValueSet(file, Time_Zone);
    return 0;
}

static int32_t PARAM_LoadFrequenceEnable(const char *file, CVI_PARAM_VALUESET_S *Frequence)
{
    CVI_PARAM_LoadValueSet(file, Frequence);
    return 0;
}

static int32_t PARAM_LoadParkingEnable(const char *file, CVI_PARAM_VALUESET_S *Parking)
{
    CVI_PARAM_LoadValueSet(file, Parking);
    return 0;
}

static int32_t PARAM_LoadUserData(const char *file, CVI_PARAM_USER_MENU_S *UserData)
{
    char *section = "UserData";
    CVI_PARAM_LoadUserDataSet(file, UserData, section);
    return 0;
}

static int32_t PARAM_LoadCarNumStampEnable(const char *file, CVI_PARAM_VALUESET_S *carnum_stamp)
{
    CVI_PARAM_LoadValueSet(file, carnum_stamp);
    return 0;
}

static int32_t PARAM_LoadRecLoopEnable(const char *file, CVI_PARAM_VALUESET_S *rec_loop)
{
    CVI_PARAM_LoadValueSet(file, rec_loop);
    return 0;
}

static int32_t PARAM_LoadPhotoSize(const char *file, CVI_PARAM_VALUESET_S *photo_size)
{
    CVI_PARAM_LoadValueSet(file, photo_size);
    return 0;
}

static int32_t PARAM_LoadMenu(const char *file, CVI_PARAM_MENU_S *Menu)
{
   int32_t  s32Ret = 0;
    s32Ret = PARAM_LoadVideoSize(file, &Menu->VideoSize);
    s32Ret |= PARAM_LoadVideoLoop(file, &Menu->VideoLoop);
    s32Ret |= PARAM_LoadVideoCodec(file, &Menu->VideoCodec);
    s32Ret |= PARAM_LoadLapseTime(file, &Menu->LapseTime);
    s32Ret |= PARAM_LoadAudioEnable(file, &Menu->AudioEnable);
    s32Ret |= PARAM_LoadOsdEnable(file, &Menu->Osd);
    s32Ret |= PARAM_LoadScreenDormant(file, &Menu->ScreenDormant);
    s32Ret |= PARAM_LoadKeyToneEnable(file, &Menu->KeyTone);
    s32Ret |= PARAM_LoadFatigueEnable(file, &Menu->FatigueDirve);
    s32Ret |= PARAM_LoadSpeedStampEnable(file, &Menu->SpeedStamp);
    s32Ret |= PARAM_LoadGPSStampEnable(file, &Menu->GPSStamp);
    s32Ret |= PARAM_LoadSpeedUnitEnable(file, &Menu->SpeedUnit);
    s32Ret |= PARAM_LoadRearCamMirrorEnable(file, &Menu->CamMirror);
    s32Ret |= PARAM_LoadLanguageEnable(file, &Menu->Language);
    s32Ret |= PARAM_LoadTimeFormatEnable(file, &Menu->TimeFormat);
    s32Ret |= PARAM_LoadTimeZoneEnable(file, &Menu->TimeZone);
    s32Ret |= PARAM_LoadFrequenceEnable(file, &Menu->Frequence);
    s32Ret |= PARAM_LoadParkingEnable(file, &Menu->Parking);
    s32Ret |= PARAM_LoadUserData(file, &Menu->UserData);
    s32Ret |= PARAM_LoadCarNumStampEnable(file, &Menu->CarNumStamp);
    s32Ret |= PARAM_LoadRecLoopEnable(file, &Menu->RecLoop);
    s32Ret |= PARAM_LoadPhotoSize(file, &Menu->PhotoSize);
    if (s32Ret != 0) {
        printf("load error\n");
    }
    return 0;
}

int32_t  CVI_INI_PARAM_LoadMenuCfg(CVI_PARAM_MENU_S *MenuMng)
{
    printf("\n---enter: %s\n", __func__);

    uint32_t i = 0;
    char filepath[CVI_PARAM_MODULE_NAME_LEN] = {0};

    for (i = 0; i < g_ParamAccess.module_num; i++) {
        if (strstr(g_ParamAccess.modules[i].name, "config_menu")) {
            memset(filepath, 0, sizeof(filepath));
            snprintf(filepath, sizeof(filepath), "%s%s",
                g_ParamAccess.modules[i].path, g_ParamAccess.modules[i].name);
            // find a media comm file
            PARAM_LoadMenu(filepath, MenuMng);
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
