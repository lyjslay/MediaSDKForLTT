#ifndef __CVI_MAPI_OSD_H__
#define __CVI_MAPI_OSD_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "string.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CVI_MAPI_OSD_MAX_CNT  (16)
#define CVI_MAPI_OSD_MAX_DISP_CNT (1)
#define CVI_MAPI_OSD_MAX_STR_LEN  (120)

#define CVI_MAPI_OSDC_MAX_REC_CNT  (8)
#define CVI_MAPI_OSDC_MAX_LINE_CNT  (2)

typedef int (*CVI_MAPI_OSD_GETFONTMOD_CALLBACK_FN_PTR)(char* Character,uint8_t** FontMod,int* FontModLen);

typedef enum cviMAPI_OSD_BIND_MOD_E {
    CVI_MAPI_OSD_BINDMOD_VPROC = 0,
    CVI_MAPI_OSD_BINDMOD_DISP,
    CVI_MAPI_OSD_BINDMOD_BUTT
} CVI_MAPI_OSD_BIND_MOD_E;

typedef enum cviMAPI_OSD_COORDINATE_E {
    CVI_MAPI_OSD_COORDINATE_RATIO_COOR = 0,
    CVI_MAPI_OSD_COORDINATE_ABS_COOR
} CVI_MAPI_OSD_COORDINATE_E;

typedef enum cviMAPI_OSD_TYPE_E {
    CVI_MAPI_OSD_TYPE_TIME = 0,
    CVI_MAPI_OSD_TYPE_STRING,
    CVI_MAPI_OSD_TYPE_BITMAP,
    CVI_MAPI_OSD_TYPE_CIRCLE,
    CVI_MAPI_OSD_TYPE_OBJECT,
    CVI_MAPI_OSD_TYPE_BUTT
} CVI_MAPI_OSD_TYPE_E;

typedef enum cviMAPI_OSD_TIMEFMT_E {
    CVI_MAPI_OSD_TIMEFMT_YMD24H = 0, /**< eg. 2017-03-10 23:00:59 */
    CVI_MAPI_OSD_TIMEFMT_BUTT
} CVI_MAPI_OSD_TIMEFMT_E;

typedef struct cviMAPI_OSD_TIME_CONTENT_S {
    CVI_MAPI_OSD_TIMEFMT_E enTimeFmt;
    SIZE_S  stFontSize;
    uint32_t  u32BgColor;
}CVI_MAPI_OSD_TIME_CONTENT_S;

typedef struct cviMAPI_OSD_STR_CONTENT_S {
    char szStr[CVI_MAPI_OSD_MAX_STR_LEN];
    SIZE_S  stFontSize;
    uint32_t  u32BgColor;
}CVI_MAPI_OSD_STR_CONTENT_S;

typedef struct cviMAPI_OSD_CIRCLE_CONTENT_S {
    uint32_t u32Width;
    uint32_t u32Height;
}CVI_MAPI_OSD_CIRCLE_CONTENT_S;

typedef struct cviMAPI_OSD_DISP_ATTR_S {
    bool bShow;
    CVI_MAPI_OSD_BIND_MOD_E enBindedMod;
    uint32_t ModHdl;
    uint32_t ChnHdl;
    CVI_MAPI_OSD_COORDINATE_E enCoordinate;
    POINT_S stStartPos;
    uint32_t u32Batch;
} CVI_MAPI_OSD_DISP_ATTR_S;

typedef struct _CVI_MAPI_OSD_OBJECTINFO_S {
    int32_t camid;
    uint32_t rec_cnt;
    uint32_t line_cnt;
    int32_t rec_coordinates[CVI_MAPI_OSDC_MAX_REC_CNT * 4]; // [x1,y1,x2,y2]
    int32_t line_coordinates[CVI_MAPI_OSDC_MAX_LINE_CNT * 4]; // [x1,y1,x2,y2]
}CVI_MAPI_OSD_OBJECTINFO_S;

typedef struct cviMAPI_OSD_OBJECT_CONTENT_S {
    uint32_t u32Width;
    uint32_t u32Height;
    CVI_MAPI_OSD_OBJECTINFO_S objectInfo;
}CVI_MAPI_OSD_OBJECT_CONTENT_S;

typedef struct cviMAPI_OSD_CONTENT_S {
    CVI_MAPI_OSD_TYPE_E enType;
    uint32_t  u32Color;
    union {
        CVI_MAPI_OSD_TIME_CONTENT_S stTimeContent;
        CVI_MAPI_OSD_STR_CONTENT_S stStrContent;
        CVI_MAPI_OSD_CIRCLE_CONTENT_S stCircleContent;
        CVI_MAPI_OSD_OBJECT_CONTENT_S stObjectContent;
        BITMAP_S stBitmapContent;
    };
} CVI_MAPI_OSD_CONTENT_S;

typedef struct cviMAPI_OSD_ATTR_S {
    uint32_t u32DispNum;
    bool bFlip;
    bool bMirror;
    CVI_MAPI_OSD_DISP_ATTR_S astDispAttr[CVI_MAPI_OSD_MAX_DISP_CNT];
    CVI_MAPI_OSD_CONTENT_S stContent;
} CVI_MAPI_OSD_ATTR_S;

typedef struct cviMAPI_OSD_FONTS_S {
    uint32_t u32FontWidth;
    uint32_t u32FontHeight;
} CVI_MAPI_OSD_FONTS_S;

int CVI_MAPI_OSD_Init(const CVI_MAPI_OSD_FONTS_S* pstFonts);
int CVI_MAPI_OSD_Deinit(void);
int CVI_MAPI_OSD_SetAttr(int s32OsdIdx, CVI_MAPI_OSD_ATTR_S* pstAttr);
int CVI_MAPI_OSD_GetAttr(int s32OsdIdx, CVI_MAPI_OSD_ATTR_S* pstAttr);
int CVI_MAPI_OSD_Start(int s32OsdIdx);
int CVI_MAPI_OSD_Stop(int s32OsdIdx);
int CVI_MAPI_OSD_Batch(uint32_t u32Batch, bool bShow);
int CVI_MAPI_OSD_Show(int32_t s32OsdIdx, uint32_t u32DispIdx, bool bShow);

#ifdef __cplusplus
}
#endif

#endif