#ifndef __CVI_PARAM_INI2BIN_H__
#define __CVI_PARAM_INI2BIN_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#pragma pack(push)
#pragma pack(8)

#include "cvi_param.h"

#define CVI_PARAM_CHECK_LOAD_RESULT(ret, name)      \
    do {                                            \
        if (0 != ret){                              \
            printf("[Error] Load [%s] failed\n", name);  \
            return -1;                     \
        }                                           \
    } while (0)

#define CVI_PARAM_CHECK_FOPEN_RESULT(ret, name)     \
    do {                                            \
        if (!ret){                                  \
            printf("[Error] fopen [%s] failed\n", name); \
            return -1;                     \
        }                                           \
    } while (0)

#define CVI_PARAM_MODULE_PATH_LEN   (16)
#define CVI_PARAM_MODULE_NAME_LEN   (64)
#define CVI_PARAM_MODULE_MAX        (20)

#define CIV_PARAM_SECTION_LEN       (32)
#define CVI_PARAM_KEY_LEN           (16)

typedef struct _CVI_PARAM_MODULE_INFO_S {
    char path[CVI_PARAM_MODULE_NAME_LEN];
    char name[CVI_PARAM_MODULE_NAME_LEN];
} CVI_PARAM_MODULE_INFO_S;

typedef struct _CVI_PARAM_ACCESS {
    uint32_t module_num;
    CVI_PARAM_MODULE_INFO_S modules[CVI_PARAM_MODULE_MAX];
} CVI_PARAM_ACCESS;



int32_t  CVI_INI_PARAM_LoadAccessEntry();
int32_t  CVI_INI_PARAM_LoadWorkModeCfg(CVI_PARAM_WORK_MODE_S *WorkMode);
int32_t  CVI_INI_PARAM_LoadMediaCommCfg(CVI_PARAM_MEDIA_COMM_S *MediaParam);
int32_t  CVI_INI_PARAM_LoadMediaCamCfg(CVI_PARAM_CAM_CFG *MediaMode);
int32_t  CVI_INI_PARAM_LoadDevmngCfg(CVI_PARAM_DEVMNG_S *DevMng);
int32_t  CVI_INI_PARAM_LoadFilemngCfg(CVI_PARAM_FILEMNG_S *FileMng);
int32_t  CVI_INI_PARAM_LoadMenuCfg(CVI_PARAM_MENU_S *MenuMng);
int32_t  CVI_INI_PARAM_MediaString2Uint(uint32_t *MediaMode, char *String);
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_PARAM_INI2BIN_H__ */