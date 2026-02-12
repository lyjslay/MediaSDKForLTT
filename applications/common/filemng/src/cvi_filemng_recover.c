#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "cvi_filemng_dtcf.h"
#ifdef COMPONENTS_FILE_RECOVER_ON
#include "cvi_file_recover/cvi_file_recover.h"
#endif
#include "filemng_comm.h"
#ifdef SERVICES_PLAYER_ON
#include "cvi_player_service.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define FILEMNG_REPAIRER_MAX_BACKUP_NUM (4)
static char SdConfigPath[64] = {0};
static bool PreallocFlage = true;

void CVI_FILEMNG_SetSdConfigPath(char* path)
{
    memset(SdConfigPath, 0, sizeof(SdConfigPath));
    memcpy(SdConfigPath, path, strlen(path));
    strcat(SdConfigPath, "cvi_sdconfig");
}

void CVI_FILEMNG_PreallocateState(CVI_FILEMNG_DTCF_CFG_S *pstDTCF_Cfg)
{
    PreallocFlage = pstDTCF_Cfg->preAllocFilesEnable;
}

static int32_t file_recover(char *file_path) {
    int32_t ret = 0;
#if defined(COMPONENTS_FILE_RECOVER_ON) && !defined(FLUSH_MOOV_STREAM_ON)
    CVI_FILE_RECOVER_HANDLE_T handle = NULL;
    #endif

#ifdef SERVICES_PLAYER_ON
    ret = CVI_PLAYER_SERVICE_GetFileMediaInfo(file_path);
    if (ret == 0) {
        CVI_LOGI("CVI_PLAYER_SERVICE_GetFileMediaInfo, file ok, no recover");
        return 1;
    }
#endif

#if defined(COMPONENTS_FILE_RECOVER_ON) && !defined(FLUSH_MOOV_STREAM_ON)
    ret = CVI_FILE_RECOVER_Create(&handle);
    if (ret != 0) {
        CVI_LOGE("CVI_FILE_RECOVER_Create failed");
        goto FUNC_OUT;
    }

    //PreallocFlage = true;
    CVI_FILE_RECOVER_PreallocateState(handle, PreallocFlage);

    ret = CVI_FILE_RECOVER_Open(handle, file_path);
    if (ret != 0) {
        CVI_LOGE("CVI_FILE_RECOVER_Open failed");
        goto FUNC_OUT;
    }

    ret = CVI_FILE_RECOVER_Recover(handle, file_path, "", true);
    if (ret != 0) {
        CVI_LOGE("CVI_FILE_RECOVER_Recover failed");
    }

FUNC_OUT:
    CVI_FILE_RECOVER_Destroy(&handle);
#endif
    return ret;
}

#define MB(x) (x * (2<<19))
static int32_t FILEMNG_Send_Filename2Recover(char *file_path, bool enftruncate, uint32_t size)
{
    struct stat st;
    stat(file_path, &st);
    int32_t ret = 0;
    ret = file_recover(file_path);
    if ((0 != ret)) {
        if (1 == ret) {
            return 0;
        }
        CVI_LOGE("file_path:%s cannot recovery", file_path);
        return -1;
    }

    int32_t fd = open(file_path, O_WRONLY);
    if(fd < 0) {
        CVI_LOGE("file_path:%s not exist", file_path);
        return -1;
    }
    if (enftruncate == true) {
        ftruncate(fd, (st.st_size/MB(size) + 1)*MB(size));
        //CVI_LOGI("Truncate %s Origin: %jd Now: %jd\n", file_path, st.st_size, (st.st_size/MB(size)+1)*MB(size));
    }

    close(fd);

    return 0;
}

/**
cvi_sdconfig
**/
int32_t CVI_FILEMNG_CreateSDConfigFile(void)
{
    int32_t ret;
    int32_t fd = -1;

    fd = open(SdConfigPath, O_RDWR | O_CREAT | O_EXCL | O_TRUNC);
    if (fd == -1) {
        CVI_LOGE("create cvi_sdconfig file [%s] fail:%s\n", SdConfigPath, strerror(errno));
        return CVI_FILEMNG_EINTER;
    }

    ret = FILEMNG_HideFile(SdConfigPath, true);
    if (ret == -1) {
        close(fd);
        CVI_LOGE("create cvi_sdconfig file [%s] fail:%s\n", SdConfigPath, strerror(errno));
        return CVI_FILEMNG_EINTER;
    }

    CVI_LOGD("create %s success \n", SdConfigPath);

    close(fd);
    return 0;
}

bool CVI_FILEMNG_SDConfigFileIsExist(void)
{
    int32_t fd = -1;

    fd = open(SdConfigPath, O_RDONLY | O_SYNC);
    if (fd == -1) {
        CVI_LOGE("cvi_sdconfig file [%s] is not exist:%s\n", SdConfigPath, strerror(errno));
        return false;
    }

    close(fd);

    return true;
}

