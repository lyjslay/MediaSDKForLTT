#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <inttypes.h>

#include "cvi_eventhub.h"
#include "cvi_filemng_dtcf.h"
#include "filemng_comm.h"
#include "cvi_storagemng.h"

#if defined(__UCLIBC__)
#include <sys/syscall.h>
#define FALLOC_FL_KEEP_SIZE              0x01
#define HIDWORD(a)                       ((uint32_t)(((uint64_t)(a)) >> 32))
#define LODWORD(a)                       ((uint32)(uint64_t)(a))
#define fallocate(fd, mode, offset, len) syscall(__NR_fallocate, fd, mode, LODWORD(offset), HIDWORD(offset), LODWORD(len), HIDWORD(len))
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define FILEMNG_PREALLOC_FILE_MAX_NUM     (100)
#define FILEMNG_PREALLOC_FILE_NAME_PREFIX ".PreAllocFile"
#define FILEMNG_PrintInfo()               \
    {                                                                                                                                                                                      \
        CVI_LOGI("FileObjCount %u(movie:%u+emr:%u+photo:%u) \n", s_u32FILEMNGAllObjCnt, s_u32FILEMNGRecObjCnt - s_u32FILEMNGEmrRecObjCnt, s_u32FILEMNGEmrRecObjCnt, s_u32FILEMNGPhotoObjCnt); \
    }

typedef struct tagFILEMNG_FILES_S{
    char szFileName[CVI_APPCOMM_MAX_PATH_LEN];  /* <prealloc files name */
} FILEMNG_FILES_S;

typedef struct tagFILEMNG_PREALLOC_FILES_S {
    uint32_t u32UnusedCnt;          /* < unused prealloc files counter */
    uint32_t u32Index;             /* < current prealloc files index */
    uint32_t u32PreAllocFileCnt;  /**<pre allocate file count */
    FILEMNG_FILES_S *List;     /* <prealloc files name list */
} FILEMNG_PREALLOC_FILES_S;

typedef struct tagFILEMNG_PHOTO_QUEUE_S {
    uint32_t u32Index;
    char szFileName[CVI_APPCOMM_MAX_PATH_LEN];
} FILEMNG_PHOTO_QUEUE_S;

typedef struct tagFILEMNG_PHOTO_CTX_S {
    uint32_t u32Count;
    FILEMNG_PHOTO_QUEUE_S *List;
} FILEMNG_PHOTO_CTX_S;

static pthread_mutex_t s_FILEMNGMutex = PTHREAD_MUTEX_INITIALIZER;
static CVI_FILEMNG_COMM_CFG_S s_stFILEMNGCfg;
static CVI_FILEMNG_DTCF_CFG_S s_stFILEMNGDTCF_Cfg;
static bool s_bFILEMNGInit = false;
static FILEMNG_DISK_STATE_E s_enFILEMNGDiskState = FILEMNG_DISK_STATE_NOT_AVAILABLE;
static uint32_t s_u32FILEMNGRecObjCnt = 0;    /**<record file object count */
static uint32_t s_u32FILEMNGEmrRecObjCnt = 0; /**<emr record file object count */
static uint32_t s_u32FILEMNGPhotoObjCnt = 0;  /**<photo file object count */
static uint32_t s_u32FILEMNGAllObjCnt = 0;    /**<all file object count */
static FILEMNG_PHOTO_CTX_S g_Photos[CVI_FILEMNG_DTCF_MAX_PHOTO_DIR];
static FILEMNG_PREALLOC_FILES_S g_PreAllocFiles[DTCF_DIR_BUTT];
static CVI_DTCF_DIR_E s_aenDirs[DTCF_DIR_BUTT];
static uint32_t s_u32DirCount = 0;
static uint32_t s_u32MaxDirCount = 0;
static uint32_t s_u32MovieSpace_MB = 0;
static uint32_t s_u32EmrSpace_MB = 0;
static uint32_t s_u32PhotoSpace_MB = 0;

/**-------------------------internal function interface------------------------- */

static uint32_t FILEMNG_GetFileSize(const char *fileNamePath)
{
    struct stat st;
    stat(fileNamePath, &st);
    return (st.st_size >> 20);
}

static int32_t FILEMNG_Disable(void)
{
    int32_t s32Ret = CVI_DTCF_DeInit();
    if (0 != s32Ret) {
    }

    s_u32FILEMNGRecObjCnt = 0;
    s_u32FILEMNGPhotoObjCnt = 0;
    s_enFILEMNGDiskState = FILEMNG_DISK_STATE_NOT_AVAILABLE;
    return 0;
}

static int32_t FILEMNG_GetMovieSpace(uint32_t *pu32Size_MB)
{
    uint64_t u64Size_KB = 0;
    uint64_t u64MovieSpace = 0;
    char szPath[CVI_APPCOMM_MAX_PATH_LEN];

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT]);
        cvi_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT_SUB], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT_SUB]);
        cvi_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_FRONT], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_FRONT]);
        cvi_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_FRONT_SUB], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_FRONT_SUB]);
        cvi_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR]);
        cvi_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR_SUB], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR_SUB]);
        cvi_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR]);
        cvi_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR_SUB], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR_SUB]);
        cvi_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR_SUB], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR_SUB]);
        cvi_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORMAL_AHD], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORMAL_AHD]);
        cvi_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORMAL_AHD_SUB], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORMAL_AHD_SUB]);
        cvi_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_AHD], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_AHD]);
        cvi_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_AHD_SUB], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_AHD_SUB]);
        cvi_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    *pu32Size_MB = u64MovieSpace >> 10;
    return 0;
}

static int32_t FILEMNG_GetEmrSpace(uint32_t *pu32Size_MB)
{
    uint64_t u64Size_KB = 0;
    uint64_t u64EmrSpace = 0;
    char szPath[CVI_APPCOMM_MAX_PATH_LEN];

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT]);
        cvi_du(szPath, &u64Size_KB);
        u64EmrSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT_SUB], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT_SUB]);
        cvi_du(szPath, &u64Size_KB);
        u64EmrSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR]);
        cvi_du(szPath, &u64Size_KB);
        u64EmrSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR_SUB], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR_SUB]);
        cvi_du(szPath, &u64Size_KB);
        u64EmrSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_AHD], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_AHD]);
        cvi_du(szPath, &u64Size_KB);
        u64EmrSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_AHD_SUB], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_AHD_SUB]);
        cvi_du(szPath, &u64Size_KB);
        u64EmrSpace += u64Size_KB;
    }

    *pu32Size_MB = u64EmrSpace >> 10;
    return 0;
}

static int32_t FILEMNG_GetPhotoSpace(uint32_t *pu32Size_MB)
{
    uint64_t u64Size_KB = 0;
    uint64_t u64PhotoSpace = 0;
    char szPath[CVI_APPCOMM_MAX_PATH_LEN];

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_FRONT], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_FRONT]);
        cvi_du(szPath, &u64Size_KB);
        u64PhotoSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_REAR], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_REAR]);
        cvi_du(szPath, &u64Size_KB);
        u64PhotoSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_AHD], CVI_DIR_LEN_MAX)) {
        snprintf(szPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_AHD]);
        cvi_du(szPath, &u64Size_KB);
        u64PhotoSpace += u64Size_KB;
    }

    *pu32Size_MB = u64PhotoSpace >> 10;
    return 0;
}

static int32_t FILEMNG_GetEmrCnt(uint32_t *pu32Count)
{
    int32_t s32Ret = 0;
    uint32_t u32DirCount = 0;
    CVI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT];

    /**scan scope */
    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_EMR_FRONT;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_EMR_REAR;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_AHD], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_EMR_AHD;
    }

    if (u32DirCount > 0) {
        s32Ret = CVI_DTCF_Scan(enDirs, u32DirCount, pu32Count);
        if (0 != s32Ret) {
            return CVI_FILEMNG_EINTER;
        }
    } else {
        *pu32Count = 0;
    }
    return 0;
}

static int32_t FILEMNG_DeleteFileFromDTCFList(CVI_DTCF_DIR_E enDir, const char *pszFileName)
{
    uint32_t s32Ret = 0;
    CVI_DTCF_DIR_E enTmpDir;
    char szFileName[CVI_APPCOMM_MAX_PATH_LEN];

    /* delete file from DTCF list */
    for (uint32_t j = 0; j < s_u32FILEMNGAllObjCnt; j++) {
        s32Ret = CVI_DTCF_GetFileByIndex(j, szFileName, CVI_APPCOMM_MAX_PATH_LEN, &enTmpDir);
        if (0 != s32Ret) {
            CVI_LOGE("CVI_DTCF_GetFileByIndex return. %d, ret = %d\n", j, s32Ret);
            return s32Ret;
        }
        if (enDir != enTmpDir)
            continue;

        if (0 == strncmp(pszFileName, szFileName, CVI_APPCOMM_MAX_PATH_LEN)) {
            uint32_t u32FileAmount;
            CVI_DTCF_DelFileByIndex(j, &u32FileAmount);
            if (DTCF_DIR_EMR_FRONT == enDir || DTCF_DIR_EMR_REAR == enDir || DTCF_DIR_EMR_AHD == enDir){
                s_u32FILEMNGEmrRecObjCnt--;
            }
            s_u32FILEMNGRecObjCnt--;
            s_u32FILEMNGAllObjCnt--;
            break;
        }
    }
    return 0;
}

