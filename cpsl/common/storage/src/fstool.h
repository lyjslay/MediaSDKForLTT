#ifndef __FSTOOL__
#define __FSTOOL__
#include "cvi_stg.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

//#define PATH_LEN_MAX 30
#define STG_FSTOOL_FAT_BOOT_SEC_LEN (512)
#define STG_FSTOOL_PARTITION_DETECT_CNT (100)
#define STG_FSTOOL_PARTITION_DETECT_INTERVAL (10000)  // unit:us

#define STG_FSTOOL_TEST_PARTITION_FILE_NAME "test.file"
#define STG_FSTOOL_TEST_PARTITION_DATA_LEN  (4096)

/**storage file system  information */

int32_t Stg_FsTool_GetFsType(const char *pszPartition, char *pszFSType,
			 uint32_t typeLen);
int32_t Stg_Fstool_Mount(const char *pszPartition, const char *pszMountPath);
int32_t Stg_Fstool_Umount(const char *pszPartition);
int32_t Stg_Fstool_GetFsInfo(const char *pszMntPath, CVI_STG_FS_INFO_S *pstInfo);
int32_t Stg_Fstool_DetectPartition(const char *pszPartition);
int32_t Stg_Fstool_Format(const char *pszDevPath);
int32_t Stg_Fstool_Format_with_Label(const char *pszDevPath, char *labelname);
int32_t Stg_Fstool_TestPartition(const char *pszMountPath, const char *pszDevPath);
int32_t Stg_Fstool_CheckFaile(const char *pszMntPath);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif