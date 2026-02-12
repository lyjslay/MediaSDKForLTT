#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <inttypes.h>
#include <sys/mman.h>

#include "cvi_mapi.h"
#include "cvi_param.h"
#include "cvi_appcomm.h"
#include "cvi_flash.h"
#include "cvi_board_memmap.h"
#include "cvi_mode.h"

#pragma pack(push)
#pragma pack(8)

#define CVI_PARAM_CUR_BIN_PATH  "/mnt/system/param/app_cfg.bin"
#define CVI_PARAM_DEF_BIN_PATH  "/mnt/system/param/app_cfg_def.bin"
#define PARAM_PARTITION_ADDR    (0x780000)
#define PARAM_DEF_PARTI_ADDR    (0x800000)
#define PARAM_PARTITION_SIZE    (0x80000)  // 512*1024 bit
#define PARAM_PARTITION_NAME    "/dev/mtd3"
#define PARAM_DEF_PARTI_NAME    "/dev/mtd4"
#define PARAM_DEF_BIN_NAME      "./app_cfg.bin"
#define APP_CFG_PART_NUM    3
#define DEF_APP_CFG_PART_NUM    4

static CVI_PARAM_CONTEXT_S g_stParamCtx = {
    .bInit = 0,
    .bChanged = 0,
    .mutexLock = PTHREAD_MUTEX_INITIALIZER,
    .pstCfg = NULL
};

typedef struct _CVI_PARAM_SAVETSK_CTX_S {
    bool bRun;
    pthread_t tskId;
    pthread_mutex_t mutexLock;
    bool bSaveFlg;
} CVI_PARAM_SAVETSK_CTX_S;

static CVI_PARAM_SAVETSK_CTX_S s_stPARAMSaveTskCtx = {
    .bRun = false,
    .tskId = 0,
    .mutexLock = PTHREAD_MUTEX_INITIALIZER,
    .bSaveFlg = false
};

static uint32_t CVI_PARAM_Crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    static const uint32_t crcTable[256] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
        0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
        0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
        0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
        0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
        0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
        0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
        0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
        0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
        0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
        0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
        0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
        0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
        0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
        0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
        0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
        0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
        0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
        0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
        0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
        0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
        0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
        0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
        0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
        0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
        0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
        0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
        0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
        0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
    };

    for (size_t i = 0; i < length; i++) {
        crc = (crc >> 8) ^ crcTable[(crc ^ data[i]) & 0xFF];
    }

    return crc ^ 0xFFFFFFFF;
}

static void *CVI_PARAM_SaveTsk(void *pParam)
{
    prctl(PR_SET_NAME, (unsigned long)"CviParamSave", 0, 0, 0);
    CVI_PARAM_SAVETSK_CTX_S *pstTskCtx = (CVI_PARAM_SAVETSK_CTX_S *)pParam;
    while (pstTskCtx->bRun) {
        usleep(3 * 1000 * 1000);
        if (true == pstTskCtx->bSaveFlg) {
            pstTskCtx->bSaveFlg = false;
            pthread_mutex_lock(&pstTskCtx->mutexLock);
            CVI_PARAM_SaveParam();
            pthread_mutex_unlock(&pstTskCtx->mutexLock);
        }
    }
    pthread_mutex_lock(&pstTskCtx->mutexLock);
    CVI_PARAM_SaveParam();
    pthread_mutex_unlock(&pstTskCtx->mutexLock);
    return NULL;
}

/* add for ini param */
int32_t  CVI_PARAM_Init(void)
{
    if (g_stParamCtx.bInit) {
        CVI_LOGW("has already inited\n");
        return 0;
    }
   int32_t  s32Ret = 0;
#ifdef CVI_PARAM_RAW_ON
    if (CVIMMAP_SHARE_PARAM_SIZE == 0) {
        CVI_U64 cfg_addr_s, cfg_addr_e;
        s32Ret = CVI_Flash_GetStartAddr(APP_CFG_PART_NUM, &cfg_addr_s, &cfg_addr_e);
        if(s32Ret != 0) {
            printf("get cfg flash part addr failed\n");
            exit(-1);
        }
        printf("cfg_s = 0x%llx, cfg_e = 0x%llx\n", cfg_addr_s, cfg_addr_e);
        s32Ret = CVI_PARAM_LoadFromFlash(PARAM_PARTITION_NAME, cfg_addr_s, cfg_addr_e - cfg_addr_s + 1);
        if(s32Ret != 0) {
            printf("load failed\n");
            CVI_U64 def_cfg_addr_s, def_cfg_addr_e;
            s32Ret = CVI_Flash_GetStartAddr(DEF_APP_CFG_PART_NUM, &def_cfg_addr_s, &def_cfg_addr_e);
            if(s32Ret != 0) {
                printf("get cfg flash part addr failed\n");
                exit(-1);
            }
            printf("cfg_s = 0x%llx, cfg_e = 0x%llx\n", cfg_addr_s, cfg_addr_e);
            s32Ret = CVI_PARAM_LoadFromFlash(PARAM_DEF_PARTI_NAME, def_cfg_addr_s, def_cfg_addr_e - def_cfg_addr_s + 1);
            if(s32Ret != 0) {
                printf("load failed\n");
            }
        }
    } else {
       int32_t  fd = -1;

        fd = open("/dev/mem", O_RDWR | O_NDELAY);
        if (fd < 0) {
            CVI_LOGE("open /dev/mem/ failed !\n");
            return -1;
        }
        g_stParamCtx.pstCfg = (CVI_PARAM_CFG_S *)mmap(NULL, CVIMMAP_SHARE_PARAM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CVIMMAP_SHARE_PARAM_ADDR);
        CVI_LOGI("CVI_PARAM_SaveParam lcrc32 %u", g_stParamCtx.pstCfg->crc32);
        uint32_t crc32 = CVI_PARAM_Crc32((const uint8_t *)g_stParamCtx.pstCfg, sizeof(CVI_PARAM_CFG_S) - sizeof(uint32_t));
        CVI_LOGI("CVI_PARAM_SaveParam ccrc32 %u", crc32);
        if ((g_stParamCtx.pstCfg->MagicStart != CVI_PARAM_MAGIC_START)
                || (g_stParamCtx.pstCfg->MagicEnd != CVI_PARAM_MAGIC_END) || crc32 != g_stParamCtx.pstCfg->crc32) {
            CVI_LOGE("error, magic error\n");
            munmap((void *)g_stParamCtx.pstCfg, CVIMMAP_SHARE_PARAM_SIZE);
            CVI_LOGE("load back param\n");
            g_stParamCtx.pstCfg = (CVI_PARAM_CFG_S *)mmap(NULL, CVIMMAP_SHARE_PARAM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CVIMMAP_SHARE_PARAM_ADDR_BAK);
            if ((g_stParamCtx.pstCfg->MagicStart != CVI_PARAM_MAGIC_START)
                || (g_stParamCtx.pstCfg->MagicEnd != CVI_PARAM_MAGIC_END)){
                CVI_LOGE("error, back param magic error\n");
            } else {
                CVI_PARAM_SetSaveFlg();
                CVI_LOGI("read  back param ok\n");
                CVI_LOGI("power on mode: %d\n", g_stParamCtx.pstCfg->MediaComm.PowerOnMode);
            }
        } else {
            CVI_LOGI("read param ok\n");
            CVI_LOGI("power on mode: %d\n", g_stParamCtx.pstCfg->MediaComm.PowerOnMode);
        }
        close(fd);
    }
#else
    // open bin file
    pthread_mutex_lock(&g_stParamCtx.mutexLock);
    s32Ret = CVI_PARAM_LoadFromBin(CVI_PARAM_CUR_BIN_PATH);
    pthread_mutex_unlock(&g_stParamCtx.mutexLock);

    if(s32Ret != 0) {
        printf("load failed\n");
        return s32Ret;
    }
    printf("magic start: %x, %x\n", g_stParamCtx.pstCfg->MagicStart, CVI_PARAM_MAGIC_START);
    printf("magic end: %x, %x\n", g_stParamCtx.pstCfg->MagicEnd, CVI_PARAM_MAGIC_END);
    CVI_LOGI("CVI_PARAM_SaveParam lcrc32 %u", g_stParamCtx.pstCfg->crc32);
    uint32_t crc32 = CVI_PARAM_Crc32((const uint8_t *)g_stParamCtx.pstCfg, sizeof(CVI_PARAM_CFG_S) - sizeof(uint32_t));
    CVI_LOGI("CVI_PARAM_SaveParam ccrc32 %u", crc32);

    if ((g_stParamCtx.pstCfg->MagicStart != CVI_PARAM_MAGIC_START)
            || (g_stParamCtx.pstCfg->MagicEnd != CVI_PARAM_MAGIC_END) || crc32 != g_stParamCtx.pstCfg->crc32) {
        printf("error, magic error\n");
        pthread_mutex_lock(&g_stParamCtx.mutexLock);
        //CVI_PARAM_LoadFromBin(CVI_PARAM_DEF_BIN_PATH);
        //CVI_PARAM_Save2Bin();
        pthread_mutex_unlock(&g_stParamCtx.mutexLock);
    }

#endif

    s_stPARAMSaveTskCtx.bRun = true;
    s32Ret = pthread_create(&s_stPARAMSaveTskCtx.tskId, NULL, CVI_PARAM_SaveTsk, &s_stPARAMSaveTskCtx);
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, -1, "CreateSaveTsk");
    // CVI_LOGD("Param SaveTsk(%u) create successful\n", s_stPARAMSaveTskCtx.tskId);
    g_stParamCtx.bInit = 1;
    return 0;
}

