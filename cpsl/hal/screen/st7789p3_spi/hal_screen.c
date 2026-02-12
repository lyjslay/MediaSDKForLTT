#include "cvi_hal_screen.h"
#include "cvi_hal_gpio.h"

#if CONFIG_PWM_ON
#include "cvi_hal_pwm.h"
#endif
#include "hal_screen.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup 	SCREEN */
/** @{ */  /** <!-- [SCREEN] */

typedef unsigned char u8;
static CVI_HAL_SCREEN_STATE_E g_halScreenSt7789p3Spi = CVI_HAL_SCREEN_STATE_ON;

#if CONFIG_PWM_ON
// CVI_HAL_PWM_S screen_bl = {
// 	.group  = 2,   //  pwmchip0/4/8/12
// 	.channel = 0,  //  pwmchip0:pwm0~3 pwmchip1:pwm4~7  20us
// 	.period = 10000,    //unit:ns
// 	.duty_cycle = 10000 //unit:ns
// };
CVI_HAL_PWM_S screen_bl = { 0 };
#endif

/**MIPI Screen do reset in HAL, RGB do reset in DRV*/
static void HAL_SCREEN_Reset(void)
{
	CVI_GPIO_Export(REST_LIGHT);
	CVI_GPIO_Direction_Output(REST_LIGHT);
	CVI_GPIO_Export(POWER_LIGHT);
	CVI_GPIO_Direction_Output(POWER_LIGHT);
	#if CONFIG_PWM_ON

	#else
	CVI_GPIO_Export(BACK_LIGHT);
	CVI_GPIO_Direction_Output(BACK_LIGHT);
	#endif

	CVI_GPIO_Set_Value(REST_LIGHT, CVI_GPIO_VALUE_L);
    usleep(10 * 1000);
    //ctrl power
    CVI_GPIO_Set_Value(POWER_LIGHT, CVI_GPIO_VALUE_H);
    usleep(10 * 1000);
    //ctrl reset
    CVI_GPIO_Set_Value(REST_LIGHT, CVI_GPIO_VALUE_H);
    usleep(10 * 1000);
    CVI_GPIO_Set_Value(REST_LIGHT, CVI_GPIO_VALUE_L);
    usleep(10 * 1000);
    CVI_GPIO_Set_Value(REST_LIGHT, CVI_GPIO_VALUE_H);
    usleep(RESET_DELAY);
}

/**Use GPIO high_low level must be set GPIO pin reuse*/
/**Use PWM. must set Pin reuse and PWM config*/
static void HAL_SCREEN_LumaInit(void)
{
}

static int32_t HAL_SCREEN_CmdInit(int32_t devno)
{
	return 0;
}

static int32_t HAL_SCREEN_ST7789P3SPI_Init(void)
{
    HAL_SCREEN_Reset();
    HAL_SCREEN_LumaInit();
    g_halScreenSt7789p3Spi = true;
    return 0;
}

static int32_t HAL_SCREEN_ST7789P3SPI_GetAttr(CVI_HAL_SCREEN_ATTR_S* pstAttr)
{
    pstAttr->enType = SCREEN_TYPE;

    /* these magic value are given from screen attribute */
    pstAttr->stAttr.u32Width = 320;
    pstAttr->stAttr.u32Height = 240;

	pstAttr->stAttr.u32Framerate = 60;

    return 0;
}

static int32_t HAL_SCREEN_ST7789P3SPI_GetDisplayState(CVI_HAL_SCREEN_STATE_E* penDisplayState)
{
    *penDisplayState = g_halScreenSt7789p3Spi;
    return 0;
}

static int32_t HAL_SCREEN_ST7789P3SPI_SetDisplayState(CVI_HAL_SCREEN_STATE_E enDisplayState)
{
    g_halScreenSt7789p3Spi = enDisplayState;
    return 0;
}

CVI_HAL_SCREEN_PWM_S HAL_SCREEN_ST7789P3SPI_GetLuma(void)
{
	CVI_HAL_SCREEN_PWM_S hal_screen_pwm = {0};
	#if CONFIG_PWM_ON
	hal_screen_pwm.group = screen_bl.group;
	hal_screen_pwm.channel = screen_bl.channel;
	hal_screen_pwm.period = screen_bl.period;
	hal_screen_pwm.duty_cycle = screen_bl.duty_cycle;
	#endif
	return hal_screen_pwm;
}

