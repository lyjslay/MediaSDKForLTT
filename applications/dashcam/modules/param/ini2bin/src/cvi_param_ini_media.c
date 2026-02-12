#include <string.h>
#include "cvi_param.h"
#include "cvi_param_ini2bin.h"
#include "cvi_ini.h"
#include "cvi_media_init.h"
#include "cvi_mode.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


extern CVI_PARAM_ACCESS g_ParamAccess;
// /* common func */
// static uint32_t HexString2Dec(const unsigned char *line, uint32_t len)
// {
//     unsigned char c;
//     unsigned char ch;
//     uint32_t value = 0;

//     if (len == 0) {
//         return -1;
//     }
//     for (value = 0; len--; line++) {
//         ch = *line;
//         if ((ch >= '0') && (ch <= '9')) {
//             value = value * 16 + (ch - '0');
//             continue;
//         }

//         c = (unsigned char) (ch | 0x20);
//         if (c >= 'a' && c <= 'f') {
//             value = value * 16 + (c - 'a' + 10);
//             continue;
//         }
//         return -1;
//     }
//     return value;
// }


/* common config start */
static int32_t PARAM_LoadMode(const char *filename, uint32_t *mode)
{
    char tmp_mode[16] = {0};

    CVI_INI_GetString("work_mode", "poweron_mode", "",
        tmp_mode, 16, filename);
    printf("%s: poweron_mode: %s\n", __func__, tmp_mode);

    if(strcmp(tmp_mode, "record") == 0) {
        *mode = CVI_WORK_MODE_MOVIE;
    } else if(strcmp(tmp_mode, "photo") == 0) {
        *mode = CVI_WORK_MODE_PHOTO;
    } else if(strcmp(tmp_mode, "playback") == 0) {
        *mode = CVI_WORK_MODE_PLAYBACK;
    } else if(strcmp(tmp_mode, "usbcam") == 0) {
        *mode = CVI_WORK_MODE_USBCAM;
    } else if(strcmp(tmp_mode, "usb") == 0) {
        *mode = CVI_WORK_MODE_USBCAM;
    } else {
        printf("error mode: %s\n", tmp_mode);
        return -1;
    }

    printf("work_mode index: %u\n", *mode);

    return 0;
}

static int32_t PARAM_LoadVO(const char *file, CVI_PARAM_DISP_ATTR_S *Vo)
{
    long int i = 0;
    long int vo_cnt = 0;
    long int width = 0;
    long int height = 0;
    long int rotate = 0;
    char tmp_section[32] = {0};
    long int fps = 25;

    vo_cnt = CVI_INI_GetLong("vo_config", "vo_cnt", 0, file);
    printf("%s: vo_cnt: %ld\n", __func__, vo_cnt);

    for (i = 0; i < vo_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "vo%ld", i);
        width = CVI_INI_GetLong(tmp_section, "width", 0, file);
        height = CVI_INI_GetLong(tmp_section, "height", 0, file);
        rotate = CVI_INI_GetLong(tmp_section, "rotate", 0, file);
        fps = CVI_INI_GetLong(tmp_section, "fps", 0, file);

        Vo->Width = width;
        Vo->Height = height;
        Vo->Rotate = rotate;
        Vo->Fps = fps;

        printf("%ld %ld %ld %ld\n", width, height, rotate, fps);
    }

    return 0;
}

static int32_t PARAM_LoadWindow(const char *file, CVI_PARAM_WND_ATTR_S *Window)
{
#ifdef SERVICES_LIVEVIEW_ON
    long int i = 0;
    long int window_cnt = 0;
    long int enable = 0;
    long int used_crop = 0;
    long int small_win_enable = 0;
    long int bind_vproc_id = 0;
    long int bind_vproc_chn_id = 0;
    long int x = 0;
    long int y = 0;
    long int width = 0;
    long int s_x = 0;
    long int s_y = 0;
    long int height = 0;
    long int s_width = 0;
    long int s_height = 0;
    long int onestep = 0;
    long int ystep = 0;
    long int mirror = 0;
    long int filp = 0;
    char tmp_section[32] = {0};

    window_cnt = CVI_INI_GetLong("window_config", "window_cnt", 0, file);
    printf("%s: window_cnt: %ld\n", __func__, window_cnt);
    Window->WndCnt = window_cnt;

    for (i = 0; i < window_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "window%ld", i);

        enable = CVI_INI_GetLong(tmp_section, "enable", 0, file);
        used_crop = CVI_INI_GetLong(tmp_section, "used_crop", 0, file);
        small_win_enable = CVI_INI_GetLong(tmp_section, "small_win_enable", 0, file);
        bind_vproc_id = CVI_INI_GetLong(tmp_section, "bind_vproc_id", 0, file);
        bind_vproc_chn_id = CVI_INI_GetLong(tmp_section, "bind_vproc_chn_id", 0, file);
        x = CVI_INI_GetLong(tmp_section, "x", 0, file);
        y = CVI_INI_GetLong(tmp_section, "y", 0, file);
        width = CVI_INI_GetLong(tmp_section, "width", 0, file);
        height = CVI_INI_GetLong(tmp_section, "height", 0, file);
        s_x = CVI_INI_GetLong(tmp_section, "s_x", 0, file);
        s_y = CVI_INI_GetLong(tmp_section, "s_y", 0, file);
        s_width = CVI_INI_GetLong(tmp_section, "s_width", 0, file);
        s_height = CVI_INI_GetLong(tmp_section, "s_height", 0, file);
        onestep = CVI_INI_GetLong(tmp_section, "onestep", 0, file);
        ystep = CVI_INI_GetLong(tmp_section, "ystep", 0, file);
        mirror = CVI_INI_GetLong(tmp_section, "mirror", 0, file);
        filp = CVI_INI_GetLong(tmp_section, "filp", 0, file);

        Window->Wnds[i].WndEnable = enable;
        Window->Wnds[i].UsedCrop = used_crop;
        Window->Wnds[i].SmallWndEnable = small_win_enable;
        Window->Wnds[i].BindVprocId = bind_vproc_id;
        Window->Wnds[i].BindVprocChnId = bind_vproc_chn_id;
        Window->Wnds[i].WndX = x;
        Window->Wnds[i].WndY = y;
        Window->Wnds[i].WndWidth = width;
        Window->Wnds[i].WndHeight = height;
        Window->Wnds[i].WndsX = s_x;
        Window->Wnds[i].WndsY = s_y;
        Window->Wnds[i].WndsWidth = s_width;
        Window->Wnds[i].WndsHeight = s_height;
        Window->Wnds[i].OneStep = onestep;
        Window->Wnds[i].yStep = ystep;
        Window->Wnds[i].WndMirror = mirror;
        Window->Wnds[i].WndFilp = filp;

        printf("%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
            enable, used_crop, small_win_enable, bind_vproc_id,
            bind_vproc_chn_id, x, y, width, height, s_x, s_y,
            s_width, s_height);
    }
#endif
    return 0;
}

static int32_t PARAM_LoadAi(const char *file, CVI_MAPI_ACAP_ATTR_S *Ai)
{
    // uint32_t i = 0;
    // long int ai_cnt = 0;
    long int sample_rate = 0;
    long int channel = 0;
    long int num_per_frm = 0;
    long int isVquOn = 0;
    long int audio_channel = 0;
    long int volume = 0;
    //long int resample = 0;
    char tmp_section[16] = {0};

    // ai_cnt = CVI_INI_GetLong("ai_config", "ai_cnt", 0, file);
    // printf("%s: ai_cnt: %ld\n", __func__, ai_cnt);
    // Ai->ChnCnt = ai_cnt;

    //for (i = 0; i < ai_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "ai");
        sample_rate = CVI_INI_GetLong(tmp_section, "sample_rate", 0, file);
        channel = CVI_INI_GetLong(tmp_section, "channel", 0, file);
        num_per_frm = CVI_INI_GetLong(tmp_section, "num_per_frm", 0, file);
        isVquOn = CVI_INI_GetLong(tmp_section, "isVquOn", 0, file);
        audio_channel = CVI_INI_GetLong(tmp_section, "audio_channel", 0, file);
        volume = CVI_INI_GetLong(tmp_section, "volume", 20, file);
        //resample = CVI_INI_GetLong(tmp_section, "resample", 0, file);

        Ai->enSampleRate = sample_rate;
        Ai->channel = channel;
        Ai->u32PtNumPerFrm = num_per_frm;
        Ai->bVqeOn = isVquOn;
        Ai->AudioChannel = audio_channel;
        Ai->volume = volume;
        //Ai->resample = resample;

        printf("%ld %ld %ld\n", sample_rate, channel, num_per_frm);
    //}

    return 0;
}

