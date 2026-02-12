#include <string.h>
#include "cvi_param.h"
#include "cvi_param_ini2bin.h"
#include "cvi_ini.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

extern CVI_PARAM_ACCESS g_ParamAccess;
/* load common start */
static int32_t PARAM_LoadComm(const char *file, CVI_FILEMNG_COMM_CFG_S *Comm)
{
    char tmp_path[16] = {0};

    CVI_INI_GetString("common", "mnt_path", "", tmp_path, 16, file);
    printf("%s: %s\n", __func__, tmp_path);
    memcpy(Comm->szMntPath, tmp_path, sizeof(tmp_path));

    return 0;
}
/* load common end */


/* load dtcf start */
static int32_t PARAM_LoadDtcf(const char *file, CVI_FILEMNG_DTCF_CFG_S *Dtcf)
{
    // TODO: Sam: fix this part
    char root_path[16] = {0};
    char dir_name[16] = {0};
    long int prealloccnt = 0;
    long int preallocunit = 0;
    long int preallocenable = 0;
    long int preallocreservedmemory = 0;
    long int preallocpercentage[DTCF_DIR_BUTT] = {0};
    long int preallocfileunit[DTCF_DIR_BUTT] = {0};
    long int sharepercent = 0;
    long int warning = 0;
    long int guaranteed = 0;
   int32_t  i = 0;
    char tmpstr[64] = {""};
    CVI_INI_GetString("dtcf", "root_path", "", root_path, 16, file);
    sharepercent = CVI_INI_GetLong("dtcf", "sharepercent", 0, file);
    warning = CVI_INI_GetLong("dtcf", "warning", 0, file);
    guaranteed = CVI_INI_GetLong("dtcf", "guaranteed", 0, file);
    prealloccnt = CVI_INI_GetLong("dtcf", "pre_alloc_cnt", 0, file);
    preallocunit = CVI_INI_GetLong("dtcf", "pre_alloc_unit", 0, file);
    preallocenable = CVI_INI_GetLong("prealloc", "enable", 0, file);
    preallocreservedmemory = CVI_INI_GetLong("prealloc", "reserve_memory", 0, file);
    for (i = 0; i < DTCF_DIR_BUTT; i++){
        memset(tmpstr, 0, sizeof(tmpstr));
        snprintf(tmpstr, 64, "pre_alloc_cnt_dir_%d", i);
        preallocpercentage[i] = CVI_INI_GetLong("prealloc", tmpstr, 0, file);
        printf("%s: %s = %ld \n", __func__, tmpstr, preallocpercentage[i]);

        memset(tmpstr, 0, sizeof(tmpstr));
        snprintf(tmpstr, 64, "pre_alloc_unit_dir_%d", i);
        preallocfileunit[i] = CVI_INI_GetLong("prealloc", tmpstr, 0, file);
        printf("%s, %s = %ld \n", __func__, tmpstr, preallocfileunit[i]);
        Dtcf->u32PreAllocPercentage[i] = preallocpercentage[i];
        Dtcf->u32PreAllocFileUnit[i] = preallocfileunit[i] * 1024 * 1024;
    }
    for (i = 0; i < DTCF_DIR_BUTT; i++){
        char buf[64] = {0};
        dir_name[0] = '\0';
        snprintf(buf, sizeof(buf), "dir_name%d", i);
        CVI_INI_GetString("dir_name", buf, "", dir_name, 16, file);
        if(dir_name[0] != '\0'){
            snprintf(Dtcf->aszDirNames[i], CVI_DIR_LEN_MAX, "%s",dir_name);
            printf("%s ", Dtcf->aszDirNames[i]);
        }
        printf("\n");
    }

    printf("%s: %s %ld %ld %ld \n", __func__, root_path, sharepercent, warning, guaranteed);

    memcpy(Dtcf->szRootDir, root_path, 16);
    for (i = 0; i < CVI_FILEMNG_DTCF_MAX_PHOTO_DIR; i++) {
        Dtcf->u32PreAllocCnt[i] = prealloccnt;
        Dtcf->u32PreAllocUnit[i] = preallocunit;
    }
    Dtcf->preAllocFilesEnable = preallocenable;
    Dtcf->u32PreallocReservedMemory = preallocreservedmemory;
    Dtcf->u8SharePercent = sharepercent;
    Dtcf->u32WarningStage = warning;
    Dtcf->u32GuaranteedStage = guaranteed;
    return 0;
}
/* load dtcf end */

static int32_t PARAM_LoadFilemng(const char *file, CVI_PARAM_FILEMNG_S *FileMng)
{
   int32_t  s32Ret = 0;
    s32Ret = PARAM_LoadComm(file, &FileMng->FileMngComm);
    s32Ret = PARAM_LoadDtcf(file, &FileMng->FileMngDtcf);
    return s32Ret;
}

int32_t  CVI_INI_PARAM_LoadFilemngCfg(CVI_PARAM_FILEMNG_S *FileMng)
{
    printf("\n---enter: %s\n", __func__);

    uint32_t i = 0;
    char filepath[CVI_PARAM_MODULE_NAME_LEN] = {0};

    for (i = 0; i < g_ParamAccess.module_num; i++) {
        if (strstr(g_ParamAccess.modules[i].name, "config_filemng")) {
            memset(filepath, 0, sizeof(filepath));
            snprintf(filepath, sizeof(filepath), "%s%s",
                g_ParamAccess.modules[i].path, g_ParamAccess.modules[i].name);
            // find a media comm file
            PARAM_LoadFilemng(filepath, FileMng);
            break;
        }
    }
    return 0;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