int32_t  CVI_PARAM_Deinit(void)
{
    /* Destroy SaveParam Task */
    s_stPARAMSaveTskCtx.bRun = false;
    pthread_join(s_stPARAMSaveTskCtx.tskId, NULL);
    s_stPARAMSaveTskCtx.tskId = 0;
    s_stPARAMSaveTskCtx.bSaveFlg = false;
    g_stParamCtx.bInit = 0;
    g_stParamCtx.bChanged = 0;

    if(g_stParamCtx.pstCfg != NULL && CVIMMAP_SHARE_PARAM_SIZE == 0) {
        free(g_stParamCtx.pstCfg);
        g_stParamCtx.pstCfg = NULL;
    }
    return 0;
}

CVI_PARAM_CONTEXT_S *CVI_PARAM_GetCtx(void)
{
    return &g_stParamCtx;
}

int32_t  CVI_PARAM_GetParam(CVI_PARAM_CFG_S *param)
{
    if (!g_stParamCtx.bInit) {
        CVI_LOGE(" Param not init\n");
        return -1;
    }
    if (param) {
        pthread_mutex_lock(&g_stParamCtx.mutexLock);
        memcpy(param, g_stParamCtx.pstCfg, sizeof(CVI_PARAM_CFG_S));
        pthread_mutex_unlock(&g_stParamCtx.mutexLock);
    }
    return 0;
}

int32_t  CVI_PARAM_Save2Bin(void)
{
    if (!g_stParamCtx.bChanged) {
        printf("%s: param not changed\n", __func__);
        return 0;
    }

    if (g_stParamCtx.pstCfg == NULL) {
        printf("[Error] global param is NULL\n");
        return 0;
    }

   int32_t  writeCnt = 0;
    FILE *fp = NULL;
    fp = fopen(CVI_PARAM_CUR_BIN_PATH, "w+b");
    if (fp == NULL) {
        printf("[Error] %s: fopen failed\n", CVI_PARAM_CUR_BIN_PATH);
        return -1;
    }

    writeCnt = fwrite(&g_stParamCtx.pstCfg, sizeof(CVI_PARAM_CFG_S), 1, fp);
    if (writeCnt != 1) {
        printf("[Error] write %d\n", writeCnt);
    }
    fflush(fp);
    fclose(fp);
    fp = NULL;
    return 0;
}

static int32_t CVI_PARAM_Save2Flash(const char *partition, uint64_t addr, uint64_t len)
{
   int32_t  s32Ret = 0;
    CVI_U64 u64Address = addr;
	CVI_U64 u64Len = len;
    uint8_t *pBuf = NULL;
    CVI_CHAR *pPartitionName = NULL;
#ifdef STORAGE_TYPE_NOR
    CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_SPI_0;
#elif STORAGE_TYPE_NAND
    CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_NAND_0;
#else
	CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_EMMC_0;
#endif
    // TODO: Change the way of validcheck
    if (addr == 0) {
        printf("[Error] input flash addr is NULL\n");
        s32Ret = -1;
        goto EXIT;
    }

    if (g_stParamCtx.pstCfg == NULL) {
        printf("[Error] global param is NULL\n");
        goto EXIT;
    }
    // end of todo
    pBuf = (uint8_t *)malloc(u64Len);
    memcpy(pBuf, g_stParamCtx.pstCfg, sizeof(CVI_PARAM_CFG_S));

    // flash open
	s32Ret = CVI_Flash_Open(enFlashType, pPartitionName, u64Address, u64Len);
	if (s32Ret == CVI_FAILURE) {
		printf("Open flash fail\n");
        goto EXIT;
	} else {
		printf("Open flash success\n");
	}

    //erase
	s32Ret = CVI_Flash_Erase(enFlashType, u64Address, u64Len);
	if (s32Ret == CVI_FAILURE) {
		printf("Erase flash fail\n");
        goto CLOSE;
    } else {
		printf("Erase flash success\n");
    }

    //write buffer
	s32Ret = CVI_Flash_Write(enFlashType, 0, (uint8_t *)pBuf, (CVI_U32)u64Len, CVI_FLASH_RW_FLAG_RAW);
	if (s32Ret == CVI_FAILURE) {
		printf("write flash fail\n");
        goto CLOSE;
    } else {
		printf("write flash success\n");
    }

CLOSE:
    // flash close
	s32Ret = CVI_Flash_Close(enFlashType);
	if (s32Ret == CVI_FAILURE) {
		printf("close flash fail\n");
    } else {
		printf("close flash success\n");
    }
EXIT:
    if(pBuf){
        free(pBuf);
    }
    return s32Ret;
}

int32_t  CVI_PARAM_LoadFromBin(const char *path)
{
   int32_t  fd = 0;
   int32_t  readCount = 0;
    // TODO: change the way of validcheck
    if (path == NULL) {
        printf("[Error] bin file path is NULL\n");
        return -1;
    }
    // endof todo

    if(g_stParamCtx.pstCfg != NULL) {
        return -1;
    }

    g_stParamCtx.pstCfg = (CVI_PARAM_CFG_S*)malloc(sizeof(CVI_PARAM_CFG_S));

    fd = open(path, O_RDWR, 777);
    if (fd < 0) {
        printf("load [%s] failed\n", path);
        return -1;
    }

    readCount = read(fd, g_stParamCtx.pstCfg, sizeof(CVI_PARAM_CFG_S));
    if (0 > readCount) {
        printf("read %s failed\n", path);
        free(g_stParamCtx.pstCfg);
        close(fd);
        return -1;
    }
    close(fd);
    printf("[INFO] total: %zd, read: %d\n", sizeof(CVI_PARAM_CFG_S), readCount);
    return 0;
}