static int32_t PARAM_LoadAenc(const char *file, CVI_MAPI_AENC_ATTR_S *Aenc)
{
    long int format = 0;
    long int sample_rate = 0;
    long int num_per_frm = 0;
    long int channel = 0;
    char tmp_section[16] = {0};

    snprintf(tmp_section, sizeof(tmp_section), "aenc");
    format = CVI_INI_GetLong(tmp_section, "format", 0, file);
    sample_rate = CVI_INI_GetLong(tmp_section, "sample_rate", 0, file);
    num_per_frm = CVI_INI_GetLong(tmp_section, "num_per_frm", 0, file);
    channel = CVI_INI_GetLong(tmp_section, "channel", 0, file);

    Aenc->enAencFormat = format;
    Aenc->src_samplerate = sample_rate;
    Aenc->u32PtNumPerFrm = num_per_frm;
    Aenc->channels = channel;

    printf("anec param: format=%ld, sample_rate=%ld, num_per_frm=%ld, channel=%ld\n",
            format, sample_rate, num_per_frm, channel);

    return 0;
}

static int32_t PARAM_LoadAo(const char *file, CVI_MAPI_AO_ATTR_S *Ao)
{
    // uint32_t i = 0;
    // long int ao_cnt = 0;
    long int sample_rate = 0;
    long int channel = 0;
    long int num_per_frm = 0;
    long int audio_channel = 0;
    long int volume = 0;
    char tmp_section[16] = {0};

    // ao_cnt = CVI_INI_GetLong("ao_config", "ao_cnt", 0, file);
    // printf("%s: ao_cnt: %ld\n", __func__, ao_cnt);
    // Ao->ChnCnt = ao_cnt;

    // for (i = 0; i < ao_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "ao");
        sample_rate = CVI_INI_GetLong(tmp_section, "sample_rate", 0, file);
        channel = CVI_INI_GetLong(tmp_section, "channel", 0, file);
        num_per_frm = CVI_INI_GetLong(tmp_section, "num_per_frm", 0, file);
        audio_channel = CVI_INI_GetLong(tmp_section, "audio_channel", 0, file);
        volume = CVI_INI_GetLong(tmp_section, "volume", 32, file);

        Ao->enSampleRate = sample_rate;
        Ao->channels = channel;
        Ao->u32PtNumPerFrm = num_per_frm;
        Ao->AudioChannel = audio_channel;
        Ao->volume = volume;

        printf("%ld %ld %ld\n", sample_rate, channel, num_per_frm);
    // }

    return 0;
}

#ifdef SERVICES_SPEECH_ON
static int32_t PARAM_LoadSpeech(const char *file, CVI_SPEECHMNG_PARAM_S *Speech)
{
    long int enable = 0;
    long int sample_rate = 0;
    long int bitWidth = 0;
    char model_path[128] = "";
    char tmp_section[16] = {0};

    snprintf(tmp_section, sizeof(tmp_section), "speech");
    enable = CVI_INI_GetLong(tmp_section, "enable", 0, file);
    sample_rate = CVI_INI_GetLong(tmp_section, "sample_rate", 0, file);
    bitWidth = CVI_INI_GetLong(tmp_section, "bitWidth", 0, file);
    CVI_INI_GetString(tmp_section, "model_path", "", model_path, sizeof(model_path), file);

    Speech->enable = enable;
    Speech->SampleRate = sample_rate;
    Speech->BitWidth = bitWidth;
    strncpy(Speech->ModelPath, model_path, sizeof(Speech->ModelPath));

    printf("%ld %ld %s\n", sample_rate, bitWidth, model_path);

    return 0;
}
#endif

static int32_t PARAM_LoadOsd(const char *file, CVI_PARAM_MEDIA_OSD_ATTR_S *Osd)
{
    long int i = 0, j = 0;
    long int osd_cnt = 0;
    // display params
    long int enable = 0;
    long int bind_mode = 0;
    long int mod_hdl = 0;
    long int chn_hdl = 0;
    long int coordinate = 0;
    long int start_x = 0;
    long int start_y = 0;
    long int batch = 0;
    long int display_num = 0;
    // content params
    long int type = 0;
    long int color = 0;
    // -time
    long int time_fmt = 0;
    long int width = 0;
    long int height = 0;
    long int bg_color = 0;
    // -str
    char str[CVI_MAPI_OSD_MAX_STR_LEN] = {0};
    // -circle
    // -bmp
    long int pixel_fmt = 0;

    char tmp_section[64] = {0};

    osd_cnt = CVI_INI_GetLong("osd_config", "osd_cnt", 0, file);
    printf("%s: osd_cnt: %ld\n", __func__, osd_cnt);
    Osd->OsdCnt = osd_cnt;

    for (i = 0; i < osd_cnt; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "osd_content%ld", i);
        type = CVI_INI_GetLong(tmp_section, "type", 0, file);
        // TODO: char converter to uint32_t: "0xffff" --> 65535
        color = CVI_INI_GetLong(tmp_section, "color", 0, file);
        display_num = CVI_INI_GetLong(tmp_section, "display_num", 0, file);
        Osd->OsdAttrs[i].u32DispNum = display_num;
        Osd->OsdAttrs[i].stContent.enType = type;
        Osd->OsdAttrs[i].stContent.u32Color = color;

        if (type == 0) {
            // time
            time_fmt = CVI_INI_GetLong(tmp_section, "time_fmt", 0, file);
            width = CVI_INI_GetLong(tmp_section, "width", 0, file);
            height = CVI_INI_GetLong(tmp_section, "height", 0, file);
            // TODO: char converter to uint32_t: "0xffff" --> 65535
            bg_color = CVI_INI_GetLong(tmp_section, "bg_color", 0, file);
            Osd->OsdAttrs[i].stContent.stTimeContent.enTimeFmt = time_fmt;
            Osd->OsdAttrs[i].stContent.stTimeContent.stFontSize.u32Width = width;
            Osd->OsdAttrs[i].stContent.stTimeContent.stFontSize.u32Height = height;
            Osd->OsdAttrs[i].stContent.stTimeContent.u32BgColor = bg_color;
        } else if (type == 1) {
            // string
            // TODO: get str
            char string[CVI_MAPI_OSD_MAX_STR_LEN] = {0};
            width = CVI_INI_GetLong(tmp_section, "width", 0, file);
            height = CVI_INI_GetLong(tmp_section, "height", 0, file);
            // TODO: char converter to uint32_t: "0xffff" --> 65535
            bg_color = CVI_INI_GetLong(tmp_section, "bg_color", 0, file);
            CVI_INI_GetString(tmp_section, "string", "", string, CVI_MAPI_OSD_MAX_STR_LEN, file);
            snprintf(Osd->OsdAttrs[i].stContent.stStrContent.szStr, CVI_MAPI_OSD_MAX_STR_LEN, "%s", string);
            Osd->OsdAttrs[i].stContent.stStrContent.stFontSize.u32Width = width;
            Osd->OsdAttrs[i].stContent.stStrContent.stFontSize.u32Height = height;
            Osd->OsdAttrs[i].stContent.stStrContent.u32BgColor = bg_color;
        } else if (type == 2) {
            // bmp
            pixel_fmt = CVI_INI_GetLong(tmp_section, "pixel_fmt", 0, file);
            Osd->OsdAttrs[i].stContent.stBitmapContent.enPixelFormat = pixel_fmt;
        } else if (type == 3) {
            // circle
            width = CVI_INI_GetLong(tmp_section, "width", 0, file);
            height = CVI_INI_GetLong(tmp_section, "height", 0, file);
            Osd->OsdAttrs[i].stContent.stCircleContent.u32Width = width;
            Osd->OsdAttrs[i].stContent.stCircleContent.u32Height = height;
         } else if (type == 4) {
            // object
            width = CVI_INI_GetLong(tmp_section, "width", 0, file);
            height = CVI_INI_GetLong(tmp_section, "height", 0, file);
            Osd->OsdAttrs[i].stContent.stObjectContent.u32Width = width;
            Osd->OsdAttrs[i].stContent.stObjectContent.u32Height = height;
        } else {
            printf("[Error] no such osd type\n");
            return -1;
        }
        printf("%ld %ld %ld %ld %ld %ld %s %ld %ld\n",
                type, color, time_fmt, width, height, bg_color, str, pixel_fmt, display_num);
        for (j = 0 ;j < display_num; j++) {
            memset(tmp_section, 0, sizeof(tmp_section));
            snprintf(tmp_section, sizeof(tmp_section), "osd_content%ld.osd_display%ld", i,j);
            enable = CVI_INI_GetLong(tmp_section, "enable", 0, file);
            bind_mode = CVI_INI_GetLong(tmp_section, "bind_mode", 0, file);
            mod_hdl = CVI_INI_GetLong(tmp_section, "mod_hdl", 0, file);
            chn_hdl = CVI_INI_GetLong(tmp_section, "chn_hdl", 0, file);
            coordinate = CVI_INI_GetLong(tmp_section, "coordinate", 0, file);
            start_x = CVI_INI_GetLong(tmp_section, "start_x", 0, file);
            start_y = CVI_INI_GetLong(tmp_section, "start_y", 0, file);
            batch = CVI_INI_GetLong(tmp_section, "batch", 0, file);

            Osd->OsdAttrs[i].astDispAttr[j].bShow = enable;
            Osd->OsdAttrs[i].astDispAttr[j].enBindedMod = bind_mode;
            Osd->OsdAttrs[i].astDispAttr[j].ModHdl = mod_hdl;
            Osd->OsdAttrs[i].astDispAttr[j].ChnHdl = chn_hdl;
            Osd->OsdAttrs[i].astDispAttr[j].enCoordinate = coordinate;
            Osd->OsdAttrs[i].astDispAttr[j].stStartPos.s32X = start_x;
            Osd->OsdAttrs[i].astDispAttr[j].stStartPos.s32Y = start_y;
            Osd->OsdAttrs[i].astDispAttr[j].u32Batch = batch;

            printf("%ld %ld %ld %ld %ld %ld %ld %ld\n",
                enable, bind_mode, mod_hdl, chn_hdl, coordinate,
                start_x, start_y, batch);
        }

    }
    return 0;
}