int32_t CVI_FILEMNG_RecoverAddFileName(const char *filePath)
{
    char szFilePath[FILEMNG_REPAIRER_MAX_BACKUP_NUM][CVI_APPCOMM_MAX_PATH_LEN] = { 0 };
    uint32_t u32BackupCnt = 0;
    int32_t i = 0;
    bool bCover = false;
    FILE *fp = fopen(SdConfigPath, "ab+");
    if (NULL == fp) {
        CVI_LOGE("fopen[%s] Error:%s\n", SdConfigPath, strerror(errno));
        return CVI_FILEMNG_EINTER;
    }
    fseek(fp, 0L, SEEK_SET);
    while (NULL != fgets(szFilePath[u32BackupCnt], CVI_APPCOMM_MAX_PATH_LEN, fp)) {
        u32BackupCnt++;
        if (u32BackupCnt >= FILEMNG_REPAIRER_MAX_BACKUP_NUM) {
            bCover = true;
            fclose(fp);
            fp = fopen(SdConfigPath, "wb");
            if (NULL == fp) {
                CVI_LOGE("fopen[%s] Error:%s\n", SdConfigPath, strerror(errno));
                return CVI_FILEMNG_EINTER;
            }
            break;
        }
    }
    if (bCover) {
        fseek(fp, 0L, SEEK_SET);
        for (i = 1; i < FILEMNG_REPAIRER_MAX_BACKUP_NUM; i++) {
            if (0 > fputs(szFilePath[i], fp)) {
                CVI_LOGE("save LastFileName fail!%s \n", strerror(errno));
                fclose(fp);
                return CVI_FILEMNG_EINTER;
            }
        }
    } else {
        fseek(fp, 0L, SEEK_END);
    }
    if (0 > fputs(filePath, fp)) {
        CVI_LOGE("save LastFileName fail!%s \n", strerror(errno));
        fclose(fp);
        return -1;
    }
    fwrite("\n", 1, 1, fp);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);

    return 0;
}

int32_t CVI_FILEMNG_RecoverRemoveFileName(const char *filePath)
{
    char szFilePath[FILEMNG_REPAIRER_MAX_BACKUP_NUM][CVI_APPCOMM_MAX_PATH_LEN] = { 0 };
    char szUpdateFilePath[FILEMNG_REPAIRER_MAX_BACKUP_NUM][CVI_APPCOMM_MAX_PATH_LEN] = { 0 };
    uint32_t u32BackupCnt = 0;
    bool bUpdate = false;
    int32_t i = 0;
    FILE *fp = fopen(SdConfigPath, "ab+");
    if (NULL == fp) {
        CVI_LOGE("fopen[%s] Error:%s\n", SdConfigPath, strerror(errno));
        return CVI_FILEMNG_EINTER;
    }
    fseek(fp, 0L, SEEK_SET);
    while (NULL != fgets(szFilePath[u32BackupCnt], CVI_APPCOMM_MAX_PATH_LEN, fp)) {
        if (strncmp(szFilePath[u32BackupCnt], filePath, strlen(filePath)) != 0) {
            bUpdate = true;
            snprintf(szUpdateFilePath[i], CVI_APPCOMM_MAX_PATH_LEN, "%s", szFilePath[u32BackupCnt]);
            i++;
        }
        u32BackupCnt++;
        if (u32BackupCnt == FILEMNG_REPAIRER_MAX_BACKUP_NUM) {
            break;
        }
    }
    fclose(fp);
    fp = fopen(SdConfigPath, "wb");
    if (NULL == fp) {
        CVI_LOGE("fopen[%s] Error:%s\n", SdConfigPath, strerror(errno));
        return CVI_FILEMNG_EINTER;
    }
    fseek(fp, 0L, SEEK_SET);
    if (bUpdate == true) {
        for (i = 0; i < FILEMNG_REPAIRER_MAX_BACKUP_NUM; i++) {
            if (0 > fputs(szUpdateFilePath[i], fp)) {
                CVI_LOGE("save LastFileName fail!%s \n", strerror(errno));
                fclose(fp);
                return CVI_FILEMNG_EINTER;
            }
        }
    }

    fflush(fp);
    fsync(fileno(fp));

    fclose(fp);

    return 0;
}

int32_t CVI_FILEMNG_AlignRecordFileSize(bool enftruncate, uint32_t size)
{
    int32_t ret = 0;
    char szFilePath[FILEMNG_REPAIRER_MAX_BACKUP_NUM][CVI_APPCOMM_MAX_PATH_LEN] = { 0 };
    uint32_t u32BackupCnt = 0;
    uint32_t i = 0;
    FILE *fp = fopen(SdConfigPath, "ab+");
    if (NULL == fp) {
        CVI_LOGE("fopen[%s] Error:%s\n", SdConfigPath, strerror(errno));
        return CVI_FILEMNG_EINTER;
    }
    fseek(fp, 0L, SEEK_SET);
    while (NULL != fgets(szFilePath[u32BackupCnt], CVI_APPCOMM_MAX_PATH_LEN, fp)) {
        u32BackupCnt++;
        if (u32BackupCnt == FILEMNG_REPAIRER_MAX_BACKUP_NUM) {
            break;
        }
    }
    fclose(fp);
    for (i = 0; i < u32BackupCnt; i++) {
        szFilePath[i][strlen(szFilePath[i]) - 1] = '\0';
        CVI_LOGI("Recover file name : %s\n", szFilePath[i]);
        if (strlen(szFilePath[i]) == 0) {
           CVI_LOGE("the path is NULL");
           continue;
        }
        char *fileextern = NULL;
        fileextern = (strrchr(szFilePath[i], '.') + 1);
        if (0 == (strcmp(fileextern, "TS")) || 0 == (strcmp(fileextern, "ts"))) {
            CVI_LOGI("file is ts, no file recover");
            return ret;
        }
        ret = FILEMNG_Send_Filename2Recover(szFilePath[i], enftruncate, size);
        if (ret == -1) {
            CVI_LOGE("FILEMNG_Send_Filename2Recover failed !\n");
            continue;
        }
    }
    fp = fopen(SdConfigPath, "wb");
    if (NULL == fp) {
        CVI_LOGE("fopen[%s] Error:%s\n", SdConfigPath, strerror(errno));
        return CVI_FILEMNG_EINTER;
    }
    fseek(fp, 0L, SEEK_SET);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);

    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */