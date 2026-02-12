/**
 * @file    filemng_common.c
 * @brief   file manager common function.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0

 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <time.h>

#include <linux/msdos_fs.h>
#include "filemng_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define FILEMNG_EXTENSION_LENGTH (4)

extern int32_t FILEMNG_MARKER_GetFlag(const char *pszFileName, uint8_t *pu8Flag);

static int32_t FILEMNG_GetFileSuffix(const char *pszFileName, char *pszSuffix)
{
    const char *nameSuffix = strrchr(pszFileName, '.');
    if (NULL == nameSuffix) {
        CVI_LOGE("input filename don't have . suffix \n");
        return CVI_FILEMNG_EINVAL;
    }

    if (strnlen(nameSuffix, FILEMNG_EXTENSION_LENGTH + 1) > FILEMNG_EXTENSION_LENGTH ||
        strnlen(nameSuffix, FILEMNG_EXTENSION_LENGTH) < 2) {
        CVI_LOGE("input filename .suffix:%s  length too long, max len:%d \n", nameSuffix, FILEMNG_EXTENSION_LENGTH);
        return CVI_FILEMNG_EINVAL;
    }

    snprintf(pszSuffix, FILEMNG_EXTENSION_LENGTH, nameSuffix + 1);
    return 0;
}

char *FILEMNG_Strerror(int32_t s32ErrorCode)
{
    char *pszStrerror = NULL;

    switch (s32ErrorCode) {
        case CVI_FILEMNG_EINVAL:
            pszStrerror = (char *)"Invalid argument";
            break;
        case CVI_FILEMNG_ENOTINIT:
            pszStrerror = (char *)"Not inited";
            break;
        case CVI_FILEMNG_EINITIALIZED:
            pszStrerror = (char *)"Already Initialized";
            break;
        case CVI_FILEMNG_ELOST:
            pszStrerror = (char *)"No such file";
            break;
        case CVI_FILEMNG_ENOTREADY:
            pszStrerror = (char *)"No such device or address";
            break;
        case CVI_FILEMNG_EACCES:
            pszStrerror = (char *)"Permission denied";
            break;
        case CVI_FILEMNG_EEXIST:
            pszStrerror = (char *)"File exists";
            break;
        case CVI_FILEMNG_EFULL:
            pszStrerror = (char *)"No space left on device";
            break;
        case CVI_FILEMNG_EBUSY:
            pszStrerror = (char *)"Operation now in progress";
            break;
        case CVI_FILEMNG_ENORES:
            pszStrerror = (char *)"Too many files,not enough filename";
            break;
        case CVI_FILEMNG_EINTER:
            pszStrerror = (char *)"Internal error";
            break;
        case CVI_FILEMNG_ENOTMAIN:
            pszStrerror = (char *)"Not Basic File";
            break;
        case 0:
            pszStrerror = (char *)"Success";
            break;
        default:
            pszStrerror = (char *)"Unknown error";
            break;
    }

    return pszStrerror;
}

int32_t FILEMNG_CheckPath(const char *pszMntPath, const char *pszRootDir)
{
    int32_t s32Ret = 0;
    char szRootPath[CVI_APPCOMM_MAX_PATH_LEN];
    s32Ret = cvi_PathIsDirectory(pszMntPath);

    if (1 != s32Ret) {
        return CVI_FILEMNG_ENOTREADY;
    }

    snprintf(szRootPath, CVI_APPCOMM_MAX_PATH_LEN, "%s%s", pszMntPath, pszRootDir);
    s32Ret = cvi_PathIsDirectory(szRootPath);
    if (1 == s32Ret) {
        return 0;
    } else if (-1 == s32Ret) {
        s32Ret = cvi_mkdir(szRootPath, 0777);
        if (0 != s32Ret) {
            return CVI_FILEMNG_EACCES;
        }
    } else {
        return CVI_FILEMNG_EEXIST;
    }

    return 0;
}

bool FILEMNG_IsMP4(const char *pszFilePath)
{
    char szSuffix[FILEMNG_EXTENSION_LENGTH] = { 0 };
    if (0 == FILEMNG_GetFileSuffix(pszFilePath, szSuffix)) {
        if ((0 == strcasecmp(szSuffix, "MP4")) || (0 == strcasecmp(szSuffix, "LRV"))) {
            return true;
        }
    }
    return false;
}

int32_t FILEMNG_GetFileInfo(const char *pszFilePath, CVI_FILEMNG_FILE_INFO_S *pstFileInfo)
{
    struct stat FileStat;
    memcpy(pstFileInfo->szAbsPath, pszFilePath, CVI_APPCOMM_MAX_PATH_LEN);

    if (0 == stat(pszFilePath, &FileStat)) {
        pstFileInfo->u64FileSize_byte = FileStat.st_size;
        struct tm time;
        localtime_r(&FileStat.st_mtime, &time);
        snprintf(pstFileInfo->szCreateTime, CVI_FILEMNG_MAX_DATETIME_LEN, "%04d/%02d/%02d %02d:%02d:%02d",
                 time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                 time.tm_hour, time.tm_min, time.tm_sec);
        pstFileInfo->u32Duration_sec = 0;

        #if 0
        if (FILEMNG_IsMP4(pszFilePath)) {
            HI_MW_PTR Handle;
            HI_MP4_CONFIG_S stMP4Cfg;
            snprintf(stMP4Cfg.aszFileName, HI_MP4_MAX_FILE_NAME, pszFilePath);
            stMP4Cfg.enConfigType = HI_MP4_CONFIG_DEMUXER;
            stMP4Cfg.stDemuxerConfig.u32VBufSize = 1 << 20;

            if (0 == HI_MP4_Create(&Handle, &stMP4Cfg)) {
                HI_U64 u64Duration = 0;
                HI_MP4_Destroy(Handle, &u64Duration);
                pstFileInfo->u32Duration_sec = u64Duration;
            }
        }
        #endif
#ifdef SUPPORT_FILE_MARKER
        FILEMNG_MARKER_GetFlag(pszFilePath, &pstFileInfo->u8Flag);
#endif
    } else {
        pstFileInfo->u64FileSize_byte = 0;
        CVI_LOGE("%s:%s\n", FILEMNG_Strerror(CVI_FILEMNG_ELOST), pszFilePath);
        return CVI_FILEMNG_ELOST;
    }

    return 0;
}

int32_t FILEMNG_HideFile(const char *pFilePath, bool bHide)
{
    int32_t s32Ret = 0;
    int32_t fd = open(pFilePath, O_RDWR);
    if (fd == -1) {
        CVI_LOGE("open %s error:%s\n", pFilePath, strerror(errno));
        return -1;
    } else {
        uint32_t attr;
        s32Ret = ioctl(fd, FAT_IOCTL_GET_ATTRIBUTES, &attr);
        if (s32Ret == -1) {
            CVI_LOGE("ioctl FAT_IOCTL_GET_ATTRIBUTES error:%s\n", strerror(errno));
        } else {
            if (bHide == true) {
                attr |= ATTR_HIDDEN;
            } else {
                attr &= ~ATTR_HIDDEN;
            }
            s32Ret = ioctl(fd, FAT_IOCTL_SET_ATTRIBUTES, &attr);
            if (s32Ret == -1) {
                CVI_LOGE("ioctl FAT_IOCTL_SET_ATTRIBUTES error:%s\n", strerror(errno));
            }
        }
        close(fd);
    }

    return s32Ret;
}

int32_t FILEMNG_HideDir(const char *pDirPath, bool bHide)
{
    int32_t s32Ret = 0;
    DIR *dp = opendir(pDirPath);
    if (dp == NULL) {
        CVI_LOGE("open %s error:%s\n", pDirPath, strerror(errno));
        return -1;
    } else {
        uint32_t attr;
        int32_t fd = dirfd(dp);
        s32Ret = ioctl(fd, FAT_IOCTL_GET_ATTRIBUTES, &attr);
        if (s32Ret == -1) {
            CVI_LOGE("ioctl FAT_IOCTL_GET_ATTRIBUTES error:%s\n", strerror(errno));
        } else {
            if (bHide == true) {
                attr |= ATTR_HIDDEN;
            } else {
                attr &= ~ATTR_HIDDEN;
            }
            s32Ret = ioctl(fd, FAT_IOCTL_SET_ATTRIBUTES, &attr);
            if (s32Ret == -1) {
                CVI_LOGE("ioctl FAT_IOCTL_SET_ATTRIBUTES error:%s\n", strerror(errno));
            }
        }
        closedir(dp);
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

