#ifndef __FILEMNG_COMM_H__
#define __FILEMNG_COMM_H__

#include "cvi_filemng_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** rename module name */
#ifdef CVI_MODULE
#undef CVI_MODULE
#endif
#define CVI_MODULE "FILEMNG"

/**define full flag mask */
#define SPACEMONITOR_MASK_ENOUGH    (0x000)
#define SPACEMONITOR_MASK_TOTALFULL (0x001)
#define SPACEMONITOR_MASK_MOVIEFULL (0x010)
#define SPACEMONITOR_MASK_EMRFULL   (0x100)

/**disk state */
typedef enum tagFILEMNG_DISK_STATE_E {
    FILEMNG_DISK_STATE_NOT_AVAILABLE = 0,
    FILEMNG_DISK_STATE_AVAILABLE = 1,
    FILEMNG_DISK_STATE_SCAN_COMPLETED = 2,
    FILEMNG_DISK_STATE_BUTT
} FILEMNG_DISK_STATE_E;

/** callback function for cover file object */
typedef int32_t (*FILEMNG_SPACEMONITOR_Cover)(int32_t s32SMFullFlag);
typedef int32_t (*FILEMNG_SPACEMONITOR_GetRatioSpace)(uint32_t *pu32MovieSpace, uint32_t *pu32EmrSpace);

typedef struct tagSPACEMONITOR_CFG_S {
    char szMntPath[CVI_APPCOMM_MAX_PATH_LEN];
    uint32_t u32WarningStage;
    uint32_t u32GuaranteedStage;
    uint32_t u32Interval;
    uint32_t u32MaxCheckDelay;
    FILEMNG_SPACEMONITOR_Cover pfnCoverCB;
    uint8_t u8SharePercent;
    FILEMNG_SPACEMONITOR_GetRatioSpace pfnGetRatioSpace;
    bool    bStopCover;
} SPACEMONITOR_CFG_S;

char *FILEMNG_Strerror(int32_t s32ErrorCode);
int32_t FILEMNG_CheckPath(const char *pszMntPath, const char *pszRootDir);
bool FILEMNG_IsMP4(const char *pszFilePath);
int32_t FILEMNG_GetFileInfo(const char *pszFilePath, CVI_FILEMNG_FILE_INFO_S *pstFileInfo);
int32_t FILEMNG_HideFile(const char *pFilePath, bool bHide);
int32_t FILEMNG_HideDir(const char *pDirPath, bool bHide);

int32_t FILEMNG_SPACEMONITOR_Create(const SPACEMONITOR_CFG_S *pstConfig);
int32_t CVI_FILEMNG_SpacemonitorCheckSpace(void);
int32_t FILEMNG_SPACEMONITOR_Destroy(void);
int32_t FILEMNG_SPACEMONITOR_JudgeStage(uint64_t u64RealUsedSize_MB);
int32_t FILEMNG_SPACEMONITOR_SetCoverStatus(bool en);
int32_t FILEMNG_MARKER_Init(const char *pszTopDir);
int32_t FILEMNG_MARKER_SetFlag(const char *pszFileName, uint8_t u8Flag);
int32_t FILEMNG_MARKER_CleanFlag(const char *pszFileName);

/** @} *//** <!-- ==== FILEMNG End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __FILEMNG_COMM_H__ */