int32_t  CVI_PARAM_LoadFromFlash(char *partition, uint64_t addr, uint64_t len)
{
   int32_t  s32Ret = 0;
    //int32_t  readCount = 0;
    uint64_t u64Addr = addr;
    uint64_t u64Len = len;
#ifdef STORAGE_TYPE_NOR
    CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_SPI_0;
#elif STORAGE_TYPE_NAND
    CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_NAND_0;
#else
	CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_EMMC_0;
#endif
    uint8_t *pBuf = (uint8_t *)malloc(len);

    if (addr == 0) {
        printf("[Error] input flash addr is NULL\n");
        s32Ret = -1;
        goto EXIT;
    }

    // flash open
    s32Ret = CVI_Flash_Open(enFlashType, partition, u64Addr, u64Len);
    if (s32Ret == -1) {
		printf("Open flash fail\n");
        goto EXIT;
    } else {
		printf("Open flash success\n");
    }

    // read flash
    s32Ret = CVI_Flash_Read(enFlashType, u64Addr, pBuf, u64Len, CVI_FLASH_RW_FLAG_RAW);
	if (s32Ret == -1) {
		printf("read flash fail\n");
        goto EXIT;
    } else {
		printf("read flash success\n");
    }

    g_stParamCtx.pstCfg = (CVI_PARAM_CFG_S*)malloc(sizeof(CVI_PARAM_CFG_S));
    memcpy(g_stParamCtx.pstCfg, pBuf, sizeof(CVI_PARAM_CFG_S));

    if ((g_stParamCtx.pstCfg->MagicStart != CVI_PARAM_MAGIC_START)
            || (g_stParamCtx.pstCfg->MagicEnd != CVI_PARAM_MAGIC_END)) {
        printf("error, magic error\n");
        s32Ret = -1;
        free(g_stParamCtx.pstCfg);
        CVI_Flash_Close(enFlashType);
        goto EXIT;
    } else {
        printf("read param ok\n");
        printf("power on mode: %d\n", g_stParamCtx.pstCfg->MediaComm.PowerOnMode);
    }
    // printf("head: %x end: %x, length: %d\n",
    //     g_stParamCtx.pstCfg->MagicStart, g_stParamCtx.pstCfg->MagicEnd,
    //     g_stParamCtx.pstCfg->Head.ParamLen);

    //file close
	s32Ret = CVI_Flash_Close(enFlashType);
	if (s32Ret == CVI_FAILURE) {
		printf("close flash fail\n");
        // goto EXIT;
    } else {
		printf("close flash success\n");
    }

EXIT:
    if(pBuf){
        free(pBuf);
    }
    return s32Ret;
}

int32_t  CVI_PARAM_SaveParam(void)
{
   int32_t  s32Ret = 0;
    static CVI_PARAM_CFG_S s_stParam;
    CVI_U64 def_cfg_addr_s, def_cfg_addr_e;
    if (memcmp(&s_stParam, g_stParamCtx.pstCfg, sizeof(CVI_PARAM_CFG_S)) == 0) {
        return 0;
    } else {
        memcpy(&s_stParam, g_stParamCtx.pstCfg, sizeof(CVI_PARAM_CFG_S));
    }
    CVI_LOGI("CVI_PARAM_SaveParam lcrc32 %u", s_stParam.crc32);
    uint32_t crc32 = CVI_PARAM_Crc32((const uint8_t *)&s_stParam, sizeof(CVI_PARAM_CFG_S) - sizeof(uint32_t));
    s_stParam.crc32 = crc32;
    g_stParamCtx.pstCfg->crc32 = crc32;
    CVI_LOGI("CVI_PARAM_SaveParam ccrc32 %u", crc32);

    s32Ret = CVI_Flash_GetStartAddr(APP_CFG_PART_NUM, &def_cfg_addr_s, &def_cfg_addr_e);
    if(s32Ret != 0) {
        printf("get cfg flash part addr failed\n");
        exit(-1);
    }
    s32Ret = CVI_PARAM_Save2Flash(PARAM_PARTITION_NAME, def_cfg_addr_s, def_cfg_addr_e - def_cfg_addr_s + 1);
    if (s32Ret != 0) {
        CVI_LOGE("CVI_PARAM_Save2Flash faile \n");
        return s32Ret;
    }

    return s32Ret;

}

void CVI_PARAM_SetSaveFlg(void)
{
    s_stPARAMSaveTskCtx.bSaveFlg = true;
}