static int32_t PARAM_LoadRecord(const char *file, CVI_PARAM_RECORD_ATTR_S *Record)
{
    long int i = 0;
    long int rec_cnt = 0;
    long int enable = 0;
    long int sub_video_en = 0;
    long int sub_bind_venc_id = 0;
    long int bind_venc_id = 0;
    long int audio_status = 0;
    long int file_type = 0;
    long int split_time = 0;
    long int pre_time = 0;
    long int post_time = 0;
    float timelapse_recorder_fps = 0;
    long int timelapse_recorder_gop = 0;
    long int memory_buffer_sec = 0;
    long int pre_alloc_unit = 0;
    float normal_extend_video_buffer_sec = -1;
    float event_extend_video_buffer_sec = -1;
    float extend_other_buffer_sec = -1;
    float short_file_ms = -1;
    char model[32] = {0};
    char tmp_section[32] = {0};

    rec_cnt = CVI_INI_GetLong("record_config", "rec_cnt", 0, file);
    printf("%s: rec_cnt: %ld\n", __func__, rec_cnt);
    Record->ChnCnt = rec_cnt;

    for (i = 0; i < rec_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "record%ld", i);
        enable = CVI_INI_GetLong(tmp_section, "enable", 0, file);
        sub_video_en = CVI_INI_GetLong(tmp_section, "sub_video_en", 0, file);
        bind_venc_id = CVI_INI_GetLong(tmp_section, "bind_venc_id", 0, file);
        audio_status = CVI_INI_GetLong(tmp_section, "audio_status", 0, file);
        file_type = CVI_INI_GetLong(tmp_section, "file_type", 0, file);
        split_time = CVI_INI_GetLong(tmp_section, "split_time", 0, file);
        pre_time = CVI_INI_GetLong(tmp_section, "pre_time", 0, file);
        post_time = CVI_INI_GetLong(tmp_section, "post_time", 0, file);
        timelapse_recorder_fps = CVI_INI_GetFloat(tmp_section, "timelapse_recorder_fps", 0.f, file);
        timelapse_recorder_gop = CVI_INI_GetLong(tmp_section, "timelapse_recorder_gop", 0, file);
        memory_buffer_sec = CVI_INI_GetLong(tmp_section, "memory_buffer_sec", 0, file);
        pre_alloc_unit = CVI_INI_GetLong(tmp_section, "pre_alloc_unit", 0, file);
        normal_extend_video_buffer_sec = CVI_INI_GetFloat(tmp_section, "normal_extend_video_buffer_sec", -1, file);
        event_extend_video_buffer_sec = CVI_INI_GetFloat(tmp_section, "event_extend_video_buffer_sec", -1, file);
        extend_other_buffer_sec = CVI_INI_GetFloat(tmp_section, "extend_other_buffer_sec", -1, file);
        short_file_ms = CVI_INI_GetFloat(tmp_section, "short_file_ms", -1, file);
        CVI_INI_GetString(tmp_section, "devmodel", "", model, 31, file);

        if(sub_video_en == 1){
            char tmp_sub_section[32] = {0};
            snprintf(tmp_sub_section, sizeof(tmp_sub_section), "record%ld_sub", i);
            sub_bind_venc_id = CVI_INI_GetLong(tmp_sub_section, "sub_bind_venc_id", 0, file);
        }


        Record->ChnAttrs[i].Enable = enable;
        Record->ChnAttrs[i].Subvideoen = sub_video_en;
        Record->ChnAttrs[i].SubBindVencId = sub_bind_venc_id;
        Record->ChnAttrs[i].AudioStatus = audio_status;
        Record->ChnAttrs[i].BindVencId = bind_venc_id;
        Record->ChnAttrs[i].FileType = file_type;
        Record->ChnAttrs[i].SplitTime = split_time;
        Record->ChnAttrs[i].PreTime = pre_time;
        Record->ChnAttrs[i].PostTime = post_time;
        Record->ChnAttrs[i].TimelapseFps = timelapse_recorder_fps;
        Record->ChnAttrs[i].TimelapseGop = timelapse_recorder_gop;
        Record->ChnAttrs[i].MemoryBufferSec = memory_buffer_sec;
        Record->ChnAttrs[i].PreallocUnit = pre_alloc_unit;
        Record->ChnAttrs[i].NormalExtendVideoBufferSec = normal_extend_video_buffer_sec;
        Record->ChnAttrs[i].EventExtendVideoBufferSec = event_extend_video_buffer_sec;
        Record->ChnAttrs[i].ExtendOtherBufferSec = extend_other_buffer_sec;
        Record->ChnAttrs[i].ShortFileMs = short_file_ms;
        strncpy(Record->ChnAttrs[i].devmodel, model, sizeof(Record->ChnAttrs[i].devmodel) - 1);

        printf("%ld %ld %ld %ld %ld %ld %ld %f %ld %ld %f %f %f %f\n",
            enable, audio_status, bind_venc_id, file_type, split_time, pre_time,
            post_time, timelapse_recorder_fps, timelapse_recorder_gop, pre_alloc_unit,
            normal_extend_video_buffer_sec, event_extend_video_buffer_sec, extend_other_buffer_sec, short_file_ms);
    }

    return 0;
}

static int32_t PARAM_LoadRtsp(const char *file, CVI_PARAM_RTSP_ATTR_S *Rtsp)
{
    long int i = 0;
    long int rtsp_cnt = 0;
    long int enable = 0;
    long int bind_venc_id = 0;
    long int max_connections = 0;
    long int timeout = 0;
    long int port = 0;
    char name[32] = {0};
    char tmp_section[32] = {0};

    rtsp_cnt = CVI_INI_GetLong("rtsp_config", "rtsp_cnt", 0, file);
    port = CVI_INI_GetLong("rtsp_config", "port", 554, file);
    timeout = CVI_INI_GetLong("rtsp_config", "timeout", 120, file);
    printf("%s: rtsp_cnt: %ld %ld %ld\n", __func__, rtsp_cnt, port, timeout);
    //Rtsp->ChnCnt = rtsp_cnt;

    for (i = 0; i < rtsp_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "rtsp%ld", i);
        enable = CVI_INI_GetLong(tmp_section, "enable", 0, file);
        bind_venc_id = CVI_INI_GetLong(tmp_section, "bind_venc_id", 0, file);
        max_connections = CVI_INI_GetLong(tmp_section, "max_conn", 0, file);
        CVI_INI_GetString(tmp_section, "name", "", name, 31, file);
        if(strlen(name) == 0){
            snprintf(name, sizeof(name), "cvi_cam_%ld", i);
        }

        Rtsp->ChnAttrs[i].Enable = enable;
        Rtsp->ChnAttrs[i].BindVencId = bind_venc_id;
        Rtsp->ChnAttrs[i].MaxConn = max_connections;
        Rtsp->ChnAttrs[i].Timeout = timeout;
        Rtsp->ChnAttrs[i].Port = port;
        strncpy(Rtsp->ChnAttrs[i].Name, name, sizeof(Rtsp->ChnAttrs[i].Name) - 1);

        printf("%ld %ld\n", enable, bind_venc_id);
    }

    return 0;
}

