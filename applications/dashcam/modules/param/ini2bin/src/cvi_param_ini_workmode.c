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
static char* PARAM_GetVideoModeName(int32_t  vidoemode)
{
    switch (vidoemode)
    {
        case CVI_UVC_STREAM_FORMAT_YUV420:
            return "yuv420";
        case CVI_UVC_STREAM_FORMAT_MJPEG:
            return "mjpeg";
        case CVI_UVC_STREAM_FORMAT_H264:
            return "h264";
        default:
            return "mjpeg";
    }
}

static int32_t PARAM_LoadUsbMode(const char *filepath, CVI_PARAM_USB_MODE_S *UsbModeCfg)
{
    long int usb_mode = 0;
    long int count = 0;
    long int defmode = 0;
    long int videomode = 0;
    long int bitrate = 0;
    long int vcaphdl = 0;
    long int vprochdl = 0;
    long int venchdl = 0;
    long int acaphdl = 0;
    long int bufcnt = 0;
    long int bufsize = 0;
    char tmp_section[128] = {0};
    long int i = 0, j = 0;

    usb_mode = CVI_INI_GetLong("common", "usb_mode", 0, filepath);
    UsbModeCfg->UsbWorkMode = usb_mode;
    CVI_INI_GetString("uvc", "dev_path", "", tmp_section, sizeof(tmp_section), filepath);
    snprintf(UsbModeCfg->UvcParam.UvcCfg.szDevPath, sizeof(tmp_section), "%s", tmp_section);
    for (i = 0; i < CVI_UVC_STREAM_FORMAT_BUTT; i++) {
        count = CVI_INI_GetLong(PARAM_GetVideoModeName(i), "count", 0, filepath);
        defmode = CVI_INI_GetLong(PARAM_GetVideoModeName(i), "video_defmode", 0, filepath);
        UsbModeCfg->UvcParam.UvcCfg.stDevCap.astFmtCaps[i].u32Cnt = count;
        UsbModeCfg->UvcParam.UvcCfg.stDevCap.astFmtCaps[i].enDefMode = defmode;
        for (j = 0; j < count && j < CVI_UVC_VIDEOMODE_BUTT; j++) {
            videomode = CVI_INI_GetLong(PARAM_GetVideoModeName(i), "video_mode0", 0, filepath);
            bitrate = CVI_INI_GetLong(PARAM_GetVideoModeName(i), "video_bitrate0", 0, filepath);
            UsbModeCfg->UvcParam.UvcCfg.stDevCap.astFmtCaps[i].astModes[i].u32BitRate = bitrate;
            UsbModeCfg->UvcParam.UvcCfg.stDevCap.astFmtCaps[i].astModes[i].enMode = videomode;
        }
    }
    vcaphdl = CVI_INI_GetLong("datasource", "vcap_id", 0, filepath);
    vprochdl = CVI_INI_GetLong("datasource", "vporc_id", 0, filepath);
    venchdl = CVI_INI_GetLong("datasource", "vporc_chn_id", 0, filepath);
    acaphdl = CVI_INI_GetLong("datasource", "acap_id", 0, filepath);
    UsbModeCfg->UvcParam.VcapId = vcaphdl;
    UsbModeCfg->UvcParam.VprocId = vprochdl;
    UsbModeCfg->UvcParam.VprocChnId = venchdl;
    UsbModeCfg->UvcParam.AcapId = acaphdl;
    bufsize = CVI_INI_GetLong("buffer", "buffer_size", 0, filepath);
    bufcnt = CVI_INI_GetLong("buffer", "buffer_count", 0, filepath);
    UsbModeCfg->UvcParam.UvcCfg.stBufferCfg.u32BufCnt = bufcnt;
    UsbModeCfg->UvcParam.UvcCfg.stBufferCfg.u32BufSize = bufsize;

    memset(tmp_section, 0, sizeof(tmp_section));
    CVI_INI_GetString("storage", "dev_path", "", tmp_section, sizeof(tmp_section), filepath);
    snprintf(UsbModeCfg->StorageCfg.szDevPath, sizeof(tmp_section), "%s", tmp_section);

    memset(tmp_section, 0, sizeof(tmp_section));
    CVI_INI_GetString("storage", "sysfile", "", tmp_section, sizeof(tmp_section), filepath);
    snprintf(UsbModeCfg->StorageCfg.szSysFile, sizeof(tmp_section), "%s", tmp_section);

    memset(tmp_section, 0, sizeof(tmp_section));
    CVI_INI_GetString("storage", "usb_state_proc", "", tmp_section, sizeof(tmp_section), filepath);
    snprintf(UsbModeCfg->StorageCfg.szProcFile, sizeof(tmp_section), "%s", tmp_section);

    printf("%s------->usb_mode: %ld, szDevPath: %s\n", __func__, usb_mode, UsbModeCfg->UvcParam.UvcCfg.szDevPath);
    printf("%s------->bufcnt: %ld, bufsize: %ld\n", __func__, bufcnt, bufsize);
    printf("%s------->szProcFile: %s\n", __func__, UsbModeCfg->StorageCfg.szProcFile);

    return 0;
}

static int32_t PARAM_LoadVpss(const char *file, CVI_PARAM_VPSS_ATTR_S *Vpss)
{
    long int i = 0;
    long int mode = 0;
    long int input = 0;
    long int pipe = 0;
    char tmp_section[32] = {0};

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++){
        snprintf(tmp_section, sizeof(tmp_section), "pipe%ld", i);
        pipe = CVI_INI_GetLong("vi_vpss_mode", tmp_section, 0, file);
        Vpss->stVIVPSSMode.aenMode[i] = pipe;
        printf("vivpssmode0-------------> %ld \n", pipe);
    }

    mode = CVI_INI_GetLong("vpss_mode", "mode", 0, file);
    Vpss->stVPSSMode.enMode = mode;
    printf("vpssmode-------------> %ld \n", mode);
    for(i = 0; i < VPSS_IP_NUM; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vpss_mode.dev%ld", i);
        input = CVI_INI_GetLong(tmp_section, "input", 0, file);
        pipe = CVI_INI_GetLong(tmp_section, "pipe", 0, file);
        Vpss->stVPSSMode.aenInput[i] = input;
        Vpss->stVPSSMode.ViPipe[i] = pipe;
        printf("Dev%ld input-------> %ld pipe-----> %ld \n", i, input, pipe);
    }

    return 0;
}

static int32_t PARAM_LoadMediaMode(const char *filepath, CVI_PARAM_MODE_S *MediaModeCfg)
{
    long int cam_num = 0;
    PARAM_LoadVpss(filepath, &(MediaModeCfg->Vpss));
    cam_num = CVI_INI_GetLong("common", "cam_num", 0, filepath);
    MediaModeCfg->CamNum = cam_num;
    for (int32_t  i = 0; i < cam_num; i++) {
        long int camid = 0;
        uint32_t curmediamode = 0;
        char tmp[16] = {0};
        char tmp_section[16] = {0};
        snprintf(tmp, sizeof(tmp), "config%d", i);
        camid = CVI_INI_GetLong(tmp, "cam_id", 0, filepath);
        CVI_INI_GetString(tmp, "media_mode", "", tmp_section,
                        CVI_PARAM_MODULE_NAME_LEN, filepath);
        CVI_INI_PARAM_MediaString2Uint(&curmediamode, tmp_section);
        MediaModeCfg->CamMediaInfo[i].CamID = camid;
        MediaModeCfg->CamMediaInfo[i].CurMediaMode = curmediamode;
        printf("%s: cam_num: %ld, camid: %ld, curmediamode: %d\n", __func__, cam_num, camid, curmediamode);
    }

    return 0;
}

static int32_t PARAM_LoadWorkMode(const char *filepath, CVI_PARAM_WORK_MODE_S *WorkModeCfg)
{

    char mode_name[CVI_PARAM_MODULE_NAME_LEN] = {0};

    CVI_INI_GetString("common", "work_mode", "", mode_name, CVI_PARAM_MODULE_NAME_LEN, filepath);

    if(strcmp(mode_name, "record") == 0) {
        PARAM_LoadMediaMode(filepath, &WorkModeCfg->RecordMode);
    } else if(strcmp(mode_name, "photo") == 0) {
        PARAM_LoadMediaMode(filepath, &WorkModeCfg->PhotoMode);
    } else if(strcmp(mode_name, "playback") == 0) {

    } else if(strcmp(mode_name, "usbcam") == 0) {

    } else if(strcmp(mode_name, "usb") == 0) {
        PARAM_LoadUsbMode(filepath, &WorkModeCfg->UsbMode);
    } else {
        printf("error mode: %s\n", mode_name);
        return -1;
    }
    printf("%s: work_mode: %s\n", __func__, mode_name);
    return 0;
}

int32_t  CVI_INI_PARAM_LoadWorkModeCfg(CVI_PARAM_WORK_MODE_S *WorkModeCfg)
{
    printf("\n---enter: %s\n", __func__);
    uint32_t i = 0;
    char filepath[CVI_PARAM_MODULE_NAME_LEN];

    for (i = 0; i < g_ParamAccess.module_num; i++) {
        if (strstr(g_ParamAccess.modules[i].name, "config_workmode")) {
            memset(filepath, 0, sizeof(filepath));
            snprintf(filepath, sizeof(filepath), "%s%s",
                g_ParamAccess.modules[i].path, g_ParamAccess.modules[i].name);
            // find a workmode file
            PARAM_LoadWorkMode(filepath, WorkModeCfg);
        }
    }
    // WorkMode->ModeCnt = j;
    return 0;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
