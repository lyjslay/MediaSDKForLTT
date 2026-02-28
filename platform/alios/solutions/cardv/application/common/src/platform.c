#include <stdlib.h>
#include "platform.h"
#include <drv/pin.h>
#include <pinctrl-mars.h>
#include "cvi_type.h"
#include "cvi_board_pinmux.c"
#include <drv/adc.h>

void PLATFORM_SpkMute(int value)
{
//0静音 ，1非静音
    if(value){
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 1);
    }else{
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 0);
    }
}

void PLATFORM_IoInit(void)
{
//pinmux 切换接口
    _BoardPinmux();
}

void PLATFORM_PowerOff(void)
{
//下电休眠前调用接口
}

int PLATFORM_PanelInit(void)
{
    return CVI_SUCCESS;
}

void PLATFORM_PanelBacklightCtl(int level)
{

}

int PLATFORM_IrCutCtl(int duty)
{
    return 0;
}

int PLATFORM_GetADCValue(int chip_id, int chn_id)
{
    /* For cv181xc: chip0, ch1 --> ADC; chip1, ch1 --> PWR_GPIO[2], ch2 --> PWR_GPIO[1];
    For cv181xh：chip0, ch1, 2, 3 --> ADC1,2,3; chip1, ch1 --> PWR_GPIO[2], ch2 --> PWR_GPIO[1];*/
	csi_adc_t adc;
	// enable adc chip_id
    csi_adc_init(&adc, chip_id);
    // enable adc channe(chn_id): 2
    csi_adc_channel_enable(&adc, chn_id, true);
    csi_adc_start(&adc);
    int ADC_state = csi_adc_read(&adc);
    printf("the ADC_state: %d\n", ADC_state);
    csi_adc_uninit(&adc);
	//free(adc.priv);

    return ADC_state;
}

void PLATFORM_ShutDown(void)
{
    // m 0x0502603C 1
	// m 0x05025004 0xAB18
	// m 0x050260C0 1
	// m 0x05025008 0xFFFF0801
    mmio_write_32(0x0502603C, 0x01);
    mmio_write_32(0x05025004, 0xAB18);
    mmio_write_32(0x050260C0, 0x01);
    mmio_write_32(0x05025008, 0xFFFF0801);
}
