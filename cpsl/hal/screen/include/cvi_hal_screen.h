#ifndef __CVI_HAL_SCREEN_H__
#define __CVI_HAL_SCREEN_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
//#include "cvi_hal_pwm.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_SCREEN */
/** @{ */  /** <!-- [HAL_SCREEN] */
#define CVI_HAL_SCREEN_LANE_MAX_NUM 5

/** Screen Index, Currently can support two screen used in same time*/
typedef enum cviHAL_SCREEN_IDX_E {
	CVI_HAL_SCREEN_IDX_0 = 0,
	CVI_HAL_SCREEN_IDX_1,
	CVI_HAL_SCREEN_IDX_BUTT
} CVI_HAL_SCREEN_IDX_E;

typedef enum cviHAL_SCREEN_TYPE_E {
	CVI_HAL_SCREEN_INTF_TYPE_LCD = 0, /**<RGB interface type, such as lcd_6bit lcd_8bit, parallel communication protocal*/
	CVI_HAL_SCREEN_INTF_TYPE_MIPI, /**<MIPI interface type, serial communication protocal*/
	CVI_HAL_SCREEN_INTF_TYPE_I80,
	CVI_HAL_SCREEN_INTF_TYPE_BUIT
} CVI_HAL_SCREEN_TYPE_E;

/** brief general interface screen type enum*/
typedef enum cviHAL_SCREEN_LCD_INTF_TYPE_E {
	CVI_HAL_SCREEN_LCD_INTF_6BIT = 0, /**<6bit intf type*/
	CVI_HAL_SCREEN_LCD_INTF_8BIT,
	CVI_HAL_SCREEN_LCD_INTF_16BIT,
	CVI_HAL_SCREEN_LCD_INTF_24BIT,
	CVI_HAL_SCREEN_LCD_INTF_BUIT
} CVI_HAL_SCREEN_LCD_INTF_TYPE_E;

/* @brief mipi screen packet transport type*/
typedef enum cviHAL_SCREEN_MIPI_OUTPUT_TYPE_E {
	CVI_HAL_SCREEN_MIPI_OUTPUT_TYPE_CMD = 0x0,
	CVI_HAL_SCREEN_MIPI_OUTPUT_TYPE_VIDEO,
	CVI_HAL_SCREEN_MIPI_OUTPUT_TYPE_BUIT
} CVI_HAL_SCREEN_MIPI_OUTPUT_TYPE_E;

/* @brief mipi screen video mode type enum*/
typedef enum cviHAL_SCREEN_MIPI_VIDEO_MODE_E {
	CVI_HAL_SCREEN_MIPI_VIDEO_MODE_BURST = 0x0,/**<Burst Mode*/
	CVI_HAL_SCREEN_MIPI_VIDEO_MODE_PULSES,/**<Non-Burst Mode with Sync Pulses*/
	CVI_HAL_SCREEN_MIPI_VIDEO_MODE_EVENTS,/**<Non-Burst Mode with Sync Events*/
	CVI_HAL_SCREEN_MIPI_VIDEO_MODE_BUIT
} CVI_HAL_SCREEN_MIPI_VIDEO_MODE_E;

/* @brief mipi screen video format type enum*/
typedef enum cviHAL_SCREEN_MIPI_VIDEO_FORMAT_E {
	CVI_HAL_SCREEN_MIPI_VIDEO_RGB_16BIT = 0x0,
	CVI_HAL_SCREEN_MIPI_VIDEO_RGB_18BIT,
	CVI_HAL_SCREEN_MIPI_VIDEO_RGB_24BIT,
	CVI_HAL_SCREEN_MIPI_VIDEO_BUIT
} CVI_HAL_SCREEN_MIPI_VIDEO_FORMAT_E;

/** screen sync info*/
typedef struct cviHAL_SCREEN_SYNC_ATTR_S {
	uint16_t   u16Vact ;  /* vertical active area */
	uint16_t   u16Vbb;    /* vertical back blank porch */
	uint16_t   u16Vfb;    /* vertical front blank porch */
	uint16_t   u16Hact;   /* herizontal active area */
	uint16_t   u16Hbb;    /* herizontal back blank porch */
	uint16_t   u16Hfb;    /* herizontal front blank porch */

	uint16_t   u16Hpw;    /* horizontal pulse width */
	uint16_t   u16Vpw;    /* vertical pulse width */

	bool bIdv;/**< data Level polarity,0 mean high level valid,default 0,can not config*/
	bool bIhs;/**< horizon Level polarity,0 mean high level valid*/
	bool bIvs;/**< vertical Level polarity,0 mean high level valid*/
} CVI_HAL_SCREEN_SYNC_ATTR_S;