static int32_t FILEMNG_CreatePreAllocFile(char *pszFileName, uint32_t u32PreAllocUnit)
{
    int32_t s32Ret = 0;
    int32_t fd = -1;
    fd = open(pszFileName, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (0 >= fd) {
        CVI_LOGE("create prealloc file[%s] fail:%s\n", pszFileName, strerror(errno));
        return CVI_FILEMNG_EINTER;
    }

    s32Ret = fallocate(fd, FALLOC_FL_KEEP_SIZE, 0, u32PreAllocUnit);
    if (0 != s32Ret) {
        close(fd);
        CVI_LOGE("fallocate error:%s\n", strerror(errno));
        return CVI_FILEMNG_EINTER;
    }

    ftruncate(fd, u32PreAllocUnit);
    close(fd);
    return FILEMNG_HideFile(pszFileName, true);
}

static int32_t FILEMNG_PreAllocFileCheck(char *pszFileName, uint32_t u32PreAllocUnit)
{
    struct stat FileStat;
    if (0 != stat(pszFileName, &FileStat)) {
        CVI_LOGE("stat %s error:%s\n", pszFileName, strerror(errno));
        return CVI_FILEMNG_EINTER;
    }

    if (u32PreAllocUnit != (uint32_t) FileStat.st_size) {
        CVI_LOGW("PreAllocFile(%lu) is not expected(%u)\n", (ulong)FileStat.st_size, u32PreAllocUnit);
        return CVI_FILEMNG_EINTER;
    }
    return 0;
}

static int32_t FILEMNG_CaculatePreAllocFileCnt()
{
    int32_t s32Ret = 0;
    uint32_t s32Percentage = 0;
    uint64_t s32AvailebalTotalSize = 0;
    CVI_STG_FS_INFO_S stFSInfo;

    s32Ret = CVI_STORAGEMNG_GetFSInfo(&stFSInfo);
    if (0 != s32Ret) {
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_EINTER));
        return CVI_FILEMNG_EINTER;
    }
    s32AvailebalTotalSize = stFSInfo.u64TotalSize - stFSInfo.u64TotalSize*(s_stFILEMNGDTCF_Cfg.u32PreallocReservedMemory/100.0);
    CVI_LOGI("Disk Availebal Total Size(%"PRIu64")", s32AvailebalTotalSize);

    /*  the sumary of percentage should not be 0 or bigger than 100%.
        u32PreAllocPercentage is unsign type, would not be minus*/
    for (int32_t enDir = 0; enDir < DTCF_DIR_BUTT; enDir++){
        s32Percentage += s_stFILEMNGDTCF_Cfg.u32PreAllocPercentage[enDir];
    }
    if (100 < s32Percentage || 0 >= s32Percentage){
        CVI_LOGE("ERROR: Wrong preAlloc Percentage(%d) setting. \
            Please check the SD source usage in ini file. %s\n",
            s32Percentage, FILEMNG_Strerror(CVI_FILEMNG_EINTER));
        return CVI_FILEMNG_EINTER;
    }

    for (int32_t enDir = 0; enDir < DTCF_DIR_BUTT; enDir++){
        if (0 >= s_stFILEMNGDTCF_Cfg.u32PreAllocFileUnit[enDir] || 0 >= s_stFILEMNGDTCF_Cfg.u32PreAllocPercentage[enDir]){
            continue;
        }
        g_PreAllocFiles[enDir].u32PreAllocFileCnt = \
            s32AvailebalTotalSize*(s_stFILEMNGDTCF_Cfg.u32PreAllocPercentage[enDir]/100.0)/s_stFILEMNGDTCF_Cfg.u32PreAllocFileUnit[enDir];
        CVI_LOGI("enDir(%d): u32PreAllocFileCnt(%u), u32PreAllocPercentage(%u),u32PreAllocFileUnit(%u)",
            enDir,
            g_PreAllocFiles[enDir].u32PreAllocFileCnt,
            s_stFILEMNGDTCF_Cfg.u32PreAllocPercentage[enDir],
            s_stFILEMNGDTCF_Cfg.u32PreAllocFileUnit[enDir]);
    }
    return 0;
}


static int32_t FILEMNG_CreateMoviePreAllocFiles(CVI_DTCF_DIR_E *enDirs, uint32_t u32DirCount)
{
    int32_t s32Ret = 0;
    uint32_t i, j = 0;
    char szFileName[CVI_APPCOMM_MAX_PATH_LEN];
    uint32_t u32FileAmount;

    s32Ret = FILEMNG_CaculatePreAllocFileCnt();
    if (0 != s32Ret){
        CVI_LOGE("%s\n", FILEMNG_Strerror(s32Ret));
        return s32Ret;
    }

    for (uint32_t num = 0; num < u32DirCount; num++ ){
        CVI_DTCF_DIR_E u32DirIdx = enDirs[num];

        if (0 == strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[u32DirIdx], CVI_DIR_LEN_MAX)){
            continue;
        }

        s32Ret = CVI_DTCF_Scan(&u32DirIdx, 1, &u32FileAmount);
        if (0 != s32Ret) {
            CVI_LOGE("%s\n", FILEMNG_Strerror(s32Ret));
            continue;
        }
        CVI_LOGI("u32DirIdx(%u) u32PreAllocFileCnt(%u) FileAmount(%d)  \n",
            u32DirIdx, g_PreAllocFiles[u32DirIdx].u32PreAllocFileCnt, u32FileAmount);

        if (0 < g_PreAllocFiles[u32DirIdx].u32PreAllocFileCnt) {
            /* alloc preAllocFiles list memory*/

            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[u32DirIdx], CVI_DIR_LEN_MAX)) {
                CVI_APPCOMM_SAFE_FREE(g_PreAllocFiles[u32DirIdx].List);
                g_PreAllocFiles[u32DirIdx].List = \
                        (FILEMNG_FILES_S *)malloc(g_PreAllocFiles[u32DirIdx].u32PreAllocFileCnt * sizeof(FILEMNG_FILES_S));
                if (NULL == g_PreAllocFiles[u32DirIdx].List) {
                    FILEMNG_SPACEMONITOR_Destroy();
                    CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_EINTER));
                    return CVI_FILEMNG_EINTER;
                }
            }

            memset(g_PreAllocFiles[u32DirIdx].List, 0,
                        (g_PreAllocFiles[u32DirIdx].u32PreAllocFileCnt * sizeof(FILEMNG_FILES_S)));
            g_PreAllocFiles[u32DirIdx].u32Index = 0;
            g_PreAllocFiles[u32DirIdx].u32UnusedCnt = 0;

            if (u32FileAmount > g_PreAllocFiles[u32DirIdx].u32PreAllocFileCnt) {
                CVI_LOGW("The count of Dir(%d): %u is over pre allocation %u\n", u32DirIdx, u32FileAmount, \
                    g_PreAllocFiles[u32DirIdx].u32PreAllocFileCnt);
            }

            /* create preAlloc Files list */
            for (i = 0, j = 0; i < g_PreAllocFiles[u32DirIdx].u32PreAllocFileCnt; i++) {
                if (i + u32FileAmount < g_PreAllocFiles[u32DirIdx].u32PreAllocFileCnt) {
                    /* New pre allocated files join the team at first. */
                    snprintf(szFileName, CVI_APPCOMM_MAX_PATH_LEN, \
                        "%s%s/%s/%s_%02d", \
                        s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, \
                        s_stFILEMNGDTCF_Cfg.aszDirNames[u32DirIdx], \
                        FILEMNG_PREALLOC_FILE_NAME_PREFIX, i);
                    if (0 != access(szFileName, F_OK)) {
                        /**PreAllocFile not exsit,create it */
                        s32Ret = FILEMNG_CreatePreAllocFile(szFileName, \
                        s_stFILEMNGDTCF_Cfg.u32PreAllocFileUnit[u32DirIdx]);
                        if (0 != s32Ret) {
                            CVI_LOGE("ERROR: create preAlloc file failed.\n");
                            return CVI_FILEMNG_EINTER;
                        }
                    } else {
                        /**PreAllocFile exsit,check filesize is expected or not */
                        s32Ret = FILEMNG_PreAllocFileCheck(szFileName, \
                            s_stFILEMNGDTCF_Cfg.u32PreAllocFileUnit[u32DirIdx]);
                        if (0 != s32Ret) {
                            CVI_LOGE("ERROR: check prealloc file failed.\n");
                            CVI_EVENT_S stEvent = { .topic = CVI_EVENT_FILEMNG_UNIDENTIFICATION };
                            CVI_EVENTHUB_Publish(&stEvent);
                            return CVI_FILEMNG_EINTER;
                        }
                    }

                    g_PreAllocFiles[u32DirIdx].u32UnusedCnt++;
                } else {
                    /* then existing files join the team */
                    CVI_DTCF_DIR_E enDir;
                    s32Ret = CVI_DTCF_GetFileByIndex(j, szFileName, CVI_APPCOMM_MAX_PATH_LEN, &enDir);
                    if (0 != s32Ret) {
                        CVI_LOGE("ERROR: CVI_DTCF_GetFileByIndex failed.\n");
                        return CVI_FILEMNG_EINTER;
                    }

                    j++;
                }

                memcpy(g_PreAllocFiles[u32DirIdx].List[i].szFileName, szFileName, CVI_APPCOMM_MAX_PATH_LEN);
            }

            if (0 < g_PreAllocFiles[u32DirIdx].u32PreAllocFileCnt) {
                g_PreAllocFiles[u32DirIdx].u32Index  \
                    = u32FileAmount >= g_PreAllocFiles[u32DirIdx].u32PreAllocFileCnt \
                    ? g_PreAllocFiles[u32DirIdx].u32PreAllocFileCnt \
                    : g_PreAllocFiles[u32DirIdx].u32PreAllocFileCnt - u32FileAmount;
            }

            CVI_LOGI("Dir[%d] create prealloc file Cnt(%d), currentIdx:%d SUCCESS;\n",
                u32DirIdx, g_PreAllocFiles[u32DirIdx].u32PreAllocFileCnt, g_PreAllocFiles[u32DirIdx].u32Index);
        }
    }
    return 0;
}


static void FILEMNG_DestroyMoviePreAllocFiles()
{
    for (uint32_t enDir = 0; enDir < DTCF_DIR_BUTT; enDir++ ){
        CVI_APPCOMM_SAFE_FREE(g_PreAllocFiles[enDir].List);
        g_PreAllocFiles[enDir].u32Index = 0;
        g_PreAllocFiles[enDir].u32UnusedCnt = 0;
        g_PreAllocFiles[enDir].u32PreAllocFileCnt = 0;
    }
}

static int32_t FILEMNG_GetMoivePreAllocFile(CVI_DTCF_DIR_E enDir, char **pszFileName)
{
    if (g_PreAllocFiles[enDir].u32Index <= 0 \
            || g_PreAllocFiles[enDir].u32Index > g_PreAllocFiles[enDir].u32PreAllocFileCnt){
        g_PreAllocFiles[enDir].u32Index = g_PreAllocFiles[enDir].u32PreAllocFileCnt;
    }

    if (0 < g_PreAllocFiles[enDir].u32UnusedCnt){
        g_PreAllocFiles[enDir].u32UnusedCnt--;
    }

    g_PreAllocFiles[enDir].u32Index--;
    *pszFileName = g_PreAllocFiles[enDir].List[g_PreAllocFiles[enDir].u32Index].szFileName;
    FILEMNG_HideFile(*pszFileName, false);
    CVI_LOGI("Dir[%d] u32UnusedCnT(%d) PraAlloc Files u32Index(%d):%s \n",
        enDir, g_PreAllocFiles[enDir].u32UnusedCnt, g_PreAllocFiles[enDir].u32Index, *pszFileName);
    FILEMNG_DeleteFileFromDTCFList(enDir, *pszFileName);
    return 0;
}


