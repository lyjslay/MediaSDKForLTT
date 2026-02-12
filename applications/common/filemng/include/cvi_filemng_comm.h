#ifndef __CVI_FILEMNG_COMM_H__
#define __CVI_FILEMNG_COMM_H__

#include "cvi_appcomm.h"
#include "cvi_sysutils.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** macro define */
#define CVI_FILEMNG_MAX_DATETIME_LEN   (20) /**<max date time string length */
#define CVI_FILEMNG_MAX_FILECNT_IN_OBJ (6)  /**<max file count in one file object */

/** error code define */
#define CVI_FILEMNG_EINVAL            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_FILEMNG, CVI_EINVAL)                  /**<Invalid argument */
#define CVI_FILEMNG_ENOTINIT          CVI_APPCOMM_ERR_ID(CVI_APP_MOD_FILEMNG, CVI_ENOINIT)                 /**<Not inited */
#define CVI_FILEMNG_ELOST             CVI_APPCOMM_ERR_ID(CVI_APP_MOD_FILEMNG, CVI_ELOST)                   /**<No such file */
#define CVI_FILEMNG_ENOTREADY         CVI_APPCOMM_ERR_ID(CVI_APP_MOD_FILEMNG, CVI_ENOTREADY)               /**<No such device or address */
#define CVI_FILEMNG_EACCES            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_FILEMNG, CVI_EACCES)                  /**<Permission denied */
#define CVI_FILEMNG_EEXIST            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_FILEMNG, CVI_EEXIST)                  /**<File exists */
#define CVI_FILEMNG_EFULL             CVI_APPCOMM_ERR_ID(CVI_APP_MOD_FILEMNG, CVI_EFULL)                   /**<No space left on device */
#define CVI_FILEMNG_EBUSY             CVI_APPCOMM_ERR_ID(CVI_APP_MOD_FILEMNG, CVI_EBUSY)                   /**<Operation now in progress */
#define CVI_FILEMNG_ENORES            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_FILEMNG, CVI_ENORES)                  /**<Too many files,not enough filename */
#define CVI_FILEMNG_EINTER            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_FILEMNG, CVI_EINTER)                  /**<Internal error */
#define CVI_FILEMNG_EINITIALIZED      CVI_APPCOMM_ERR_ID(CVI_APP_MOD_FILEMNG, CVI_EINITIALIZED)            /**<Already Initialized */
#define CVI_FILEMNG_ENOTMAIN          CVI_APPCOMM_ERR_ID(CVI_APP_MOD_FILEMNG, CVI_ERRNO_CUSTOM_BOTTOM + 1) /**<Not Basic File */
#define CVI_FILEMNG_EUNIDENTIFICATION CVI_APPCOMM_ERR_ID(CVI_APP_MOD_FILEMNG, CVI_ERRNO_CUSTOM_BOTTOM + 2) /**<Too many Unrecognized files */

/** filemng callback: add repair file name to file list cfg */
typedef int32_t (*CVI_FILEMNG_REPAIR_ADD_FILE_NAME_CALLBACK_FN_PTR)(const char *filePath, void *argv);

/** filemng callback: add repair file name to file list cfg */
typedef int32_t (*CVI_FILEMNG_REPAIR_GET_FILE_NAME_CALLBACK_FN_PTR)(uint32_t fileIndex,
                                                                  char *filePath,
                                                                  uint32_t maxFilePathLen,
                                                                  void *argv);

/** filemng callback: add repair file name to file list cfg */
typedef int32_t (*CVI_FILEMNG_REPAIR_GET_FILE_CNT_CALLBACK_FN_PTR)(uint32_t *repairFileListCnt, void *argv);

/** struct define */
/** define media operating function struct of record manager */
typedef struct CVIFILEMNG_REPAIR_USER_OPERATION_S {
    CVI_FILEMNG_REPAIR_ADD_FILE_NAME_CALLBACK_FN_PTR addRepairFileName; /**<Add repair file name to user */
    CVI_FILEMNG_REPAIR_GET_FILE_NAME_CALLBACK_FN_PTR getRepairFileName; /**<Get repair file name from user */
    CVI_FILEMNG_REPAIR_GET_FILE_CNT_CALLBACK_FN_PTR getRepairFileCnt;   /**<Get repair file count from user */
    void *argv;
} CVI_FILEMNG_REPAIR_OPERATION_S;

/** file type */
typedef enum cviFILEMNG_FILE_TYPE_E {
    CVI_FILEMNG_FILE_TYPE_RECORD = 0, /**<record file. eg. *.MP4,*.LRV,*.MOV,etc */
    CVI_FILEMNG_FILE_TYPE_PHOTO,      /**<photo file. eg. *.JPG,*.DNG,etc */
    CVI_FILEMNG_FILE_TYPE_BUTT
} CVI_FILEMNG_FILE_TYPE_E;

/** file information */
typedef struct cviFILEMNG_FILE_INFO_S {
    char szAbsPath[CVI_APPCOMM_MAX_PATH_LEN];        /**<file name ,eg. "/app/sd/CAM/Photo/2017_05_27_11281500.JPG" */
    char szCreateTime[CVI_FILEMNG_MAX_DATETIME_LEN]; /**<file create time ,eg."2017/05/27 11:28:15" */
    uint64_t u64FileSize_byte;                           /**<file size in bytes. eg. 120,100,100 */
    uint32_t u32Duration_sec;                            /**<record file duration in seconds. eg. 300 */
} CVI_FILEMNG_FILE_INFO_S;

/** file name list in file object */
typedef struct cviFILEMNG_OBJ_FILENAME_S {
    uint8_t u8FileCnt;                                                            /**<file count in the file object */
    char szFileName[CVI_FILEMNG_MAX_FILECNT_IN_OBJ][CVI_APPCOMM_MAX_PATH_LEN]; /**<file name list int32_t the file object */
} CVI_FILEMNG_OBJ_FILENAME_S;

/** file object information */
typedef struct cviFILEMNG_FILEOBJ_S {
    CVI_FILEMNG_FILE_INFO_S stBasicFile;    /**<basic file information */
    CVI_FILEMNG_OBJ_FILENAME_S stFileNames; /**<file name list in the file object */
} CVI_FILEMNG_FILEOBJ_S;

/** file manager configuration */
typedef struct cviFILEMNG_COMM_CFG_S {
    char szMntPath[CVI_APPCOMM_MAX_PATH_LEN]; /**<disk mount path. eg."/app/sd/" */
} CVI_FILEMNG_COMM_CFG_S;

/** event ID define */
typedef enum cviEVENT_FILEMNG_E {
    CVI_EVENT_FILEMNG_SCAN_COMPLETED = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_FILEMNG, 0),
    CVI_EVENT_FILEMNG_SCAN_FAIL = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_FILEMNG, 1),
    CVI_EVENT_FILEMNG_SPACE_FULL = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_FILEMNG, 2),
    CVI_EVENT_FILEMNG_SPACE_ENOUGH = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_FILEMNG, 3),
    CVI_EVENT_FILEMNG_REPAIR_BEGIN = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_FILEMNG, 4),
    CVI_EVENT_FILEMNG_REPAIR_END = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_FILEMNG, 5),
    CVI_EVENT_FILEMNG_REPAIR_FAILED = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_FILEMNG, 6),
    CVI_EVENT_FILEMNG_UNIDENTIFICATION = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_FILEMNG, 7),
    CVI_EVENT_FILEMNG_BUTT
} CVI_EVENT_FILEMNG_E;

/** @} *//** <!-- ==== FILEMNG End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_FILEMNG_COMM_H__ */