/** vo clk type*/
typedef enum cviHAL_SCREEN_CLK_TYPE_E {
	CVI_HAL_SCREEN_CLK_TYPE_PLL = 0x0,
	CVI_HAL_SCREEN_CLK_TYPE_LCDMCLK,
	CVI_HAL_SCREEN_CLK_TYPE_BUTT
} CVI_HAL_SCREEN_CLK_TYPE_E;

typedef struct cviHAL_SCREEN_CLK_PLL_S {
	uint32_t  u32Fbdiv;
	uint32_t  u32Frac;
	uint32_t  u32Refdiv;
	uint32_t  u32Postdiv1;
	uint32_t  u32Postdiv2;
} CVI_HAL_SCREEN_CLK_PLL_S;

typedef struct cviHAL_SCREEN_CLK_ATTR_S {
	bool bClkReverse; /**< vo clock reverse or not, if screen datasheet not mentioned, the value is true */
	uint32_t u32DevDiv;    /**< vo clock division factor, RGB6&8 is 3,RGB16&18 is 1, MIPI DSI is 1 */
	CVI_HAL_SCREEN_CLK_TYPE_E enClkType;  /**< vo clk type, pll or lcdmlck*/
	union {
		CVI_HAL_SCREEN_CLK_PLL_S stClkPll;
		uint32_t u32OutClk;    /**< for serial:(vbp+vact+vfp+u16Vpw)*(hbp+hact+hfp+u16hpw)*fps*total_clk_per_pixel,
                                    for parallel:(vbp+vact+vfp+u16Vpw)*(hbp+hact+hfp+u16hpw)*fps */
	};
} CVI_HAL_SCREEN_CLK_ATTR_S;

/** screen common attr*/
typedef struct cviHAL_SCREEN_COMMON_ATTR_S {
	CVI_HAL_SCREEN_SYNC_ATTR_S stSynAttr;/**<screen sync attr*/
	uint32_t u32Width;
	uint32_t u32Height;
	uint32_t u32Framerate;
} CVI_HAL_SCREEN_COMMON_ATTR_S;

/** screen mipi attr*/
typedef struct cviHAL_SCREEN_MIPI_ATTR_S {
	CVI_HAL_SCREEN_MIPI_OUTPUT_TYPE_E enType;
	CVI_HAL_SCREEN_MIPI_VIDEO_MODE_E enMode;
	CVI_HAL_SCREEN_MIPI_VIDEO_FORMAT_E enVideoFormat;
	int8_t as8LaneId[CVI_HAL_SCREEN_LANE_MAX_NUM];/**<lane use: value is index from zero start,lane not use:value is -1 */
	uint32_t u32PhyDataRate;  /**<mbps* (vbp+vact+vfp+u16Vpw)*(hbp+hact+hfp+u16hpw)*total_bit_per_pixel/lane_num/100000 */
	uint32_t u32PixelClk;  /**<KHz* (vbp+vact+vfp+u16Vpw)*(hbp+hact+hfp+u16hpw)*fps/1000/*/

} CVI_HAL_SCREEN_MIPI_ATTR_S;

typedef enum cviHAL_LCD_MUX_E {
	CVI_HAL_LCD_MUX_B_0 = 0,
	CVI_HAL_LCD_MUX_B_1,
	CVI_HAL_LCD_MUX_B_2,
	CVI_HAL_LCD_MUX_B_3,
	CVI_HAL_LCD_MUX_B_4,
	CVI_HAL_LCD_MUX_B_5,
	CVI_HAL_LCD_MUX_B_6,
	CVI_HAL_LCD_MUX_B_7,
	CVI_HAL_LCD_MUX_G_0,
	CVI_HAL_LCD_MUX_G_1,
	CVI_HAL_LCD_MUX_G_2,
	CVI_HAL_LCD_MUX_G_3,
	CVI_HAL_LCD_MUX_G_4,
	CVI_HAL_LCD_MUX_G_5,
	CVI_HAL_LCD_MUX_G_6,
	CVI_HAL_LCD_MUX_G_7,
	CVI_HAL_LCD_MUX_R_0,
	CVI_HAL_LCD_MUX_R_1,
	CVI_HAL_LCD_MUX_R_2,
	CVI_HAL_LCD_MUX_R_3,
	CVI_HAL_LCD_MUX_R_4,
	CVI_HAL_LCD_MUX_R_5,
	CVI_HAL_LCD_MUX_R_6,
	CVI_HAL_LCD_MUX_R_7,
	CVI_HAL_LCD_MUX_VS,
	CVI_HAL_LCD_MUX_HS,
	CVI_HAL_LCD_MUX_HDE,
	CVI_HAL_LCD_MUX_MAX
} CVI_HAL_LCD_MUX_E;