static int32_t FILEMNG_ReturnMoviePreAllocFiles(const char *pszFilePath, CVI_DTCF_DIR_E enDir)
{
    uint32_t i = 0;

    if (0 != access(pszFilePath, F_OK)){
        CVI_LOGE("Wrong filepath. %s (%s) \n", FILEMNG_Strerror(CVI_FILEMNG_ELOST), pszFilePath);
        return CVI_FILEMNG_ELOST;
    }

    FILEMNG_DeleteFileFromDTCFList(enDir, pszFilePath);

    if (0 < g_PreAllocFiles[enDir].u32PreAllocFileCnt) {
        for (i = 0; i < g_PreAllocFiles[enDir].u32PreAllocFileCnt; i++) {
            /* search file in team */
            if (0 == strncmp(g_PreAllocFiles[enDir].List[i].szFileName, pszFilePath, CVI_APPCOMM_MAX_PATH_LEN)) {
                break;
            }
        }

        if (i < g_PreAllocFiles[enDir].u32PreAllocFileCnt) {
            /* file is in the team, rename it and set it's index to team header */
            snprintf(g_PreAllocFiles[enDir].List[i].szFileName, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s/%s_%02d", \
                s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, \
                s_stFILEMNGDTCF_Cfg.aszDirNames[enDir], FILEMNG_PREALLOC_FILE_NAME_PREFIX, \
                g_PreAllocFiles[enDir].u32UnusedCnt);
            rename(pszFilePath, g_PreAllocFiles[enDir].List[i].szFileName);
            CVI_LOGI("rename %s to %s\n", pszFilePath, g_PreAllocFiles[enDir].List[i].szFileName);
            FILEMNG_HideFile(g_PreAllocFiles[enDir].List[i].szFileName, true);
            g_PreAllocFiles[enDir].u32UnusedCnt++;

            FILEMNG_FILES_S filePath = g_PreAllocFiles[enDir].List[i];
            if (i < g_PreAllocFiles[enDir].u32Index){
                for (; i < g_PreAllocFiles[enDir].u32Index; i++){
                    g_PreAllocFiles[enDir].List[i] = g_PreAllocFiles[enDir].List[i+1];
                }
                g_PreAllocFiles[enDir].List[g_PreAllocFiles[enDir].u32Index-1] = filePath;
            }else{
                for (; i >= g_PreAllocFiles[enDir].u32Index; i--){
                    g_PreAllocFiles[enDir].List[i] = g_PreAllocFiles[enDir].List[i-1];
                }
                g_PreAllocFiles[enDir].List[g_PreAllocFiles[enDir].u32Index] = filePath;
                g_PreAllocFiles[enDir].u32Index++;
            }
        } else {
            CVI_LOGE("g_PreAllocFiles[%d].u32PreAllocFileCnt is not correct\n", enDir);
            return CVI_FILEMNG_EINTER;
        }
    }

    return 0;
}


static int32_t FILEMNG_RemoveMoiveFiles(const char *pszFilePath, CVI_DTCF_DIR_E enDir)
{
    u_int32_t s32Ret = 0;
    u_int32_t enDir_sub = enDir + 1;

    CVI_LOGI(" (%s) enter.\n", __FUNCTION__);

    if (0 != access(pszFilePath, F_OK)){
        CVI_LOGE("Wrong filepath. File not exist! (%s)\n", pszFilePath);
        return CVI_FILEMNG_ELOST;
    }

    s32Ret = FILEMNG_ReturnMoviePreAllocFiles(pszFilePath, enDir);
    if (0 != s32Ret) {
        CVI_LOGI("isn't prealloc file! remove (%s)\n", pszFilePath);
        s32Ret = remove(pszFilePath);
        if (0 != s32Ret){
            CVI_LOGE("remove file(%s) failed! error:%s\n", pszFilePath, strerror(errno));
            return CVI_FILEMNG_EINTER;
        }
    }

    /* remove related file in sub folder*/
    char szSubFilePath[CVI_APPCOMM_MAX_PATH_LEN];
    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[enDir_sub], CVI_DIR_LEN_MAX)){
        s32Ret = CVI_DTCF_GetRelatedFilePath(pszFilePath, enDir_sub, szSubFilePath, CVI_APPCOMM_MAX_PATH_LEN);
        if (0 == s32Ret) {
            s32Ret = FILEMNG_ReturnMoviePreAllocFiles(szSubFilePath, enDir_sub);
            if (0 != s32Ret) {
                CVI_LOGI("related file in enDir_sub(%d) isn't prealloc file! remove (%s)\n", enDir_sub, szSubFilePath);
                s32Ret = remove(szSubFilePath);
                if (0 != s32Ret){
                    CVI_LOGE("remove related file(%s) failed! error:%s\n", szSubFilePath, strerror(errno));
                    return CVI_FILEMNG_EINTER;
                }
            }
        }
    }

    if (DTCF_DIR_EMR_FRONT == enDir || DTCF_DIR_EMR_REAR == enDir || DTCF_DIR_EMR_AHD == enDir){
        if (0 < s_u32FILEMNGEmrRecObjCnt) {
            s_u32FILEMNGEmrRecObjCnt--;
        } else {
            CVI_LOGE("s_u32FILEMNGEmrRecObjCnt is not correct\n");
            return CVI_FILEMNG_EINTER;
        }
    }

   if (0 < s_u32FILEMNGRecObjCnt) {
        s_u32FILEMNGRecObjCnt--;
        s_u32FILEMNGAllObjCnt = s_u32FILEMNGPhotoObjCnt + s_u32FILEMNGRecObjCnt;
    } else {
        CVI_LOGE("s_u32FILEMNGRecObjCnt is not correct\n");
        return CVI_FILEMNG_EINTER;
    }

    return 0;
}

static int32_t FILEMNG_RenamePreAllocFile(char *pstPreAllocFilename, char *pstFileName)
{
    if(NULL == pstFileName || NULL == pstPreAllocFilename){
        CVI_LOGE("%s", FILEMNG_Strerror(CVI_FILEMNG_EINTER));
        return CVI_FILEMNG_EINTER;
    }

    rename(pstPreAllocFilename, pstFileName);
    memcpy(pstPreAllocFilename, pstFileName, CVI_APPCOMM_MAX_PATH_LEN);
    return 0;
}

static int32_t FILEMNG_GenerateMovieFileName(CVI_DTCF_FILE_TYPE_E enType, CVI_DTCF_DIR_E enDir, char *pstFileName)
{
    int32_t s32Ret = 0;
    char *preFilename = NULL;

    s32Ret = CVI_DTCF_CreateFilePath(enType, enDir, pstFileName, CVI_APPCOMM_MAX_PATH_LEN);
    if (0 != s32Ret) {
        CVI_LOGE("CVI_DTCF_CreateFilePath Failed ret = %d\n", s32Ret);
        return CVI_FILEMNG_EINTER;
    }
    CVI_LOGI("generate file name: %s\n", pstFileName);
    s32Ret = FILEMNG_GetMoivePreAllocFile(enDir, &preFilename);
    if ( 0 != s32Ret || NULL == preFilename){
        CVI_LOGE(": %s\n", FILEMNG_Strerror(s32Ret));
        return s32Ret;
    }

    FILEMNG_RenamePreAllocFile(preFilename, pstFileName);
    return 0;
}

int32_t CVI_FILEMNG_RenameMovToEmr(const char *pazFilePath)
{
    int32_t s32Ret = 0;
    CVI_DTCF_DIR_E s32EnDir = 0;
    CVI_DTCF_DIR_E s32EmrDir = 0;
    char *szfileName = NULL;
    char tmpEmrPath[CVI_DIR_LEN_MAX] = {""};
    char *preFilePath = NULL;

    if (NULL == pazFilePath){
        FILEMNG_Strerror(CVI_FILEMNG_EINVAL);
        return CVI_FILEMNG_EINVAL;
    }
    if (0 != access(pazFilePath, F_OK)){
        FILEMNG_Strerror(CVI_FILEMNG_ELOST);
         return CVI_FILEMNG_ELOST;
    }

    if (0 != CVI_DTCF_GetFileDirType(pazFilePath, &s32EnDir)) {
        return CVI_FILEMNG_EINVAL;
    }

    szfileName = strrchr(pazFilePath, '/') + strlen("/") ;

    switch(s32EnDir){
    case DTCF_DIR_NORM_FRONT:
    case DTCF_DIR_PARK_FRONT:
        s32EmrDir = DTCF_DIR_EMR_FRONT;
        break;
    case DTCF_DIR_NORM_FRONT_SUB:
    case DTCF_DIR_PARK_FRONT_SUB:
        s32EmrDir = DTCF_DIR_EMR_FRONT_SUB;
        break;
    case DTCF_DIR_NORM_REAR:
    case DTCF_DIR_PARK_REAR:
        s32EmrDir = DTCF_DIR_EMR_REAR;
        break;
    case DTCF_DIR_NORM_REAR_SUB:
    case DTCF_DIR_PARK_REAR_SUB:
        s32EmrDir = DTCF_DIR_EMR_REAR_SUB;
        break;
    case DTCF_DIR_NORMAL_AHD:
    case DTCF_DIR_PARK_AHD:
        s32EmrDir = DTCF_DIR_EMR_AHD;
        break;
    case DTCF_DIR_NORMAL_AHD_SUB:
    case DTCF_DIR_PARK_AHD_SUB:
        s32EmrDir = DTCF_DIR_EMR_AHD_SUB;
        break;
    default:
        CVI_LOGI("%s\n", FILEMNG_Strerror(CVI_FILEMNG_EINTER));
        return CVI_FILEMNG_EINTER;
    }

    snprintf(tmpEmrPath, CVI_DIR_LEN_MAX, "%s%s/%s/%s", \
        s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, \
        s_stFILEMNGDTCF_Cfg.aszDirNames[s32EmrDir], szfileName);
    rename(pazFilePath, tmpEmrPath);
    CVI_LOGI("rename %s to %s\n", pazFilePath, tmpEmrPath);
    if (s_stFILEMNGDTCF_Cfg.preAllocFilesEnable){
        if(0 != FILEMNG_GetMoivePreAllocFile(s32EmrDir, &preFilePath)){
            CVI_LOGI("get pre allocf file failed. %s\n", FILEMNG_Strerror(CVI_FILEMNG_EINTER));
            return CVI_FILEMNG_EINTER;
        }
        CVI_LOGI("rename %s to %s\n", preFilePath, pazFilePath);
        rename(preFilePath, pazFilePath);

        /* delete file from DTCF list */
        for (uint32_t j = 0; j < s_u32FILEMNGAllObjCnt; j++) {
            char szFileName[CVI_APPCOMM_MAX_PATH_LEN];
            s32Ret = CVI_DTCF_GetFileByIndex(j, szFileName, CVI_APPCOMM_MAX_PATH_LEN, &s32EmrDir);
            if (0 != s32Ret) {
                CVI_LOGE("CVI_DTCF_GetFileByIndex Failed ret = %d\n", s32Ret);
            }

            if (0 == strncmp(preFilePath, szFileName, CVI_APPCOMM_MAX_PATH_LEN)) {
                uint32_t u32FileAmount;
                CVI_DTCF_DelFileByIndex(j, &u32FileAmount);
                if (DTCF_DIR_EMR_FRONT == s32EmrDir || DTCF_DIR_EMR_REAR == s32EmrDir){
                    s_u32FILEMNGEmrRecObjCnt--;
                }
                s_u32FILEMNGAllObjCnt--;
                break;
            }
        }
        /* add tmpEmrPath to pre alloc file list */
        memcpy(preFilePath, tmpEmrPath, CVI_DIR_LEN_MAX);

        /* return mov pre alloc file list */
        if(0 != FILEMNG_ReturnMoviePreAllocFiles(pazFilePath, s32EnDir)){
            CVI_LOGI("return pre allocf file failed. %s\n", FILEMNG_Strerror(CVI_FILEMNG_EINTER));
            return CVI_FILEMNG_EINTER;
        }
    }
    CVI_FILEMNG_AddFile(tmpEmrPath);

    return 0;
}

static int32_t FILEMNG_GetPhotoCntByDir(CVI_DTCF_DIR_E enDir, uint32_t *pu32Count)
{
    int32_t s32Ret = 0;
    uint32_t i, j;
    uint32_t u32DirIdx = enDir - DTCF_DIR_PHOTO_FRONT;
    CVI_DTCF_DIR_E enDirs[1] = { enDir };
    s32Ret = CVI_DTCF_Scan(enDirs, 1, pu32Count);
    if (0 != s32Ret) {
        return CVI_FILEMNG_EINTER;
    }

    bool PreAllocUnitChecked = false;

    /**create prealloc photo files */
    if (0 < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]) {
        if (*pu32Count > s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]) {
            CVI_LOGW("The count of photos %u is over pre allocation %u\n", *pu32Count,
                  s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]);
        }

        for (i = 0, j = *pu32Count - 1; i < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]; i++) {
            if (i + *pu32Count < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]) {
                /* New pre allocated files join the team at first. */
                uint32_t number = s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx] - *pu32Count - i - 1;
                snprintf(g_Photos[u32DirIdx].List[i].szFileName, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s/%s_%02d",
                         s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[enDir],
                         FILEMNG_PREALLOC_FILE_NAME_PREFIX, number);
                char szFileName[CVI_APPCOMM_MAX_PATH_LEN];

                if (NULL == realpath(g_Photos[u32DirIdx].List[i].szFileName, szFileName)) {
                    /**PreAllocFile not exsit,create it */
                    s32Ret = FILEMNG_CreatePreAllocFile(g_Photos[u32DirIdx].List[i].szFileName,
                                                        s_stFILEMNGDTCF_Cfg.u32PreAllocUnit[u32DirIdx]);
                    if (0 != s32Ret) {
                        return CVI_FILEMNG_EINTER;
                    }
                } else {
                    if (PreAllocUnitChecked == false) {
                        /**PreAllocFile exsit,check filesize is expected or not */
                        s32Ret = FILEMNG_PreAllocFileCheck(g_Photos[u32DirIdx].List[i].szFileName,
                                                           s_stFILEMNGDTCF_Cfg.u32PreAllocUnit[u32DirIdx]);
                        if (0 != s32Ret) {
                            CVI_EVENT_S stEvent = { .topic = CVI_EVENT_FILEMNG_UNIDENTIFICATION };
                            CVI_EVENTHUB_Publish(&stEvent);
                            return CVI_FILEMNG_EINTER;
                        }

                        PreAllocUnitChecked = true; /* Check each directory only once to improve performance */
                    }
                }
            } else {
                /* then existing files join the team */
                CVI_DTCF_DIR_E enDir;
                s32Ret = CVI_DTCF_GetFileByIndex(j, g_Photos[u32DirIdx].List[i].szFileName, CVI_APPCOMM_MAX_PATH_LEN, &enDir);
                if (0 != s32Ret) {
                    return CVI_FILEMNG_EINTER;
                }

                j--;
            }

            g_Photos[u32DirIdx].List[i].u32Index = i;
        }
    }

    return 0;
}

static int32_t FILEMNG_GetPhotoCnt(uint32_t *pu32Count)
{
    int32_t s32Ret = 0;
    *pu32Count = 0;
    int32_t dir_inx = 0;
    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_FRONT], CVI_DIR_LEN_MAX)) {
        s32Ret = FILEMNG_GetPhotoCntByDir(DTCF_DIR_PHOTO_FRONT, &g_Photos[dir_inx].u32Count);
        if (0 != s32Ret) {
            return CVI_FILEMNG_EINTER;
        }
        *pu32Count += g_Photos[dir_inx].u32Count;
        dir_inx++;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_REAR], CVI_DIR_LEN_MAX)) {
        s32Ret = FILEMNG_GetPhotoCntByDir(DTCF_DIR_PHOTO_REAR, &g_Photos[dir_inx].u32Count);
        if (0 != s32Ret) {
            return CVI_FILEMNG_EINTER;
        }
        *pu32Count += g_Photos[dir_inx].u32Count;
        dir_inx++;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_AHD], CVI_DIR_LEN_MAX)) {
        s32Ret = FILEMNG_GetPhotoCntByDir(DTCF_DIR_PHOTO_AHD, &g_Photos[dir_inx].u32Count);
        if (0 != s32Ret) {
            return CVI_FILEMNG_EINTER;
        }
        *pu32Count += g_Photos[dir_inx].u32Count;
        dir_inx++;
    }
    return 0;
}

static int32_t FILEMNG_GetTotalCnt(uint32_t *pu32Count)
{
    int32_t s32Ret = 0;
    uint32_t u32DirCount = 0;
    CVI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT];

    /**scan scope */
    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_EMR_FRONT;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_NORM_FRONT;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_FRONT], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_PARK_FRONT;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_EMR_REAR;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_NORM_REAR;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_PARK_REAR;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_FRONT], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_PHOTO_FRONT;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_REAR], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_PHOTO_REAR;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORMAL_AHD], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_NORMAL_AHD;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_AHD], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_EMR_AHD;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_AHD], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_PHOTO_AHD;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_AHD], CVI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_PARK_AHD;
    }

    if (u32DirCount > 0) {
        s32Ret = CVI_DTCF_Scan(enDirs, u32DirCount, &s_u32FILEMNGAllObjCnt);
        if (0 != s32Ret) {
            return CVI_FILEMNG_EINTER;
        }
    } else {
        *pu32Count = 0;
    }

    return 0;
}

static int32_t FILEMNG_AddRecord(const char *pszFilePath, CVI_DTCF_DIR_E enDir)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_DTCF_AddFile(pszFilePath, enDir);

    if (0 != s32Ret) {
        if (CVI_DTCF_SAME_FILENAME_PATH == s32Ret) {
            return CVI_FILEMNG_EEXIST;
        }
        return CVI_FILEMNG_EINVAL;
    } else {
        if (DTCF_DIR_EMR_FRONT == enDir || DTCF_DIR_EMR_REAR == enDir || DTCF_DIR_EMR_AHD == enDir) {
            s_u32FILEMNGEmrRecObjCnt++;
            s_u32EmrSpace_MB += FILEMNG_GetFileSize(pszFilePath);
        } else {
            s_u32MovieSpace_MB += FILEMNG_GetFileSize(pszFilePath);
        }

        s_u32FILEMNGRecObjCnt++;
    }

    return 0;
}

static int32_t FILEMNG_AddPhoto(const char *pszFilePath, CVI_DTCF_DIR_E enDir)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_DTCF_AddFile(pszFilePath, enDir);
    if (0 != s32Ret) {
        if (CVI_DTCF_SAME_FILENAME_PATH == s32Ret) {
            return CVI_FILEMNG_EEXIST;
        }
        return CVI_FILEMNG_EINVAL;
    } else {
        g_Photos[enDir - DTCF_DIR_PHOTO_FRONT].u32Count++;
        s_u32FILEMNGPhotoObjCnt++;
    }

    return 0;
}