#ifdef ENABLE_ISP_IRCUT
static int32_t PARAM_LoadIsp(const char *file, CVI_PARAM_ISPIR_ATTR_S *IspIr)
{
    int32_t i = 0;
    int32_t isp_cnt = 0;
    int32_t bEnable = 0;
    int32_t s32IRControlMode = 0;
    int32_t s32CamId = 0;
    int32_t s32IrCutA = 0;
    int32_t s32IrCutB = 0;
    int32_t s32LedIr = 0;
    int16_t s16Normal2IrIsoThr = 0;
    int16_t s16Ir2NormalIsoThr = 0;
    char tmp_section[32] = {0};

    isp_cnt = CVI_INI_GetLong("ispir_config", "isp_cnt", 0, file);
    printf("%s: sp_cnt: %d\n", __func__, isp_cnt);

    for (i = 0; i < isp_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "ispir%d", i);
        bEnable = CVI_INI_GetLong(tmp_section, "enable", 0, file);
        s32IRControlMode = CVI_INI_GetLong(tmp_section, "ir_control", 0, file);
        s32CamId = CVI_INI_GetLong(tmp_section, "cam_id", 0, file);
        s32IrCutA = CVI_INI_GetLong(tmp_section, "ir_cut_a", 0, file);
        s32IrCutB = CVI_INI_GetLong(tmp_section, "ir_cut_b", 0, file);
        s32LedIr = CVI_INI_GetLong(tmp_section, "led_ir", 0, file);
        s16Normal2IrIsoThr = CVI_INI_GetLong(tmp_section, "Normal2IrIsoThr", 0, file);
        s16Ir2NormalIsoThr = CVI_INI_GetLong(tmp_section, "Ir2NormalIsoThr", 0, file);

        char DayBinPath[128] = "";
        char NightBinPath[128] = "";
        CVI_INI_GetString(tmp_section, "day_bin_path", "", DayBinPath, sizeof(DayBinPath), file);
        CVI_INI_GetString(tmp_section, "night_bin_path", "", NightBinPath, sizeof(NightBinPath), file);

        IspIr->stIspIrChnAttrs[i].bEnable = bEnable;
        IspIr->stIspIrChnAttrs[i].s32IRControlMode = s32IRControlMode;
        IspIr->stIspIrChnAttrs[i].s32CamId = s32CamId;
        IspIr->stIspIrChnAttrs[i].s32IrCutA = s32IrCutA;
        IspIr->stIspIrChnAttrs[i].s32IrCutB = s32IrCutB;
        IspIr->stIspIrChnAttrs[i].s32LedIr = s32LedIr;
        IspIr->stIspIrChnAttrs[i].s16Normal2IrIsoThr = s16Normal2IrIsoThr;
        IspIr->stIspIrChnAttrs[i].s16Ir2NormalIsoThr = s16Ir2NormalIsoThr;
        memcpy(IspIr->stIspIrChnAttrs[i].DayBinPath, DayBinPath, sizeof(DayBinPath));
        memcpy(IspIr->stIspIrChnAttrs[i].NightBinPath, NightBinPath, sizeof(NightBinPath));

        printf("isp_config%d:bEnable:%d,u32CamId:%d,bEnable:%d,s32IrCutA:%d,s32IrCutB:%d,u32Normal2IrIsoThr:%d,u32Ir2NormalIsoThr:%d\n", i, bEnable, s32CamId, s32IrCutA, s32IrCutB, s32LedIr, s16Normal2IrIsoThr, s16Ir2NormalIsoThr);
    }

    return 0;
}
#endif

static int32_t PARAM_LoadPiv(const char *file, CVI_PARAM_PIV_ATTR_S *Piv)
{
    long int i = 0;
    long int piv_cnt = 0;
    long int bind_venc_id = 0;
    char tmp_section[32] = {0};

    piv_cnt = CVI_INI_GetLong("piv_config", "piv_cnt", 0, file);
    printf("%s: piv_cnt: %ld\n", __func__, piv_cnt);
    //Piv->ChnCnt = piv_cnt;

    for (i = 0; i < piv_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "piv%ld", i);
        bind_venc_id = CVI_INI_GetLong(tmp_section, "bind_venc_id", 0, file);

        Piv->ChnAttrs[i].BindVencId = bind_venc_id;
        printf("%ld\n", bind_venc_id);
    }
    return 0;
}

static int32_t PARAM_LoadThm(const char *file, CVI_PARAM_THUMBNAIL_ATTR_S *Thm)
{
    long int i = 0;
    long int thm_cnt = 0;
    long int bind_venc_id = 0;
    char tmp_section[32] = {0};

    thm_cnt = CVI_INI_GetLong("thm_config", "thm_cnt", 0, file);
    printf("%s: thm_cnt: %ld\n", __func__, thm_cnt);

    for (i = 0; i < thm_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "thm%ld", i);
        bind_venc_id = CVI_INI_GetLong(tmp_section, "bind_venc_id", 0, file);

        Thm->ChnAttrs[i].BindVencId = bind_venc_id;

        printf("%ld\n", bind_venc_id);
    }
    return 0;
}


static int32_t PARAM_LoadPhoto(const char *file, CVI_PARAM_PHOTO_ATTR_S *Photo)
{
    long int i = 0;
    char tmp_section[32] = {0};
    long int ph_cnt = 0;

    ph_cnt = CVI_INI_GetLong("photo_config", "photo_cnt", 0, file);
    Photo->photoid = 0;
    Photo->VprocDev_id = CVI_INI_GetLong("photo_config", "vprocdev_id", 0, file);
    for (i = 0; i < ph_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "photo%ld", i);
        Photo->ChnAttrs[i].Enable = CVI_INI_GetLong(tmp_section, "enable", 0, file);
        Photo->ChnAttrs[i].BindVencId = CVI_INI_GetLong(tmp_section, "bind_venc_id", 0, file);
    }

    printf("photo config %u\n", Photo->VprocDev_id);
    return 0;
}

static int32_t PARAM_LoadVideoMd(const char *file, CVI_PARAM_MD_ATTR_S *MDConfig)
{
    long int i = 0;
    char tmp_section[32] = {0};
    long int md_cnt = 0;
    long int motionSensitivity = 0;

    md_cnt = CVI_INI_GetLong("motionDet_config", "md_cnt", 0, file);
    motionSensitivity = CVI_INI_GetLong("motionDet_config", "motionSensitivity", 0, file);
    MDConfig->motionSensitivity = motionSensitivity;
    for (i = 0; i < md_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "md%ld", i);
        MDConfig->ChnAttrs[i].Enable = CVI_INI_GetLong(tmp_section, "enable", 0, file);
        MDConfig->ChnAttrs[i].BindVprocId = CVI_INI_GetLong(tmp_section, "bind_vproc_id", 0, file);
        MDConfig->ChnAttrs[i].BindVprocChnId = CVI_INI_GetLong(tmp_section, "bind_vproc_chn_id", 0, file);
    }

    printf("md motionSensitivity = %d\n", MDConfig->motionSensitivity);
    return 0;
}

#ifdef SERVICES_ADAS_ON
static int32_t PARAM_LoadADAS(const char *file, CVI_PARAM_ADAS_ATTR_S *ADASConfig)
{
    int32_t i = 0;
    char tmp_section[32] = {0};
    int32_t adas_cnt = 0;
    int32_t width = 0;
    int32_t height = 0;
    char car_model_path[128] = "";
    char lane_model_path[128] = "";
    float fps = 0;

    adas_cnt = CVI_INI_GetLong("adas_config", "adas_cnt", 0, file);
    width = CVI_INI_GetLong("adas_config", "model_vb_width", 0, file);
    height = CVI_INI_GetLong("adas_config", "model_vb_heigt", 0, file);
    fps = CVI_INI_GetFloat("adas_config", "model_fps", 0.f, file);
    CVI_INI_GetString("adas_config", "car_model_path", "", car_model_path, sizeof(car_model_path), file);
    CVI_INI_GetString("adas_config", "lane_model_path", "", lane_model_path, sizeof(lane_model_path), file);

    ADASConfig->adas_cnt = adas_cnt;
    ADASConfig->stADASModelAttr.fps = fps;
    ADASConfig->stADASModelAttr.width = width;
    ADASConfig->stADASModelAttr.height = height;
    strncpy(ADASConfig->stADASModelAttr.CarModelPath, car_model_path, sizeof(ADASConfig->stADASModelAttr.CarModelPath));
    strncpy(ADASConfig->stADASModelAttr.LaneModelPath, lane_model_path, sizeof(ADASConfig->stADASModelAttr.LaneModelPath));
    for (i = 0; i < adas_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "adas%d", i);
        ADASConfig->ChnAttrs[i].Enable = CVI_INI_GetLong(tmp_section, "enable", 0, file);
        ADASConfig->ChnAttrs[i].BindVprocId = CVI_INI_GetLong(tmp_section, "bind_vproc_id", 0, file);
        ADASConfig->ChnAttrs[i].BindVprocChnId = CVI_INI_GetLong(tmp_section, "bind_vproc_chn_id", 0, file);
    }

    printf("adas_cnt = %d\n", adas_cnt);
    return 0;
}
#endif

#ifdef SERVICES_QRCODE_ON
static int32_t PARAM_LoadQRCode(const char *file, CVI_PARAM_QRCODE_ATTR_S *QRCodeConfig)
{
    long int i = 0;
    char tmp_section[32] = {0};
    long int qrcode_cnt = 0;

    qrcode_cnt = CVI_INI_GetLong("qrcode_config", "qrcode_cnt", 0, file);
    QRCodeConfig->qrcode_cnt = qrcode_cnt;
    for (i = 0; i < qrcode_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "qrcode%ld", i);
        QRCodeConfig->ChnAttrs[i].Enable = CVI_INI_GetLong(tmp_section, "enable", 0, file);
        QRCodeConfig->ChnAttrs[i].BindVprocId = CVI_INI_GetLong(tmp_section, "bind_vproc_id", 0, file);
        QRCodeConfig->ChnAttrs[i].BindVprocChnId = CVI_INI_GetLong(tmp_section, "bind_vproc_chn_id", 0, file);
    }

    printf("md qrcode_cnt = %ld\n", qrcode_cnt);
    return 0;
}
#endif