int32_t  CVI_PARAM_GetCamStatus(uint32_t CamId, bool *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
    uint32_t i = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {
            memcpy(Param, &pstParamCtx->pstCfg->CamCfg[i].CamEnable, sizeof(bool));
            break;
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetMediaMode(uint32_t CamId, CVI_PARAM_MEDIA_SPEC_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
    uint32_t i = 0, j = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {

            for(j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < CVI_PARAM_MEDIA_CNT)); j++){
                if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode == pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].MediaMode) {
                    memcpy(Param, &pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j], sizeof(CVI_PARAM_MEDIA_SPEC_S));
                    break;
                }
            }
            break;
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t CVI_PARAM_GetAhdDefaultMode(uint32_t CamId, int32_t *mode)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
    uint32_t i = 0, value = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {
            value = pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode;
            break;
        }
    }

    switch (value) {
        case CVI_MEDIA_VIDEO_SIZE_1280X720P25:
            *mode = AHD_MODE_1280X720P25;
            break;
        case CVI_MEDIA_VIDEO_SIZE_1280X720P30:
           *mode = AHD_MODE_1280X720P30;
            break;
        case CVI_MEDIA_VIDEO_SIZE_1920X1080P25:
           *mode = AHD_MODE_1920X1080P25;
            break;
        case CVI_MEDIA_VIDEO_SIZE_1920X1080P30:
           *mode = AHD_MODE_1920X1080P30;
            break;
        default:
            CVI_LOGE("value %d is invalid\n", value);
            break;
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetMediaComm(CVI_PARAM_MEDIA_COMM_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    memcpy(Param, &pstParamCtx->pstCfg->MediaComm, sizeof(CVI_PARAM_MEDIA_COMM_S));
    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    return 0;
}

int32_t  CVI_PARAM_SetMediaComm(CVI_PARAM_MEDIA_COMM_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    memcpy(&pstParamCtx->pstCfg->MediaComm, Param, sizeof(CVI_PARAM_MEDIA_COMM_S));
    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    return 0;
}

int32_t  CVI_PARAM_GetVbParam(CVI_MAPI_MEDIA_SYS_ATTR_T *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
    uint32_t i = 0, j = 0, z = 0, t = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(&Param->stVIVPSSMode, &pstParamCtx->pstCfg->MediaComm.Vpss.stVIVPSSMode, sizeof(VI_VPSS_MODE_S));
    memcpy(&Param->stVPSSMode, &pstParamCtx->pstCfg->MediaComm.Vpss.stVPSSMode, sizeof(VPSS_MODE_S));
    CVI_PARAM_MEDIA_VB_ATTR_S vb[MAX_CAMERA_INSTANCES] = {0};
    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for (j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < CVI_PARAM_MEDIA_CNT)); j++) {
            if (pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode == pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].MediaMode) {
                memcpy(&vb[i], &pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Vb, sizeof(CVI_PARAM_MEDIA_VB_ATTR_S));
                if (i == 0) {
                    Param->vb_pool_num = vb[i].Poolcnt;
                    for (z = 0; z < Param->vb_pool_num; z++) {
                        memcpy(&Param->vb_pool[z], &vb[i].Vbpool[z], sizeof(CVI_MAPI_MEDIA_SYS_VB_POOL_T));
                    }
                } else {
                    CVI_MAPI_MEDIA_SYS_ATTR_T tmp;
                    memcpy(&tmp, Param, sizeof(CVI_MAPI_MEDIA_SYS_ATTR_T));
                    for (z = 0; z < vb[i].Poolcnt; z++) {
                        for (t = 0; t < tmp.vb_pool_num; t++) {
                            if (vb[i].Vbpool[z].is_frame && tmp.vb_pool[z].is_frame) {
                                if (vb[i].Vbpool[z].vb_blk_size.frame.width == tmp.vb_pool[t].vb_blk_size.frame.width &&\
                                    vb[i].Vbpool[z].vb_blk_size.frame.height == tmp.vb_pool[t].vb_blk_size.frame.height &&\
                                    vb[i].Vbpool[z].vb_blk_size.frame.fmt == tmp.vb_pool[t].vb_blk_size.frame.fmt) {
                                        Param->vb_pool[t].vb_blk_num += vb[i].Vbpool[z].vb_blk_num;
                                        break;
                                }
                            } else {
                                if (vb[i].Vbpool[t].vb_blk_size.size == tmp.vb_pool[z].vb_blk_size.size) {
                                        Param->vb_pool[z].vb_blk_num += vb[i].Vbpool[t].vb_blk_num;
                                        break;
                                }
                            }
                        }
                        if (t == tmp.vb_pool_num) {
                            memcpy(&Param->vb_pool[Param->vb_pool_num], &vb[i].Vbpool[z], sizeof(CVI_MAPI_MEDIA_SYS_VB_POOL_T));
                            Param->vb_pool_num += 1;
                        }
                    }
                }
            }
        }
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetWorkModeParam(CVI_PARAM_WORK_MODE_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->WorkModeCfg, sizeof(CVI_PARAM_WORK_MODE_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetVpssModeParam(CVI_PARAM_VPSS_ATTR_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Vpss, sizeof(CVI_PARAM_VPSS_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetKeyMngCfg(CVI_KEYMNG_CFG_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    memcpy(Param, &pstParamCtx->pstCfg->DevMng.stkeyMngCfg, sizeof(CVI_KEYMNG_CFG_S));
    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    return 0;
}

int32_t  CVI_PARAM_GetGaugeMngCfg(CVI_GAUGEMNG_CFG_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    memcpy(Param, &pstParamCtx->pstCfg->DevMng.GaugeCfg, sizeof(CVI_GAUGEMNG_CFG_S));
    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    return 0;
}

int32_t  CVI_PARAM_GetVoParam(CVI_PARAM_DISP_ATTR_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Vo, sizeof(CVI_PARAM_DISP_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetWndParam(CVI_PARAM_WND_ATTR_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Window, sizeof(CVI_PARAM_WND_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetAiParam(CVI_MAPI_ACAP_ATTR_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Ai, sizeof(CVI_MAPI_ACAP_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetAencParam(CVI_MAPI_AENC_ATTR_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Aenc, sizeof(CVI_MAPI_AENC_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetAoParam(CVI_MAPI_AO_ATTR_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Ao, sizeof(CVI_MAPI_AO_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetOsdParam(CVI_PARAM_MEDIA_OSD_ATTR_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
   int32_t  OsdCnt = 0;
    for (int32_t  i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for (uint32_t j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < CVI_PARAM_MEDIA_CNT)); j++) {
            if (pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode == pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].MediaMode) {
                for (int32_t  k = 0; k < pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdCnt; k++) {
                    Param->OsdAttrs[OsdCnt++] = pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k];
                }
            }
        }
    }
    Param->OsdCnt = OsdCnt;

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_SetOsdParam(CVI_PARAM_MEDIA_OSD_ATTR_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

   int32_t  OsdCnt = 0;
    for (int32_t  i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for (uint32_t j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < CVI_PARAM_MEDIA_CNT)); j++) {
            if (pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode == pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].MediaMode) {
                for (int32_t  k = 0; k < pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdCnt; k++) {
                    pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k] = Param->OsdAttrs[OsdCnt++];
                }
            }
        }
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    CVI_PARAM_SetSaveFlg();
    return 0;
}

int32_t  CVI_PARAM_GetStgInfoParam(STG_DEVINFO_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->DevMng.Stg, sizeof(STG_DEVINFO_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetFileMngParam(CVI_PARAM_FILEMNG_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->FileMng, sizeof(CVI_PARAM_FILEMNG_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetUsbParam(CVI_PARAM_USB_MODE_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->WorkModeCfg.UsbMode, sizeof(CVI_PARAM_USB_MODE_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

#ifdef SERVICES_SPEECH_ON
int32_t  CVI_PARAM_GetSpeechParam(CVI_SPEECHMNG_PARAM_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }
    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Speech, sizeof(CVI_SPEECHMNG_PARAM_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_SetSpeechParam(CVI_SPEECHMNG_PARAM_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(&pstParamCtx->pstCfg->MediaComm.Speech, Param, sizeof(CVI_SPEECHMNG_PARAM_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    CVI_PARAM_SetSaveFlg();

    return 0;
}
#endif

int32_t  CVI_PARAM_GetMediaPhotoSize(CVI_PARAM_MEDIA_COMM_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    memcpy(Param, &pstParamCtx->pstCfg->MediaComm, sizeof(CVI_PARAM_MEDIA_COMM_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetMenuParam(CVI_PARAM_MENU_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->Menu, sizeof(CVI_PARAM_MENU_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetDevParam(CVI_PARAM_DEVMNG_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->DevMng, sizeof(CVI_PARAM_DEVMNG_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetWifiParam(CVI_PARAM_WIFI_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (true != pstParamCtx->bInit) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->DevMng.Wifi, sizeof(CVI_PARAM_WIFI_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_SetWifiParam(CVI_PARAM_WIFI_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(&pstParamCtx->pstCfg->DevMng.Wifi, Param, sizeof(CVI_PARAM_WIFI_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    CVI_PARAM_SetSaveFlg();

    return 0;
}

int32_t  CVI_PARAM_GetPWMParam(CVI_PARAM_PWM_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (true != pstParamCtx->bInit) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->DevMng.PWM, sizeof(CVI_PARAM_PWM_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_SetPWMParam(CVI_PARAM_PWM_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(&pstParamCtx->pstCfg->DevMng.PWM, Param, sizeof(CVI_PARAM_PWM_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    CVI_PARAM_SetSaveFlg();

    return 0;
}


int32_t  CVI_PARAM_GetGsensorParam(CVI_GSENSORMNG_CFG_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }
    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->DevMng.Gsensor, sizeof(CVI_GSENSORMNG_CFG_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}
int32_t  CVI_PARAM_SetGsensorParam(CVI_GSENSORMNG_CFG_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(&pstParamCtx->pstCfg->DevMng.Gsensor, Param, sizeof(CVI_GSENSORMNG_CFG_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetMenuScreenDormantParam(int32_t  *Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    switch (pstParamCtx->pstCfg->Menu.ScreenDormant.Current)
    {
    case CVI_MENU_SCREENDORMANT_OFF:
        *Value = 0;
        break;
    case CVI_MENU_SCREENDORMANT_1MIN:
        *Value = 60;
        break;
    case CVI_MENU_SCREENDORMANT_3MIN:
        *Value = 180;
        break;
    case CVI_MENU_SCREENDORMANT_5MIN:
        *Value = 300;
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}
int32_t  CVI_PARAM_GetKeyTone(int32_t  *Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    switch (pstParamCtx->pstCfg->Menu.KeyTone.Current)
    {
    case CVI_MEDIA_AUDIO_KEYTONE_OFF:
        *Value = 0;
        break;
    case CVI_MEDIA_AUDIO_KEYTONE_ON:
        *Value = 1;
        break;
    case CVI_MEDIA_AUDIO_KEYTONE_BUIT:
        *Value = 2;
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetFatigueDrive(int32_t  *Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    switch (pstParamCtx->pstCfg->Menu.FatigueDirve.Current)
    {
    case CVI_MENU_FATIGUEDRIVE_OFF:
        *Value = 0;
        break;
    case CVI_MENU_FATIGUEDRIVE_1HOUR:
        *Value = 1;
        break;
    case CVI_MENU_FATIGUEDRIVE_2HOUR:
        *Value = 2;
        break;
    case CVI_MENU_FATIGUEDRIVE_3HOUR:
        *Value = 3;
        break;
    case CVI_MENU_FATIGUEDRIVE_4HOUR:
        *Value = 4;
        break;
    case CVI_MENU_FATIGUEDRIVE_BUIT:
        *Value = 5;
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetMenuSpeedStampParam(int32_t  *Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    switch (pstParamCtx->pstCfg->Menu.SpeedStamp.Current)
    {
    case CVI_MENU_SPEEDSTAMP_OFF:
        *Value = 0;
        break;
    case CVI_MENU_SPEEDSTAMP_ON:
        *Value = 1;
        break;
    case CVI_MENU_SPEEDSTAMP_BUIT:
        *Value = 2;
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetMenuGPSStampParam(int32_t  *Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    switch (pstParamCtx->pstCfg->Menu.GPSStamp.Current)
    {
    case CVI_MENU_GPSSTAMP_OFF:
        *Value = 0;
        break;
    case CVI_MENU_GPSSTAMP_ON:
        *Value = 1;
        break;
    case CVI_MENU_GPSSTAMP_BUIT:
        *Value = 2;
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_SetParam(CVI_PARAM_CFG_S  *param)
{
    if (!g_stParamCtx.bInit) {
        CVI_LOGE(" Param not init\n");
        return -1;
    }

    pthread_mutex_lock(&g_stParamCtx.mutexLock);
    memcpy(g_stParamCtx.pstCfg, param, sizeof(CVI_PARAM_CFG_S));
    pthread_mutex_unlock(&g_stParamCtx.mutexLock);

    return 0;
}


void CVI_PARAM_GetOsdCarNameParam(char *string_carnum_stamp, CVI_MENU_LANGUAGE_E* lang)
{
    CVI_PARAM_MEDIA_OSD_ATTR_S Osd;
    CVI_PARAM_GetOsdParam(&Osd);
    CVI_PARAM_MENU_S param;
    CVI_PARAM_GetMenuParam(&param);

    pthread_mutex_lock(&g_stParamCtx.mutexLock);
    for(int32_t  i = 0; i < Osd.OsdCnt; i++){
        if(Osd.OsdAttrs[i].stContent.enType == CVI_MAPI_OSD_TYPE_STRING){
           int32_t  length = strlen(Osd.OsdAttrs[i].stContent.stStrContent.szStr);
            strncpy(string_carnum_stamp,Osd.OsdAttrs[i].stContent.stStrContent.szStr,length);
            *lang = param.Language.Current;
            break;
        }
    }
    pthread_mutex_unlock(&g_stParamCtx.mutexLock);
}

void CVI_PARAM_SetOsdCarNameParam(char *string_carnum_stamp)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
   int32_t  str_size;

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    string_carnum_stamp[strlen(string_carnum_stamp)] = '\0';
    for (int32_t  i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for (uint32_t j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < CVI_PARAM_MEDIA_CNT)); j++) {
            for (int32_t  k = 0; k < pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdCnt; k++) {
                if(pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].stContent.enType == CVI_MAPI_OSD_TYPE_STRING){
                    str_size = sizeof(pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].stContent.stStrContent.szStr);
                    memset(pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].stContent.stStrContent.szStr, 0 , str_size);
                    strncpy(pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].stContent.stStrContent.szStr, string_carnum_stamp, strlen(string_carnum_stamp));
                    // printf("set car num = %s\n", pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].stContent.stStrContent.szStr);
                }
            }
        }
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    CVI_PARAM_SetSaveFlg();
}

void CVI_PARAM_GetMdConfigParam(CVI_PARAM_MD_ATTR_S *Md)
{
        CVI_PARAM_MEDIA_COMM_S MediaComm = {0};
        CVI_PARAM_GetMediaComm(&MediaComm);

        pthread_mutex_lock(&g_stParamCtx.mutexLock);
        //Md = &MediaComm.Md;
        memcpy(Md, &MediaComm.Md, sizeof(CVI_PARAM_MD_ATTR_S));
        CVI_LOGD("Md.motionSensitivity = %d, MediaComm.Md.motionSensitivity = %d\n", Md->motionSensitivity, MediaComm.Md.motionSensitivity);
        pthread_mutex_unlock(&g_stParamCtx.mutexLock);
}

#ifdef SERVICES_ADAS_ON
void CVI_PARAM_GetADASConfigParam(CVI_PARAM_ADAS_ATTR_S *ADAS)
{
        CVI_PARAM_MEDIA_COMM_S MediaComm = {0};
        CVI_PARAM_GetMediaComm(&MediaComm);

        pthread_mutex_lock(&g_stParamCtx.mutexLock);
        memcpy(ADAS, &MediaComm.ADAS, sizeof(CVI_PARAM_ADAS_ATTR_S));
        pthread_mutex_unlock(&g_stParamCtx.mutexLock);
}
#endif

#ifdef ENABLE_ISP_IRCUT
void CVI_PARAM_GetISPIrConfigParam(CVI_PARAM_ISPIR_ATTR_S *ISPIR)
{
        CVI_PARAM_MEDIA_COMM_S MediaComm = {0};
        CVI_PARAM_GetMediaComm(&MediaComm);

        pthread_mutex_lock(&g_stParamCtx.mutexLock);
        memcpy(ISPIR, &MediaComm.IspIr, sizeof(CVI_PARAM_ISPIR_ATTR_S));
        pthread_mutex_unlock(&g_stParamCtx.mutexLock);
}
#endif

#ifdef SERVICES_QRCODE_ON
void CVI_PARAM_GetQRCodeConfigParam(CVI_PARAM_QRCODE_ATTR_S *QRCODE)
{
        CVI_PARAM_MEDIA_COMM_S MediaComm = {0};
        CVI_PARAM_GetMediaComm(&MediaComm);

        pthread_mutex_lock(&g_stParamCtx.mutexLock);
        memcpy(QRCODE, &MediaComm.QRCODE, sizeof(CVI_PARAM_QRCODE_ATTR_S));
        pthread_mutex_unlock(&g_stParamCtx.mutexLock);
}
#endif

int32_t  CVI_PARAM_SetCamStatus(uint32_t CamId, bool Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
    uint32_t i = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {
            pstParamCtx->pstCfg->CamCfg[i].CamEnable = Param;
            uint32_t enSns = pstParamCtx->pstCfg->Menu.ViewWin.Current & 0xFFFF;
            if(Param == true){
                enSns |= (0x1 << CamId);
            }else{
                enSns &= ((~(0x1 << CamId)) & 0xFFFF);
            }
            pstParamCtx->pstCfg->Menu.ViewWin.Current = ((enSns << 16) | enSns);
            break;
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_SetCamIspInfoStatus(uint32_t CamId, bool Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
    uint32_t i = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {
            pstParamCtx->pstCfg->CamCfg[i].CamIspEnable = Param;
            // CVI_LOGD("CamCfg[i].CamIspEnable  = %d, i = %d, CamId = %d\n", pstParamCtx->pstCfg->CamCfg[i].CamIspEnable , i, CamId);
            break;
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_SetWndParam(CVI_PARAM_WND_ATTR_S *Param)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(&pstParamCtx->pstCfg->MediaComm.Window, Param, sizeof(CVI_PARAM_WND_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetMenuVideoSize(uint32_t CamId,int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
    uint32_t i = 0;
    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    switch (Value) {
        case CVI_MEDIA_VIDEO_SIZE_1280X720P25:
            for (i = 0; i < pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt; i++) {
                if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[i].Desc, CVI_MEDIA_VIDEO_SIZE_1280X720P25)) {
                    break;
                }
            }
            break;
        case CVI_MEDIA_VIDEO_SIZE_1280X720P30:
            for (i = 0; i < pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt; i++) {
                if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[i].Desc, CVI_MEDIA_VIDEO_SIZE_1280X720P30)) {
                    break;
                }
            }
            break;
        case CVI_MEDIA_VIDEO_SIZE_1920X1080P25:
            for (i = 0; i < pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt; i++) {
                if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[i].Desc, CVI_MEDIA_VIDEO_SIZE_1920X1080P25)) {
                    break;
                }
            }
            break;
        case CVI_MEDIA_VIDEO_SIZE_1920X1080P30:
            for (i = 0; i < pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt; i++) {
                if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[i].Desc, CVI_MEDIA_VIDEO_SIZE_1920X1080P30)) {
                    break;
                }
            }
            break;
        case CVI_MEDIA_VIDEO_SIZE_2304X1296P25:
            for (i = 0; i < pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt; i++) {
                if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[i].Desc, CVI_MEDIA_VIDEO_SIZE_2304X1296P25)) {
                    break;
                }
            }
            break;
        case CVI_MEDIA_VIDEO_SIZE_2304X1296P30:
            for (i = 0; i < pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt; i++) {
                if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[i].Desc, CVI_MEDIA_VIDEO_SIZE_2304X1296P30)) {
                    break;
                }
            }
            break;
        case CVI_MEDIA_VIDEO_SIZE_2560X1440P25:
            for (i = 0; i < pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt; i++) {
                if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[i].Desc, CVI_MEDIA_VIDEO_SIZE_2560X1440P25)) {
                    break;
                }
            }
            break;
        case CVI_MEDIA_VIDEO_SIZE_2560X1440P30:
            for (i = 0; i < pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt; i++) {
                if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[i].Desc, CVI_MEDIA_VIDEO_SIZE_2560X1440P30)) {
                    break;
                }
            }
            break;
        case CVI_MEDIA_VIDEO_SIZE_2560X1600P25:
            for (i = 0; i < pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt; i++) {
                if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[i].Desc, CVI_MEDIA_VIDEO_SIZE_2560X1600P25)) {
                    break;
                }
            }
            break;
        case CVI_MEDIA_VIDEO_SIZE_2560X1600P30:
            for (i = 0; i < pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt; i++) {
                if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[i].Desc, CVI_MEDIA_VIDEO_SIZE_2560X1600P30)) {
                    break;
                }
            }
            break;
        case CVI_MEDIA_VIDEO_SIZE_3840X2160P25:
            for (i = 0; i < pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt; i++) {
                if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[i].Desc, CVI_MEDIA_VIDEO_SIZE_3840X2160P25)) {
                    break;
                }
            }
            break;
        case CVI_MEDIA_VIDEO_SIZE_3840X2160P30:
            for (i = 0; i < pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt; i++) {
                if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[i].Desc, CVI_MEDIA_VIDEO_SIZE_3840X2160P30)) {
                    break;
                }
            }
            break;
        default:
            CVI_LOGE("Value %d is invalid\n", Value);
            break;
    }

    if (i >= pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt) {
        CVI_LOGE("Value %d is invalid\n", Value);
        pthread_mutex_unlock(&pstParamCtx->mutexLock);
        return CVI_PARAM_ENOTINIT;
    }
    pstParamCtx->pstCfg->Menu.VideoSize.Current = i;
    pstParamCtx->pstCfg->CamCfg[CamId].CamMediaInfo.CurMediaMode = Value;

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetMenuVideoLoop(uint32_t CamId,int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    pstParamCtx->pstCfg->Menu.VideoLoop.Current = Value;
    CVI_PARAM_RECORD_CHN_ATTR_S *astRecord = &pstParamCtx->pstCfg->MediaComm.Record.ChnAttrs[CamId];
    switch (Value) {
        case CVI_MEDIA_VIDEO_LOOP_1MIN:
            astRecord->SplitTime = 60000; //msec
            break;
        case CVI_MEDIA_VIDEO_LOOP_3MIN:
            astRecord->SplitTime = 180000; //msec
            break;
        case CVI_MEDIA_VIDEO_LOOP_5MIN:
            astRecord->SplitTime = 300000; //msec
            break;
        default:
            CVI_LOGE("Value is invalid");
            break;
    }
    printf("pstParamCtx->pstCfg->MagicStart == %0x\n", pstParamCtx->pstCfg->MagicStart);
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}



static int32_t CVI_PARAM_SetMenuVideoCodec(uint32_t CamId,int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
    uint32_t i = 0, j = 0, z = 0;
    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    pstParamCtx->pstCfg->Menu.VideoCodec.Current = Value;
    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for(j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < CVI_PARAM_MEDIA_CNT)); j++){
            for (z = 0; z < MAX_VENC_CNT; z++) {
                if (pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[z].VencChnEnable == true) {
                    CVI_MAPI_VENC_CHN_PARAM_T *attr = &pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[z].MapiVencAttr;
                    if (attr->codec == CVI_MAPI_VCODEC_H264 || attr->codec == CVI_MAPI_VCODEC_H265) {
                        switch (Value) {
                            case CVI_MEDIA_VIDEO_VENCTYPE_H264:
                                attr->codec = CVI_MAPI_VCODEC_H264;
                                break;
                            case CVI_MEDIA_VIDEO_VENCTYPE_H265:
                                attr->codec = CVI_MAPI_VCODEC_H265;
                                break;
                            default:
                                CVI_LOGE("Value is invalid");
                                break;
                        }
                    }
                }
            }
        }
    }

    printf("pstParamCtx->pstCfg->MagicStart == %0x\n", pstParamCtx->pstCfg->MagicStart);
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_SetVENCParam()
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
    uint32_t i = 0, j = 0, z = 0;
    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for(j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < CVI_PARAM_MEDIA_CNT)); j++){
            for (z = 0; z < MAX_VENC_CNT; z++) {
                CVI_MEDIA_VENC_CHN_ATTR_S *s_VencAttr = &pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[0];
                CVI_MEDIA_VENC_CHN_ATTR_S *st_VencAttr = &pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[1];
                s_VencAttr->bindMode = 0;
                s_VencAttr->MapiVencAttr.pixel_format = 19;
                s_VencAttr->MapiVencAttr.gop = 1;
                s_VencAttr->MapiVencAttr.bitrate_kbps = 500;
                s_VencAttr->MapiVencAttr.bufSize = 1048576;
                s_VencAttr->MapiVencAttr.initialDelay = 2000;
                s_VencAttr->MapiVencAttr.thrdLv = 3;
                s_VencAttr->MapiVencAttr.statTime = 2;
                s_VencAttr->MapiVencAttr.videoSignalTypePresentFlag = 0;
                s_VencAttr->MapiVencAttr.videoFullRangeFlag = 0;
                s_VencAttr->MapiVencAttr.videoFormat = 0;
                st_VencAttr->bindMode = 0;
                st_VencAttr->MapiVencAttr.pixel_format = 19;
                st_VencAttr->MapiVencAttr.gop = 1;
                st_VencAttr->MapiVencAttr.bitrate_kbps = 1500;
                st_VencAttr->MapiVencAttr.bufSize = 262144;
                st_VencAttr->MapiVencAttr.initialDelay = 2000;
                st_VencAttr->MapiVencAttr.thrdLv = 3;
                st_VencAttr->MapiVencAttr.statTime = 2;
                st_VencAttr->MapiVencAttr.videoSignalTypePresentFlag = 0;
                st_VencAttr->MapiVencAttr.videoFullRangeFlag = 0;
                st_VencAttr->MapiVencAttr.videoFormat = 0;
            }
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    CVI_PARAM_SetSaveFlg();

    return 0;
}

static int32_t CVI_PARAM_SetMenuLapseTime(uint32_t CamId,int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
   int32_t  gop = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    pstParamCtx->pstCfg->Menu.LapseTime.Current = Value;

    CVI_PARAM_RECORD_CHN_ATTR_S *astRecord = &pstParamCtx->pstCfg->MediaComm.Record.ChnAttrs[CamId];
    switch (Value) {
        case CVI_MEDIA_VIDEO_LAPSETIME_OFF:
            gop = 0;
            break;
        case CVI_MEDIA_VIDEO_LAPSETIME_1S:
            gop = 1; //sec
            break;
        case CVI_MEDIA_VIDEO_LAPSETIME_2S:
            gop = 2; //sec
            break;
        case CVI_MEDIA_VIDEO_LAPSETIME_3S:
            gop = 3; //sec
            break;
        default:
            CVI_LOGE("Value is invalid");
            break;
    }
    astRecord->TimelapseGop = gop;

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetMenuAudioStatus(uint32_t CamId,int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    pstParamCtx->pstCfg->Menu.AudioEnable.Current = Value;

    CVI_PARAM_RECORD_CHN_ATTR_S *astRecord = &pstParamCtx->pstCfg->MediaComm.Record.ChnAttrs[CamId];
    switch (Value) {
        case CVI_MEDIA_VIDEO_AUDIO_OFF:
            astRecord->AudioStatus = false;
            break;
        case CVI_MEDIA_VIDEO_AUDIO_ON:
            astRecord->AudioStatus = true;
            break;
        default:
            CVI_LOGE("Value is invalid");
            break;
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetMenuOsd(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.Osd.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_Set_Pwm_Bri(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.PwmBri.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_Set_View_Win(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.ViewWin.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_Get_View_Win(void)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
   int32_t  cur = pstParamCtx->pstCfg->Menu.ViewWin.Current;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return cur;
}

static int32_t CVI_PARAM_SetMenuScreenDormant(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.ScreenDormant.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetKeyTone(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.KeyTone.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetFatigueDrive(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.FatigueDirve.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetSpeedStamp(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.SpeedStamp.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetGPSStamp(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.GPSStamp.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetMenuGsensor(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    switch (Value)
    {
    case CVI_MENU_GENSOR_OFF:
        pstParamCtx->pstCfg->DevMng.Gsensor.enSensitity = 0;
        break;
    case CVI_MENU_GENSOR_LOW:
        pstParamCtx->pstCfg->DevMng.Gsensor.enSensitity = 1;
        break;
    case CVI_MENU_GENSOR_MID:
        pstParamCtx->pstCfg->DevMng.Gsensor.enSensitity = 2;
        break;
    case CVI_MENU_GENSOR_HIGH:
        pstParamCtx->pstCfg->DevMng.Gsensor.enSensitity = 3;
        break;
    case CVI_MENU_GENSOR_BUIT:
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    return 0;
}

static int32_t CVI_PARAM_SetSpeedUnit(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.SpeedUnit.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetRearCamMirror(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.CamMirror.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetLanguage(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.Language.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetTimeFormat(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.TimeFormat.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetTimeZone(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.TimeZone.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_Frequence(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.Frequence.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetRecEn(uint32_t CamId,int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    CVI_PARAM_RECORD_CHN_ATTR_S *astRecord = &pstParamCtx->pstCfg->MediaComm.Record.ChnAttrs[CamId];
    astRecord->Enable = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_GetRecLoop(int32_t  *Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    *Value = pstParamCtx->pstCfg->Menu.RecLoop.Current;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetRecLoop(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.RecLoop.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_SetParking(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.Parking.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetMotionDet(uint32_t CamId,int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.MotionDet.Current = Value;
    CVI_PARAM_MD_CHN_ATTR_S *astMd = &pstParamCtx->pstCfg->MediaComm.Md.ChnAttrs[CamId];
    astMd->Enable = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetPhotoSize(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.PhotoSize.Current = Value;
    pstParamCtx->pstCfg->MediaComm.Photo.photoid = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t CVI_PARAM_SetPhotoQuality(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.PhotoQuality.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_SetCarNumStamp(int32_t  Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.CarNumStamp.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    //CVI_PARAM_SetSaveFlg();
    return 0;
}
int32_t  CVI_PARAM_SetBootFirstFlag(CVI_BOOL Value)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.UserData.bBootFirst = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    CVI_PARAM_SetSaveFlg();
    return 0;
}

int32_t  CVI_PARAM_LoadDefaultParamFromFlash(CVI_PARAM_CFG_S* param)
{
   int32_t  ret = 0;
    CVI_U64 def_cfg_addr_s, def_cfg_addr_e;

#ifdef STORAGE_TYPE_NOR
    CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_SPI_0;
#elif STORAGE_TYPE_NAND
    CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_NAND_0;
#else
	CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_EMMC_0;
#endif


    ret = CVI_Flash_GetStartAddr(DEF_APP_CFG_PART_NUM, &def_cfg_addr_s, &def_cfg_addr_e);
    if(ret != 0) {
        printf("get cfg flash part addr failed\n");
        exit(-1);
    }
    uint64_t u64Len = def_cfg_addr_e - def_cfg_addr_s + 1;
    printf("cfg_s = 0x%llx, cfg_e = 0x%llx\n", def_cfg_addr_s, def_cfg_addr_e);
    uint8_t *pBuf = (uint8_t *)malloc(u64Len);

    // flash open
    ret = CVI_Flash_Open(enFlashType, PARAM_DEF_PARTI_NAME, def_cfg_addr_s, u64Len);
    if (ret == -1) {
        printf("Open flash fail\n");
        goto EXIT;
    } else {
        printf("Open flash success\n");
    }

    // read flash
    ret = CVI_Flash_Read(enFlashType, def_cfg_addr_s, pBuf, u64Len, CVI_FLASH_RW_FLAG_RAW);
    if (ret == -1) {
        printf("read flash fail\n");
        goto EXIT;
    } else {
        printf("read flash success\n");
    }

    memcpy(param, pBuf, sizeof(CVI_PARAM_CFG_S));

    if ((param->MagicStart != CVI_PARAM_MAGIC_START) ||
        (param->MagicEnd != CVI_PARAM_MAGIC_END)) {
        printf("error, magic error\n");
        ret = -1;
        CVI_Flash_Close(enFlashType);
        goto EXIT;
    } else {
        printf("read param ok\n");
        printf("power on mode: %d\n", param->MediaComm.PowerOnMode);
    }
    printf("CVI_PARAM_SaveParam in load default\n");

    // file close
    ret = CVI_Flash_Close(enFlashType);
    if (ret == CVI_FAILURE) {
        printf("close flash fail\n");
        // goto EXIT;
    } else {
        printf("close flash success\n");
    }

EXIT:
    if(pBuf){
        free(pBuf);
    }
    return ret;
}

int32_t  CVI_PARAM_GetVideoSizeEnum(int32_t  Value, CVI_MEDIA_VIDEO_SIZE_E *VideoSize)
{
    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return CVI_PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    if (pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt < (uint32_t)Value) {
        CVI_LOGE("Value is illeage!, Value = %d, ItemCnt = %d\n", Value, pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt);
        pthread_mutex_unlock(&pstParamCtx->mutexLock);
        return CVI_PARAM_ENOTINIT;
    }
    if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, CVI_MEDIA_VIDEO_SIZE_1280X720P25)) {
        *VideoSize = CVI_MEDIA_VIDEO_SIZE_1280X720P25;
    } else if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, CVI_MEDIA_VIDEO_SIZE_1280X720P30)) {
        *VideoSize = CVI_MEDIA_VIDEO_SIZE_1280X720P30;
    } else if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, CVI_MEDIA_VIDEO_SIZE_1920X1080P25)) {
        *VideoSize = CVI_MEDIA_VIDEO_SIZE_1920X1080P25;
    } else if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, CVI_MEDIA_VIDEO_SIZE_1920X1080P30)) {
        *VideoSize = CVI_MEDIA_VIDEO_SIZE_1920X1080P30;
    } else if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, CVI_MEDIA_VIDEO_SIZE_2304X1296P25)) {
        *VideoSize = CVI_MEDIA_VIDEO_SIZE_2304X1296P25;
    } else if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, CVI_MEDIA_VIDEO_SIZE_2304X1296P30)) {
        *VideoSize = CVI_MEDIA_VIDEO_SIZE_2304X1296P30;
    } else if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, CVI_MEDIA_VIDEO_SIZE_2560X1440P25)) {
        *VideoSize = CVI_MEDIA_VIDEO_SIZE_2560X1440P25;
    } else if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, CVI_MEDIA_VIDEO_SIZE_2560X1440P30)) {
        *VideoSize = CVI_MEDIA_VIDEO_SIZE_2560X1440P30;
    } else if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, CVI_MEDIA_VIDEO_SIZE_2560X1600P25)) {
        *VideoSize = CVI_MEDIA_VIDEO_SIZE_2560X1600P25;
    } else if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, CVI_MEDIA_VIDEO_SIZE_2560X1600P30)) {
        *VideoSize = CVI_MEDIA_VIDEO_SIZE_2560X1600P30;
    } else if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, CVI_MEDIA_VIDEO_SIZE_3840X2160P25)) {
        *VideoSize = CVI_MEDIA_VIDEO_SIZE_3840X2160P25;
    } else if (!CVI_APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, CVI_MEDIA_VIDEO_SIZE_3840X2160P30)) {
        *VideoSize = CVI_MEDIA_VIDEO_SIZE_3840X2160P30;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  CVI_PARAM_SetMenuParam(uint32_t CamId, CVI_PARAM_MENU_E MenuItem,int32_t  Value)
{

    switch(MenuItem) {
        case CVI_PARAM_MENU_VIDEO_SIZE:
            CVI_PARAM_SetMenuVideoSize(CamId, Value);
            break;
        case CVI_PARAM_MENU_VIDEO_LOOP:
            CVI_PARAM_SetMenuVideoLoop(CamId, Value);
            break;
        case CVI_PARAM_MENU_VIDEO_CODEC:
            CVI_PARAM_SetMenuVideoCodec(CamId, Value);
            break;
        case CVI_PARAM_MENU_LAPSE_TIME:
            CVI_PARAM_SetMenuLapseTime(CamId, Value);
            break;
        case CVI_PARAM_MENU_AUDIO_STATUS:
            CVI_PARAM_SetMenuAudioStatus(CamId, Value);
            break;
        case CVI_PARAM_MENU_OSD_STATUS:
            CVI_PARAM_SetMenuOsd(Value);
            break;
        case CVI_PARAM_MENU_PWM_BRI_STATUS:
            CVI_PARAM_Set_Pwm_Bri(Value);
            break;
        case CVI_PARAM_MENU_VIEW_WIN_STATUS:
            CVI_PARAM_Set_View_Win(Value);
            break;
        case CVI_PARAM_MENU_SCREENDORMANT:
            CVI_PARAM_SetMenuScreenDormant(Value);
            break;
        case CVI_PARAM_MENU_GSENSOR:
            CVI_PARAM_SetMenuGsensor(Value);
            break;
        case CVI_PARAM_MENU_FATIGUE_DRIVE:
            CVI_PARAM_SetFatigueDrive(Value);
            break;
        case CVI_PARAM_MENU_SPEED_STAMP:
            CVI_PARAM_SetSpeedStamp(Value);
            break;
        case CVI_PARAM_MENU_GPS_STAMP:
            CVI_PARAM_SetGPSStamp(Value);
            break;
        case CVI_PARAM_MENU_SPEED_UNIT:
            CVI_PARAM_SetSpeedUnit(Value);
            break;
        case CVI_PARAM_MENU_REARCAM_MIRROR:
            CVI_PARAM_SetRearCamMirror(Value);
            break;
        case CVI_PARAM_MENU_LANGUAGE:
            CVI_PARAM_SetLanguage(Value);
            break;
        case CVI_PARAM_MENU_TIME_FORMAT:
            CVI_PARAM_SetTimeFormat(Value);
            break;
        case CVI_PARAM_MENU_TIME_ZONE:
            CVI_PARAM_SetTimeZone(Value);
            break;
        case CVI_PARAM_MENU_FREQUENCY:
            CVI_PARAM_Frequence(Value);
            break;
        case CVI_PARAM_MENU_KEYTONE:
            CVI_PARAM_SetKeyTone(Value);
            break;
        case CVI_PARAM_MENU_PARKING:
            CVI_PARAM_SetParking(Value);
            break;
        case CVI_PARAM_MENU_REC_INX:
            CVI_PARAM_SetRecEn(CamId, Value);
            break;
        case CVI_PARAM_MENU_REC_LOOP:
            CVI_PARAM_SetRecLoop(Value);
            break;
        case CVI_PARAM_MENU_CARNUM: //stamp
            CVI_PARAM_SetCarNumStamp( Value);
            break;
        case CVI_PARAM_MENU_PHOTO_SIZE:
            CVI_PARAM_SetPhotoSize( Value);
            break;
        case CVI_PARAM_MENU_PHOTO_QUALITY:
            CVI_PARAM_SetPhotoQuality( Value);
            break;
        case CVI_PARAM_MENU_MOTION_DET:
            CVI_PARAM_SetMotionDet(CamId, Value);
            break;
        default:
            CVI_LOGE("not menu item\n");
            break;
    }

    CVI_PARAM_SetSaveFlg();

    return 0;
}

#pragma pack(pop)