static int32_t FILEMNG_RemoveRecord(const char *pszFilePath, CVI_DTCF_DIR_E enDir)
{
    int32_t s32Ret = 0;
    char szSubFilePath[CVI_APPCOMM_MAX_PATH_LEN];
    if (DTCF_DIR_EMR_FRONT == enDir || DTCF_DIR_EMR_REAR == enDir || DTCF_DIR_EMR_AHD == enDir) {
        if (DTCF_DIR_EMR_FRONT == enDir &&
            0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT_SUB], CVI_DIR_LEN_MAX)) {
            s32Ret = CVI_DTCF_GetRelatedFilePath(pszFilePath, DTCF_DIR_EMR_FRONT_SUB, szSubFilePath, CVI_APPCOMM_MAX_PATH_LEN);
            if (0 == s32Ret) {
                CVI_LOGI("remove(%s)\n", szSubFilePath);
                remove(szSubFilePath);
            } else {
                return CVI_FILEMNG_EINTER;
            }
        }

        if (DTCF_DIR_EMR_REAR == enDir &&
            0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR_SUB], CVI_DIR_LEN_MAX)) {
            s32Ret = CVI_DTCF_GetRelatedFilePath(pszFilePath, DTCF_DIR_EMR_REAR_SUB, szSubFilePath, CVI_APPCOMM_MAX_PATH_LEN);
            if (0 == s32Ret) {
                CVI_LOGI("remove(%s)\n", szSubFilePath);
                remove(szSubFilePath);
            } else {
                return CVI_FILEMNG_EINTER;
            }
        }

        if (DTCF_DIR_EMR_AHD == enDir &&
            0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_AHD_SUB], CVI_DIR_LEN_MAX)) {
            s32Ret = CVI_DTCF_GetRelatedFilePath(pszFilePath, DTCF_DIR_EMR_AHD_SUB, szSubFilePath, CVI_APPCOMM_MAX_PATH_LEN);
            if (0 == s32Ret) {
                CVI_LOGI("remove(%s)\n", szSubFilePath);
                remove(szSubFilePath);
            } else {
                return CVI_FILEMNG_EINTER;
            }
        }

        if (0 < s_u32FILEMNGEmrRecObjCnt) {
            s_u32FILEMNGEmrRecObjCnt--;
        } else {
            CVI_LOGE("s_u32FILEMNGEmrRecObjCnt is not correct\n");
            return CVI_FILEMNG_EINTER;
        }
        s_u32EmrSpace_MB -= FILEMNG_GetFileSize(pszFilePath);
    } else {
        if (DTCF_DIR_NORM_FRONT == enDir &&
            0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT_SUB], CVI_DIR_LEN_MAX)) {
            s32Ret = CVI_DTCF_GetRelatedFilePath(pszFilePath, DTCF_DIR_NORM_FRONT_SUB, szSubFilePath, CVI_APPCOMM_MAX_PATH_LEN);
            if (0 == s32Ret) {
                CVI_LOGI("remove(%s)\n", szSubFilePath);
                remove(szSubFilePath);
            } else {
                return CVI_FILEMNG_EINTER;
            }
        }

        if (DTCF_DIR_NORM_REAR == enDir &&
            0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR_SUB], CVI_DIR_LEN_MAX)) {
            s32Ret = CVI_DTCF_GetRelatedFilePath(pszFilePath, DTCF_DIR_NORM_REAR_SUB, szSubFilePath, CVI_APPCOMM_MAX_PATH_LEN);
            if (0 == s32Ret) {
                CVI_LOGI("remove(%s)\n", szSubFilePath);
                remove(szSubFilePath);
            } else {
                return CVI_FILEMNG_EINTER;
            }
        }

        if (DTCF_DIR_NORMAL_AHD == enDir &&
            0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORMAL_AHD_SUB], CVI_DIR_LEN_MAX)) {
            s32Ret = CVI_DTCF_GetRelatedFilePath(pszFilePath, DTCF_DIR_NORMAL_AHD_SUB, szSubFilePath, CVI_APPCOMM_MAX_PATH_LEN);
            if (0 == s32Ret) {
                CVI_LOGI("remove(%s)\n", szSubFilePath);
                remove(szSubFilePath);
            } else {
                return CVI_FILEMNG_EINTER;
            }
        }
        s_u32MovieSpace_MB -= FILEMNG_GetFileSize(pszFilePath);
    }
    CVI_LOGI("remove(%s)\n", pszFilePath);
    remove(pszFilePath);
    if (0 < s_u32FILEMNGRecObjCnt) {
        s_u32FILEMNGRecObjCnt--;
        s_u32FILEMNGAllObjCnt = s_u32FILEMNGPhotoObjCnt + s_u32FILEMNGRecObjCnt;
    } else {
        CVI_LOGE("s_u32FILEMNGRecObjCnt is not correct\n");
        return CVI_FILEMNG_EINTER;
    }
    return 0;
}

static int32_t FILEMNG_RemovePhoto(const char *pszFilePath, CVI_DTCF_DIR_E enDir)
{
    uint32_t i, j = 0;
    uint32_t u32DirIdx = enDir - DTCF_DIR_PHOTO_FRONT;

    if (0 < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]) {
        for (i = 0; i < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]; i++) {
            /* search file in team */
            if (0 == strncmp(g_Photos[u32DirIdx].List[i].szFileName, pszFilePath, CVI_APPCOMM_MAX_PATH_LEN)) {
                break;
            }
        }

        if (i == s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]) {
            /* file is not in the team,just remove it */
            CVI_LOGI("remove(%s)\n", pszFilePath);
            remove(pszFilePath);
        } else {
            /* file is in the team,rename it and set it's index to team header */
            uint32_t u32FreeIndex = s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx] - MIN(g_Photos[u32DirIdx].u32Count,
                                                                                      s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]);
            snprintf(g_Photos[u32DirIdx].List[i].szFileName, CVI_APPCOMM_MAX_PATH_LEN, "%s%s/%s/%s_%02d",
                     s_stFILEMNGCfg.szMntPath,
                     s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[enDir], FILEMNG_PREALLOC_FILE_NAME_PREFIX,
                     u32FreeIndex);
            rename(pszFilePath, g_Photos[u32DirIdx].List[i].szFileName);
            CVI_LOGI("rename %s to %s\n", pszFilePath, g_Photos[u32DirIdx].List[i].szFileName);
            FILEMNG_HideFile(g_Photos[u32DirIdx].List[i].szFileName, true);

            /* increase the indexs of other files */
            for (j = 0; j < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]; j++) {
                if (g_Photos[u32DirIdx].List[j].u32Index < g_Photos[u32DirIdx].List[i].u32Index) {
                    g_Photos[u32DirIdx].List[j].u32Index++;
                }
            }

            g_Photos[u32DirIdx].List[i].u32Index = 0;
        }
    } else {
        CVI_LOGI("remove(%s)\n", pszFilePath);
        remove(pszFilePath);
    }

    if (0 < g_Photos[u32DirIdx].u32Count) {
        g_Photos[u32DirIdx].u32Count--;
        s_u32FILEMNGPhotoObjCnt--;
        s_u32FILEMNGAllObjCnt = s_u32FILEMNGPhotoObjCnt + s_u32FILEMNGRecObjCnt;
    } else {
        CVI_LOGE("g_Photos[%d].u32Count is not correct\n", u32DirIdx);
        return CVI_FILEMNG_EINTER;
    }

    return 0;
}

static int32_t FILEMNG_DTCF_GetRatioSpace(uint32_t *pu32MovieSpace, uint32_t *pu32EmrSpace)
{
    FILEMNG_GetMovieSpace(pu32MovieSpace);
    FILEMNG_GetEmrSpace(pu32EmrSpace);
    return 0;
}

static int32_t FILEMNG_DTCF_GetSpace(uint32_t *pu32MovieSpace, uint32_t *pu32EmrSpace)
{
    *pu32MovieSpace = s_u32MovieSpace_MB;
    *pu32EmrSpace = s_u32EmrSpace_MB;

    return 0;
}

static int32_t FILEMNG_DTCF_Cover(int32_t s32FullFlag)
{
    int32_t s32Ret = 0;
    uint32_t u32Index = 0;
    uint32_t u32RemoveSize_MB = 0;
    uint32_t u32FileAmount = 0;
    char szAbsPath[CVI_APPCOMM_MAX_PATH_LEN];
    struct stat FileStat;
    uint32_t u32FileObjCnt = 0;
    int32_t s32TimeOut_ms = 2000;

    if(s_stFILEMNGDTCF_Cfg.u32RemoveLoopEn == 0){
        return s32Ret;
    }

    CVI_MUTEX_LOCK(s_FILEMNGMutex);

    while (s32TimeOut_ms > 0) {
        if (FILEMNG_DISK_STATE_SCAN_COMPLETED != s_enFILEMNGDiskState) {
            CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
            CVI_LOGI("wait disk scan completed...\n");
            cvi_usleep(500 * 1000);
            s32TimeOut_ms -= 500;
            CVI_MUTEX_LOCK(s_FILEMNGMutex);
        } else {
            break;
        }
    }

    if (s_u32DirCount < s_u32MaxDirCount) {
        s32Ret |= FILEMNG_GetEmrCnt(&s_u32FILEMNGEmrRecObjCnt);
        s32Ret |= FILEMNG_GetPhotoCnt(&s_u32FILEMNGPhotoObjCnt);
        s32Ret |= FILEMNG_GetTotalCnt(&s_u32FILEMNGAllObjCnt);
        if (0 != s32Ret) {
            goto end;
        }
        s_u32FILEMNGRecObjCnt = s_u32FILEMNGAllObjCnt - s_u32FILEMNGPhotoObjCnt;
    }
    FILEMNG_PrintInfo();

    /**<0x000:space enough;0x001:total space full;0x010:movie space full;0x100:emr movie space full */
    if (SPACEMONITOR_MASK_ENOUGH == s32FullFlag) {
        goto end;
    }

    if (0 == s_u32FILEMNGAllObjCnt) {
        CVI_EVENT_S stEvent;
        stEvent.topic = CVI_EVENT_FILEMNG_UNIDENTIFICATION;
        CVI_EVENTHUB_Publish(&stEvent);
        if (SPACEMONITOR_MASK_TOTALFULL == (SPACEMONITOR_MASK_TOTALFULL & s32FullFlag)) {
            CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
            return -1;
        }
        goto end;
    }

    if (SPACEMONITOR_MASK_EMRFULL == (SPACEMONITOR_MASK_EMRFULL & s32FullFlag)) {
        /* remove emr */
        do {
            CVI_DTCF_DIR_E aenDirs[] = {
                DTCF_DIR_EMR_FRONT,
                DTCF_DIR_EMR_REAR,
                DTCF_DIR_EMR_AHD,
            };

            s32Ret = CVI_DTCF_GetOldestFileIndex(aenDirs, ARRAY_SIZE(aenDirs), &u32Index);
            if (0 == s32Ret) {
                CVI_DTCF_DIR_E enDir = DTCF_DIR_BUTT;
                CVI_DTCF_GetFileByIndex(u32Index, szAbsPath, CVI_APPCOMM_MAX_PATH_LEN, &enDir);
                stat(szAbsPath, &FileStat);
                CVI_DTCF_DelFileByIndex(u32Index, &u32FileAmount);
                FILEMNG_RemoveRecord(szAbsPath, enDir);
                u32RemoveSize_MB += FileStat.st_size >> 20;
            } else {
                break;
            }
        } while (u32RemoveSize_MB <= (s_stFILEMNGDTCF_Cfg.u32GuaranteedStage - s_stFILEMNGDTCF_Cfg.u32WarningStage));
    }

    if (SPACEMONITOR_MASK_TOTALFULL == (SPACEMONITOR_MASK_TOTALFULL & s32FullFlag)
        || SPACEMONITOR_MASK_MOVIEFULL == (SPACEMONITOR_MASK_MOVIEFULL & s32FullFlag)) {
        /* remove movies */
        u32RemoveSize_MB = 0;
        do {
            CVI_DTCF_DIR_E aenDirs[] = {
                DTCF_DIR_NORM_FRONT,
                DTCF_DIR_NORM_REAR,
                DTCF_DIR_NORMAL_AHD,
                DTCF_DIR_PARK_FRONT,
                DTCF_DIR_PARK_REAR,
                DTCF_DIR_PARK_AHD,
            };
            s32Ret = CVI_DTCF_GetOldestFileIndex(aenDirs, ARRAY_SIZE(aenDirs), &u32Index);
            if (0 == s32Ret) {
                CVI_DTCF_DIR_E enDir = DTCF_DIR_BUTT;
                CVI_DTCF_GetFileByIndex(u32Index, szAbsPath, CVI_APPCOMM_MAX_PATH_LEN, &enDir);
                stat(szAbsPath, &FileStat);
                CVI_DTCF_DelFileByIndex(u32Index, &u32FileAmount);
                FILEMNG_RemoveRecord(szAbsPath, enDir);
                u32RemoveSize_MB += FileStat.st_size >> 20;
            } else {
                CVI_LOGE("CVI_DTCF_GetOldestFileIndex faile\n");
                break;
            }
        } while (u32RemoveSize_MB <= (s_stFILEMNGDTCF_Cfg.u32GuaranteedStage - s_stFILEMNGDTCF_Cfg.u32WarningStage));
    }

    FILEMNG_PrintInfo();
end:
    if (s_u32DirCount < s_u32MaxDirCount) {
        s32Ret = CVI_DTCF_Scan(s_aenDirs, s_u32DirCount, &u32FileObjCnt);
        if (0 != s32Ret) {
            CVI_LOGE("Scan faile\n");
        }
    }

    CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return s32Ret;
}