static int32_t PARAM_LoadMediaComm(const char *filename, CVI_PARAM_MEDIA_COMM_S *MediaComm)
{
   int32_t  s32Ret = 0;

    printf("enter: %s, filename: %s\n", __func__, filename);

    s32Ret = PARAM_LoadMode(filename, &MediaComm->PowerOnMode);
    s32Ret = PARAM_LoadVO(filename, &MediaComm->Vo);
    s32Ret = PARAM_LoadWindow(filename, &MediaComm->Window);
    s32Ret = PARAM_LoadAi(filename, &MediaComm->Ai);
    s32Ret = PARAM_LoadAenc(filename, &MediaComm->Aenc);
    s32Ret = PARAM_LoadAo(filename, &MediaComm->Ao);
    s32Ret = PARAM_LoadRecord(filename, &MediaComm->Record);
    s32Ret = PARAM_LoadRtsp(filename, &MediaComm->Rtsp);
    s32Ret = PARAM_LoadPiv(filename, &MediaComm->Piv);
    s32Ret = PARAM_LoadThm(filename, &MediaComm->Thumbnail);
    s32Ret = PARAM_LoadPhoto(filename, &MediaComm->Photo);
    s32Ret = PARAM_LoadVideoMd(filename, &MediaComm->Md);
#ifdef SERVICES_SPEECH_ON
    s32Ret = PARAM_LoadSpeech(filename, &MediaComm->Speech);
#endif
#ifdef SERVICES_ADAS_ON
    s32Ret = PARAM_LoadADAS(filename, &MediaComm->ADAS);
#endif
#ifdef SERVICES_QRCODE_ON
    s32Ret = PARAM_LoadQRCode(filename, &MediaComm->QRCODE);
#endif
#ifdef ENABLE_ISP_IRCUT
    s32Ret = PARAM_LoadIsp(filename, &MediaComm->IspIr);
#endif
    return s32Ret;
}

int32_t  CVI_INI_PARAM_LoadMediaCommCfg(CVI_PARAM_MEDIA_COMM_S *MediaComm)
{
    printf("\n---enter: %s\n", __func__);
    uint32_t i = 0;
    char filepath[CVI_PARAM_MODULE_NAME_LEN] = {0};

    for (i = 0; i < g_ParamAccess.module_num; i++) {
        if (strstr(g_ParamAccess.modules[i].name, "config_media_comm")) {
            memset(filepath, 0, sizeof(filepath));
            snprintf(filepath, sizeof(filepath), "%s%s",
                g_ParamAccess.modules[i].path, g_ParamAccess.modules[i].name);
            // find a media comm file
            PARAM_LoadMediaComm(filepath, MediaComm);
            break;
        }
    }

    if(i >= g_ParamAccess.module_num) {
        printf("error , can not find media comm file\n");
        return -1;
    }

    return 0;
}
/* common config end */


/* special config begin */
static int32_t PARAM_LoadMediaMode(const char *file, uint32_t *MediaMode)
{
   int32_t  s32Ret = 0;
    char tmp_section[16] = {0};
    CVI_INI_GetString("common", "media_mode", "", tmp_section,
                        CVI_PARAM_MODULE_NAME_LEN, file);

    s32Ret = CVI_INI_PARAM_MediaString2Uint(MediaMode, tmp_section);
    if(s32Ret == -1) {
        return -1;
    }
    printf("%s: file: %s modename: %d\n", __func__, file, *MediaMode);
    return 0;
}

static int32_t PARAM_LoadSns(const char *file, CVI_PARAM_MEDIA_SNS_ATTR_S *Sns)
{
    long int enable = 0;
    long int id = 0;
    long int wdrmode = 0;
    long int i2cid = 0;
    long int i2caddr = 0;
    long int hwsync = 0;
    long int cam_clk_id = 0;
    long int rst_gpio_inx = 0;
    long int rst_gpio_pin = 0;
    long int rst_gpio_pol = 0;
    long int mipidev = 0;
   int32_t  i = 0;
    char tmp_section[16] = {0};
    long int tmpid = 0;

    enable = CVI_INI_GetLong("sensor_config", "enable", 0, file);
    id = CVI_INI_GetLong("sensor_config", "id", 0, file);
    wdrmode = CVI_INI_GetLong("sensor_config", "wdrmode", 0, file);
    i2cid = CVI_INI_GetLong("sensor_config", "i2cid", 0, file);
    i2caddr = CVI_INI_GetLong("sensor_config", "i2caddr", 0, file);
    hwsync = CVI_INI_GetLong("sensor_config", "hwsync", 0, file);
    cam_clk_id = CVI_INI_GetLong("sensor_config", "cam_clk_id", 0, file);
    rst_gpio_inx = CVI_INI_GetLong("sensor_config", "rst_gpio_inx", 0, file);
    rst_gpio_pin = CVI_INI_GetLong("sensor_config", "rst_gpio_pin", 0, file);
    rst_gpio_pol = CVI_INI_GetLong("sensor_config", "rst_gpio_pol", 0, file);
    mipidev = CVI_INI_GetLong("sensor_config", "mipidev", 0, file);

    printf("%s: %ld %ld %ld %ld %ld %ld\n",
        __func__, enable, id, wdrmode, i2cid, i2caddr, hwsync);

    Sns->SnsEnable = enable;
    Sns->SnsChnAttr.u8SnsId = id;
    Sns->SnsChnAttr.u8WdrMode = wdrmode;
    Sns->SnsChnAttr.u8I2cBusId = i2cid;
    Sns->SnsChnAttr.u8I2cSlaveAddr = i2caddr;
    Sns->SnsChnAttr.u8HwSync = hwsync;
    Sns->SnsChnAttr.u8MipiDev = mipidev;
    Sns->SnsChnAttr.u8CamClkId = cam_clk_id;
    Sns->SnsChnAttr.u8RstGpioInx = rst_gpio_inx;
    Sns->SnsChnAttr.u8RstGpioPin = rst_gpio_pin;
    Sns->SnsChnAttr.u8RstGpioPol = rst_gpio_pol;
    for (i = 0; i < 5; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "laneid%d", i);
        tmpid = CVI_INI_GetLong("sensor_config", tmp_section, 0, file);
        printf(" lanid%d : %ld", i, tmpid);
        Sns->SnsChnAttr.as8LaneId[i] = tmpid;
    }
    for (i = 0; i < 5; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "swap%d", i);
        tmpid = CVI_INI_GetLong("sensor_config", tmp_section, 0, file);
        printf(" swap%d : %ld", i, tmpid);
        Sns->SnsChnAttr.as8PNSwap[i] = tmpid;
    }

    return 0;
}

static int32_t PARAM_LoadVcap(const char *file, CVI_PARAM_MEDIA_VACP_ATTR_S *Vcap)
{
    long int enable = 0;
    long int id = 0;
    long int width = 0;
    long int height = 0;
    long int fmt = 0;
    long int cpress = 0;
    long int mirror = 0;
    long int filp = 0;
    long int fbmEnable = 0;
    long int vbcnt = 0;
    float fps = 0;
    enable = CVI_INI_GetLong("vcap_config", "enable", 0, file);
    id = CVI_INI_GetLong("vcap_config", "id", 0, file);
    width = CVI_INI_GetLong("vcap_config", "width", 0, file);
    height = CVI_INI_GetLong("vcap_config", "height", 0, file);
    fmt = CVI_INI_GetLong("vcap_config", "fmt", 0, file);
    cpress = CVI_INI_GetLong("vcap_config", "cpress", 0, file);
    mirror = CVI_INI_GetLong("vcap_config", "mirror", 0, file);
    filp = CVI_INI_GetLong("vcap_config", "filp", 0, file);
    fps = CVI_INI_GetFloat("vcap_config", "fps", 0.f, file);
    fbmEnable = CVI_INI_GetFloat("vcap_config", "enfbm", 0, file);
    vbcnt = CVI_INI_GetLong("vcap_config", "chn_vbcnt", 0, file);

    printf("%s: %ld %ld %ld %ld %ld %ld %ld %ld %f\n",
        __func__, enable, id, width, height, fmt, cpress, mirror, filp, fps);

    Vcap->VcapEnable = enable;
    Vcap->VcapId = id;
    Vcap->VcapChnAttr.u32Width = width;
    Vcap->VcapChnAttr.u32Height = height;
    Vcap->VcapChnAttr.enPixelFmt = fmt;
    Vcap->VcapChnAttr.enCompressMode = cpress;
    Vcap->VcapChnAttr.bMirror = mirror;
    Vcap->VcapChnAttr.bFlip = filp;
    Vcap->VcapChnAttr.f32Fps = fps;
    Vcap->VcapChnAttr.fbmEnable = fbmEnable;
    Vcap->VcapChnAttr.vbcnt = vbcnt;

    return 0;
}

static int32_t PARAM_LoadVproc(const char *file, CVI_PARAM_MEDIA_VPROC_ATTR_S *Vproc)
{
    long int i = 0;
    long int id = 0;
    long int width = 0;
    long int height = 0;
    long int fmt = 0;
    long int srcfps = 0;
    long int dstfps = 0;
    long int chn_num = 0;

    long int chn_id = 0;
    long int chn_width = 0;
    long int chn_height = 0;
    long int chn_fmt = 0;
    long int chn_depth = 0;
    long int chn_srcfps = 0;
    long int chn_dstfps = 0;
    long int chn_vfmt = 0;
    long int chn_enable = 0;
    long int chn_mirror = 0;
    long int chn_filp = 0;
    long int chn_vbcnt = 0;
    long int lowdelay_cnt = 0;
    long int fb_on_vpss = 0;

    long int bind_vproc_chn_id = 0;
    char tmp_section[16] = {0};

    id = CVI_INI_GetLong("vproc_config", "id", 0, file);
    width = CVI_INI_GetLong("vproc_config", "width", 0, file);
    height = CVI_INI_GetLong("vproc_config", "height", 0, file);
    fmt = CVI_INI_GetLong("vproc_config", "fmt", 0, file);
    srcfps = CVI_INI_GetLong("vproc_config", "srcfps", 0, file);
    dstfps = CVI_INI_GetLong("vproc_config", "dstfps", 0, file);
    chn_vfmt = CVI_INI_GetLong("vproc_config", "chn_vfmt", 0, file);
    chn_num = CVI_INI_GetLong("vproc_config", "chn_num", 0, file);

    Vproc->Vprocid = id;
    Vproc->VpssGrpAttr.u32MaxW = width;
    Vproc->VpssGrpAttr.u32MaxH = height;
    Vproc->VpssGrpAttr.enPixelFormat = fmt;
    Vproc->VpssGrpAttr.stFrameRate.s32SrcFrameRate = srcfps;
    Vproc->VpssGrpAttr.stFrameRate.s32DstFrameRate = dstfps;

    printf("%s: %ld %ld %ld %ld %ld %ld\n",
        __func__, id, width, height, fmt, srcfps, chn_num);

    for (i = 0; i < chn_num; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "vproc_chn%ld", i);
        printf("section: %s\n", tmp_section);

        chn_id = CVI_INI_GetLong(tmp_section, "chn_id", 0, file);
        chn_width = CVI_INI_GetLong(tmp_section, "chn_width", 0, file);
        chn_height = CVI_INI_GetLong(tmp_section, "chn_height", 0, file);
        chn_fmt = CVI_INI_GetLong(tmp_section, "chn_fmt", 0, file);
        chn_depth = CVI_INI_GetLong(tmp_section, "chn_depth", 0, file);
        chn_srcfps = CVI_INI_GetLong(tmp_section, "chn_srcfps", 0, file);
        chn_dstfps = CVI_INI_GetLong(tmp_section, "chn_dstfps", 0, file);
        chn_enable = CVI_INI_GetLong(tmp_section, "chn_enable", 0, file);
        chn_mirror = CVI_INI_GetLong(tmp_section, "chn_mirror", 0, file);
        chn_filp = CVI_INI_GetLong(tmp_section, "chn_filp", 0, file);
        chn_vbcnt = CVI_INI_GetLong(tmp_section, "chn_vbcnt", 0, file);
        lowdelay_cnt = CVI_INI_GetLong(tmp_section, "lowdelay_cnt", 0, file);
        fb_on_vpss = CVI_INI_GetLong(tmp_section, "fb_on_vpss", 0, file);

        printf("%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
            chn_id, chn_width, chn_height, chn_fmt, chn_depth, chn_srcfps,
            chn_enable, chn_mirror, chn_filp, chn_vbcnt);

        Vproc->VprocChnAttr[i].VprocChnid = chn_id;
        Vproc->VprocChnAttr[i].VprocChnEnable = chn_enable;
        Vproc->VprocChnAttr[i].VpssChnAttr.u32Width = chn_width;
        Vproc->VprocChnAttr[i].VpssChnAttr.u32Height = chn_height;
        Vproc->VprocChnAttr[i].VpssChnAttr.enVideoFormat = chn_vfmt;
        Vproc->VprocChnAttr[i].VpssChnAttr.enPixelFormat = chn_fmt;
        Vproc->VprocChnAttr[i].VpssChnAttr.u32Depth = chn_depth;
        Vproc->VprocChnAttr[i].VpssChnAttr.stFrameRate.s32SrcFrameRate = chn_srcfps;
        Vproc->VprocChnAttr[i].VpssChnAttr.stFrameRate.s32DstFrameRate = chn_dstfps;
        Vproc->VprocChnAttr[i].VpssChnAttr.bMirror = chn_mirror;
        Vproc->VprocChnAttr[i].VpssChnAttr.bFlip = chn_filp;
        Vproc->VprocChnAttr[i].VprocChnVbCnt = chn_vbcnt;
        Vproc->VprocChnAttr[i].VprocChnLowDelayCnt = lowdelay_cnt;
        Vproc->VprocChnAttr[i].bFb = fb_on_vpss;
    }

    for (i = 0; i < CVI_MAPI_VPROC_MAX_CHN_NUM; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "ext_chn%ld", i);
        printf("section: %s\n", tmp_section);
        chn_enable = CVI_INI_GetLong(tmp_section, "chn_enable", 0, file);
        if(chn_enable == 0) {
            continue;
        }
        bind_vproc_chn_id = CVI_INI_GetLong(tmp_section, "bind_vproc_chn_id", 0, file);
        chn_id = CVI_INI_GetLong(tmp_section, "chn_id", 0, file);
        chn_width = CVI_INI_GetLong(tmp_section, "chn_width", 0, file);
        chn_height = CVI_INI_GetLong(tmp_section, "chn_height", 0, file);
        chn_fmt = CVI_INI_GetLong(tmp_section, "chn_fmt", 0, file);
        chn_depth = CVI_INI_GetLong(tmp_section, "chn_depth", 0, file);
        chn_srcfps = CVI_INI_GetLong(tmp_section, "chn_srcfps", 0, file);
        chn_dstfps = CVI_INI_GetLong(tmp_section, "chn_dstfps", 0, file);
        chn_mirror = CVI_INI_GetLong(tmp_section, "chn_mirror", 0, file);
        chn_filp = CVI_INI_GetLong(tmp_section, "chn_filp", 0, file);

        printf("ext_chn:-----> %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
            chn_id, bind_vproc_chn_id, chn_width, chn_height, chn_fmt, chn_depth, chn_srcfps,
            chn_enable, chn_mirror, chn_filp);

        Vproc->ExtChnAttr[i].ChnEnable = chn_enable;
        Vproc->ExtChnAttr[i].ChnAttr.ChnId = chn_id;
        Vproc->ExtChnAttr[i].ChnAttr.BindVprocChnId = bind_vproc_chn_id;
        Vproc->ExtChnAttr[i].ChnAttr.VpssChnAttr.u32Width = chn_width;
        Vproc->ExtChnAttr[i].ChnAttr.VpssChnAttr.u32Height = chn_height;
        Vproc->ExtChnAttr[i].ChnAttr.VpssChnAttr.enVideoFormat = chn_vfmt;
        Vproc->ExtChnAttr[i].ChnAttr.VpssChnAttr.enPixelFormat = chn_fmt;
        Vproc->ExtChnAttr[i].ChnAttr.VpssChnAttr.u32Depth = chn_depth;
        Vproc->ExtChnAttr[i].ChnAttr.VpssChnAttr.stFrameRate.s32SrcFrameRate = chn_srcfps;
        Vproc->ExtChnAttr[i].ChnAttr.VpssChnAttr.stFrameRate.s32DstFrameRate = chn_dstfps;
        Vproc->ExtChnAttr[i].ChnAttr.VpssChnAttr.bMirror = chn_mirror;
        Vproc->ExtChnAttr[i].ChnAttr.VpssChnAttr.bFlip = chn_filp;
    }

    return 0;
}

