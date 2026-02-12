#ifndef __HAL_SCREEN_H__
#define __HAL_SCREEN_H__

// #include "cvi_type.h"
// #include "cvi_mipi_tx.h"
#include "cvi_comm_mipi_tx.h"
#include "cvi_hal_gpio.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define HGJ_BOARD
#define HORIZONTAL_SYNC_ACTIVE      100//8
#define HORIZONTAL_BACK_PROCH       100//58
#define HORIZONTAL_FRONT_PROCH      100//58
#define HORIZONTAL_ACTIVE           600
#define VERTICAL_SYNC_ACTIVE        4
#define VERTICAL_BACK_PROCH         16
#define VERTICAL_FRONT_PROCH        16
#define VERTICAL_ACTIVE             1600
#define HORIZONTAL_SYNC_POLIRATY    0
#define VERTICAL_SYNC_POLIRATY      0

#define BACK_LIGHT	CVI_GPIOE_00
#ifdef HGJ_BOARD
#define REST_LIGHT	CVI_GPIOA_15//CVI_GPIOB_01//CVI_GPIOA_20
#define POWER_LIGHT	CVI_GPIOA_19//CVI_GPIOA_30
#else
#define REST_LIGHT	CVI_GPIOA_15
#define POWER_LIGHT	CVI_GPIOA_30
#endif


#define  SCREEN_TYPE  CVI_HAL_SCREEN_INTF_TYPE_MIPI;

#define RESET_DELAY (10 * 1000)

static const struct combo_dev_cfg_s dev_cfg = {
	.devno = 0,
	.lane_id = {MIPI_TX_LANE_CLK, MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_2, MIPI_TX_LANE_3},
	.lane_pn_swap = {false, false, false, false, false},
	.output_mode = OUTPUT_MODE_DSI_VIDEO,
	.video_mode = BURST_MODE,
	.output_format = OUT_FORMAT_RGB_24_BIT,
	.sync_info = {
		.vid_hsa_pixels = HORIZONTAL_SYNC_ACTIVE,
		.vid_hbp_pixels = HORIZONTAL_BACK_PROCH,
		.vid_hfp_pixels = HORIZONTAL_FRONT_PROCH,
		.vid_hline_pixels = HORIZONTAL_ACTIVE,
		.vid_vsa_lines = VERTICAL_SYNC_ACTIVE,
		.vid_vbp_lines = VERTICAL_BACK_PROCH,
		.vid_vfp_lines = VERTICAL_FRONT_PROCH,
		.vid_active_lines = VERTICAL_ACTIVE,
		.vid_vsa_pos_polarity = HORIZONTAL_SYNC_POLIRATY,
		.vid_hsa_pos_polarity = VERTICAL_SYNC_POLIRATY,
	},
	.pixel_clk = 68712,
};

//pixel_clk=(htotal*vtotal)*fps/1000
//htotal=vid_hsa_pixels+ vid_hbp_pixels+ vid_hfp_pixels+ vid_hline_pixels
//vtotal= vid_vsa_lines+ vid_vbp_lines+ vid_vfp_lines+ vid_active_lines
//fps = 60


static unsigned char data_fl7707n_0[] = { 0xB9, 0xF1, 0x12, 0x87 };

static unsigned char data_fl7707n_1[] = { 0xB2, 0x90, 0x05, 0x78 };

static unsigned char data_fl7707n_2[] = {
	0xB3, 0x10, 0x10, 0x28, 0x28, 0x03, 0xFF, 0x00, 0x00, 0x00,
	0x00
};

static unsigned char data_fl7707n_3[] = { 0xB4, 0x80 };

static unsigned char data_fl7707n_4[] = { 0xB5, 0x0C, 0x0C };

static unsigned char data_fl7707n_5[] = { 0xB6, 0x6C, 0x6C };

static unsigned char data_fl7707n_6[] = { 0xB8, 0x26, 0x22, 0xF0, 0x13 };

static unsigned char data_fl7707n_7[] = {
	0xBA, 0x33, 0x81, 0x05, 0xF9, 0x0E, 0x0E, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x25, 0x00, 0x91, 0x0A,
	0x00, 0x00, 0x01, 0x4F, 0x01, 0x00, 0x00, 0x37
};

static unsigned char data_fl7707n_8[] = { 0xBC, 0x47 };

static unsigned char data_fl7707n_9[] = { 0xBF, 0x02, 0x10, 0x00, 0x80, 0x04 };

static unsigned char data_fl7707n_10[] = {
	0xC0, 0x73, 0x73, 0x50, 0x50, 0x00, 0x00, 0x12, 0x73, 0x00
};

static unsigned char data_fl7707n_11[] = {
	0xC1, 0x65, 0xC0, 0x32, 0x32, 0x77, 0xF4, 0x77, 0x77, 0xCC,
	0xCC, 0xFF, 0xFF, 0x11, 0x11, 0x00, 0x00, 0x32
};
static unsigned char data_fl7707n_12[] = {
	0xC7, 0x10, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0xED,
	0xC7, 0x00, 0xA5
};
static unsigned char data_fl7707n_13[] = { 0xC8, 0x10, 0x40, 0x1E, 0x03 };