int32_t CVI_FILEMNG_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_FILEMNG_SCAN_COMPLETED);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_FILEMNG_SCAN_FAIL);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_FILEMNG_SPACE_FULL);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_FILEMNG_SPACE_ENOUGH);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_FILEMNG_UNIDENTIFICATION);
    CVI_APPCOMM_CHECK_RETURN(s32Ret, CVI_FILEMNG_EINTER);
    return 0;
}

int32_t CVI_FILEMNG_Init(const CVI_FILEMNG_COMM_CFG_S *pstCfg, const CVI_FILEMNG_DTCF_CFG_S *pstDTCF_Cfg)
{
    uint32_t u32DirIdx = 0;
    int32_t s32Ret = 0;
    CVI_APPCOMM_CHECK_POINTER(pstCfg, CVI_FILEMNG_EINVAL);
    CVI_APPCOMM_CHECK_POINTER(pstDTCF_Cfg, CVI_FILEMNG_EINVAL);
    CVI_APPCOMM_CHECK_EXPR(pstDTCF_Cfg->u8SharePercent < 100, CVI_FILEMNG_EINVAL);

    for (u32DirIdx = 0; u32DirIdx < CVI_FILEMNG_DTCF_MAX_PHOTO_DIR; u32DirIdx++) {
        if (0 < strnlen(pstDTCF_Cfg->aszDirNames[u32DirIdx + DTCF_DIR_PHOTO_FRONT], CVI_DIR_LEN_MAX)) {
            CVI_APPCOMM_CHECK_EXPR(FILEMNG_PREALLOC_FILE_MAX_NUM >= pstDTCF_Cfg->u32PreAllocCnt[u32DirIdx], CVI_FILEMNG_EINVAL);
            if (0 < pstDTCF_Cfg->u32PreAllocCnt[u32DirIdx]) {
                CVI_APPCOMM_CHECK_EXPR(0 < pstDTCF_Cfg->u32PreAllocUnit[u32DirIdx], CVI_FILEMNG_EINVAL);
            }
        }
    }

    if (0 != pstDTCF_Cfg->u32GuaranteedStage) {
        CVI_APPCOMM_CHECK_EXPR(pstDTCF_Cfg->u32GuaranteedStage >= pstDTCF_Cfg->u32WarningStage, CVI_FILEMNG_EINVAL);
    }

    CVI_MUTEX_LOCK(s_FILEMNGMutex);

    if (true == s_bFILEMNGInit) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGD("%s\n", FILEMNG_Strerror(CVI_FILEMNG_EINITIALIZED));
        return CVI_FILEMNG_EINITIALIZED;
    }
    memcpy(&s_stFILEMNGCfg, pstCfg, sizeof(CVI_FILEMNG_COMM_CFG_S));
    memcpy(&s_stFILEMNGDTCF_Cfg, pstDTCF_Cfg, sizeof(CVI_FILEMNG_DTCF_CFG_S));

    if (!s_stFILEMNGDTCF_Cfg.preAllocFilesEnable){
        SPACEMONITOR_CFG_S stConfig;
        snprintf(stConfig.szMntPath, CVI_APPCOMM_MAX_PATH_LEN, pstCfg->szMntPath);
        stConfig.u32WarningStage = pstDTCF_Cfg->u32WarningStage;
        stConfig.u32GuaranteedStage = pstDTCF_Cfg->u32GuaranteedStage;
        stConfig.u8SharePercent = pstDTCF_Cfg->u8SharePercent;
        stConfig.u32Interval = 0;
        stConfig.u32MaxCheckDelay = 10;
        stConfig.pfnCoverCB = FILEMNG_DTCF_Cover;
        stConfig.pfnGetRatioSpace = FILEMNG_DTCF_GetSpace;
        s32Ret = FILEMNG_SPACEMONITOR_Create(&stConfig);
        if (0 != s32Ret) {
            CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
            return s32Ret;
        }
    }
    s_u32FILEMNGRecObjCnt = 0;
    s_u32FILEMNGPhotoObjCnt = 0;
    s_enFILEMNGDiskState = FILEMNG_DISK_STATE_NOT_AVAILABLE;

    for (u32DirIdx = 0; u32DirIdx < CVI_FILEMNG_DTCF_MAX_PHOTO_DIR; u32DirIdx++) {
        if (0 < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx] &&
            0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[u32DirIdx + DTCF_DIR_PHOTO_FRONT], CVI_DIR_LEN_MAX)) {
            g_Photos[u32DirIdx].List = (FILEMNG_PHOTO_QUEUE_S *)malloc(s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx] * sizeof(FILEMNG_PHOTO_QUEUE_S));
            if (NULL == g_Photos[u32DirIdx].List) {
                FILEMNG_SPACEMONITOR_Destroy();
                CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
                CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_EINTER));
                return CVI_FILEMNG_EINTER;
            }

            memset(g_Photos[u32DirIdx].List, 0,
                   (s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx] * sizeof(FILEMNG_PHOTO_QUEUE_S)));
            g_Photos[u32DirIdx].u32Count = 0;
        }
    }

    s_bFILEMNGInit = true;
    CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return 0;
}

int32_t CVI_FILEMNG_DeInit(void)
{
    int32_t s32Ret = 0;
    CVI_MUTEX_LOCK(s_FILEMNGMutex);

    if (false == s_bFILEMNGInit) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_ENOTINIT));
        return CVI_FILEMNG_ENOTINIT;
    }

    if (FILEMNG_DISK_STATE_NOT_AVAILABLE != s_enFILEMNGDiskState) {
        FILEMNG_Disable();
    }

    s32Ret = FILEMNG_SPACEMONITOR_Destroy();
    if (0 != s32Ret) {
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_ENOTINIT));
    }

    FILEMNG_DestroyMoviePreAllocFiles();

    uint32_t u32DirIdx = 0;
    for (u32DirIdx = 0; u32DirIdx < CVI_FILEMNG_DTCF_MAX_PHOTO_DIR; u32DirIdx++) {
        if (0 < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]) {
            CVI_APPCOMM_SAFE_FREE(g_Photos[u32DirIdx].List);
        }
    }

    s_bFILEMNGInit = false;
    CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return 0;
}

int32_t CVI_FILEMNG_SetDiskState(bool bAvailable)
{
    int32_t s32Ret = 0;
    CVI_MUTEX_LOCK(s_FILEMNGMutex);

    if (false == s_bFILEMNGInit) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_ENOTINIT));
        return CVI_FILEMNG_ENOTINIT;
    }

    if ((true == bAvailable && FILEMNG_DISK_STATE_NOT_AVAILABLE != s_enFILEMNGDiskState)
        || (false == bAvailable && FILEMNG_DISK_STATE_NOT_AVAILABLE == s_enFILEMNGDiskState)) {
        /**disk available status not change,do nothing. */
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return 0;
    } else {
        if (bAvailable) {
            s32Ret = FILEMNG_CheckPath(s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir);
            if (0 != s32Ret) {
                CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
                CVI_LOGE("FILEMNG_CheckPath  %s\n", FILEMNG_Strerror(s32Ret));
                return s32Ret;
            }

            char szRootPath[CVI_APPCOMM_MAX_PATH_LEN];
            snprintf(szRootPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s", s_stFILEMNGCfg.szMntPath,
                     s_stFILEMNGDTCF_Cfg.szRootDir);
            s32Ret = CVI_DTCF_Init(szRootPath, (const char(*)[CVI_DIR_LEN_MAX])(s_stFILEMNGDTCF_Cfg.aszDirNames));
            if (0 != s32Ret) {
                CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
                CVI_LOGE("CVI_DTCF_Init  %s\n", FILEMNG_Strerror(s32Ret));
                return CVI_FILEMNG_EINTER;
            }

            if (s_stFILEMNGDTCF_Cfg.preAllocFilesEnable){
                CVI_LOGI("=============create prealloc file=================");
                CVI_DTCF_DIR_E enDir = DTCF_DIR_EMR_FRONT;
                CVI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT] = {0};
                for ( ; enDir < DTCF_DIR_BUTT; enDir++){
                    enDirs[enDir] = enDir;
                }
                s32Ret = FILEMNG_CreateMoviePreAllocFiles(enDirs, DTCF_DIR_BUTT);
                if (0 != s32Ret) {
                    CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
                    CVI_LOGE("FILEMNG_PreAllocFiles  %s\n", FILEMNG_Strerror(s32Ret));
                    return CVI_FILEMNG_EINTER;
                }
            }

            s32Ret |= FILEMNG_GetEmrCnt(&s_u32FILEMNGEmrRecObjCnt);
            s32Ret |= FILEMNG_GetPhotoCnt(&s_u32FILEMNGPhotoObjCnt);
            s32Ret |= FILEMNG_GetTotalCnt(&s_u32FILEMNGAllObjCnt);
            // CVI_EVENT_S stEvent;
            s_enFILEMNGDiskState = FILEMNG_DISK_STATE_SCAN_COMPLETED;
            if (0 != s32Ret) {
                CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
                // stEvent.topic = CVI_EVENT_FILEMNG_SCAN_FAIL;
                // CVI_EVENTHUB_Publish(&stEvent);
                CVI_LOGE("%s\n", FILEMNG_Strerror(s32Ret));
                return CVI_FILEMNG_EINTER;
            }

            /**scan scope */
            s_u32DirCount = 0;
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT], CVI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_EMR_FRONT;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT], CVI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_NORM_FRONT;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_FRONT], CVI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_PARK_FRONT;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR], CVI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_EMR_REAR;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR], CVI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_NORM_REAR;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR], CVI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_PARK_REAR;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_FRONT], CVI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_PHOTO_FRONT;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_REAR], CVI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_PHOTO_REAR;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_AHD], CVI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_EMR_AHD;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORMAL_AHD], CVI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_NORMAL_AHD;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_AHD], CVI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_PARK_AHD;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_AHD], CVI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_PHOTO_AHD;
            }
            FILEMNG_DTCF_GetRatioSpace(&s_u32MovieSpace_MB, &s_u32EmrSpace_MB);
            FILEMNG_GetPhotoSpace(&s_u32PhotoSpace_MB);
            s_u32MaxDirCount = s_u32DirCount;
            s_u32FILEMNGRecObjCnt = s_u32FILEMNGAllObjCnt - s_u32FILEMNGPhotoObjCnt;
            FILEMNG_PrintInfo();
            // stEvent.topic = CVI_EVENT_FILEMNG_SCAN_COMPLETED;
            // CVI_EVENTHUB_Publish(&stEvent);
            CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
            return 0;
        } else {
            FILEMNG_Disable();
            CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
            return 0;
        }
    }
}