static int32_t PARAM_LoadVenc(const char *file, CVI_PARAM_MEDIA_VENC_ATTR_S *Venc)
{
    long int i = 0;
    long int chn_num = 0;

    long int enable = 0;
    long int id = 0;
    long int bind_vproc_id = 0;
    long int bind_vproc_chn_id = 0;
    long int venc_bind_mode = 0;
    long int width = 0;
    long int height = 0;
    long int codec = 0;
    long int pixel_fmt = 0;
    long int gop = 0;
    long int profile = 0;
    float fps = 0;
    long int rate_ctrl_mode = 0;
    long int bit_rate = 0;
    long int iqp = 0;
    long int pqp = 0;
    long int min_iqp = 0;
    long int max_iqp = 0;
    long int min_qp = 0;
    long int max_qp = 0;
    long int initialDelay = 0;
    long int thrdLv = 0;
    long int statTime = 0;
    long int change_pos = 0;
    long int jpeg_quality = 0;
    long int single_EsBuf = 0;
    long int bitstream_bufSize = 0;
    long int datafifo_len = 0;
    char tmp_section[16] = {0};
    long int src_framerate = 0;
    long int dst_framerate = 0;
    long int aspectRatio_Flag = 0;
    long int overscan_Flag = 0;
    long int videoSignalType_Flag = 0;
    long int video_Format = 0;
    long int videoFull_Flag = 0;
    long int colourDescription_Flag = 0;
    long int firstFrameStartQp = 0;
    long int maxBitRate = 0;
    long int gop_mode = 0;
    long int maxIprop = 0;
    long int minIprop = 0;
    long int minStillPercent = 0;
    long int maxStillQP = 0;
    long int avbrPureStillThr = 0;
    long int motionSensitivity = 0;
    long int bgDeltaQp = 0;
    long int rowQpDelta = 0;
    long int ipqpDelta = 0;

    chn_num = CVI_INI_GetLong("venc_config", "chn_num", 0, file);
    printf("%s: chn_num: %ld\n", __func__, chn_num);
    // Venc->ChnCnt = chn_num;

    for (i = 0; i < chn_num; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "venc_chn%ld", i);
        printf("section: %s\n", tmp_section);

        enable = CVI_INI_GetLong(tmp_section, "enable", 0, file);
        id = CVI_INI_GetLong(tmp_section, "id", 0, file);
        bind_vproc_id = CVI_INI_GetLong(tmp_section, "bind_vproc_id", 0, file);
        bind_vproc_chn_id = CVI_INI_GetLong(tmp_section, "bind_vproc_chn_id", 0, file);
        venc_bind_mode = CVI_INI_GetLong(tmp_section, "venc_bind_mode", 0, file);
        width = CVI_INI_GetLong(tmp_section, "width", 0, file);
        height = CVI_INI_GetLong(tmp_section, "height", 0, file);
        codec = CVI_INI_GetLong(tmp_section, "codec", 0, file);
        pixel_fmt = CVI_INI_GetLong(tmp_section, "pixel_fmt", 0, file);
        gop = CVI_INI_GetLong(tmp_section, "gop", 0, file);
        profile = CVI_INI_GetLong(tmp_section, "profile", 0, file);
        fps = CVI_INI_GetFloat(tmp_section, "fps", 0.f, file);
        rate_ctrl_mode = CVI_INI_GetLong(tmp_section, "rate_ctrl_mode", 0, file);
        bit_rate = CVI_INI_GetLong(tmp_section, "bit_rate", 0, file);
        iqp = CVI_INI_GetLong(tmp_section, "iqp", 0, file);
        pqp = CVI_INI_GetLong(tmp_section, "pqp", 0, file);
        min_iqp = CVI_INI_GetLong(tmp_section, "min_iqp", 0, file);
        max_iqp = CVI_INI_GetLong(tmp_section, "max_iqp", 0, file);
        min_qp = CVI_INI_GetLong(tmp_section, "min_qp", 0, file);
        max_qp = CVI_INI_GetLong(tmp_section, "max_qp", 0, file);
        change_pos = CVI_INI_GetLong(tmp_section, "change_pos", 0, file);
        jpeg_quality = CVI_INI_GetLong(tmp_section, "jpeg_quality", 0, file);
        single_EsBuf = CVI_INI_GetLong(tmp_section, "single_EsBuf", 0, file);
        bitstream_bufSize = CVI_INI_GetLong(tmp_section, "bufSize", 0, file);
        datafifo_len = CVI_INI_GetLong(tmp_section, "datafifoLen", 10, file);
        src_framerate = CVI_INI_GetLong(tmp_section, "src_framerate", 0, file);
        dst_framerate = CVI_INI_GetLong(tmp_section, "dst_framerate", 0, file);
        initialDelay = CVI_INI_GetLong(tmp_section, "initialdelay", 0, file);
        thrdLv = CVI_INI_GetLong(tmp_section, "thrdlv", 0, file);
        statTime = CVI_INI_GetLong(tmp_section, "stattime", 0, file);
        aspectRatio_Flag = CVI_INI_GetLong(tmp_section, "aspectRatio_Flag", 0, file);
        overscan_Flag = CVI_INI_GetLong(tmp_section, "overscan_Flag", 0, file);
        videoSignalType_Flag = CVI_INI_GetLong(tmp_section, "videoSignalType_Flag", 0, file);
        video_Format = CVI_INI_GetLong(tmp_section, "video_Format", 0, file);
        videoFull_Flag = CVI_INI_GetLong(tmp_section, "videoFull_Flag", 0, file);
        colourDescription_Flag = CVI_INI_GetLong(tmp_section, "colourDescription_Flag", 0, file);
        firstFrameStartQp = CVI_INI_GetLong(tmp_section, "firstFrameStartQp", 0, file);
        maxBitRate = CVI_INI_GetLong(tmp_section, "maxBitRate", 0, file);
        gop_mode = CVI_INI_GetLong(tmp_section, "gop_mode", 0, file);
        maxIprop = CVI_INI_GetLong(tmp_section, "maxIprop", 0, file);
        minIprop = CVI_INI_GetLong(tmp_section, "minIprop", 0, file);
        minStillPercent = CVI_INI_GetLong(tmp_section, "minStillPercent", 0, file);
        maxStillQP = CVI_INI_GetLong(tmp_section, "maxStillQP", 0, file);
        avbrPureStillThr = CVI_INI_GetLong(tmp_section, "avbrPureStillThr", 0, file);
        motionSensitivity = CVI_INI_GetLong(tmp_section, "motionSensitivity", 0, file);
        bgDeltaQp = CVI_INI_GetLong(tmp_section, "bgDeltaQp", 0, file);
        rowQpDelta = CVI_INI_GetLong(tmp_section, "rowQpDelta", 0, file);
        ipqpDelta = CVI_INI_GetLong(tmp_section, "ipqpDelta", 0, file);
        printf("%ld %ld %ld %ld %ld \
                %ld %ld %ld %ld %f %ld \
                %ld %ld %ld %ld %ld %ld %ld \
                %ld %ld %ld %ld %ld \
                %ld %ld %ld %ld %ld \
                %ld %ld %ld %ld\n",
            enable, id, bind_vproc_id, bind_vproc_chn_id, width,
            height, codec, pixel_fmt, gop, fps, rate_ctrl_mode,
            bit_rate, iqp, pqp, min_iqp, max_iqp, min_qp, max_qp,
            single_EsBuf, bitstream_bufSize, datafifo_len, src_framerate, dst_framerate,
            initialDelay, thrdLv, statTime, aspectRatio_Flag, overscan_Flag,
            videoSignalType_Flag, video_Format, videoFull_Flag, colourDescription_Flag);

        /* set float fps into src_framerate & dst_framerate */
        if (fps>0 && src_framerate==0 && dst_framerate==0) {
            long int fpsNum = (long int)fps, fpsDenom=0;
            if ((float)fpsNum != fps){
                fpsDenom = 100;
                float tmp = fps*fpsDenom;
                fpsNum=(long int)tmp;
                src_framerate = (fpsDenom<<16) + fpsNum;
                dst_framerate = src_framerate;
            } else {
                src_framerate = fpsNum;
                dst_framerate = fpsNum;
            }
            printf("float venc fps: %ld, %ld\n", src_framerate, dst_framerate);
        }

        Venc->VencChnAttr[i].VencChnEnable = enable;
        Venc->VencChnAttr[i].VencId = id;
        Venc->VencChnAttr[i].BindVprocId = bind_vproc_id;
        Venc->VencChnAttr[i].BindVprocChnId = bind_vproc_chn_id;
        Venc->VencChnAttr[i].bindMode = venc_bind_mode;
        Venc->VencChnAttr[i].framerate = fps;
        Venc->VencChnAttr[i].MapiVencAttr.width = width;
        Venc->VencChnAttr[i].MapiVencAttr.height = height;
        Venc->VencChnAttr[i].MapiVencAttr.codec = codec;
        Venc->VencChnAttr[i].MapiVencAttr.pixel_format = pixel_fmt;
        Venc->VencChnAttr[i].MapiVencAttr.gop = gop;
        Venc->VencChnAttr[i].MapiVencAttr.profile = profile;
        Venc->VencChnAttr[i].MapiVencAttr.rate_ctrl_mode = rate_ctrl_mode;
        Venc->VencChnAttr[i].MapiVencAttr.bitrate_kbps = bit_rate;
        Venc->VencChnAttr[i].MapiVencAttr.iqp = iqp;
        Venc->VencChnAttr[i].MapiVencAttr.pqp = pqp;
        Venc->VencChnAttr[i].MapiVencAttr.minIqp = min_iqp;
        Venc->VencChnAttr[i].MapiVencAttr.maxIqp = max_iqp;
        Venc->VencChnAttr[i].MapiVencAttr.minQp = min_qp;
        Venc->VencChnAttr[i].MapiVencAttr.maxQp = max_qp;
        Venc->VencChnAttr[i].MapiVencAttr.changePos = change_pos;
        Venc->VencChnAttr[i].MapiVencAttr.single_EsBuf = single_EsBuf;
        Venc->VencChnAttr[i].MapiVencAttr.bufSize = bitstream_bufSize;
        Venc->VencChnAttr[i].MapiVencAttr.datafifoLen = datafifo_len;
        Venc->VencChnAttr[i].MapiVencAttr.jpeg_quality = jpeg_quality;
        Venc->VencChnAttr[i].MapiVencAttr.src_framerate = src_framerate;
        Venc->VencChnAttr[i].MapiVencAttr.dst_framerate = dst_framerate;
        Venc->VencChnAttr[i].MapiVencAttr.initialDelay = initialDelay;
        Venc->VencChnAttr[i].MapiVencAttr.thrdLv = thrdLv;
        Venc->VencChnAttr[i].MapiVencAttr.statTime = statTime;
        Venc->VencChnAttr[i].MapiVencAttr.aspectRatioInfoPresentFlag = aspectRatio_Flag;
        Venc->VencChnAttr[i].MapiVencAttr.overscanInfoPresentFlag = overscan_Flag;
        Venc->VencChnAttr[i].MapiVencAttr.videoSignalTypePresentFlag = videoSignalType_Flag;
        Venc->VencChnAttr[i].MapiVencAttr.videoFormat = video_Format;
        Venc->VencChnAttr[i].MapiVencAttr.videoFullRangeFlag = videoFull_Flag;
        Venc->VencChnAttr[i].MapiVencAttr.colourDescriptionPresentFlag = colourDescription_Flag;
        Venc->VencChnAttr[i].MapiVencAttr.firstFrameStartQp = firstFrameStartQp;
        Venc->VencChnAttr[i].MapiVencAttr.maxBitRate = maxBitRate;
        Venc->VencChnAttr[i].MapiVencAttr.gop_mode = gop_mode;
        Venc->VencChnAttr[i].MapiVencAttr.maxIprop = maxIprop;
        Venc->VencChnAttr[i].MapiVencAttr.minIprop = minIprop;
        Venc->VencChnAttr[i].MapiVencAttr.minStillPercent = minStillPercent;
        Venc->VencChnAttr[i].MapiVencAttr.maxStillQP = maxStillQP;
        Venc->VencChnAttr[i].MapiVencAttr.avbrPureStillThr = avbrPureStillThr;
        Venc->VencChnAttr[i].MapiVencAttr.motionSensitivity = motionSensitivity;
        Venc->VencChnAttr[i].MapiVencAttr.bgDeltaQp = bgDeltaQp;
        Venc->VencChnAttr[i].MapiVencAttr.rowQpDelta = rowQpDelta;
        Venc->VencChnAttr[i].MapiVencAttr.ipqpDelta = ipqpDelta;
    }
    return 0;
}

