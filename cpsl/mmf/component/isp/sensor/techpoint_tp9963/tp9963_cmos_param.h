#ifndef __TP9963_CMOS_PARAM_H_
#define __TP9963_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
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
#include "tp9963_cmos_ex.h"

static const TP9963_MODE_S g_astTP9963_mode[TP9963_MODE_NUM] = {
	[TP9963_MODE_1080P_60P] = {
		.name = "1080p30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
		},
	},
	[TP9963_MODE_1080P_30P] = {
		.name = "1080p30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
		},
	},
	[TP9963_MODE_1080P_30P_2CH] = {
		.name = "1080p30_2ch",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
		},
	},
	[TP9963_MODE_1080P_25P] = {
		.name = "1080p25",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
		},
	},
	[TP9963_MODE_1080P_25P_2CH] = {
		.name = "1080p25_2ch",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
		},
	},
	[TP9963_MODE_720P_30P] = {
		.name = "720p30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stMaxSize = {
				.u32Width = 1280,
				.u32Height = 720,
			},
		},
	},
	[TP9963_MODE_720P_30P_2CH] = {
		.name = "720p30_2ch",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stMaxSize = {
				.u32Width = 1280,
				.u32Height = 720,
			},
		},
	},
	[TP9963_MODE_720P_25P_2CH] = {
		.name = "720p25_2ch",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stMaxSize = {
				.u32Width = 1280,
				.u32Height = 720,
			},
		},
	},
};

struct combo_dev_attr_s tp9963_multi_rx_attr = {
	// .input_mode = INPUT_MODE_MIPI,
	// // .mac_clk = RX_MAC_CLK_400M,
	// .mac_clk = RX_MAC_CLK_600M,
	// .mipi_attr = {
	// 	.raw_data_type = YUV422_8BIT,
	// 	.lane_id = {0, 1, 2, -1, -1},
	// 	.pn_swap = {1, 1, 1, 1, 1},
	// 	.wdr_mode = CVI_MIPI_WDR_MODE_VC,
	// 	.demux = {
	// 		.demux_en = 1,
	// 		.vc_mapping = {0, 1, 2, 3},
	// 	},
	// 	.dphy = {
	// 		.enable = 0,
	// 		.hs_settle = 8,
	// 	}
	// },
	// .mclk = {
	// 	.cam = 0,
	// 	// .freq = CAMPLL_FREQ_NONE,
	// 	.freq = CAMPLL_FREQ_27M,
	// },
	// .devno = 0,
		.input_mode = INPUT_MODE_BT656_9B,
	.mac_clk = RX_MAC_CLK_200M,
	.mclk = {
		.cam = 1,
		.freq = CAMPLL_FREQ_27M,
	},
	.ttl_attr = {
		.vi = TTL_VI_SRC_VI1,
		.func = {
			-1, -1, -1, -1,
			2, 3, 4, 5, 6, 7, 8, 9,
			-1, -1, -1, -1,
			-1, -1, -1, -1,
		},
	},
	.devno = 1,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __TP9963_CMOS_PARAM_H_ */