int32_t CVI_FILEMNG_CheckDiskSpace(void)
{
    int32_t s32Ret = 0;
    CVI_MUTEX_LOCK(s_FILEMNGMutex);

    if (false == s_bFILEMNGInit) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_ENOTINIT));
        return CVI_FILEMNG_ENOTINIT;
    }

    if (FILEMNG_DISK_STATE_SCAN_COMPLETED != s_enFILEMNGDiskState) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_ENOTREADY));
        return CVI_FILEMNG_ENOTREADY;
    }
    uint64_t u64RealUsedSize = 0;
    // uint32_t u32MovieSpace_MB = 0;
    // uint32_t u32EmrSpace_MB = 0;
    // uint32_t u32PhotoSpace_MB = 0;
    // FILEMNG_DTCF_GetRatioSpace(&u32MovieSpace_MB, &u32EmrSpace_MB);
    // FILEMNG_GetPhotoSpace(&u32PhotoSpace_MB);

    u64RealUsedSize = s_u32MovieSpace_MB + s_u32EmrSpace_MB + s_u32PhotoSpace_MB;
    s32Ret = FILEMNG_SPACEMONITOR_JudgeStage(u64RealUsedSize);
    if (0 != s32Ret) {
        CVI_LOGE("%s\n", FILEMNG_Strerror(s32Ret));
    }

    CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return s32Ret;
}

int32_t CVI_FILEMNG_AddFile(const char *pszFilePath)
{
    int32_t s32Ret = 0;
    CVI_APPCOMM_CHECK_POINTER(pszFilePath, CVI_FILEMNG_EINVAL);
    CVI_MUTEX_LOCK(s_FILEMNGMutex);

    if (false == s_bFILEMNGInit) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_ENOTINIT));
        return CVI_FILEMNG_ENOTINIT;
    }

    s32Ret = cvi_PathIsDirectory(pszFilePath);
    if (0 == s32Ret) {
        CVI_DTCF_DIR_E enDir = DTCF_DIR_BUTT;
        s32Ret = CVI_DTCF_GetFileDirType(pszFilePath, &enDir);
        if (0 != s32Ret) {
            CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
            return CVI_FILEMNG_EINVAL;
        }

        if (DTCF_DIR_EMR_FRONT_SUB == enDir || DTCF_DIR_NORM_FRONT_SUB == enDir
            || DTCF_DIR_EMR_REAR_SUB == enDir || DTCF_DIR_NORM_REAR_SUB == enDir
            || DTCF_DIR_NORMAL_AHD_SUB == enDir || DTCF_DIR_EMR_AHD_SUB == enDir) {
            CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
            return CVI_FILEMNG_ENOTMAIN;
        } else if (DTCF_DIR_PHOTO_FRONT == enDir || DTCF_DIR_PHOTO_REAR == enDir || DTCF_DIR_PHOTO_AHD == enDir) {
            s32Ret = FILEMNG_AddPhoto(pszFilePath, enDir);
        } else {
            s32Ret = FILEMNG_AddRecord(pszFilePath, enDir);
        }

        if (0 != s32Ret) {
            CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
            CVI_LOGE("%s\n", FILEMNG_Strerror(s32Ret));
            return s32Ret;
        }

        s_u32FILEMNGAllObjCnt = s_u32FILEMNGPhotoObjCnt + s_u32FILEMNGRecObjCnt;
        FILEMNG_PrintInfo();
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return 0;
    }

    CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
    CVI_LOGE("%s:%s\n", FILEMNG_Strerror(CVI_FILEMNG_ELOST), pszFilePath);
    return CVI_FILEMNG_ELOST;
}

int32_t CVI_FILEMNG_RemoveFile(const char *pszFilePath)
{
    int32_t s32Ret = 0;
    int32_t s32IsDir = 0;
    uint32_t i = 0;
    CVI_APPCOMM_CHECK_POINTER(pszFilePath, CVI_FILEMNG_EINVAL);
    CVI_MUTEX_LOCK(s_FILEMNGMutex);

    if (false == s_bFILEMNGInit) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_ENOTINIT));
        return CVI_FILEMNG_ENOTINIT;
    }

    s32IsDir = cvi_PathIsDirectory(pszFilePath);
    char szFileName[CVI_APPCOMM_MAX_PATH_LEN];
    CVI_DTCF_DIR_E enDir = DTCF_DIR_BUTT;
    uint32_t u32FileAmount;
    if (!s_stFILEMNGDTCF_Cfg.preAllocFilesEnable){
        for (i = 0; i < s_u32FILEMNGAllObjCnt; i++, enDir = DTCF_DIR_BUTT) {
            s32Ret = CVI_DTCF_GetFileByIndex(i, szFileName, CVI_APPCOMM_MAX_PATH_LEN, &enDir);
            if (0 != s32Ret) {
                CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
                return CVI_FILEMNG_EINTER;
            }

            if (0 == strncmp(pszFilePath, szFileName, CVI_APPCOMM_MAX_PATH_LEN)) {
                CVI_DTCF_DelFileByIndex(i, &u32FileAmount);
                break;
            }
        }
    }
    if (DTCF_DIR_BUTT == enDir) {
        s32Ret = CVI_DTCF_GetFileDirType(pszFilePath, &enDir);
        if (0 != s32Ret) {
            CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
            return CVI_FILEMNG_EINVAL;
        }
    }

    if (DTCF_DIR_EMR_FRONT_SUB == enDir || DTCF_DIR_NORM_FRONT_SUB == enDir
        || DTCF_DIR_EMR_REAR_SUB == enDir || DTCF_DIR_NORM_REAR_SUB == enDir
        || DTCF_DIR_EMR_AHD_SUB == enDir || DTCF_DIR_NORMAL_AHD_SUB == enDir) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return CVI_FILEMNG_ENOTMAIN;
    } else if (DTCF_DIR_PHOTO_FRONT == enDir || DTCF_DIR_PHOTO_REAR == enDir || DTCF_DIR_PHOTO_AHD == enDir) {
        s32Ret = FILEMNG_RemovePhoto(pszFilePath, enDir);
    } else {
        if (s_stFILEMNGDTCF_Cfg.preAllocFilesEnable){
            s32Ret = FILEMNG_RemoveMoiveFiles(pszFilePath, enDir);
        }else{
            s32Ret = FILEMNG_RemoveRecord(pszFilePath, enDir);
        }
    }

    if (0 != s32Ret) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(s32Ret));
        return s32Ret;
    }

    FILEMNG_PrintInfo();
    if (0 == s32IsDir) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return 0;
    } else {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s:%s\n", FILEMNG_Strerror(CVI_FILEMNG_ELOST), pszFilePath);
        return CVI_FILEMNG_ELOST;
    }
}

int32_t CVI_FILEMNG_SetSearchScope(CVI_DTCF_DIR_E aenDirs[DTCF_DIR_BUTT], uint32_t u32DirCount, uint32_t *pu32FileObjCnt)
{
    int32_t s32Ret = 0;
    CVI_APPCOMM_CHECK_POINTER(pu32FileObjCnt, CVI_FILEMNG_EINVAL);
    CVI_APPCOMM_CHECK_EXPR(u32DirCount <= DTCF_DIR_BUTT, CVI_FILEMNG_EINVAL);
    CVI_MUTEX_LOCK(s_FILEMNGMutex);

    if (false == s_bFILEMNGInit) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_ENOTINIT));
        return CVI_FILEMNG_ENOTINIT;
    }

    if (FILEMNG_DISK_STATE_NOT_AVAILABLE == s_enFILEMNGDiskState) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_ENOTREADY));
        return CVI_FILEMNG_ENOTREADY;
    }

    if (FILEMNG_DISK_STATE_AVAILABLE == s_enFILEMNGDiskState) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_EBUSY));
        return CVI_FILEMNG_EBUSY;
    }

    uint32_t ScopeDirCount = 0;
    CVI_DTCF_DIR_E ScopeDirs[DTCF_DIR_BUTT];
    uint32_t i = 0;
    for (i = 0; i < u32DirCount; i++) {
        if (aenDirs[i] == DTCF_DIR_EMR_FRONT_SUB ||
            aenDirs[i] == DTCF_DIR_NORM_FRONT_SUB ||
            aenDirs[i] == DTCF_DIR_PARK_FRONT_SUB ||
            aenDirs[i] == DTCF_DIR_EMR_REAR_SUB ||
            aenDirs[i] == DTCF_DIR_NORM_REAR_SUB ||
            aenDirs[i] == DTCF_DIR_PARK_REAR_SUB ||
            aenDirs[i] == DTCF_DIR_EMR_AHD_SUB ||
            aenDirs[i] == DTCF_DIR_NORMAL_AHD_SUB ||
            aenDirs[i] == DTCF_DIR_PARK_AHD_SUB) {
            CVI_LOGD("Ignore substream directories.\n");
        } else {
            ScopeDirs[ScopeDirCount] = aenDirs[i];
            ScopeDirCount++;
        }
    }
    s32Ret = CVI_DTCF_Scan(ScopeDirs, ScopeDirCount, pu32FileObjCnt);
    if (0 != s32Ret) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return CVI_FILEMNG_EINTER;
    }

    if (ScopeDirCount <= s_u32MaxDirCount) {
        s_u32DirCount = ScopeDirCount;
        memcpy(s_aenDirs, ScopeDirs, DTCF_DIR_BUTT * sizeof(CVI_DTCF_DIR_E));
    } else {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_EINVAL));
        return CVI_FILEMNG_EINVAL;
    }

    CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return 0;
}

