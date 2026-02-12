#ifndef __CVI_FILEMNG_DTCF_H__
#define __CVI_FILEMNG_DTCF_H__

#include "cvi_dtcf.h"
#include "cvi_filemng_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define CVI_FILEMNG_DTCF_MAX_PHOTO_DIR (MAX_CAMERA_INSTANCES)

/** struct define */
/** DTCF configuration */
typedef struct cviFILEMNG_DTCF_CFG_S {
    uint32_t u32PreAllocCnt[CVI_FILEMNG_DTCF_MAX_PHOTO_DIR];  /**<pre allocate file count */
    uint32_t u32PreAllocUnit[CVI_FILEMNG_DTCF_MAX_PHOTO_DIR]; /**<pre allocate file size ,suggest 2M, unit:byte */
    bool preAllocFilesEnable;
    uint32_t u32PreallocReservedMemory;
    uint32_t u32PreAllocPercentage[DTCF_DIR_BUTT];  /**<pre allocate file count */
    uint32_t u32PreAllocFileUnit[DTCF_DIR_BUTT]; /**<pre allocate file size ,suggest 2M, unit:byte */
    uint8_t u8SharePercent;                                  /**<0:share 1~100:movie space ratio;eg.80 */
    uint32_t u32WarningStage;                                /**<unit:MB;filemanager will publish a warning when the remaining space is less than u32WarningStage */
    uint32_t u32GuaranteedStage;                             /**<unit:MB;u32GuaranteedStage should NOT be less than u32WarningStage;the loop coverage is disable when u32GuaranteedStage is 0 */
    char szRootDir[CVI_APPCOMM_MAX_PATH_LEN];            /**<file manager top directory name in mount path. eg."CVICAM" */
    char aszDirNames[DTCF_DIR_BUTT][CVI_DIR_LEN_MAX];    /**<eg.{"EMR", "EMR_s", "Movie", "Movie_s", "", "", "", "", "Photo"} */
    uint32_t u32RemoveLoopEn;
} CVI_FILEMNG_DTCF_CFG_S;

int32_t CVI_FILEMNG_RegisterEvent(void);
int32_t CVI_FILEMNG_Init(const CVI_FILEMNG_COMM_CFG_S *pstCfg, const CVI_FILEMNG_DTCF_CFG_S *pstDTCF_Cfg);
int32_t CVI_FILEMNG_DeInit(void);
int32_t CVI_FILEMNG_SetDiskState(bool bAvailable);
int32_t CVI_FILEMNG_CheckDiskSpace(void);
int32_t CVI_FILEMNG_AddFile(const char *pszFilePath);
int32_t CVI_FILEMNG_RemoveFile(const char *pszFilePath);
int32_t CVI_FILEMNG_SetSearchScope(CVI_DTCF_DIR_E aenDirs[DTCF_DIR_BUTT], uint32_t u32DirCount, uint32_t *pu32FileObjCnt);
int32_t CVI_FILEMNG_GetFileObjCnt(CVI_FILEMNG_FILE_TYPE_E enType, uint32_t *pu32FileObjCnt);
int32_t CVI_FILEMNG_GetFileByIndex(uint32_t u32FileIdx, char *pazFileName, uint32_t u32Length);
int32_t CVI_FILEMNG_GetFileInfoByName(const char *pszFilePath, CVI_FILEMNG_FILE_INFO_S *pstFileInfo);
int32_t CVI_FILEMNG_SpacemonitorCheckSpace(void);
int32_t CVI_FILEMNG_GeneratePhotoName(CVI_DTCF_FILE_TYPE_E enType, CVI_DTCF_DIR_E enDir, bool bPreAlloc, char *FileName);
int32_t CVI_FILEMNG_GenerateRecordName(CVI_DTCF_FILE_TYPE_E enType, CVI_DTCF_DIR_E enDir, char *pstFileName);
int32_t CVI_FILEMNG_RenameMovToEmr(const char *pazFilePath);
int32_t CVI_FILEMNG_FileCoverStatus(bool en);
int32_t CVI_FILEMNG_AlignRecordFileSize(bool enftruncate, uint32_t size);
int32_t CVI_FILEMNG_CreateSDConfigFile(void);
int32_t CVI_FILEMNG_RecoverAddFileName(const char *filePath);
int32_t CVI_FILEMNG_RecoverRemoveFileName(const char *filePath);
bool CVI_FILEMNG_SDConfigFileIsExist(void);
void CVI_FILEMNG_PreallocateState(CVI_FILEMNG_DTCF_CFG_S *pstDTCF_Cfg);
int32_t CVI_FILEMNG_GetDirType(int32_t id, CVI_DTCF_DIR_E base);
int32_t CVI_FILEMNG_SetRemoveLoop(int32_t en);
void CVI_FILEMNG_SetSdConfigPath(char* path);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_FILEMNG_DTCF_H__ */
