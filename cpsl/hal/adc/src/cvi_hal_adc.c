/*
 ********************************************************************
 * Demo program on CviTek cv183x
 *
 * Copyright CviTek Techanelologies. All Rights Reserved.
 *
 ********************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "cvi_sysutils.h"
#include "cvi_log.h"
#include "cvi_appcomm.h"

static uint8_t adc_init = 0;

#define ADC_KO_PATH CVI_KOMOD_PATH "/" CHIP_TYPE "_saradc.ko"
#define ADC_SYS_PATH "/sys/bus/iio/devices/iio:device0/"

static int32_t CVI_ADC_Insmod(void)
{
	return cvi_insmod(ADC_KO_PATH, NULL);
}

static int32_t CVI_ADC_Rmmod(void)
{
	return cvi_rmmod(ADC_KO_PATH);
}

static int32_t cvi_adc_get_value(const char *node)
{
	int32_t val_pre = 0;
	int32_t fd_adc = -1;
    char buf[5];
	memset(buf, 0, sizeof(buf));
	ssize_t len;

	fd_adc = open(node, O_RDWR|O_NOCTTY|O_NDELAY);
	if (fd_adc < 0) {
		CVI_LOGE("open adc failed\n");
		return -1;
	}
	len = read(fd_adc, buf, sizeof(buf));
	buf[len] = 0;
	val_pre = atoi(buf);
	close(fd_adc);

	return val_pre;
}

int32_t CVI_HAL_ADC_GetValue(int32_t int_adc_channel)
{
	char *adc_path = ADC_SYS_PATH;
	char char_channel[128] = {0};
	int32_t ret = 0;

	if (int_adc_channel < 1 || int_adc_channel > 6) {
		CVI_LOGE("channel should between in 1 and 6\n");
		return -1;
	}
	sprintf(char_channel, "%sin_voltage%d_raw",adc_path, int_adc_channel);
	if (NULL == char_channel) {
		CVI_LOGE("point must vailable\n");
		return -1;
	}
	ret = cvi_adc_get_value(char_channel);
	return ret;
}

int32_t CVI_HAL_ADC_Init(void)
{
    int32_t ret = 0;
	if(adc_init == 0) {
		ret |= CVI_ADC_Insmod();
		adc_init = 1;
	}
    return ret;
}

int32_t CVI_HAL_ADC_Deinit(void)
{
	if(adc_init == 0)
		return 0;

	adc_init = 0;
    return CVI_ADC_Rmmod();
}
