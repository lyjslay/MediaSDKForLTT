
#ifndef __CVI_UPGRADE_H__
#define __CVI_UPGRADE_H__

#include "cvi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** error code define */
#define CVI_UPGRADE_EINTR            1
#define CVI_UPGRADE_EPKG_INVALID     2
#define CVI_UPGRADE_EPKG_OVERSIZE    3          /* Package oversize */
#define CVI_UPGRADE_EIMAGE_OVERSIZE  4          /* Partition image oversize*/
#define CVI_UPGRADE_EINVAL           5
#define CVI_UPGRADE_EPKG_UNMATCH     6          /* Package un-match to partition*/

#define CVI_COMM_STR_LEN 64
#define CVI_UPGRADE_MAX_PART_CNT 10
#define CVI_COMM_PATH_MAX_LEN 128

#define CVI_UPGRADE_EVENT_PROGRESS 1

/** upgrade device information */
typedef struct cviUPGRADE_DEV_INFO_S {
	char szSoftVersion[CVI_COMM_STR_LEN]; /* Software version */
	char szModel[CVI_COMM_STR_LEN];       /* Product model */
} CVI_UPGRADE_DEV_INFO_S;

/** upgrade package head */
typedef struct cviUPGRADE_PKG_HEAD_S {
	uint32_t   u32Magic;
	uint32_t   u32Crc;         /* CRC number from HeadVer to end of image-data */
	uint32_t   u32HeadVer;     /* Package head version: 0x00000001 */
	uint32_t   u32PkgLen;      /* Package total length, including head/data */
	uint32_t   bMinusOne;      /* used for fip_spl.bin*/
	char szPkgModel[CVI_COMM_STR_LEN]; /* Package model, eg. cv1835_asic_wevb_0002a */
	char szPkgSoftVersion[CVI_COMM_STR_LEN];   /* Package version, eg. 1.0.0.0 */
	char PartitionFileName[CVI_UPGRADE_MAX_PART_CNT][32]; /*file name for partition, eg. for spinor(partition.xml), 0:fip_spl.bin, 1:yoc.bin */
	char reserved1[704];
	uint32_t   reserved2;
	int32_t  s32PartitionCnt;
	uint32_t   au32PartitionOffSet[CVI_UPGRADE_MAX_PART_CNT]; /* Partition offset in upgrade package */
} CVI_UPGRADE_PKG_HEAD_S;

/** upgrade event struct */
typedef struct cviUPGRADE_EVENT_S {
	uint32_t   eventID;
	void *argv;
} CVI_UPGRADE_EVENT_S;

int32_t CVI_UPGRADE_Init(void);

int32_t CVI_UPGRADE_Deinit(void);

int32_t CVI_UPGRADE_CheckPkg(const char *pszPkgUrl, const CVI_UPGRADE_DEV_INFO_S *pstDevInfo, unsigned char bCheckVer);

int32_t CVI_UPGRADE_DoUpgrade(const char *pszPkgUrl);

int32_t CVI_UPGRADE_DoUpgradeViaSD(const char *pszPkgUrl, const char *path);

void CVI_UPGRADE_RegisterEvent(void (*eventCb)(CVI_UPGRADE_EVENT_S *));


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __CVI_UPGRADE_H__ */