typedef enum cviHAL_LCD_SEL_E {
	CVI_HAL_VIVO_D0 = 15,
	CVI_HAL_VIVO_D1 = 16,
	CVI_HAL_VIVO_D2 = 17,
	CVI_HAL_VIVO_D3 = 18,
	CVI_HAL_VIVO_D4 = 19,
	CVI_HAL_VIVO_D5 = 20,
	CVI_HAL_VIVO_D6 = 21,
	CVI_HAL_VIVO_D7 = 22,
	CVI_HAL_VIVO_D8 = 23,
	CVI_HAL_VIVO_D9 = 24,
	CVI_HAL_VIVO_D10 = 25,
	CVI_HAL_VIVO_CLK = 1,
	CVI_HAL_MIPI_TXM4 = 26,
	CVI_HAL_MIPI_TXP4 = 27,
	CVI_HAL_MIPI_TXM3 = 28,
	CVI_HAL_MIPI_TXP3 = 29,
	CVI_HAL_MIPI_TXM2 = 2,
	CVI_HAL_MIPI_TXP2 = 0,
	CVI_HAL_MIPI_TXM1 = 4,
	CVI_HAL_MIPI_TXP1 = 3,
	CVI_HAL_MIPI_TXM0 = 6,
	CVI_HAL_MIPI_TXP0 = 5,
	CVI_HAL_MIPI_RXN5 = 14,
	CVI_HAL_MIPI_RXP5 = 13,
	CVI_HAL_MIPI_RXN2 = 12,
	CVI_HAL_MIPI_RXP2 = 11,
	CVI_HAL_MIPI_RXN1 = 10,
	CVI_HAL_MIPI_RXP1 = 9,
	CVI_HAL_MIPI_RXN0 = 8,
	CVI_HAL_MIPI_RXP0 = 7,
	CVI_HAL_LCD_PAD_MAX = 30
} CVI_HAL_LCD_SEL_E;

typedef enum _VO_MUX_G {
	CVI_HAL_VO_MUX_BT_VS = 0,
	CVI_HAL_VO_MUX_BT_HS,
	CVI_HAL_CVI_HAL_VO_MUX_BT_HDE,
	CVI_HAL_VO_MUX_BT_DATA0,
	CVI_HAL_VO_MUX_BT_DATA1,
	CVI_HAL_VO_MUX_BT_DATA2,
	CVI_HAL_VO_MUX_BT_DATA3,
	CVI_HAL_VO_MUX_BT_DATA4,
	CVI_HAL_VO_MUX_BT_DATA5,
	CVI_HAL_VO_MUX_BT_DATA6,
	CVI_HAL_VO_MUX_BT_DATA7,
	CVI_HAL_VO_MUX_BT_DATA8,
	CVI_HAL_VO_MUX_BT_DATA9,
	CVI_HAL_VO_MUX_BT_DATA10,
	CVI_HAL_VO_MUX_BT_DATA11,
	CVI_HAL_VO_MUX_BT_DATA12,
	CVI_HAL_VO_MUX_BT_DATA13,
	CVI_HAL_VO_MUX_BT_DATA14,
	CVI_HAL_VO_MUX_BT_DATA15,
	CVI_HAL_VO_MUX_MCU_CTRL0 = 0,
	CVI_HAL_VO_MUX_MCU_CTRL1,
	CVI_HAL_VO_MUX_MCU_CTRL2,
	CVI_HAL_VO_MUX_MCU_CTRL3,
	CVI_HAL_VO_MUX_MCU_DATA0,
	CVI_HAL_VO_MUX_MCU_DATA1,
	CVI_HAL_VO_MUX_MCU_DATA2,
	CVI_HAL_VO_MUX_MCU_DATA3,
	CVI_HAL_VO_MUX_MCU_DATA4,
	CVI_HAL_VO_MUX_MCU_DATA5,
	CVI_HAL_VO_MUX_MCU_DATA6,
	CVI_HAL_VO_MUX_MCU_DATA7,
	CVI_HAL_VO_MUX_MCU_DATA8,
	CVI_HAL_VO_MUX_RGB_0 = 0,
	CVI_HAL_VO_MUX_RGB_1,
	CVI_HAL_VO_MUX_RGB_2,
	CVI_HAL_VO_MUX_RGB_3,
	CVI_HAL_VO_MUX_RGB_4,
	CVI_HAL_VO_MUX_RGB_5,
	CVI_HAL_VO_MUX_RGB_6,
	CVI_HAL_VO_MUX_RGB_7,
	CVI_HAL_VO_MUX_RGB_8,
	CVI_HAL_VO_MUX_RGB_9,
	CVI_HAL_VO_MUX_RGB_10,
	CVI_HAL_VO_MUX_RGB_11,
	CVI_HAL_VO_MUX_RGB_12,
	CVI_HAL_VO_MUX_RGB_13,
	CVI_HAL_VO_MUX_RGB_14,
	CVI_HAL_VO_MUX_RGB_15,
	CVI_HAL_VO_MUX_RGB_16,
	CVI_HAL_VO_MUX_RGB_17,
	CVI_HAL_VO_MUX_RGB_18,
	CVI_HAL_VO_MUX_RGB_19,
	CVI_HAL_VO_MUX_RGB_20,
	CVI_HAL_VO_MUX_RGB_21,
	CVI_HAL_VO_MUX_RGB_22,
	CVI_HAL_VO_MUX_RGB_23,
	CVI_HAL_VO_MUX_RGB_VS,
	CVI_HAL_VO_MUX_RGB_HS,
	CVI_HAL_VO_MUX_RGB_HDE,
	CVI_HAL_VO_MUX_TG_HS_TILE = 30,
	CVI_HAL_VO_MUX_TG_VS_TILE,
	CVI_HAL_VO_MUX_MAX,
} VO_MUX_G;

