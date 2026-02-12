#ifndef __CVI_ADC_H__
#define __CVI_ADC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

int32_t CVI_HAL_ADC_Init(void);
int32_t CVI_HAL_ADC_Deinit(void);
int32_t CVI_HAL_ADC_GetValue(int32_t int_adc_channel);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_ADC_H__ */
