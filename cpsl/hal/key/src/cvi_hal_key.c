/**
* @file    cvi_hal_key.c
* @brief   HAL key implemention
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "cvi_hal_key.h"
#include "cvi_hal_adc.h"
#include "cvi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

#define ADC_CHANNEL1_KEY   (1)
#define ADC_CH_KEY_VALUE_RANGE (200)

static int32_t CVI_Vkey_Get_Value(void)
{
    int32_t ret = CVI_HAL_ADC_GetValue(ADC_CHANNEL1_KEY);
    if (-1 == ret) {
        CVI_LOGE("CVI_HAL_ADC_GetValue fail\n");
        return -1;
    }
    return ret;
}

static int32_t CVI_ADCKey_Get_Value(int32_t val, CVI_GPIO_VALUE_E *value)
{
    int32_t ret = CVI_Vkey_Get_Value();
    if (-1 == ret) {
        CVI_LOGE("CVI_Vkey_Get_Value fail\n");
        return -1;
    }

    if ((val <= ret + ADC_CH_KEY_VALUE_RANGE) && (val >= ret - ADC_CH_KEY_VALUE_RANGE)) {
        *value = CVI_GPIO_VALUE_L;//CVI_GPIO_VALUE_H;
    }
    else {
        *value = CVI_GPIO_VALUE_H;//CVI_GPIO_VALUE_L;
    }

    return 0;
}

/** macro define */
static bool key_init = false;
#if 0
static CVI_GPIO_ID_E GPIOID[] = {{CVI_HAL_KEY_IDX_0, CVI_GPIOE_08},
                          {CVI_HAL_KEY_IDX_1, CVI_GPIOE_14},
                          {CVI_HAL_KEY_IDX_2, CVI_GPIOB_04},
                          {CVI_HAL_KEY_IDX_3, CVI_GPIOE_17}};
#else
// for adc1 values using gpioid int32_t && PWR_GPIO_7&8 is gpio (PWR_GPIO_7:pwr_wakeup1:POWER_ON/OFF_DET;PWR_GPIO_8:pwr_button1)
static CVI_GPIO_ID_E GPIOID[] = {{CVI_HAL_KEY_IDX_0, CVI_GPIOE_08},
                          {CVI_HAL_KEY_IDX_1, 200},
                          {CVI_HAL_KEY_IDX_2, 1024},
                          {CVI_HAL_KEY_IDX_3, 2220}};
static CVI_HAL_KEY_IDX_E grp1_keycnt = 4;
#endif

int32_t CVI_HAL_KEY_Init(void)
{
    //int32_t gpiocunt = 0;
    int32_t s32Ret = 0;
    CVI_LOGI("CVI_HAL_ADC_Init KEY\n");
	if (key_init == false) {
		key_init = true;

        //key in adc1 init
        s32Ret = CVI_HAL_ADC_Init();
        if (0 != s32Ret) {
            CVI_LOGE("CVI_HAL_ADC_Init KEY Failed\n");
            //CVI_MUTEX_UNLOCK(s_GAUGEMNGMutex);
            return -1;
        }
        //gpio
        s32Ret = CVI_GPIO_Direction_Input(GPIOID[0].gpioid);
        if (0 != s32Ret) {
            CVI_LOGE("[Error]set gpio dir failed\n");
            return -1;
        }

	} else {
        CVI_LOGE("key already init\n");
        return -1;
    }
    return 0;
}


int32_t CVI_HAL_KEY_GetState(CVI_HAL_KEY_IDX_E enKeyIdx, CVI_HAL_KEY_STATE_E* penKeyState)
{
    uint32_t u32Val;
    int32_t s32Ret = 0;
    int32_t i = 0;
    int32_t gpiocunt = 0;
    int32_t flage = 0;
    gpiocunt = sizeof(GPIOID)/sizeof(GPIOID[0]);

    /* init check */
    if (key_init == false) {
        CVI_LOGE("key not initialized\n");
        return -1;
    }

    /* parm penKeyState check */
    if (NULL == penKeyState) {
        CVI_LOGE("penKeyState is null\n");
        return -1;
    }

    //for adc1 read key form array 1
    //default key 1~3 used adc1; Modify according to actual button conditions
    if (enKeyIdx < grp1_keycnt && enKeyIdx > 0) {
        s32Ret = CVI_ADCKey_Get_Value(GPIOID[enKeyIdx].gpioid, &u32Val);    //adc1
        if (0 != s32Ret) {
            CVI_LOGE("[Error]read adc data failed\n");
            return -1;
        }
        flage = 1;
    }else if (enKeyIdx >= grp1_keycnt && enKeyIdx < gpiocunt) {
        s32Ret = CVI_ADCKey_Get_Value_Ch2(GPIOID[enKeyIdx].gpioid, &u32Val);    //adc2
        if (0 != s32Ret) {
            CVI_LOGE("[Error]read adc data failed\n");
            return -1;
        }
        flage = 1;
    }
    //gpio key power key
    //default key 0 used gpio
    if (enKeyIdx == GPIOID[0].id) {
        s32Ret = CVI_GPIO_Get_Value(GPIOID[0].gpioid, &u32Val);
        if (0 != s32Ret) {
            CVI_LOGE("[Error]read gpio data failed\n");
            return -1;
        }
        flage = 1;
    }

    if (0 == flage) {
        CVI_LOGE("illeagel enkey(%d) out of range\n",enKeyIdx);
        return -1;
    }

    *penKeyState = (1 == u32Val) ? CVI_HAL_KEY_STATE_UP : CVI_HAL_KEY_STATE_DOWN;

    return 0;
}

int32_t CVI_HAL_KEY_Deinit(void)
{
    if (key_init == false) {
        CVI_LOGE("key not initialized,no need to close\n");
        return -1;
    }

    key_init = 0;
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