typedef enum cviHAL_LCD_FORMAT_E {
	CVI_HAL_LCD_FORMAT_RGB565 = 0,
	CVI_HAL_LCD_FORMAT_RGB666,
	CVI_HAL_LCD_FORMAT_RGB888,
	CVI_HAL_LCD_FORMAT_MAX
} CVI_HAL_LCD_FORMAT_E;

typedef struct cviHAL_LCD_D_REMAP_E {
	CVI_HAL_LCD_MUX_E mux;
	CVI_HAL_LCD_SEL_E sel;
} CVI_HAL_LCD_REMAP_S;

/* LCD's config*/
typedef struct cviHAL_SCREEN_LCD_CFG_S {
	uint32_t pixelclock;
	CVI_HAL_LCD_FORMAT_E fmt;
	uint16_t cycle_time;
	CVI_HAL_LCD_REMAP_S remap[CVI_HAL_LCD_PAD_MAX];
	uint8_t gpio_num;
} CVI_HAL_SCREEN_LCD_CFG_S;

/** screen lcd attr*/
typedef struct cviHAL_SCREEN_LCD_ATTR_S {
	CVI_HAL_SCREEN_LCD_INTF_TYPE_E enType;
	CVI_HAL_SCREEN_LCD_CFG_S stLcdCfg;
} CVI_HAL_SCREEN_LCD_ATTR_S;

typedef struct cviHAL_I80_D_REMAP_E {
	VO_MUX_G mux;
	CVI_HAL_LCD_SEL_E sel;
} CVI_HAL_I80_REMAP_S;

/* LCD's config*/
typedef struct cviHAL_SCREEN_I80_CFG_S {
	CVI_HAL_LCD_FORMAT_E fmt;
	uint16_t cycle_time;
	CVI_HAL_I80_REMAP_S remap[CVI_HAL_LCD_PAD_MAX];
	uint8_t pin_num;
} CVI_HAL_SCREEN_I80_CFG_S;

/** screen lcd attr*/
typedef struct cviHAL_SCREEN_I80_ATTR_S {
	CVI_HAL_SCREEN_LCD_INTF_TYPE_E enType;
	CVI_HAL_SCREEN_I80_CFG_S stI80Cfg;
} CVI_HAL_SCREEN_I80_ATTR_S;

/** screen attr*/
typedef struct cviHAL_SCREEN_ATTR_S {
	CVI_HAL_SCREEN_TYPE_E enType;
	union tagHAL_SCREEN_ATTR_U {
		CVI_HAL_SCREEN_LCD_ATTR_S stLcdAttr;
		CVI_HAL_SCREEN_MIPI_ATTR_S stMipiAttr;
		CVI_HAL_SCREEN_I80_ATTR_S stI80Attr;
	} unScreenAttr;
	CVI_HAL_SCREEN_COMMON_ATTR_S stAttr;
} CVI_HAL_SCREEN_ATTR_S;