static int32_t PARAM_LoadVB(const char *file, CVI_PARAM_MEDIA_VB_ATTR_S *Vb)
{
    long int i = 0;
    long int pool_cnt = 0;
    long int enable = 0;
    long int frame_width = 0;
    long int frame_height = 0;
    long int frame_fmt = 0;
    long int blk_size = 0;
    long int blk_cnt = 0;
    char tmp_section[32] = {0};

    pool_cnt = CVI_INI_GetLong("vb_config", "vbpool_cnt", 0, file);
    printf("%s: pool_cnt: %ld\n", __func__, pool_cnt);
    Vb->Poolcnt = pool_cnt;

    for (i = 0; i < pool_cnt; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vbpool%ld", i);
        enable = CVI_INI_GetLong(tmp_section, "enable", 0, file);
        blk_cnt = CVI_INI_GetLong(tmp_section, "blk_cnt", 0, file);

        Vb->Vbpool[i].is_frame = enable;
        if(enable) {
            frame_width = CVI_INI_GetLong(tmp_section, "frame_width", 0, file);
            frame_height = CVI_INI_GetLong(tmp_section, "frame_height", 0, file);
            frame_fmt = CVI_INI_GetLong(tmp_section, "frame_fmt", 0, file);

            Vb->Vbpool[i].vb_blk_size.frame.width = frame_width;
            Vb->Vbpool[i].vb_blk_size.frame.height = frame_height;
            Vb->Vbpool[i].vb_blk_size.frame.fmt = frame_fmt;
        } else {
            blk_size = CVI_INI_GetLong(tmp_section, "blk_size", 0, file);
            Vb->Vbpool[i].vb_blk_size.size = blk_size;
        }

        Vb->Vbpool[i].vb_blk_num = blk_cnt;

        printf("%ld %ld %ld %ld %ld %ld\n", enable, frame_width, frame_height, frame_fmt, blk_size, blk_cnt);
    }

    return 0;
}

static int32_t PARAM_LoadMediaSpec(const char *file, CVI_PARAM_MEDIA_SPEC_S *MediaMode)
{
   int32_t  s32Ret = 0;
    s32Ret = PARAM_LoadMediaMode(file, &MediaMode->MediaMode);
    s32Ret |= PARAM_LoadSns(file, &MediaMode->SnsAttr);
    s32Ret |= PARAM_LoadVcap(file, &MediaMode->VcapAttr);
    s32Ret |= PARAM_LoadVproc(file, &MediaMode->VprocAttr);
    s32Ret |= PARAM_LoadVenc(file, &MediaMode->VencAttr);
    s32Ret |= PARAM_LoadVB(file, &MediaMode->Vb);
    s32Ret |= PARAM_LoadOsd(file, &MediaMode->Osd);
    return s32Ret;
}


static int32_t PARAM_LoadMediaCam(const char *comm_file, CVI_PARAM_CAM_CFG *CamCfg,int32_t  index)
{
   int32_t  s32Ret = 0;
    long int enable = 0;
    long int cam_id = 0;
    long int osdshow = 0;
    uint32_t cur_mode = 0;
    long int count = 0;
    char tmp_section[16] = {0};

    uint32_t i = 0;
    uint32_t j = 0;
    char cam_name[CVI_PARAM_MODULE_NAME_LEN] = {0};
    char tmp_name[CVI_PARAM_MODULE_NAME_LEN] = {0};

    enable = CVI_INI_GetLong("camera", "enable", 0, comm_file);
    cam_id = CVI_INI_GetLong("camera", "cam_id", 0, comm_file);
    osdshow = CVI_INI_GetLong("camera", "osdshow", 0, comm_file);

    CVI_INI_GetString("camera", "cur_mode", "", tmp_section,
                        CVI_PARAM_MODULE_NAME_LEN, comm_file);

    s32Ret = CVI_INI_PARAM_MediaString2Uint(&cur_mode, tmp_section);
    if(s32Ret == -1) {
        return -1;
    }

    count = CVI_INI_GetLong("mediamode", "count", 0, comm_file);

    CamCfg->CamEnable = enable;
    CamCfg->CamMediaInfo.CamID = cam_id;
    CamCfg->CamMediaInfo.CurMediaMode = cur_mode;
    CamCfg->MediaModeCnt = count;

    printf("cam_id = %ld :%s: enable: %ld osdshow: %ld mode: %d count: %ld\n",
        cam_id, comm_file, enable, osdshow, cur_mode, count);

    memset(cam_name, 0, sizeof(cam_name));
    snprintf(cam_name, sizeof(cam_name), "config_media_cam%d", index);

    for (j = 0; j < g_ParamAccess.module_num; j++) {
        if (strstr(g_ParamAccess.modules[j].name, cam_name)) {
            memset(tmp_name, 0, sizeof(tmp_name));
            snprintf(tmp_name, sizeof(tmp_name), "%s%s",
                g_ParamAccess.modules[j].path, g_ParamAccess.modules[j].name);
            printf("cam_name = %s \n",tmp_name);
            s32Ret = PARAM_LoadMediaSpec(tmp_name, &CamCfg->MediaSpec[i++]);
        }
    }

    return s32Ret;
}


int32_t  CVI_INI_PARAM_LoadMediaCamCfg(CVI_PARAM_CAM_CFG *CamCfg)
{
    printf("\n---enter: %s\n", __func__);

   int32_t  s32Ret = 0;
    uint32_t i = 0;
    uint32_t j = 0;
    char comm_name[CVI_PARAM_MODULE_NAME_LEN] = {0};
    char comm_file[CVI_PARAM_MODULE_NAME_LEN] = {0};

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        memset(comm_name, 0, sizeof(comm_name));
        snprintf(comm_name, sizeof(comm_name), "config_mediamode_cam%d", i);

        for (j = 0; j < g_ParamAccess.module_num; j++) {
            if (strstr(g_ParamAccess.modules[j].name, comm_name)) {
                memset(comm_file, 0, sizeof(comm_file));
                snprintf(comm_file, sizeof(comm_file), "%s%s",
                    g_ParamAccess.modules[j].path, g_ParamAccess.modules[j].name);
                break;
            }
        }

        s32Ret = PARAM_LoadMediaCam(comm_file, &CamCfg[i], i);
    }

    return s32Ret;
}
/* special config end */



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