static unsigned char data_fl7707n_14[] = { 0xCC, 0x0B };

static unsigned char data_fl7707n_15[] = {
	0xE0, 0x00, 0x0B, 0x11, 0x2D, 0x31, 0x38, 0x48, 0x40, 0x07,
	0x0C, 0x0D, 0x12, 0x15, 0x11, 0x13, 0x11, 0x19, 0x00, 0x0B,
	0x11, 0x2D, 0x31, 0x38, 0x48, 0x40, 0x07, 0x0C, 0x0D, 0x12,
	0x15, 0x11, 0x13, 0x11, 0x19
};
static unsigned char data_fl7707n_16[] = { 0xE1, 0x11, 0x11, 0x91, 0x00, 0x00, 0x00, 0x00 };

static unsigned char data_fl7707n_17[] = {
	0xE3, 0x07, 0x07, 0x0B, 0x0B, 0x0B, 0x0B, 0x00, 0x00, 0x00,
	0x00, 0xFF, 0x84, 0xC0, 0x10
};

static unsigned char data_fl7707n_18[] = {
	0xE9, 0xC8, 0x10, 0x07, 0x10, 0x0B, 0x80, 0x38, 0x12, 0x31,
	0x23, 0x4F, 0x86, 0x80, 0x38, 0x47, 0x08, 0x00, 0xC0, 0x04,
	0x00, 0x00, 0x00, 0x00, 0xC0, 0x04, 0x00, 0x00, 0x00, 0xF8,
	0x48, 0xF8, 0x11, 0x33, 0x55, 0x77, 0x31, 0x88, 0x88, 0x88,
	0xF8, 0x48, 0xF8, 0x00, 0x22, 0x44, 0x66, 0x20, 0x88, 0x88,
	0x88, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};
static unsigned char data_fl7707n_19[] = {
	0xEA, 0x97, 0x12, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x8F, 0x48, 0xF8, 0x66, 0x44, 0x22, 0x00,
	0x02, 0x88, 0x88, 0x88, 0x8F, 0x48, 0xF8, 0x77, 0x55, 0x33,
	0x11, 0x13, 0x88, 0x88, 0x88, 0x23, 0x00, 0x00, 0x01, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x80, 0x38, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};
//static unsigned char data_fl7707n_20[] = { 0xEF, 0xFF, 0xFF, 0x01 };
static unsigned char data_fl7707n_21[] = { 0x11 };
static unsigned char data_fl7707n_22[] = { 0x29 };

static const struct dsc_instr dsi_init_cmds[] = {
	{.delay = 0, .data_type = 0x29, .size = 4, .data = data_fl7707n_0 },
	{.delay = 0, .data_type = 0x29, .size = 4, .data = data_fl7707n_1 },
	{.delay = 0, .data_type = 0x29, .size = 11, .data = data_fl7707n_2 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_fl7707n_3 },
	{.delay = 0, .data_type = 0x29, .size = 3, .data = data_fl7707n_4 },
	{.delay = 0, .data_type = 0x29, .size = 3, .data = data_fl7707n_5 },
	{.delay = 0, .data_type = 0x29, .size = 5, .data = data_fl7707n_6 },
	{.delay = 0, .data_type = 0x29, .size = 28, .data = data_fl7707n_7 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_fl7707n_8 },
	{.delay = 0, .data_type = 0x29, .size = 6, .data = data_fl7707n_9 },
	{.delay = 0, .data_type = 0x29, .size = 10, .data = data_fl7707n_10 },
	{.delay = 0, .data_type = 0x29, .size = 18, .data = data_fl7707n_11 },
	{.delay = 0, .data_type = 0x29, .size = 13, .data = data_fl7707n_12 },
	{.delay = 0, .data_type = 0x29, .size = 5, .data = data_fl7707n_13 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_fl7707n_14 },
	{.delay = 0, .data_type = 0x29, .size = 35, .data = data_fl7707n_15 },
	{.delay = 0, .data_type = 0x29, .size = 8, .data = data_fl7707n_16 },
	{.delay = 0, .data_type = 0x29, .size = 15, .data = data_fl7707n_17 },
	{.delay = 0, .data_type = 0x29, .size = 64, .data = data_fl7707n_18},
	{.delay = 0, .data_type = 0x29, .size = 64, .data = data_fl7707n_19},
	//{.delay = 0, .data_type = 0x29, .size = 4, .data = data_fl7707n_20 },
	{.delay = 250, .data_type = 0x05, .size = 1, .data = data_fl7707n_21 },
	{.delay = 50, .data_type = 0x05, .size = 1, .data = data_fl7707n_22}
};

static const struct hs_settle_s hs_timing_cfg = { .prepare = 6, .zero = 32, .trail = 1 };

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */

#endif /* __HAL_SCREEN_H__  */