typedef struct cviHAL_SCREEN_PWM_S {
	uint8_t group;       //组号
	uint8_t channel;      //通道号
	uint32_t period;      //周期
	uint32_t duty_cycle;  //占空比
} CVI_HAL_SCREEN_PWM_S;

/* @brief screen status enum*/
typedef enum cviHAL_SCREEN_STATE_E {
	CVI_HAL_SCREEN_STATE_OFF = 0,/**<screen off*/
	CVI_HAL_SCREEN_STATE_ON,     /**<screen on*/
	CVI_HAL_SCREEN_STATE_BUIT
} CVI_HAL_SCREEN_STATE_E;

typedef struct cviHAL_SCREEN_OBJ_S {
	int32_t (*pfnInit)(void);
	int32_t (*pfnGetAttr)(CVI_HAL_SCREEN_ATTR_S *pstAttr);
	int32_t (*pfnGetDisplayState)(CVI_HAL_SCREEN_STATE_E *penDisplayState);
	int32_t (*pfnSetDisplayState)(CVI_HAL_SCREEN_STATE_E enDisplayState);
	int32_t (*pfnGetBackLightState)(CVI_HAL_SCREEN_STATE_E *penBackLightState);
	int32_t (*pfnSetBackLightState)(CVI_HAL_SCREEN_STATE_E enBackLightState);
	CVI_HAL_SCREEN_PWM_S (*pfnGetLuma)(void);
	int32_t (*pfnSetLuma)(CVI_HAL_SCREEN_PWM_S pwmAttr);
	int32_t (*pfnGetSaturature)(uint32_t *pu32Satuature);
	int32_t (*pfnSetSaturature)(uint32_t u32Satuature);
	int32_t (*pfnGetContrast)(uint32_t *pu32Contrast);
	int32_t (*pfnSetContrast)(uint32_t u32Contrast);
	int32_t (*pfnDeinit)(void);
} CVI_HAL_SCREEN_OBJ_S;

int32_t CVI_HAL_SCREEN_Register(CVI_HAL_SCREEN_IDX_E enScreenIndex, const CVI_HAL_SCREEN_OBJ_S *pstScreenObj);
int32_t CVI_HAL_SCREEN_Init(CVI_HAL_SCREEN_IDX_E enScreenIndex);
int32_t CVI_HAL_SCREEN_GetAttr(CVI_HAL_SCREEN_IDX_E enScreenIndex, CVI_HAL_SCREEN_ATTR_S *pstAttr);
int32_t CVI_HAL_SCREEN_GetDisplayState(CVI_HAL_SCREEN_IDX_E enScreenIndex, CVI_HAL_SCREEN_STATE_E *penDisplayState);
int32_t CVI_HAL_SCREEN_SetDisplayState(CVI_HAL_SCREEN_IDX_E enScreenIndex, CVI_HAL_SCREEN_STATE_E enDisplayState);
int32_t CVI_HAL_SCREEN_GetBackLightState(CVI_HAL_SCREEN_IDX_E enScreenIndex, CVI_HAL_SCREEN_STATE_E *penBackLightState);
int32_t CVI_HAL_SCREEN_SetBackLightState(CVI_HAL_SCREEN_IDX_E enScreenIndex, CVI_HAL_SCREEN_STATE_E enBackLightState);
CVI_HAL_SCREEN_PWM_S CVI_HAL_SCREEN_GetLuma(CVI_HAL_SCREEN_IDX_E enScreenIndex);
int32_t CVI_HAL_SCREEN_SetLuma(CVI_HAL_SCREEN_IDX_E enScreenIndex, CVI_HAL_SCREEN_PWM_S pwmAttr);
int32_t CVI_HAL_SCREEN_GetSaturature(CVI_HAL_SCREEN_IDX_E enScreenIndex, uint32_t *pu32Satuature);
int32_t CVI_HAL_SCREEN_SetSaturature(CVI_HAL_SCREEN_IDX_E enScreenIndex, uint32_t u32Saturature);
int32_t CVI_HAL_SCREEN_GetContrast(CVI_HAL_SCREEN_IDX_E enScreenIndex, uint32_t *pu32Contrast);
int32_t CVI_HAL_SCREEN_SetContrast(CVI_HAL_SCREEN_IDX_E enScreenIndex, uint32_t u32Contrast);
int32_t CVI_HAL_SCREEN_Deinit(CVI_HAL_SCREEN_IDX_E enScreenIndex);

 extern CVI_HAL_SCREEN_OBJ_S stHALSCREENObj;

/** @}*/  /** <!-- ==== HAL_SCREEN End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_HAL_SCREEN_H__*/

