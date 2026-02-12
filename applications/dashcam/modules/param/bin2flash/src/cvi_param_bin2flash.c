#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <inttypes.h>

#include "cvi_flash.h"
#include "nand.h"
#include "spi_raw.h"
#include "nand_raw.h"
#include "emmc_raw.h"
#include "cvi_param.h"

#define PARAM_PARTITION_ADDR    (0x780000)
#define PARAM_DEF_PARTI_ADDR    (0x800000)
#define PARAM_PARTITION_SIZE    (0x80000)  // 512*1024 bit
#define PARAM_PARTITION_NAME    "/dev/mtd3"
#define PARAM_DEF_PARTI_NAME    "/dev/mtd4"
#define PARAM_DEF_BIN_NAME      "./app_cfg.bin"
#define APP_CFG_PART_NUM    3

int32_t main(int32_t argc, char *argv[])
{
    int32_t s32Ret = 0;
    int32_t s32Fd = 0;

    char *pPartitionName = PARAM_PARTITION_NAME;
#ifdef STORAGE_TYPE_NOR
    CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_SPI_0;
#elif STORAGE_TYPE_NAND
    CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_NAND_0;
#else
	CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_EMMC_0;
#endif

    CVI_U64 cfg_addr_s, cfg_addr_e, u64Len;
    s32Ret = CVI_Flash_GetStartAddr(APP_CFG_PART_NUM, &cfg_addr_s, &cfg_addr_e);
    if(s32Ret != 0) {
        printf("get cfg flash part addr failed\n");
        exit(-1);
    }
    u64Len = cfg_addr_e - cfg_addr_s + 1;
    printf("cfg_s = 0x%llx, cfg_e = 0x%llx u64Len = 0x%llx\n", cfg_addr_s, cfg_addr_e, u64Len);

    // open bin file
    CVI_PARAM_CFG_S paramCfg;
    memset(&paramCfg, 0, sizeof(CVI_PARAM_CFG_S));
    s32Fd = open(PARAM_DEF_BIN_NAME, O_RDWR, 777);
    if (s32Fd < 0) {
        printf("load [%s] failed\n", PARAM_DEF_BIN_NAME);
        return -1;
    }
    ssize_t readCount = read(s32Fd, &paramCfg, sizeof(CVI_PARAM_CFG_S));
    if (0 > readCount) {
        printf("read %s failed\n", PARAM_DEF_BIN_NAME);
        close(s32Fd);
        return -1;
    }
    close(s32Fd);
    printf("read result:\n");
    printf("head: %x, end: %x, mode: %d\n\n", paramCfg.MagicStart,
            paramCfg.MagicEnd, paramCfg.MediaComm.PowerOnMode);
    if (argc == 2) {
        printf("\n change some param\n");
        paramCfg.MagicStart = 0x55555555;
        paramCfg.MagicEnd = 0xAAAAAAAA;
        paramCfg.MediaComm.PowerOnMode = 3;
        printf("head: %x, end: %x, mode: %d\n\n", paramCfg.MagicStart,
            paramCfg.MagicEnd, paramCfg.MediaComm.PowerOnMode);
    }

    s32Ret = CVI_Flash_Open(enFlashType, pPartitionName, cfg_addr_s, u64Len);
    if (s32Ret == CVI_FAILURE) {
        printf("Open flash fail\n");
        return 0;
    } else {
        printf("Open flash success\n");
    }

    uint8_t *pBuf = (uint8_t *)malloc(u64Len);
    if (pBuf == NULL) {
        CVI_Flash_Close(enFlashType);
        return 0;
    }
    // write to flash
    memcpy(pBuf, &paramCfg, sizeof(CVI_PARAM_CFG_S));

    //erase
	s32Ret = CVI_Flash_Erase(enFlashType, cfg_addr_s, u64Len);
	if (s32Ret == CVI_FAILURE) {
		printf("Erase flash fail\n");
        CVI_Flash_Close(enFlashType);
        free(pBuf);
        return 0;
    } else {
		printf("Erase flash success\n");
    }
	printf("\n");

    //write buffer
    s32Ret = CVI_Flash_Write(enFlashType, 0, (CVI_U8 *)pBuf, (CVI_U32)u64Len, CVI_FLASH_RW_FLAG_RAW);
    if (s32Ret == CVI_FAILURE) {
        printf("write flash fail\n");
        CVI_Flash_Close(enFlashType);
        free(pBuf);
        return 0;
    } else {
        printf("write flash success\n");
    }
    printf("\n");

    //file close
    s32Ret = CVI_Flash_Close(enFlashType);
    if (s32Ret == CVI_FAILURE) {
        printf("close flash fail\n");
    } else {
        printf("close flash success\n");
    }
    printf("\n");
    free(pBuf);
    return 0;
}
