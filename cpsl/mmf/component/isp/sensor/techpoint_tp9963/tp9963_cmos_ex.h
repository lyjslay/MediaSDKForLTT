#ifndef __TP9963_CMOS_EX_H_
#define __TP9963_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

// #ifdef ARCH_CV182X
// #include <linux/cvi_vip_cif.h>
// #include <linux/cvi_vip_snsr.h>
// #include "cvi_type.h"
// #else
// #include <linux/cif_uapi.h>
// #include <linux/vi_snsr.h>
// #include <linux/cvi_type.h>
// #endif
// #include "cvi_sns_ctrl.h"
#include "cvi_comm_cif.h"
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"

#define syslog(level, fmt, ...)            \
do {                                                   \
	printf(fmt, ##__VA_ARGS__);                \
} while (0)

typedef enum _TP9963_MODE_E
{
	TP9963_MODE_1080P_60P,
	TP9963_MODE_1080P_30P,
	TP9963_MODE_1080P_30P_2CH,
	TP9963_MODE_1080P_25P,
	TP9963_MODE_1080P_25P_2CH,
	TP9963_MODE_720P_30P,
	TP9963_MODE_720P_30P_2CH,
	TP9963_MODE_720P_25P,
	TP9963_MODE_720P_25P_2CH,
	TP9963_MODE_NUM
} TP9963_MODE_E;
typedef struct _TP9963_MODE_S
{
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_S stAgain[2];
	SNS_ATTR_S stDgain[2];
	CVI_U8 u8DgainReg;
	char name[64];
} TP9963_MODE_S;
/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/
extern ISP_SNS_STATE_S *g_pastTP9963[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunTP9963_BusInfo[];
extern CVI_U8 tp9963_i2c_addr;
extern const CVI_U32 tp9963_addr_byte;
extern const CVI_U32 tp9963_data_byte;
extern void tp9963_init(VI_PIPE ViPipe);
extern void tp9963_exit(VI_PIPE ViPipe);
extern void tp9963_standby(VI_PIPE ViPipe);
extern void tp9963_restart(VI_PIPE ViPipe);
extern int tp9963_write_register(VI_PIPE ViPipe, int addr, int data);
extern int tp9963_read_register(VI_PIPE ViPipe, int addr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __TP9963_CMOS_EX_H_ */