static int32_t HAL_SCREEN_ST7789P3SPI_SetLuma(CVI_HAL_SCREEN_PWM_S pwmAttr)
{
	#if CONFIG_PWM_ON
	screen_bl.group = pwmAttr.group;
	screen_bl.channel = pwmAttr.channel;
	screen_bl.period = pwmAttr.period;
	screen_bl.duty_cycle = pwmAttr.duty_cycle;
	#endif
	return 0;
}

static int32_t HAL_SCREEN_ST7789P3SPI_GetSatuature(uint32_t* pu32Satuature)
{
	printf("HAL_SCREEN_ST7789P3SPI_GetSatuature = %d\n", *pu32Satuature);
    return 0;
}

static int32_t HAL_SCREEN_ST7789P3SPI_SetSatuature(uint32_t u32Satuature)
{
	printf("HAL_SCREEN_HX8394_SetSatuature = %d\n", u32Satuature);
    return 0;
}

static int32_t HAL_SCREEN_ST7789P3SPI_GetContrast(uint32_t* pu32Contrast)
{
	printf("HAL_SCREEN_HX8394_GetContrast = %d\n", *pu32Contrast);
    return 0;
}

static int32_t HAL_SCREEN_ST7789P3SPI_SetContrast(uint32_t u32Contrast)
{
	printf("HAL_SCREEN_HX8394_SetContrast = %d\n", u32Contrast);
    return 0;
}

static int32_t HAL_SCREEN_ST7789P3SPI_SetBackLightState(CVI_HAL_SCREEN_STATE_E enBackLightState)
{
	printf("HAL_SCREEN_ST7789P3SPI_SetBackLightState = %d\n", enBackLightState);
    //ctrl backlight pwm
	int32_t ret = 0;
	if (enBackLightState == CVI_HAL_SCREEN_STATE_ON) {
		#if CONFIG_PWM_ON
		ret = CVI_PWM_Init(screen_bl);
		if (ret != 0) {
			printf("HAL_SCREEN_SetBackLightState failed! \n");
		}
		#else
		(void)ret;

		CVI_GPIO_Set_Value(BACK_LIGHT, CVI_GPIO_VALUE_H);
		g_halScreenSt7789p3Spi = CVI_HAL_SCREEN_STATE_ON;
		#endif
	} else {
		#if CONFIG_PWM_ON
		ret = CVI_PWM_Deinit(screen_bl);
		if (ret != 0) {
			printf("HAL_SCREEN_SetBackLightState failed! \n");
		}
		#else
		(void)ret;
		CVI_GPIO_Set_Value(BACK_LIGHT, CVI_GPIO_VALUE_L);
		g_halScreenSt7789p3Spi = CVI_HAL_SCREEN_STATE_OFF;
		#endif
	}
    return 0;
}

static int32_t HAL_SCREEN_ST7789P3SPI_GetBackLightState(CVI_HAL_SCREEN_STATE_E* penBackLightState)
{
	*penBackLightState = g_halScreenSt7789p3Spi;
    printf("HAL_SCREEN_HX8394_GetBackLightState = %d\n", *penBackLightState);
    return 0;
}

static int32_t HAL_SCREEN_ST7789P3SPI_Deinit(void)
{
    //MIPIDSI_Deinit();
    return 0;
}

CVI_HAL_SCREEN_OBJ_S stHALSCREENObj =
{
    .pfnInit = HAL_SCREEN_ST7789P3SPI_Init,
    .pfnGetAttr = HAL_SCREEN_ST7789P3SPI_GetAttr,
    .pfnSetDisplayState = HAL_SCREEN_ST7789P3SPI_SetDisplayState,
    .pfnGetDisplayState = HAL_SCREEN_ST7789P3SPI_GetDisplayState,
    .pfnSetBackLightState = HAL_SCREEN_ST7789P3SPI_SetBackLightState,
    .pfnGetBackLightState = HAL_SCREEN_ST7789P3SPI_GetBackLightState,
    .pfnSetLuma = HAL_SCREEN_ST7789P3SPI_SetLuma,
    .pfnGetLuma = HAL_SCREEN_ST7789P3SPI_GetLuma,
    .pfnSetSaturature = HAL_SCREEN_ST7789P3SPI_SetSatuature,
    .pfnGetSaturature = HAL_SCREEN_ST7789P3SPI_GetSatuature,
    .pfnSetContrast = HAL_SCREEN_ST7789P3SPI_SetContrast,
    .pfnGetContrast = HAL_SCREEN_ST7789P3SPI_GetContrast,
    .pfnDeinit = HAL_SCREEN_ST7789P3SPI_Deinit,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
