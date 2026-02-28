/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>

#include "common_yocsystem.h"
#include "platform.h"
#include "media_init.h"
#include "cvi_sys.h"

#define TAG "app"
#include "cvi_comm_ipcm.h"
#include "cvi_ipcm.h"
#include <console_uart.h>

extern int ipcm_set_snd_cpu(int cpu_id);

static int _anon_vuart_process(CVI_VOID *priv, IPCM_ANON_MSG_S *data)
{
	unsigned char port_id, msg_id, data_type;
	unsigned int data_len;
	int ret = 0;

	if (data == NULL) {
		printf("======anon vuart fail, handle data is null.\n");
		return -1;
	}

	port_id = data->u8PortID;
	msg_id = data->u8MsgID;
	data_type = data->u8DataType;
	data_len = data->stData.u32Size;

	if ((port_id == CVI_IPCM_PORT_ANON_VUART) && (msg_id == 0) && (data_type == IPCM_MSG_TYPE_SHM)) {
		console_push_data_to_ringbuffer(data->stData.pData, data_len);
	} else {
		printf("%s port_id:%d msg_id:%d data_type:%d not handled.\n", __func__, port_id, msg_id, data_type);
	}

	return ret;
}

#ifdef CONFIG_RTOS_ANNON_MSG
static int _anon_msg_process(CVI_VOID *priv, IPCM_ANON_MSG_S *data)
{
	if (data == NULL) {
		printf("%s fail, handle data is null.\n", __func__);
		return -1;
	}

	return APP_ANONMSG_process(priv, data);
}
#endif

static int anon_test_init(void)
{
	int ret = 0;

	ret = CVI_IPCM_AnonInit();
	if (ret) {
		printf("%s CVI_IPCM_AnonInit fail:%d.\n", __func__, ret);
		return ret;
	}

	ret = CVI_IPCM_RegisterAnonHandle(CVI_IPCM_PORT_ANON_VUART, _anon_vuart_process, NULL);
	if (ret) {
		printf("%s CVI_IPCM_RegisterAnonHandle(%d) fail:%d.\n", __func__, CVI_IPCM_PORT_ANON_VUART, ret);
		CVI_IPCM_AnonUninit();
		return ret;
	}

#ifdef CONFIG_RTOS_ANNON_MSG
	ret = CVI_IPCM_RegisterAnonHandle(CVI_IPCM_PORT_ANON_MSG, _anon_msg_process, NULL);
	if (ret) {
		printf("%s CVI_IPCM_RegisterAnonHandle(%d) fail:%d.\n", __func__, CVI_IPCM_PORT_ANON_MSG, ret);
		CVI_IPCM_AnonUninit();
		return ret;
	}
#endif

	return ret;
}

int main(int argc, char *argv[])
{
	//board pinmux init
	PLATFORM_IoInit();

    CVI_IPCM_SetRtosSysBootStat();

	YOC_SYSTEM_Init();

	// //voltage of battery: PWR_GPIO[1], chip_id: 1; chn_id: 2
	// int ADC_state = PLATFORM_GetADCValue(1, 2);
	// //ADC_state: sampling value
	// //voltage；VBat_ADC = ADC_value/4095.0 * 1.5v
	// //according to Circuit voltage division law: battery voltage = 31 / 11.0 * VBat_ADC
	// float VBat_ADC = ADC_state / 4095.0 * 1.5;
	// if (VBat_ADC < 1.0) {
	// 	PLATFORM_ShutDown();
	// }
	// float V_Bat = 31 / 11.0 * VBat_ADC;
    // printf("the voltage of battery: %.2f\n", V_Bat);

	//cli and ulog init
	YOC_SYSTEM_ToolInit();

	//Fs init
	//YOC_SYSTEM_FsVfsInit();
#ifdef ARCH_ARM
	ipcm_set_snd_cpu(0);
#else
	ipcm_set_snd_cpu(1);
#endif
	//media init
	CVI_Media_Init();

	anon_test_init();
#ifdef CONFIG_SCREEN_ON
	//screen init
	CVI_Media_PanelInit();

	//show logo
	CVI_Media_Vdec_Logo();

	CVI_IPCM_SetRtosBootLogoStat();
#endif
	// audio open
	//CVI_Media_Audio_BootSound();

	while (1) {
		aos_msleep(3000);
	};
}