int32_t CVI_FILEMNG_GetFileObjCnt(CVI_FILEMNG_FILE_TYPE_E enType, uint32_t *pu32FileObjCnt)
{
    CVI_APPCOMM_CHECK_POINTER(pu32FileObjCnt, CVI_FILEMNG_EINVAL);
    CVI_MUTEX_LOCK(s_FILEMNGMutex);

    if (false == s_bFILEMNGInit) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_ENOTINIT));
        return CVI_FILEMNG_ENOTINIT;
    }

    *pu32FileObjCnt = 0;
    if (FILEMNG_DISK_STATE_AVAILABLE == s_enFILEMNGDiskState) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_EBUSY));
        return CVI_FILEMNG_EBUSY;
    } else if (FILEMNG_DISK_STATE_SCAN_COMPLETED == s_enFILEMNGDiskState) {
        switch (enType) {
            case CVI_FILEMNG_FILE_TYPE_RECORD:
                *pu32FileObjCnt = s_u32FILEMNGRecObjCnt;
                break;
            case CVI_FILEMNG_FILE_TYPE_PHOTO:
                *pu32FileObjCnt = s_u32FILEMNGPhotoObjCnt;
                break;
            default:
                *pu32FileObjCnt = s_u32FILEMNGAllObjCnt;
                break;
        }
    }

    CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return 0;
}

int32_t CVI_FILEMNG_GetFileByIndex(uint32_t u32FileIdx, char *pazFileName, uint32_t u32Length)
{
    int32_t s32Ret = 0;
    CVI_APPCOMM_CHECK_POINTER(pazFileName, CVI_FILEMNG_EINVAL);
    CVI_MUTEX_LOCK(s_FILEMNGMutex);

    if (false == s_bFILEMNGInit) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_ENOTINIT));
        return CVI_FILEMNG_ENOTINIT;
    }

    if (u32FileIdx >= s_u32FILEMNGAllObjCnt) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_EINVAL));
        return CVI_FILEMNG_EINVAL;
    }

    if (FILEMNG_DISK_STATE_SCAN_COMPLETED != s_enFILEMNGDiskState) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        CVI_LOGE("%s\n", FILEMNG_Strerror(CVI_FILEMNG_EBUSY));
        return CVI_FILEMNG_EBUSY;
    }

    CVI_DTCF_DIR_E enDir;
    s32Ret = CVI_DTCF_GetFileByIndex(u32FileIdx, pazFileName, u32Length, &enDir);
    if (0 != s32Ret) {
        CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return CVI_FILEMNG_EINVAL;
    }

    CVI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return s32Ret;
}

int32_t CVI_FILEMNG_GetFileInfoByName(const char *pszFilePath, CVI_FILEMNG_FILE_INFO_S *pstFileInfo)
{
    CVI_APPCOMM_CHECK_POINTER(pszFilePath, CVI_FILEMNG_EINVAL);
    CVI_APPCOMM_CHECK_POINTER(pstFileInfo, CVI_FILEMNG_EINVAL);
    return FILEMNG_GetFileInfo(pszFilePath, pstFileInfo);
}

int32_t CVI_FILEMNG_GeneratePhotoName(CVI_DTCF_FILE_TYPE_E enType, CVI_DTCF_DIR_E enDir, bool bPreAlloc, char *FileName)
{
    int32_t s32Ret = 0;
    uint32_t i, j;
    s32Ret = CVI_DTCF_CreateFilePath(enType, enDir, FileName, CVI_APPCOMM_MAX_PATH_LEN);
    if (0 != s32Ret) {
        CVI_LOGE("CVI_DTCF_CreateFilePath Failed ret = %d\n", s32Ret);
        return -1;
    }

    if (bPreAlloc) {
        uint32_t u32DirIdx = enDir - DTCF_DIR_PHOTO_FRONT;
        for (i = 0; i < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]; i++) {
            if (0 == g_Photos[u32DirIdx].List[i].u32Index) {
                g_Photos[u32DirIdx].List[i].u32Index = s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx] - 1;
                rename(g_Photos[u32DirIdx].List[i].szFileName, FileName);
                FILEMNG_HideFile(FileName, false);
                CVI_DTCF_DIR_E enDirType = DTCF_DIR_BUTT;
                s32Ret = CVI_DTCF_GetFileDirType(g_Photos[u32DirIdx].List[i].szFileName, &enDirType);

                if (0 == s32Ret && enDirType == enDir) {
                    for (j = 0; j < s_u32FILEMNGAllObjCnt; j++) {
                        char szFileName[CVI_APPCOMM_MAX_PATH_LEN];
                        s32Ret = CVI_DTCF_GetFileByIndex(j, szFileName, CVI_APPCOMM_MAX_PATH_LEN, &enDirType);
                        if (0 != s32Ret) {
                            CVI_LOGE("CVI_DTCF_GetFileByIndex Failed ret = %d\n", s32Ret);
                            return -1;
                        }

                        if (0 == strncmp(g_Photos[u32DirIdx].List[i].szFileName, szFileName, CVI_APPCOMM_MAX_PATH_LEN)) {
                            uint32_t u32FileAmount;
                            CVI_DTCF_DelFileByIndex(j, &u32FileAmount);
                            g_Photos[u32DirIdx].u32Count--;
                            s_u32FILEMNGPhotoObjCnt--;
                            s_u32FILEMNGAllObjCnt--;
                            break;
                        }
                    }
                }
                /*
                ** In PIV case,
                ** CVI_MODEMNG_StartPiv will check return value of CVI_FILEMNG_GeneratePhotoName.
                ** If there are PreAlloc File, CVI_DTCF_GetFileDirType will get failed since suffix check.
                ** Only get true when photo recycle case (mean PreAlloc file run out)
                ** Thus, we force to set s32Ret to 0 to avoid PIV failed.
                */
                s32Ret = 0;

                CVI_LOGI("rename %s to %s\n", g_Photos[u32DirIdx].List[i].szFileName, FileName);
                memcpy(g_Photos[u32DirIdx].List[i].szFileName, FileName, CVI_APPCOMM_MAX_PATH_LEN);
            } else {
                g_Photos[u32DirIdx].List[i].u32Index--;
            }
        }
    } else {
        CVI_FILEMNG_SpacemonitorCheckSpace();
    }
    return s32Ret;
}

int32_t CVI_FILEMNG_GetDirType(int32_t id, CVI_DTCF_DIR_E base)
{
    if(id == 0){
        switch(base){
            case DTCF_DIR_EMR_FRONT:
                return DTCF_DIR_EMR_FRONT;
            break;
            case DTCF_DIR_NORM_FRONT:
                return DTCF_DIR_NORM_FRONT;
            break;
            case DTCF_DIR_PHOTO_FRONT:
                return DTCF_DIR_PHOTO_FRONT;
            break;
            default:
            break;
        }
    }else if(id == 1){
        switch(base){
            case DTCF_DIR_EMR_FRONT:
                return DTCF_DIR_EMR_REAR;
            break;
            case DTCF_DIR_NORM_FRONT:
                return DTCF_DIR_NORM_REAR;
            break;
            case DTCF_DIR_PHOTO_FRONT:
                return DTCF_DIR_PHOTO_REAR;
            break;
            default:
            break;
        }
    }else if(id == MAX_CAMERA_INSTANCES - 1){
        switch(base){
            case DTCF_DIR_EMR_FRONT:
                return DTCF_DIR_EMR_AHD;
            break;
            case DTCF_DIR_NORM_FRONT:
                return DTCF_DIR_NORMAL_AHD;
            break;
            case DTCF_DIR_PHOTO_FRONT:
                return DTCF_DIR_PHOTO_AHD;
            break;
            default:
            break;
        }
    }

    return DTCF_DIR_BUTT;
}

int32_t CVI_FILEMNG_SetRemoveLoop(int32_t en)
{
    s_stFILEMNGDTCF_Cfg.u32RemoveLoopEn = en;
    return 0;
}

int32_t CVI_FILEMNG_GenerateRecordName(CVI_DTCF_FILE_TYPE_E enType, CVI_DTCF_DIR_E enDir,
                                         char *pstFileName)
{
    int32_t s32Ret = 0;
    // CVI_DTCF_DIR_E enSubDir = DTCF_DIR_BUTT;

    // switch (enDir) {
    //     case DTCF_DIR_EMR_FRONT:
    //         enSubDir = DTCF_DIR_EMR_FRONT_SUB;
    //         break;
    //     case DTCF_DIR_NORM_FRONT:
    //         enSubDir = DTCF_DIR_NORM_FRONT_SUB;
    //         break;
    //     case DTCF_DIR_PARK_FRONT:
    //         enSubDir = DTCF_DIR_PARK_FRONT_SUB;
    //         break;
    //     case DTCF_DIR_EMR_REAR:
    //         enSubDir = DTCF_DIR_EMR_REAR_SUB;
    //         break;
    //     case DTCF_DIR_NORM_REAR:
    //         enSubDir = DTCF_DIR_NORM_REAR_SUB;
    //         break;
    //     case DTCF_DIR_PARK_REAR:
    //         enSubDir = DTCF_DIR_PARK_REAR_SUB;
    //         break;
    //     case DTCF_DIR_PHOTO_FRONT:
    //     case DTCF_DIR_PHOTO_REAR:
    //     case DTCF_DIR_BUTT:
    //         return CVI_FILEMNG_EINVAL;
    //     default:
    //         return CVI_FILEMNG_ENOTMAIN;
    // }

    if (s_stFILEMNGDTCF_Cfg.preAllocFilesEnable){
        s32Ret = FILEMNG_GenerateMovieFileName(enType, enDir, pstFileName);
        if (0 != s32Ret) {
            CVI_LOGE("CVI_FILEMNG_GenerateFileName Failed. = %s\n", FILEMNG_Strerror(s32Ret));
            return CVI_FILEMNG_EINTER;
        }
    }else{
        s32Ret = CVI_DTCF_CreateFilePath(enType, enDir, pstFileName, CVI_APPCOMM_MAX_PATH_LEN);
        if (0 != s32Ret) {
            CVI_LOGE("CVI_DTCF_CreateFilePath Failed ret = %d\n", s32Ret);
            return CVI_FILEMNG_EINTER;
        }
    }
    // if (true == s_stFILEMNGCfg.stRepairCfg.bEnable) {
    //     int32_t i = 0;
    //     for (i = 0; i < pstFileName->u8FileCnt; i++) {
    //         FILEMNG_REPAIRER_Backup(pstFileName->szFileName[i]);
    //     }
    // }

    CVI_FILEMNG_SpacemonitorCheckSpace();

    return 0;
}

int32_t CVI_FILEMNG_FileCoverStatus(bool en)
{
    int32_t s32Ret = 0;

    s32Ret = FILEMNG_SPACEMONITOR_SetCoverStatus(en);
    if (0 != s32Ret) {
        CVI_LOGE("FILEMNG_SPACEMONITOR_SetCoverStatus Failed. = %s\n", FILEMNG_Strerror(s32Ret));
        return CVI_FILEMNG_EINTER;
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

