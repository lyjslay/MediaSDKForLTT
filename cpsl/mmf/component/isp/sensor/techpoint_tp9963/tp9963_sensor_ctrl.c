// #include <stdio.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <sys/ioctl.h>
// #include <syslog.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <linux/i2c.h>
// #include <linux/i2c-dev.h>
// #ifdef ARCH_CV182X
// #include <linux/cvi_vip_snsr.h>
// #include "cvi_comm_video.h"
// #else
// #include <linux/vi_snsr.h>
// #include <linux/cvi_comm_video.h>
// #endif
// #include "cvi_sns_ctrl.h"
#include <unistd.h>
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include "cvi_sensor.h"
#include "tp9963_cmos_ex.h"

#include <sys/types.h>
#include <sys/stat.h>

CVI_U8 tp9963_i2c_addr = 0x44; /* I2C slave address of TP9963, SA0=0:0x44, SA0=1:0x45*/
const CVI_U32 tp9963_addr_byte = 1;
const CVI_U32 tp9963_data_byte = 1;

#define AHD_PWDN_GRP 1
#define AHD_PWDN_NUM 5
csi_gpio_t g_irgpio = {0};
// static int g_fd[VI_MAX_PIPE_NUM] = {[0 ...(VI_MAX_PIPE_NUM - 1)] = -1};

#define TP9963_BLUE_SCREEN 0

enum
{
	CH_1 = 0,
	CH_2 = 1,
	CH_ALL = 4,
	MIPI_PAGE = 8,
};

enum
{
	STD_TVI, // TVI
	STD_HDA, // AHD
};

enum
{
	PAL,   //720x288i
	NTSC,  //720x240i
	HD25,  //1280x720
	HD30,  //1280x720
	FHD25, //1920x1080
	FHD30, //1920x1080
	QHD25, //2560x1440
	QHD30, //2560x1440
	QHD275,//2560X1440 27.5fps
	FHD50, //1920x1080
	FHD60, //1920x1080
	// 5M20,
	FHD275, //1080P27.5
	FHD28,//1920X1080, 2200X1205
	F_UVGA30,  //FH 1280x960p30, must use with MIPI_2CH2LANE_432M
	QHD25_4A10, //total
	FHD59, //1920x1080,Nonsatandard total 2200x1144
	FHD55,//1080P55, Nonstandard total 2400x1125
};

enum{
    MIPI_2CH2LANE_594M, //up to 2x1080p25/30
    MIPI_2CH2LANE_297M, //up to 2x720p25/30
    MIPI_2CH4LANE_594M, //up to 2x1080p50/60
    MIPI_2CH4LANE_297M, //up to 2x1080p25/30
    MIPI_2CH2LANE_432M, //only for 2xF_UVGA30
	MIPI_1CH2LANE_594M, //up to 1xQHD30
	MIPI_1CH2LANE_297M, //up to 1x1080P30
	MIPI_1CH4LANE_297M, //up to 1xQHD30
	MIPI_2CH4LANE_216M, //only for 2xF_UVGA30
};

int tp9963_gpio_init(VI_PIPE ViPipe)
{
	(void) ViPipe;

	// AHD_PWR_EN
    if (csi_gpio_init(&g_irgpio, AHD_PWDN_GRP) != CSI_OK) {
        printf("%s gpio init err \n",__func__);
        return CVI_FAILURE;
    }
	csi_gpio_dir(&g_irgpio, (1 << AHD_PWDN_NUM), GPIO_DIRECTION_OUTPUT);
	csi_gpio_write(&g_irgpio, (1 << AHD_PWDN_NUM), 1);
//	if (csi_gpio_read(&g_irgpio , (1 << IR_PWDN_NUM)) != 1){
//		return CVI_FAILURE;
//	}

	return CVI_SUCCESS;
}

int tp9963_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = g_aunTP9963_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_init(i2c_id);
}

void tp9963_exit(VI_PIPE ViPipe)
{

	CVI_U8 i2c_id = (CVI_U8)g_aunTP9963_BusInfo[ViPipe].s8I2cDev;

	sensor_i2c_exit(i2c_id);
}

int tp9963_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunTP9963_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_read(i2c_id, tp9963_i2c_addr, (CVI_U32)addr, tp9963_addr_byte, tp9963_data_byte);
}

int tp9963_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunTP9963_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_write(i2c_id, tp9963_i2c_addr, (CVI_U32)addr, tp9963_addr_byte,
		(CVI_U32)data, tp9963_data_byte);
}

void TP9963_decoder_init(VI_PIPE ViPipe, unsigned char ch, unsigned char fmt, unsigned char std)
{
	unsigned char tmp;
	const unsigned char REG42_43[]={0x01,0x02,0x00,0x00,0x03};
	const unsigned char MASK42_43[]={0xfe,0xfd,0xff,0xff,0xfc};

	// tp9963_write_register(ViPipe,0x40, ch);
	// tp9963_write_register(ViPipe,0x06, 0x12); //default value
	// tp9963_write_register(ViPipe,0x50, 0x00); //VIN1/3
	// tp9963_write_register(ViPipe,0x51, 0x00); //
	// tp9963_write_register(ViPipe,0x54, 0x03);
	if(PAL == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp |= REG42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);

		tp9963_write_register(ViPipe,0x06, 0x32);
		tp9963_write_register(ViPipe,0x02, 0x47);
		tp9963_write_register(ViPipe,0x07, 0x80);
		tp9963_write_register(ViPipe,0x0b, 0x80);
		tp9963_write_register(ViPipe,0x0c, 0x13);
		tp9963_write_register(ViPipe,0x0d, 0x51);
		tp9963_write_register(ViPipe,0x15, 0x03);
		tp9963_write_register(ViPipe,0x16, 0xf0);
		tp9963_write_register(ViPipe,0x17, 0xa0);
		tp9963_write_register(ViPipe,0x18, 0x17);
		tp9963_write_register(ViPipe,0x19, 0x20);
		tp9963_write_register(ViPipe,0x1a, 0x15);
		tp9963_write_register(ViPipe,0x1c, 0x06);
		tp9963_write_register(ViPipe,0x1d, 0xc0);
		tp9963_write_register(ViPipe,0x20, 0x48);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x37);
		tp9963_write_register(ViPipe,0x23, 0x3f);
		tp9963_write_register(ViPipe,0x2b, 0x70);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x4b);
		tp9963_write_register(ViPipe,0x2e, 0x56);
		tp9963_write_register(ViPipe,0x30, 0x7a);
		tp9963_write_register(ViPipe,0x31, 0x4a);
		tp9963_write_register(ViPipe,0x32, 0x4d);
		tp9963_write_register(ViPipe,0x33, 0xfb);
		tp9963_write_register(ViPipe,0x35, 0x65);
		tp9963_write_register(ViPipe,0x39, 0x04);
	}
	else if(NTSC == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp |= REG42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);
		tp9963_write_register(ViPipe,0x02, 0x47);
		tp9963_write_register(ViPipe,0x07, 0x80);
		tp9963_write_register(ViPipe,0x0b, 0x80);
		tp9963_write_register(ViPipe,0x0c, 0x13);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x03);
		tp9963_write_register(ViPipe,0x16, 0xd6);
		tp9963_write_register(ViPipe,0x17, 0xa0);
		tp9963_write_register(ViPipe,0x18, 0x12);
		tp9963_write_register(ViPipe,0x19, 0xf0);
		tp9963_write_register(ViPipe,0x1a, 0x05);
		tp9963_write_register(ViPipe,0x1c, 0x06);
		tp9963_write_register(ViPipe,0x1d, 0xb4);
		tp9963_write_register(ViPipe,0x20, 0x40);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x2b, 0x70);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x4b);
		tp9963_write_register(ViPipe,0x2e, 0x57);
		tp9963_write_register(ViPipe,0x30, 0x62);
		tp9963_write_register(ViPipe,0x31, 0xbb);
		tp9963_write_register(ViPipe,0x32, 0x96);
		tp9963_write_register(ViPipe,0x33, 0xcb);
		tp9963_write_register(ViPipe,0x35, 0x65);
		tp9963_write_register(ViPipe,0x39, 0x04);
	}
	else if(HD25 == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp |= REG42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);
		tp9963_write_register(ViPipe,0x02, 0x42);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x13);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x13);
		tp9963_write_register(ViPipe,0x16, 0x15);
		tp9963_write_register(ViPipe,0x17, 0x00);
		tp9963_write_register(ViPipe,0x18, 0x19);
		tp9963_write_register(ViPipe,0x19, 0xd0);
		tp9963_write_register(ViPipe,0x1a, 0x25);
		tp9963_write_register(ViPipe,0x1c, 0x07);  //1280*720, 25fps
		tp9963_write_register(ViPipe,0x1d, 0xbc);  //1280*720, 25fps
		tp9963_write_register(ViPipe,0x20, 0x30);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x30);
		tp9963_write_register(ViPipe,0x2e, 0x70);
		tp9963_write_register(ViPipe,0x30, 0x48);
		tp9963_write_register(ViPipe,0x31, 0xbb);
		tp9963_write_register(ViPipe,0x32, 0x2e);
		tp9963_write_register(ViPipe,0x33, 0x90);
		tp9963_write_register(ViPipe,0x35, 0x25);
		tp9963_write_register(ViPipe,0x39, 0x08);

		if(STD_HDA == std)
		{
    	tp9963_write_register(ViPipe,0x02, 0x46);

    	tp9963_write_register(ViPipe,0x0d, 0x71);
		tp9963_write_register(ViPipe,0x18, 0x1b);

    	tp9963_write_register(ViPipe,0x20, 0x40);
    	tp9963_write_register(ViPipe,0x21, 0x46);

    	tp9963_write_register(ViPipe,0x25, 0xfe);
    	tp9963_write_register(ViPipe,0x26, 0x01);

    	tp9963_write_register(ViPipe,0x2c, 0x3a);
    	tp9963_write_register(ViPipe,0x2d, 0x5a);
    	tp9963_write_register(ViPipe,0x2e, 0x40);

    	tp9963_write_register(ViPipe,0x30, 0x9e);
    	tp9963_write_register(ViPipe,0x31, 0x20);
    	tp9963_write_register(ViPipe,0x32, 0x10);
    	tp9963_write_register(ViPipe,0x33, 0x90);
		}
	}
	else if(HD30 == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp |= REG42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);
		tp9963_write_register(ViPipe,0x02, 0x42);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x13);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x13);
		tp9963_write_register(ViPipe,0x16, 0x15);
		tp9963_write_register(ViPipe,0x17, 0x00);
		tp9963_write_register(ViPipe,0x18, 0x19);
		tp9963_write_register(ViPipe,0x19, 0xd0);
		tp9963_write_register(ViPipe,0x1a, 0x25);
		tp9963_write_register(ViPipe,0x1c, 0x06);  //1280*720, 30fps
		tp9963_write_register(ViPipe,0x1d, 0x72);  //1280*720, 30fps
		tp9963_write_register(ViPipe,0x20, 0x30);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x30);
		tp9963_write_register(ViPipe,0x2e, 0x70);
		tp9963_write_register(ViPipe,0x30, 0x48);
		tp9963_write_register(ViPipe,0x31, 0xbb);
		tp9963_write_register(ViPipe,0x32, 0x2e);
		tp9963_write_register(ViPipe,0x33, 0x90);
		tp9963_write_register(ViPipe,0x35, 0x25);
		tp9963_write_register(ViPipe,0x39, 0x08);

		if(STD_HDA == std)
		{
    	tp9963_write_register(ViPipe,0x02, 0x46);

    	tp9963_write_register(ViPipe,0x0d, 0x70);
		tp9963_write_register(ViPipe,0x18, 0x1b);
    	tp9963_write_register(ViPipe,0x20, 0x40);
    	tp9963_write_register(ViPipe,0x21, 0x46);

    	tp9963_write_register(ViPipe,0x25, 0xfe);
    	tp9963_write_register(ViPipe,0x26, 0x01);

    	tp9963_write_register(ViPipe,0x2c, 0x3a);
    	tp9963_write_register(ViPipe,0x2d, 0x5a);
    	tp9963_write_register(ViPipe,0x2e, 0x40);

    	tp9963_write_register(ViPipe,0x30, 0x9d);
    	tp9963_write_register(ViPipe,0x31, 0xca);
    	tp9963_write_register(ViPipe,0x32, 0x01);
    	tp9963_write_register(ViPipe,0x33, 0xd0);
		}
	}
	else if(FHD30 == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);
		tp9963_write_register(ViPipe,0x02, 0x40);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x03);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x03);
		tp9963_write_register(ViPipe,0x16, 0xd2);
		tp9963_write_register(ViPipe,0x17, 0x80);
		tp9963_write_register(ViPipe,0x18, 0x29);
		tp9963_write_register(ViPipe,0x19, 0x38);
		tp9963_write_register(ViPipe,0x1a, 0x47);
		tp9963_write_register(ViPipe,0x1c, 0x08);  //1920*1080, 30fps
		tp9963_write_register(ViPipe,0x1d, 0x98);  //
		tp9963_write_register(ViPipe,0x20, 0x30);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x30);
		tp9963_write_register(ViPipe,0x2e, 0x70);
		tp9963_write_register(ViPipe,0x30, 0x48);
		tp9963_write_register(ViPipe,0x31, 0xbb);
		tp9963_write_register(ViPipe,0x32, 0x2e);
		tp9963_write_register(ViPipe,0x33, 0x90);
		tp9963_write_register(ViPipe,0x35, 0x05);
		tp9963_write_register(ViPipe,0x39, 0x0C);

		if(STD_HDA == std)
		{
			tp9963_write_register(ViPipe,0x02, 0x44);
			tp9963_write_register(ViPipe,0x0d, 0x72);
			tp9963_write_register(ViPipe,0x15, 0x01);
			tp9963_write_register(ViPipe,0x16, 0xf0);
			tp9963_write_register(ViPipe,0x18, 0x2a);
			tp9963_write_register(ViPipe,0x20, 0x38);
			tp9963_write_register(ViPipe,0x21, 0x46);
			tp9963_write_register(ViPipe,0x25, 0xfe);
			tp9963_write_register(ViPipe,0x26, 0x0d);
			tp9963_write_register(ViPipe,0x2c, 0x3a);
			tp9963_write_register(ViPipe,0x2d, 0x54);
			tp9963_write_register(ViPipe,0x2e, 0x40);
			tp9963_write_register(ViPipe,0x30, 0xa5);
			tp9963_write_register(ViPipe,0x31, 0x95);
			tp9963_write_register(ViPipe,0x32, 0xe0);
			tp9963_write_register(ViPipe,0x33, 0x60);
		}
	}
	else if(FHD25 == fmt)
	{
		printf("####### %s, %d\n", __func__, __LINE__);
		// 第一组寄存器写操作
tp9963_write_register(ViPipe,0x40, 0x04);
tp9963_write_register(ViPipe,0x02, 0xCC);
tp9963_write_register(ViPipe,0x07, 0xC0);
tp9963_write_register(ViPipe,0x0B, 0xC0);
tp9963_write_register(ViPipe,0x0C, 0x03);
tp9963_write_register(ViPipe,0x0D, 0x73);
tp9963_write_register(ViPipe,0x15, 0x01);
tp9963_write_register(ViPipe,0x16, 0xF0);
tp9963_write_register(ViPipe,0x17, 0x80);
tp9963_write_register(ViPipe,0x18, 0x2A);
tp9963_write_register(ViPipe,0x19, 0x38);
tp9963_write_register(ViPipe,0x1A, 0x47);
tp9963_write_register(ViPipe,0x1C, 0x0A);
tp9963_write_register(ViPipe,0x1D, 0x50);
tp9963_write_register(ViPipe,0x20, 0x3C);
tp9963_write_register(ViPipe,0x21, 0x46);
tp9963_write_register(ViPipe,0x22, 0x36);
tp9963_write_register(ViPipe,0x25, 0xFE);
tp9963_write_register(ViPipe,0x26, 0x0D);
tp9963_write_register(ViPipe,0x2B, 0x60);
tp9963_write_register(ViPipe,0x2C, 0x3A);
tp9963_write_register(ViPipe,0x2D, 0x54);
tp9963_write_register(ViPipe,0x2E, 0x40);
tp9963_write_register(ViPipe,0x30, 0xA5);
tp9963_write_register(ViPipe,0x31, 0x86);
tp9963_write_register(ViPipe,0x32, 0xFB);
tp9963_write_register(ViPipe,0x33, 0x60);
tp9963_write_register(ViPipe,0x38, 0x40);
tp9963_write_register(ViPipe,0x39, 0x0C);
tp9963_write_register(ViPipe,0x42, 0xF0);
tp9963_write_register(ViPipe,0x44, 0x89);
tp9963_write_register(ViPipe,0x51, 0x00);
tp9963_write_register(ViPipe,0xEA, 0x01);
tp9963_write_register(ViPipe,0xEB, 0x01);
tp9963_write_register(ViPipe,0xF6, 0x00);

// 第二组寄存器写操作
tp9963_write_register(ViPipe,0x40, 0x08);
tp9963_write_register(ViPipe,0x02, 0x80);
tp9963_write_register(ViPipe,0x03, 0x80);
tp9963_write_register(ViPipe,0x04, 0x80);
tp9963_write_register(ViPipe,0x05, 0x80);
tp9963_write_register(ViPipe,0x06, 0x80);

		// tmp = tp9963_read_register(ViPipe,0x42);
		// tmp &= MASK42_43[ch];
		// tp9963_write_register(ViPipe,0x42, tmp);

		// tmp = tp9963_read_register(ViPipe,0x43);
		// tmp &= MASK42_43[ch];
		// tp9963_write_register(ViPipe,0x43, tmp);

		// tp9963_write_register(ViPipe,0x02, 0x40);
		// tp9963_write_register(ViPipe,0x07, 0xc0);
		// tp9963_write_register(ViPipe,0x0b, 0xc0);
		// tp9963_write_register(ViPipe,0x0c, 0x03);
		// tp9963_write_register(ViPipe,0x0d, 0x50);
		// tp9963_write_register(ViPipe,0x15, 0x03);
		// tp9963_write_register(ViPipe,0x16, 0xd2);
		// tp9963_write_register(ViPipe,0x17, 0x80);
		// tp9963_write_register(ViPipe,0x18, 0x29);
		// tp9963_write_register(ViPipe,0x19, 0x38);
		// tp9963_write_register(ViPipe,0x1a, 0x47);
		// tp9963_write_register(ViPipe,0x1c, 0x0a);  //1920*1080, 25fps
		// tp9963_write_register(ViPipe,0x1d, 0x50);  //
		// tp9963_write_register(ViPipe,0x20, 0x30);
		// tp9963_write_register(ViPipe,0x21, 0x84);
		// tp9963_write_register(ViPipe,0x22, 0x36);
		// tp9963_write_register(ViPipe,0x23, 0x3c);
		// tp9963_write_register(ViPipe,0x2b, 0x60);
		// tp9963_write_register(ViPipe,0x2c, 0x2a);
		// tp9963_write_register(ViPipe,0x2d, 0x30);
		// tp9963_write_register(ViPipe,0x2e, 0x70);
		// tp9963_write_register(ViPipe,0x30, 0x48);
		// tp9963_write_register(ViPipe,0x31, 0xbb);
		// tp9963_write_register(ViPipe,0x32, 0x2e);
		// tp9963_write_register(ViPipe,0x33, 0x90);
		// tp9963_write_register(ViPipe,0x35, 0x05);
		// tp9963_write_register(ViPipe,0x39, 0x0C);

		// if(STD_HDA == std)
		// {
   		// 	printf("####### %s, %d\n", __func__, __LINE__);
		// 	tp9963_write_register(ViPipe,0x02, 0x44);
   		// 	tp9963_write_register(ViPipe,0x0d, 0x73);
   		// 	tp9963_write_register(ViPipe,0x15, 0x01);
   		// 	tp9963_write_register(ViPipe,0x16, 0xf0);
 		// 	tp9963_write_register(ViPipe,0x18, 0x2a);
   		// 	tp9963_write_register(ViPipe,0x20, 0x3c);
   		// 	tp9963_write_register(ViPipe,0x21, 0x46);
   		// 	tp9963_write_register(ViPipe,0x25, 0xfe);
   		// 	tp9963_write_register(ViPipe,0x26, 0x0d);
   		// 	tp9963_write_register(ViPipe,0x2c, 0x3a);
   		// 	tp9963_write_register(ViPipe,0x2d, 0x54);
   		// 	tp9963_write_register(ViPipe,0x2e, 0x40);
   		// 	tp9963_write_register(ViPipe,0x30, 0xa5);
   		// 	tp9963_write_register(ViPipe,0x31, 0x86);
   		// 	tp9963_write_register(ViPipe,0x32, 0xfb);
   		// 	tp9963_write_register(ViPipe,0x33, 0x60);
		// }

	}
	else if(FHD275 == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);
		tp9963_write_register(ViPipe,0x02, 0x40);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x03);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x13);
		tp9963_write_register(ViPipe,0x16, 0x88);
		tp9963_write_register(ViPipe,0x17, 0x80);
		tp9963_write_register(ViPipe,0x18, 0x29);
		tp9963_write_register(ViPipe,0x19, 0x38);
		tp9963_write_register(ViPipe,0x1a, 0x47);
		tp9963_write_register(ViPipe,0x1c, 0x09);  //1920*1080, 25fps
		tp9963_write_register(ViPipe,0x1d, 0x60);  //
		tp9963_write_register(ViPipe,0x20, 0x30);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x30);
		tp9963_write_register(ViPipe,0x2e, 0x70);
		tp9963_write_register(ViPipe,0x30, 0x48);
		tp9963_write_register(ViPipe,0x31, 0xbb);
		tp9963_write_register(ViPipe,0x32, 0x2e);
		tp9963_write_register(ViPipe,0x33, 0x90);
		tp9963_write_register(ViPipe,0x35, 0x05);
		tp9963_write_register(ViPipe,0x39, 0x0C);

		if(STD_HDA == std)
		{
#if 0 //OP1 settings
 			tp28xx_byte_write(0x02, 0x40);

    			tp28xx_byte_write(0x0d, 0x50);

    			tp28xx_byte_write(0x14, 0x40);
    			tp28xx_byte_write(0x15, 0x11);
    			tp28xx_byte_write(0x16, 0xd2);
    			tp28xx_byte_write(0x18, 0x2a);

    			tp28xx_byte_write(0x20, 0x38);
    			tp28xx_byte_write(0x21, 0x46);

    			tp28xx_byte_write(0x25, 0xfe);
    			tp28xx_byte_write(0x26, 0x0d);

    			tp28xx_byte_write(0x2c, 0x3a);
    			tp28xx_byte_write(0x2d, 0x54);
    			tp28xx_byte_write(0x2e, 0x40);

    			tp28xx_byte_write(0x30, 0x29);
    			tp28xx_byte_write(0x31, 0x85);
    			tp28xx_byte_write(0x32, 0x1e);
    			tp28xx_byte_write(0x33, 0xb0);
#endif
#if 1 //OP3 settings
			tmp = tp9963_read_register(ViPipe,0x42);
			tmp |= REG42_43[ch];
			tp9963_write_register(ViPipe,0x42, tmp);

			tmp = tp9963_read_register(ViPipe,0x43);
			tmp &= MASK42_43[ch];
			tp9963_write_register(ViPipe,0x43, tmp);
   			tp9963_write_register(ViPipe,0x0d, 0x70);
    		tp9963_write_register(ViPipe,0x15, 0x01);
    		tp9963_write_register(ViPipe,0x16, 0xd0);
    		tp9963_write_register(ViPipe,0x18, 0x2a);
    		tp9963_write_register(ViPipe,0x1c, 0x89);
    		tp9963_write_register(ViPipe,0x1d, 0x5e);
   			tp9963_write_register(ViPipe,0x20, 0x38);
   			tp9963_write_register(ViPipe,0x21, 0x84);
   			tp9963_write_register(ViPipe,0x27, 0xad);
   			tp9963_write_register(ViPipe,0x2c, 0x3a);
   			tp9963_write_register(ViPipe,0x2d, 0x48);
   			tp9963_write_register(ViPipe,0x2e, 0x40);
   			tp9963_write_register(ViPipe,0x30, 0x52);
   			tp9963_write_register(ViPipe,0x31, 0xc0);
   			tp9963_write_register(ViPipe,0x32, 0x3d);
   			tp9963_write_register(ViPipe,0x33, 0x70);
   			tp9963_write_register(ViPipe,0x35, 0x25);
#endif
		}

	}
	else if(FHD28 == fmt)	//TVI200p28. 2200x1205
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);
		tp9963_write_register(ViPipe,0x02, 0x40);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x03);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x03);
		tp9963_write_register(ViPipe,0x16, 0xd2);
		tp9963_write_register(ViPipe,0x17, 0x80);
		tp9963_write_register(ViPipe,0x18, 0x79);
		tp9963_write_register(ViPipe,0x19, 0x38);
		tp9963_write_register(ViPipe,0x1a, 0x47);
		tp9963_write_register(ViPipe,0x1c, 0x08);  //1920*1080, 28fps
		tp9963_write_register(ViPipe,0x1d, 0x98);  //
		tp9963_write_register(ViPipe,0x20, 0x30);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x30);
		tp9963_write_register(ViPipe,0x2e, 0x70);
		tp9963_write_register(ViPipe,0x30, 0x48);
		tp9963_write_register(ViPipe,0x31, 0xbb);
		tp9963_write_register(ViPipe,0x32, 0x2e);
		tp9963_write_register(ViPipe,0x33, 0x90);
		tp9963_write_register(ViPipe,0x35, 0x14);
		tp9963_write_register(ViPipe,0x36, 0xb5);
		tp9963_write_register(ViPipe,0x39, 0x0c);

	}
	else if(QHD30 == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);
		tp9963_write_register(ViPipe,0x02, 0x50);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x03);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x23);
		tp9963_write_register(ViPipe,0x16, 0x1b);
		tp9963_write_register(ViPipe,0x17, 0x00);
		tp9963_write_register(ViPipe,0x18, 0x38);
		tp9963_write_register(ViPipe,0x19, 0xa0);
		tp9963_write_register(ViPipe,0x1a, 0x5a);
		tp9963_write_register(ViPipe,0x1c, 0x0c);  //2560*1440, 30fps
		tp9963_write_register(ViPipe,0x1d, 0xe2);  //
		tp9963_write_register(ViPipe,0x20, 0x50);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x27, 0xad);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x58);
		tp9963_write_register(ViPipe,0x2e, 0x70);
		tp9963_write_register(ViPipe,0x30, 0x74);
		tp9963_write_register(ViPipe,0x31, 0x58);
		tp9963_write_register(ViPipe,0x32, 0x9f);
		tp9963_write_register(ViPipe,0x33, 0x60);
		tp9963_write_register(ViPipe,0x35, 0x15);
		tp9963_write_register(ViPipe,0x36, 0xdc);
		tp9963_write_register(ViPipe,0x38, 0x40);
		tp9963_write_register(ViPipe,0x39, 0x48);
		if(STD_HDA == std)
		{
    	tmp = tp9963_read_register(ViPipe,0x14);
    	tmp &= 0x9f;
    	tp9963_write_register(ViPipe,0x14, tmp);

    	tp9963_write_register(ViPipe,0x13, 0x00);
    	tp9963_write_register(ViPipe,0x15, 0x23);
    	tp9963_write_register(ViPipe,0x16, 0x16);
    	tp9963_write_register(ViPipe,0x18, 0x32);
		tp9963_write_register(ViPipe,0x1c, 0x8c);  //2560*1440, 30fps
		tp9963_write_register(ViPipe,0x1d, 0xe2);  //
		tp9963_write_register(ViPipe,0x0d, 0x70);  //
    	tp9963_write_register(ViPipe,0x20, 0x80);
    	tp9963_write_register(ViPipe,0x21, 0x86);
    	tp9963_write_register(ViPipe,0x22, 0x36);
    	tp9963_write_register(ViPipe,0x2b, 0x60);
    	tp9963_write_register(ViPipe,0x2d, 0xa0);
    	tp9963_write_register(ViPipe,0x2e, 0x40);
    	tp9963_write_register(ViPipe,0x30, 0x48);
    	tp9963_write_register(ViPipe,0x31, 0x6a);
    	tp9963_write_register(ViPipe,0x32, 0xbe);
    	tp9963_write_register(ViPipe,0x33, 0x80);
    	tp9963_write_register(ViPipe,0x39, 0x40);
		}
	}
	else if(QHD275 == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);
		tp9963_write_register(ViPipe,0x02, 0x50);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x03);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x33);
		tp9963_write_register(ViPipe,0x16, 0x48);
		tp9963_write_register(ViPipe,0x17, 0x00);
		tp9963_write_register(ViPipe,0x18, 0x38);
		tp9963_write_register(ViPipe,0x19, 0xa0);
		tp9963_write_register(ViPipe,0x1a, 0x5a);
		tp9963_write_register(ViPipe,0x1c, 0x0e);  //2560*1440, 275fps
		tp9963_write_register(ViPipe,0x1d, 0x0e);  //
		tp9963_write_register(ViPipe,0x20, 0x50);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x27, 0xad);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x58);
		tp9963_write_register(ViPipe,0x2e, 0x70);
		tp9963_write_register(ViPipe,0x30, 0x74);
		tp9963_write_register(ViPipe,0x31, 0x58);
		tp9963_write_register(ViPipe,0x32, 0x9f);
		tp9963_write_register(ViPipe,0x33, 0x60);
		tp9963_write_register(ViPipe,0x35, 0x15);
		tp9963_write_register(ViPipe,0x36, 0xdc);
		tp9963_write_register(ViPipe,0x38, 0x40);
		tp9963_write_register(ViPipe,0x39, 0x48);

	}
	else if(QHD25 == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);
		tp9963_write_register(ViPipe,0x02, 0x50);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x03);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x23);
		tp9963_write_register(ViPipe,0x16, 0x1b);
		tp9963_write_register(ViPipe,0x17, 0x00);
		tp9963_write_register(ViPipe,0x18, 0x38);
		tp9963_write_register(ViPipe,0x19, 0xa0);
		tp9963_write_register(ViPipe,0x1a, 0x5a);
		tp9963_write_register(ViPipe,0x1c, 0x0f);  //2560*1440, 25fps
		tp9963_write_register(ViPipe,0x1d, 0x76);  //
		tp9963_write_register(ViPipe,0x20, 0x50);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x27, 0xad);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x58);
		tp9963_write_register(ViPipe,0x2e, 0x70);
		tp9963_write_register(ViPipe,0x30, 0x74);
		tp9963_write_register(ViPipe,0x31, 0x58);
		tp9963_write_register(ViPipe,0x32, 0x9f);
		tp9963_write_register(ViPipe,0x33, 0x60);
		tp9963_write_register(ViPipe,0x35, 0x15);
		tp9963_write_register(ViPipe,0x36, 0xdc);
		tp9963_write_register(ViPipe,0x38, 0x40);
		tp9963_write_register(ViPipe,0x39, 0x48);
		if(STD_HDA == std)
		{
    	tmp = tp9963_read_register(ViPipe,0x14);
    	tmp &= 0x9f;
    	tp9963_write_register(ViPipe,0x14, tmp);

    	tp9963_write_register(ViPipe,0x13, 0x00);
    	tp9963_write_register(ViPipe,0x15, 0x23);
    	tp9963_write_register(ViPipe,0x16, 0x16);
    	tp9963_write_register(ViPipe,0x18, 0x32);
		tp9963_write_register(ViPipe,0x1c, 0x8f);  //
		tp9963_write_register(ViPipe,0x1d, 0x76);  //
		tp9963_write_register(ViPipe,0x0d, 0x70);  //
    	tp9963_write_register(ViPipe,0x20, 0x80);
    	tp9963_write_register(ViPipe,0x21, 0x86);
    	tp9963_write_register(ViPipe,0x22, 0x36);
    	tp9963_write_register(ViPipe,0x2b, 0x60);
    	tp9963_write_register(ViPipe,0x2d, 0xa0);
    	tp9963_write_register(ViPipe,0x2e, 0x40);
    	tp9963_write_register(ViPipe,0x30, 0x48);
    	tp9963_write_register(ViPipe,0x31, 0x6f);
    	tp9963_write_register(ViPipe,0x32, 0xb5);
    	tp9963_write_register(ViPipe,0x33, 0x80);
    	tp9963_write_register(ViPipe,0x39, 0x40);
		}
	}
	else if(QHD25_4A10 == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);
		tp9963_write_register(ViPipe,0x02, 0x50);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x03);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x23);
		tp9963_write_register(ViPipe,0x16, 0x1b);
		tp9963_write_register(ViPipe,0x17, 0x00);
		tp9963_write_register(ViPipe,0x18, 0xc0);
		tp9963_write_register(ViPipe,0x19, 0xa0);
		tp9963_write_register(ViPipe,0x1a, 0x5a);
		tp9963_write_register(ViPipe,0x1c, 0x0c);  //
		tp9963_write_register(ViPipe,0x1d, 0xe2);  //
		tp9963_write_register(ViPipe,0x20, 0x50);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x27, 0xad);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x58);
		tp9963_write_register(ViPipe,0x2e, 0x70);
		tp9963_write_register(ViPipe,0x30, 0x74);
		tp9963_write_register(ViPipe,0x31, 0x58);
		tp9963_write_register(ViPipe,0x32, 0x9f);
		tp9963_write_register(ViPipe,0x33, 0x60);
		tp9963_write_register(ViPipe,0x35, 0x17);
		tp9963_write_register(ViPipe,0x36, 0x08);
		tp9963_write_register(ViPipe,0x38, 0x40);
		tp9963_write_register(ViPipe,0x39, 0x48);

	}
	// else if(5M20 == fmt)
	// {
	// 	tmp = tp9963_read_register(ViPipe,0x42);
	// 	tmp &= MASK42_43[ch];
	// 	tp9963_write_register(ViPipe,0x42, tmp);

	// 	tmp = tp9963_read_register(ViPipe,0x43);
	// 	tmp &= MASK42_43[ch];
	// 	tp9963_write_register(ViPipe,0x43, tmp);
	// 	tp9963_write_register(ViPipe,0x02, 0x50);
	// 	tp9963_write_register(ViPipe,0x07, 0xc0);
	// 	tp9963_write_register(ViPipe,0x0b, 0xc0);
	// 	tp9963_write_register(ViPipe,0x0c, 0x03);
	// 	tp9963_write_register(ViPipe,0x0d, 0x50);
	// 	tp9963_write_register(ViPipe,0x15, 0x23);
	// 	tp9963_write_register(ViPipe,0x16, 0x36);
	// 	tp9963_write_register(ViPipe,0x17, 0x24);
	// 	tp9963_write_register(ViPipe,0x18, 0x1a);
	// 	tp9963_write_register(ViPipe,0x19, 0x98);
	// 	tp9963_write_register(ViPipe,0x1a, 0x7a);
	// 	tp9963_write_register(ViPipe,0x1c, 0x0e);  //2960x1660, 20fps
	// 	tp9963_write_register(ViPipe,0x1d, 0xa4);  //
	// 	tp9963_write_register(ViPipe,0x20, 0x50);
	// 	tp9963_write_register(ViPipe,0x21, 0x84);
	// 	tp9963_write_register(ViPipe,0x22, 0x36);
	// 	tp9963_write_register(ViPipe,0x23, 0x3c);
	// 	tp9963_write_register(ViPipe,0x27, 0xad);
	// 	tp9963_write_register(ViPipe,0x2b, 0x60);
	// 	tp9963_write_register(ViPipe,0x2c, 0x2a);
	// 	tp9963_write_register(ViPipe,0x2d, 0x54);
	// 	tp9963_write_register(ViPipe,0x2e, 0x70);
	// 	tp9963_write_register(ViPipe,0x30, 0x74);
	// 	tp9963_write_register(ViPipe,0x31, 0xa7);
	// 	tp9963_write_register(ViPipe,0x32, 0x18);
	// 	tp9963_write_register(ViPipe,0x33, 0x50);
	// 	tp9963_write_register(ViPipe,0x35, 0x17);
	// 	tp9963_write_register(ViPipe,0x36, 0xbc);
	// 	tp9963_write_register(ViPipe,0x38, 0x40);
	// 	tp9963_write_register(ViPipe,0x39, 0x48);

  	// if(STD_HDA == std)
  	// {
  	// 	tp9963_write_register(ViPipe,0x14, 0x40);
    // 	tp9963_write_register(ViPipe,0x20, 0x80);
    // 	tp9963_write_register(ViPipe,0x21, 0x86);
    // 	tp9963_write_register(ViPipe,0x22, 0x36);
    // 	tp9963_write_register(ViPipe,0x25, 0xff);
    // 	tp9963_write_register(ViPipe,0x26, 0x05);
    // 	tp9963_write_register(ViPipe,0x27, 0xad);
    // 	tp9963_write_register(ViPipe,0x2b, 0x60);
    // 	tp9963_write_register(ViPipe,0x2d, 0xA0);
    // 	tp9963_write_register(ViPipe,0x2e, 0x70);
    // 	tp9963_write_register(ViPipe,0x30, 0x48);
    // 	tp9963_write_register(ViPipe,0x31, 0x77);
    // 	tp9963_write_register(ViPipe,0x32, 0x0e);
    // 	tp9963_write_register(ViPipe,0x33, 0xa0);
    // 	tp9963_write_register(ViPipe,0x39, 0x48);
 	// 	 }

	// }
	else if(FHD60 == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);

		tp9963_write_register(ViPipe,0x02, 0x50);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x03);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x03);
		tp9963_write_register(ViPipe,0x16, 0xf0);
		tp9963_write_register(ViPipe,0x17, 0x80); //0x80
		tp9963_write_register(ViPipe,0x18, 0x29);
		// tp9963_write_register(ViPipe,0x19, 0xaa);	//0x38
		// tp9963_write_register(ViPipe,0x1a, 0x48);  //0x47
		tp9963_write_register(ViPipe,0x19, 0x38);	//0x38
		tp9963_write_register(ViPipe,0x1a, 0x47);  //0x47
		tp9963_write_register(ViPipe,0x1c, 0x08);  //1920*1080, 60fps
		tp9963_write_register(ViPipe,0x1d, 0x96);  //
		tp9963_write_register(ViPipe,0x20, 0x38);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x27, 0xad);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x40);
		tp9963_write_register(ViPipe,0x2e, 0x70);
		tp9963_write_register(ViPipe,0x30, 0x74);
		tp9963_write_register(ViPipe,0x31, 0x9b);
		tp9963_write_register(ViPipe,0x32, 0xa5);
		tp9963_write_register(ViPipe,0x33, 0xe0);
		tp9963_write_register(ViPipe,0x35, 0x05);
		tp9963_write_register(ViPipe,0x38, 0x40);
		tp9963_write_register(ViPipe,0x39, 0x68);
  		if(STD_HDA == std)
  		{
			tp9963_write_register(ViPipe,0x0d, 0x70);
			tp9963_write_register(ViPipe,0x0e, 0x10);
			tp9963_write_register(ViPipe,0x14, 0x00);
			tp9963_write_register(ViPipe,0x15, 0x03);
			tp9963_write_register(ViPipe,0x16, 0xd4);
			tp9963_write_register(ViPipe,0x18, 0x2a);
			tp9963_write_register(ViPipe,0x1c, 0x88);
			tp9963_write_register(ViPipe,0x1d, 0x97);
    		tp9963_write_register(ViPipe,0x20, 0x30);
    		tp9963_write_register(ViPipe,0x21, 0x84);
    		tp9963_write_register(ViPipe,0x22, 0x36);
    		tp9963_write_register(ViPipe,0x26, 0x05);
    		tp9963_write_register(ViPipe,0x27, 0xad);
    		tp9963_write_register(ViPipe,0x2b, 0x60);
    		tp9963_write_register(ViPipe,0x2d, 0x4b);
    		tp9963_write_register(ViPipe,0x2e, 0x70);
    		tp9963_write_register(ViPipe,0x30, 0x52);
    		tp9963_write_register(ViPipe,0x31, 0xca);
    		tp9963_write_register(ViPipe,0x32, 0xf0);
    		tp9963_write_register(ViPipe,0x33, 0xb0);
    		tp9963_write_register(ViPipe,0x39, 0x68);
 		 }

	}
	else if(FHD50 == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);
		tp9963_write_register(ViPipe,0x02, 0x50);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x03);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x03);
		tp9963_write_register(ViPipe,0x16, 0xe2);
		tp9963_write_register(ViPipe,0x17, 0x80);
		tp9963_write_register(ViPipe,0x18, 0x29);
		tp9963_write_register(ViPipe,0x19, 0x38);
		tp9963_write_register(ViPipe,0x1a, 0x47);
		tp9963_write_register(ViPipe,0x1c, 0x0a);  //1920*1080, 25fps
		tp9963_write_register(ViPipe,0x1d, 0x4e);  //
		tp9963_write_register(ViPipe,0x20, 0x38);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x27, 0xad);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x40);
		tp9963_write_register(ViPipe,0x2e, 0x70);
		tp9963_write_register(ViPipe,0x30, 0x74);
		tp9963_write_register(ViPipe,0x31, 0x9b);
		tp9963_write_register(ViPipe,0x32, 0xa5);
		tp9963_write_register(ViPipe,0x33, 0xe0);
		tp9963_write_register(ViPipe,0x35, 0x05);
		tp9963_write_register(ViPipe,0x38, 0x40);
		tp9963_write_register(ViPipe,0x39, 0x68);

	}
	else if(F_UVGA30 == fmt) //FH 960P30
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);
		tp9963_write_register(ViPipe,0x02, 0x44);
		tp9963_write_register(ViPipe,0x05, 0xfd);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x03);
		tp9963_write_register(ViPipe,0x0d, 0x76);
		tp9963_write_register(ViPipe,0x0e, 0x16);
		tp9963_write_register(ViPipe,0x15, 0x13);
		tp9963_write_register(ViPipe,0x16, 0x8f);
		tp9963_write_register(ViPipe,0x17, 0x00);
		tp9963_write_register(ViPipe,0x18, 0x23);
		tp9963_write_register(ViPipe,0x19, 0xc0);
		tp9963_write_register(ViPipe,0x1a, 0x35);
		tp9963_write_register(ViPipe,0x1c, 0x07);  //
		tp9963_write_register(ViPipe,0x1d, 0x08);  //
		tp9963_write_register(ViPipe,0x20, 0x60);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x26, 0x05);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x70);
		tp9963_write_register(ViPipe,0x2e, 0x50);
		tp9963_write_register(ViPipe,0x30, 0x7f);
		tp9963_write_register(ViPipe,0x31, 0x49);
		tp9963_write_register(ViPipe,0x32, 0xf4);
		tp9963_write_register(ViPipe,0x33, 0x90);
		tp9963_write_register(ViPipe,0x35, 0x13);
		tp9963_write_register(ViPipe,0x36, 0xe8);
		tp9963_write_register(ViPipe,0x38, 0x00);
		tp9963_write_register(ViPipe,0x39, 0x88);
	}

	else if(FHD59 == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);
		tp9963_write_register(ViPipe,0x02, 0x50);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x03);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x03);
		tp9963_write_register(ViPipe,0x16, 0xf0);
		tp9963_write_register(ViPipe,0x17, 0x80);
		tp9963_write_register(ViPipe,0x18, 0x29);
		tp9963_write_register(ViPipe,0x19, 0x38);
		tp9963_write_register(ViPipe,0x1a, 0x47);
		tp9963_write_register(ViPipe,0x1c, 0x08);  //1920*1080, 60fps
		tp9963_write_register(ViPipe,0x1d, 0x96);  //
		tp9963_write_register(ViPipe,0x20, 0x38);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x27, 0xad);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x40);
		tp9963_write_register(ViPipe,0x2e, 0x70);
		tp9963_write_register(ViPipe,0x30, 0x74);
		tp9963_write_register(ViPipe,0x31, 0x9b);
		tp9963_write_register(ViPipe,0x32, 0xa5);
		tp9963_write_register(ViPipe,0x33, 0xe0);
		tp9963_write_register(ViPipe,0x35, 0x14);
		tp9963_write_register(ViPipe,0x36, 0x78);
		tp9963_write_register(ViPipe,0x38, 0x40);
		tp9963_write_register(ViPipe,0x39, 0x68);

	}
	else if(FHD55 == fmt)
	{
		tmp = tp9963_read_register(ViPipe,0x42);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x42, tmp);

		tmp = tp9963_read_register(ViPipe,0x43);
		tmp &= MASK42_43[ch];
		tp9963_write_register(ViPipe,0x43, tmp);

		tp9963_write_register(ViPipe,0x02, 0x50);
		tp9963_write_register(ViPipe,0x07, 0xc0);
		tp9963_write_register(ViPipe,0x0b, 0xc0);
		tp9963_write_register(ViPipe,0x0c, 0x03);
		tp9963_write_register(ViPipe,0x0d, 0x50);
		tp9963_write_register(ViPipe,0x15, 0x03);
		tp9963_write_register(ViPipe,0x16, 0xf0);
		tp9963_write_register(ViPipe,0x17, 0x80);
		tp9963_write_register(ViPipe,0x18, 0x29);
		tp9963_write_register(ViPipe,0x19, 0x38);
		tp9963_write_register(ViPipe,0x1a, 0x47);
		tp9963_write_register(ViPipe,0x1c, 0x09);  //1920*1080, 60fps
		tp9963_write_register(ViPipe,0x1d, 0x5e);  //
		tp9963_write_register(ViPipe,0x20, 0x38);
		tp9963_write_register(ViPipe,0x21, 0x84);
		tp9963_write_register(ViPipe,0x22, 0x36);
		tp9963_write_register(ViPipe,0x23, 0x3c);
		tp9963_write_register(ViPipe,0x27, 0xad);
		tp9963_write_register(ViPipe,0x2b, 0x60);
		tp9963_write_register(ViPipe,0x2c, 0x2a);
		tp9963_write_register(ViPipe,0x2d, 0x68);
		tp9963_write_register(ViPipe,0x2e, 0x70);
		tp9963_write_register(ViPipe,0x30, 0x74);
		tp9963_write_register(ViPipe,0x31, 0x9b);
		tp9963_write_register(ViPipe,0x32, 0xa5);
		tp9963_write_register(ViPipe,0x33, 0xe0);
		tp9963_write_register(ViPipe,0x35, 0x05);
		tp9963_write_register(ViPipe,0x38, 0x40);
		tp9963_write_register(ViPipe,0x39, 0x68);
	}

		tmp = tp9963_read_register(ViPipe,0x06);
		tmp |= 0x80;
		tp9963_write_register(ViPipe,0x06, tmp);
}

void TP9963_mipi_out(VI_PIPE ViPipe, unsigned char output)
{
	//mipi setting
	tp9963_write_register(ViPipe,0x40, MIPI_PAGE); //MIPI page
    tp9963_write_register(ViPipe,0x02, 0x78);
    tp9963_write_register(ViPipe,0x03, 0x70);
    tp9963_write_register(ViPipe,0x04, 0x70);
    tp9963_write_register(ViPipe,0x05, 0x70);
    tp9963_write_register(ViPipe,0x06, 0x70);

	tp9963_write_register(ViPipe,0x13, 0xef);

	tp9963_write_register(ViPipe,0x20, 0x00);
	tp9963_write_register(ViPipe,0x21, 0x22);
	tp9963_write_register(ViPipe,0x22, 0x30);
	tp9963_write_register(ViPipe,0x23, 0x9e);

	if( MIPI_2CH2LANE_594M == output)
	{
		tp9963_write_register(ViPipe,0x21, 0x22);
		tp9963_write_register(ViPipe,0x14, 0x00);
		tp9963_write_register(ViPipe,0x15, 0x01);
		tp9963_write_register(ViPipe,0x2a, 0x08);
		tp9963_write_register(ViPipe,0x2b, 0x08);
		tp9963_write_register(ViPipe,0x2c, 0x10);
		tp9963_write_register(ViPipe,0x2e, 0x0a);
		tp9963_write_register(ViPipe,0x10, 0xa0);
		tp9963_write_register(ViPipe,0x10, 0x20);
	}
	if( MIPI_1CH2LANE_594M == output)
	{
		tp9963_write_register(ViPipe,0x21, 0x12);
		tp9963_write_register(ViPipe,0x14, 0x00);
		tp9963_write_register(ViPipe,0x15, 0x01);
		tp9963_write_register(ViPipe,0x2a, 0x08);
		tp9963_write_register(ViPipe,0x2b, 0x08);
		tp9963_write_register(ViPipe,0x2c, 0x10);
		tp9963_write_register(ViPipe,0x2e, 0x0a);
		tp9963_write_register(ViPipe,0x10, 0xa0);
		tp9963_write_register(ViPipe,0x10, 0x20);
	}

	else if( MIPI_2CH2LANE_297M == output)
	{
		tp9963_write_register(ViPipe,0x21, 0x22);
		tp9963_write_register(ViPipe,0x14, 0x41);
		tp9963_write_register(ViPipe,0x15, 0x02);
		tp9963_write_register(ViPipe,0x2a, 0x04);
		tp9963_write_register(ViPipe,0x2b, 0x03);
		tp9963_write_register(ViPipe,0x2c, 0x09);
		tp9963_write_register(ViPipe,0x2e, 0x02);
		tp9963_write_register(ViPipe,0x10, 0xa0);
		tp9963_write_register(ViPipe,0x10, 0x20);
	}
	else if( MIPI_1CH2LANE_297M == output)
	{

		tp9963_write_register(ViPipe,0x21, 0x12);
		tp9963_write_register(ViPipe,0x14, 0x41);
		tp9963_write_register(ViPipe,0x15, 0x02);
		tp9963_write_register(ViPipe,0x2a, 0x04);
		tp9963_write_register(ViPipe,0x2b, 0x03);
		tp9963_write_register(ViPipe,0x2c, 0x09);
		tp9963_write_register(ViPipe,0x2e, 0x02);
		tp9963_write_register(ViPipe,0x10, 0xa0);
		tp9963_write_register(ViPipe,0x10, 0x20);
	}
	else if( MIPI_2CH4LANE_594M == output)
	{
		tp9963_write_register(ViPipe,0x21, 0x24);
		tp9963_write_register(ViPipe,0x14, 0x00);
		tp9963_write_register(ViPipe,0x15, 0x00);
		tp9963_write_register(ViPipe,0x2a, 0x08);
		tp9963_write_register(ViPipe,0x2b, 0x08);
		tp9963_write_register(ViPipe,0x2c, 0x10);
		tp9963_write_register(ViPipe,0x2e, 0x0a);
		tp9963_write_register(ViPipe,0x10, 0xa0);
		tp9963_write_register(ViPipe,0x10, 0x20);
	}
	else if( MIPI_2CH4LANE_297M == output)
	{
		tp9963_write_register(ViPipe,0x21, 0x24);
		tp9963_write_register(ViPipe,0x14, 0x41);
		tp9963_write_register(ViPipe,0x15, 0x01);
		tp9963_write_register(ViPipe,0x2a, 0x04);
		tp9963_write_register(ViPipe,0x2b, 0x03);
		tp9963_write_register(ViPipe,0x2c, 0x09);
		tp9963_write_register(ViPipe,0x2e, 0x02);
		tp9963_write_register(ViPipe,0x10, 0xa0);
		tp9963_write_register(ViPipe,0x10, 0x20);
	}
	else if( MIPI_1CH4LANE_297M == output)
	{
		tp9963_write_register(ViPipe,0x21, 0x14);
		tp9963_write_register(ViPipe,0x14, 0x41);
		tp9963_write_register(ViPipe,0x15, 0x01);
		tp9963_write_register(ViPipe,0x2a, 0x04);
		tp9963_write_register(ViPipe,0x2b, 0x03);
		tp9963_write_register(ViPipe,0x2c, 0x09);
		tp9963_write_register(ViPipe,0x2e, 0x02);
		tp9963_write_register(ViPipe,0x10, 0xa0);
		tp9963_write_register(ViPipe,0x10, 0x20);
	}
	else if( MIPI_2CH2LANE_432M == output) //only for 2xF_UVGA30
	{

		tp9963_write_register(ViPipe,0x21, 0x22);
		tp9963_write_register(ViPipe,0x13, 0x0f);
		tp9963_write_register(ViPipe,0x12, 0x5e);
		tp9963_write_register(ViPipe,0x14, 0x00);
		tp9963_write_register(ViPipe,0x15, 0x01);
		tp9963_write_register(ViPipe,0x2a, 0x06);
		tp9963_write_register(ViPipe,0x2b, 0x05);
		tp9963_write_register(ViPipe,0x2c, 0x0d);
		tp9963_write_register(ViPipe,0x2e, 0x0a);
		tp9963_write_register(ViPipe,0x10, 0xa0);
		tp9963_write_register(ViPipe,0x10, 0x20);

	}

	else if( MIPI_2CH4LANE_216M == output) //only for 2xF_UVGA30
	{

		tp9963_write_register(ViPipe,0x21, 0x24);
		tp9963_write_register(ViPipe,0x13, 0x0f);
		tp9963_write_register(ViPipe,0x12, 0x5e);
		tp9963_write_register(ViPipe,0x14, 0x41);
		tp9963_write_register(ViPipe,0x15, 0x01);
		tp9963_write_register(ViPipe,0x2a, 0x06);
		tp9963_write_register(ViPipe,0x2b, 0x05);
		tp9963_write_register(ViPipe,0x2c, 0x0d);
		tp9963_write_register(ViPipe,0x2e, 0x0a);
		tp9963_write_register(ViPipe,0x10, 0xa0);
		tp9963_write_register(ViPipe,0x10, 0x20);

	}
	/* Enable MIPI CSI2 output */
	tp9963_write_register(ViPipe,0x28, 0x02);
	tp9963_write_register(ViPipe,0x28, 0x00);

}

void TP9963_reg_init(VI_PIPE ViPipe, CVI_U8 u8ImgMode)
{
	printf("u8ImgMode:%d \n",u8ImgMode);
	if (u8ImgMode == TP9963_MODE_1080P_30P)
	{
		TP9963_decoder_init(ViPipe, CH_1, FHD30, STD_HDA);
		TP9963_mipi_out(ViPipe, MIPI_1CH2LANE_297M);
	}
	else if (u8ImgMode == TP9963_MODE_1080P_60P)
	{

		TP9963_decoder_init(ViPipe, CH_1, FHD60, STD_HDA);
		// TP9963_mipi_out(ViPipe, MIPI_1CH2LANE_594M);
	}
	else if (u8ImgMode == TP9963_MODE_1080P_30P_2CH)
	{

		TP9963_decoder_init(ViPipe, CH_1, FHD30, STD_HDA);
		TP9963_decoder_init(ViPipe, CH_2, FHD30, STD_HDA);
		TP9963_mipi_out(ViPipe, MIPI_2CH2LANE_594M);
	}
	else if (u8ImgMode == TP9963_MODE_1080P_25P)
	{
		printf("####### %s, %d\n", __func__, __LINE__);
		TP9963_decoder_init(ViPipe, CH_1, FHD25, STD_HDA);
		// TP9963_mipi_out(ViPipe, MIPI_1CH2LANE_297M);
	}
	else if (u8ImgMode == TP9963_MODE_1080P_25P_2CH)
	{

		TP9963_decoder_init(ViPipe, CH_1, FHD25, STD_HDA);
		TP9963_decoder_init(ViPipe, CH_2, FHD25, STD_HDA);
		TP9963_mipi_out(ViPipe, MIPI_2CH2LANE_594M);
	}
	else if (u8ImgMode == TP9963_MODE_720P_30P)
	{
		TP9963_decoder_init(ViPipe, CH_1, HD30, STD_HDA);
		TP9963_mipi_out(ViPipe, MIPI_1CH2LANE_297M);
	}
	else if (u8ImgMode == TP9963_MODE_720P_30P_2CH)
	{
		TP9963_decoder_init(ViPipe, CH_1, HD30, STD_HDA);
		TP9963_decoder_init(ViPipe, CH_2, HD30, STD_HDA);
		TP9963_mipi_out(ViPipe, MIPI_2CH2LANE_297M);
	}
	else if (u8ImgMode == TP9963_MODE_720P_25P)
	{
		TP9963_decoder_init(ViPipe, CH_1, HD25, STD_HDA);
		TP9963_mipi_out(ViPipe, MIPI_1CH2LANE_297M);
	}
	else if (u8ImgMode == TP9963_MODE_720P_25P_2CH)
	{
		TP9963_decoder_init(ViPipe, CH_1, HD25, STD_HDA);
		TP9963_decoder_init(ViPipe, CH_2, HD25, STD_HDA);
		TP9963_mipi_out(ViPipe, MIPI_2CH2LANE_297M);
	}
}

void tp9963_set_1080p_25(VI_PIPE ViPipe)
{
	tp9963_write_register(ViPipe,0x40, 0x04);
	tp9963_write_register(ViPipe,0x02, 0xCC);
	tp9963_write_register(ViPipe,0x07, 0xC0);
	tp9963_write_register(ViPipe,0x0B, 0xC0);
	tp9963_write_register(ViPipe,0x0C, 0x03);
	tp9963_write_register(ViPipe,0x0D, 0x73);
	tp9963_write_register(ViPipe,0x15, 0x01);
	tp9963_write_register(ViPipe,0x16, 0xF0);
	tp9963_write_register(ViPipe,0x17, 0x80);
	tp9963_write_register(ViPipe,0x18, 0x2A);
	tp9963_write_register(ViPipe,0x19, 0x38);
	tp9963_write_register(ViPipe,0x1A, 0x47);
	tp9963_write_register(ViPipe,0x1C, 0x0A);
	tp9963_write_register(ViPipe,0x1D, 0x50);
	tp9963_write_register(ViPipe,0x20, 0x3C);
	tp9963_write_register(ViPipe,0x21, 0x46);
	tp9963_write_register(ViPipe,0x22, 0x36);
	tp9963_write_register(ViPipe,0x25, 0xFE);
	tp9963_write_register(ViPipe,0x26, 0x0D);
	tp9963_write_register(ViPipe,0x2B, 0x60);
	tp9963_write_register(ViPipe,0x2C, 0x3A);
	tp9963_write_register(ViPipe,0x2D, 0x54);
	tp9963_write_register(ViPipe,0x2E, 0x40);
	tp9963_write_register(ViPipe,0x30, 0xA5);
	tp9963_write_register(ViPipe,0x31, 0x86);
	tp9963_write_register(ViPipe,0x32, 0xFB);
	tp9963_write_register(ViPipe,0x33, 0x60);
	tp9963_write_register(ViPipe,0x38, 0x40);
	tp9963_write_register(ViPipe,0x39, 0x0C);
	tp9963_write_register(ViPipe,0x42, 0xF0);
	tp9963_write_register(ViPipe,0x44, 0x89);
	tp9963_write_register(ViPipe,0x51, 0x00);
	tp9963_write_register(ViPipe,0xEA, 0x01);
	tp9963_write_register(ViPipe,0xEB, 0x01);
	tp9963_write_register(ViPipe,0xF6, 0x00);

	// 第二组寄存器写操作
	tp9963_write_register(ViPipe,0x40, 0x08);
	tp9963_write_register(ViPipe,0x02, 0x80);
	tp9963_write_register(ViPipe,0x03, 0x80);
	tp9963_write_register(ViPipe,0x04, 0x80);
	tp9963_write_register(ViPipe,0x05, 0x80);
	tp9963_write_register(ViPipe,0x06, 0x80);

	// 第三部分
	tp9963_write_register(ViPipe,0x40, 0x00);
	printf("ViPipe:%d,===TP9963 1080P 25fps Init OK!===\n", ViPipe);
}

void tp9963_set_1080p_30(VI_PIPE ViPipe)
{
	// 第一部分
tp9963_write_register(ViPipe,0x40, 0x04);
tp9963_write_register(ViPipe,0x02, 0xcc);
tp9963_write_register(ViPipe,0x07, 0xc0);
tp9963_write_register(ViPipe,0x0b, 0xc0);
tp9963_write_register(ViPipe,0x0c, 0x03);
tp9963_write_register(ViPipe,0x0d, 0x72);
tp9963_write_register(ViPipe,0x15, 0x01);
tp9963_write_register(ViPipe,0x16, 0xf0);
tp9963_write_register(ViPipe,0x17, 0x80);
tp9963_write_register(ViPipe,0x18, 0x2a);
tp9963_write_register(ViPipe,0x19, 0x38);
tp9963_write_register(ViPipe,0x1a, 0x47);
tp9963_write_register(ViPipe,0x1c, 0x08);
tp9963_write_register(ViPipe,0x1d, 0x98);
tp9963_write_register(ViPipe,0x20, 0x38);
tp9963_write_register(ViPipe,0x21, 0x46);
tp9963_write_register(ViPipe,0x22, 0x36);
tp9963_write_register(ViPipe,0x25, 0xfe);
tp9963_write_register(ViPipe,0x26, 0x0d);
tp9963_write_register(ViPipe,0x2b, 0x60);
tp9963_write_register(ViPipe,0x2c, 0x3a);
tp9963_write_register(ViPipe,0x2d, 0x54);
tp9963_write_register(ViPipe,0x2e, 0x40);
tp9963_write_register(ViPipe,0x30, 0xa5);
tp9963_write_register(ViPipe,0x31, 0x95);
tp9963_write_register(ViPipe,0x32, 0xe0);
tp9963_write_register(ViPipe,0x33, 0x60);
tp9963_write_register(ViPipe,0x38, 0x40);
tp9963_write_register(ViPipe,0x39, 0x0c);
tp9963_write_register(ViPipe,0x42, 0xf0);
tp9963_write_register(ViPipe,0x44, 0x89);
tp9963_write_register(ViPipe,0x51, 0x00);
tp9963_write_register(ViPipe,0xea, 0x01);
tp9963_write_register(ViPipe,0xeb, 0x01);
tp9963_write_register(ViPipe,0xf6, 0x00);

// 第二部分
tp9963_write_register(ViPipe,0x40, 0x08);
tp9963_write_register(ViPipe,0x02, 0x80);
tp9963_write_register(ViPipe,0x03, 0x80);
tp9963_write_register(ViPipe,0x04, 0x80);
tp9963_write_register(ViPipe,0x05, 0x80);
tp9963_write_register(ViPipe,0x06, 0x80);

// 第三部分
tp9963_write_register(ViPipe,0x40, 0x00);
	printf("ViPipe:%d,===TP9963 1080P 30fps Init OK!===\n", ViPipe);
}

void tp9963_set_720p_30(VI_PIPE ViPipe)
{
	// 第一部分
tp9963_write_register(ViPipe,0x40, 0x04);
tp9963_write_register(ViPipe,0x02, 0xce);
tp9963_write_register(ViPipe,0x07, 0xc0);
tp9963_write_register(ViPipe,0x0b, 0xc0);
tp9963_write_register(ViPipe,0x0c, 0x03);
tp9963_write_register(ViPipe,0x0d, 0x70);
tp9963_write_register(ViPipe,0x15, 0x13);
tp9963_write_register(ViPipe,0x16, 0x15);
tp9963_write_register(ViPipe,0x17, 0x00);
tp9963_write_register(ViPipe,0x18, 0x1b);
tp9963_write_register(ViPipe,0x19, 0xd0);
tp9963_write_register(ViPipe,0x1a, 0x25);
tp9963_write_register(ViPipe,0x1c, 0x06);
tp9963_write_register(ViPipe,0x1d, 0x72);
tp9963_write_register(ViPipe,0x20, 0x40);
tp9963_write_register(ViPipe,0x21, 0x46);
tp9963_write_register(ViPipe,0x22, 0x36);
tp9963_write_register(ViPipe,0x25, 0xfe);
tp9963_write_register(ViPipe,0x26, 0x01);
tp9963_write_register(ViPipe,0x2b, 0x60);
tp9963_write_register(ViPipe,0x2c, 0x3a);
tp9963_write_register(ViPipe,0x2d, 0x5a);
tp9963_write_register(ViPipe,0x2e, 0x40);
tp9963_write_register(ViPipe,0x30, 0x9d);
tp9963_write_register(ViPipe,0x31, 0xca);
tp9963_write_register(ViPipe,0x32, 0x01);
tp9963_write_register(ViPipe,0x33, 0xd0);
tp9963_write_register(ViPipe,0x38, 0x40);
tp9963_write_register(ViPipe,0x35, 0x25);
tp9963_write_register(ViPipe,0x39, 0x08);

// 第二部分
tp9963_write_register(ViPipe,0x42, 0xf0);
tp9963_write_register(ViPipe,0x44, 0x01);
tp9963_write_register(ViPipe,0x51, 0x00);
tp9963_write_register(ViPipe,0xea, 0x01);
tp9963_write_register(ViPipe,0xeb, 0x01);
tp9963_write_register(ViPipe,0xf6, 0x00);

// 第三部分
tp9963_write_register(ViPipe,0x40, 0x08);
tp9963_write_register(ViPipe,0x02, 0x80);
tp9963_write_register(ViPipe,0x03, 0x80);
tp9963_write_register(ViPipe,0x04, 0x80);
tp9963_write_register(ViPipe,0x05, 0x80);
tp9963_write_register(ViPipe,0x06, 0x80);
tp9963_write_register(ViPipe,0x15, 0x04); // 新增行
	printf("ViPipe:%d,===TP9963 720P 30fps Init OK!===\n", ViPipe);
}

void tp9963_set_720p_25(VI_PIPE ViPipe)
{
	// 第一部分
tp9963_write_register(ViPipe,0x40, 0x04);
tp9963_write_register(ViPipe,0x02, 0xce);
tp9963_write_register(ViPipe,0x07, 0xc0);
tp9963_write_register(ViPipe,0x0b, 0xc0);
tp9963_write_register(ViPipe,0x0c, 0x03);
tp9963_write_register(ViPipe,0x0d, 0x71);
tp9963_write_register(ViPipe,0x15, 0x13);
tp9963_write_register(ViPipe,0x16, 0x15);
tp9963_write_register(ViPipe,0x17, 0x00);
tp9963_write_register(ViPipe,0x18, 0x1b);
tp9963_write_register(ViPipe,0x19, 0xd0);
tp9963_write_register(ViPipe,0x1a, 0x25);
tp9963_write_register(ViPipe,0x1c, 0x07);
tp9963_write_register(ViPipe,0x1d, 0xbc);
tp9963_write_register(ViPipe,0x20, 0x40);
tp9963_write_register(ViPipe,0x21, 0x46);
tp9963_write_register(ViPipe,0x22, 0x36);
tp9963_write_register(ViPipe,0x25, 0xfe);
tp9963_write_register(ViPipe,0x26, 0x01);
tp9963_write_register(ViPipe,0x2b, 0x60);
tp9963_write_register(ViPipe,0x2c, 0x3a);
tp9963_write_register(ViPipe,0x2d, 0x5a);
tp9963_write_register(ViPipe,0x2e, 0x40);
tp9963_write_register(ViPipe,0x30, 0x9e);
tp9963_write_register(ViPipe,0x31, 0x20);
tp9963_write_register(ViPipe,0x32, 0x10);
tp9963_write_register(ViPipe,0x33, 0x90);
tp9963_write_register(ViPipe,0x38, 0x40);
tp9963_write_register(ViPipe,0x35, 0x25);
tp9963_write_register(ViPipe,0x39, 0x08);

// 第二部分
tp9963_write_register(ViPipe,0x42, 0xf0);
tp9963_write_register(ViPipe,0x44, 0x01);
tp9963_write_register(ViPipe,0x51, 0x00);
tp9963_write_register(ViPipe,0x54, 0x03);
tp9963_write_register(ViPipe,0xea, 0x01);
tp9963_write_register(ViPipe,0xeb, 0x01);
tp9963_write_register(ViPipe,0xf6, 0x00);

// 第三部分
tp9963_write_register(ViPipe,0x40, 0x08);
tp9963_write_register(ViPipe,0x02, 0x80);
tp9963_write_register(ViPipe,0x03, 0x80);
tp9963_write_register(ViPipe,0x04, 0x80);
tp9963_write_register(ViPipe,0x05, 0x80);
tp9963_write_register(ViPipe,0x06, 0x80);
tp9963_write_register(ViPipe,0x15, 0x04); // 新增最后一行
	printf("ViPipe:%d,===TP9963 720P 25fps Init OK!===\n", ViPipe);
}

void tp9963_init(VI_PIPE ViPipe)
{
	// CVI_U8 u8ImgMode = TP9963_MODE_1080P_60P;
	// CVI_U8 u8ImgMode = TP9963_MODE_1080P_25P;

	// u8ImgMode = g_pastTP9963[ViPipe]->u8ImgMode;
	//if (isFirstInit) {
	if (tp9963_i2c_init(ViPipe) != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "tp9950 i2c init fail\n");
		return;
	}

		printf( "Loading Techpoint TP9963 sensor, ViPipe:%d\n", ViPipe);

		// check sensor chip id
		tp9963_write_register(ViPipe, 0x40, 0x0);
		if (tp9963_read_register(ViPipe, 0xfe) != 0x28 ||
			tp9963_read_register(ViPipe, 0xff) != 0x63)
		{
			printf( "read TP9963 chip id fail\n");
		}

	#if TP9963_BLUE_SCREEN
		tp9963_write_register(ViPipe, 0x40, 0x00);
		tp9963_write_register(ViPipe, 0x2A, 0x3C);
	#endif
	//}

	//return CVI_SUCCESS;

}

int AHD_tp9963_Init(VI_PIPE ViPipe, bool isFirstInit){

	if (isFirstInit) {
		if (tp9963_gpio_init(ViPipe) != CVI_SUCCESS) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "tp9963 gpio init fail\n");
			return CVI_FAILURE;
		}
	}

	usleep(20*1000);

	// if (isFirstInit) {
		tp9963_init(ViPipe);
	// }

	// tp9963_set_1080p_25(ViPipe);
	return CVI_SUCCESS;

}

int AHD_tp9963_Deinit(VI_PIPE ViPipe)
{
	tp9963_exit(ViPipe);
	return CVI_SUCCESS;
}


CVI_S32 AHD_tp9963_set_bus_info(VI_PIPE ViPipe, CVI_S32 astI2cDev)
{
	if (ViPipe > VI_MAX_PIPE_NUM - 1) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "invalid vipipe !!\n");
		return CVI_FAILURE;
	}
	g_aunTP9963_BusInfo[ViPipe].s8I2cDev = astI2cDev;
	return CVI_SUCCESS;
}


CVI_S32 AHD_tp9963_get_mode(VI_PIPE ViPipe)
{
	CVI_U8 lockstatus = 0;
	CVI_U8 detvideo = 0;
	SNS_AHD_MODE_S signal_type = AHD_MODE_NONE;

	lockstatus = tp9963_read_register(ViPipe, 0x01);
	detvideo = tp9963_read_register(ViPipe, 0x03);
	CVI_TRACE_SNS(CVI_DBG_INFO, "detvideo = 0x%2x, lockstatus = 0x%2x!!!\n",
			detvideo, lockstatus);
	// printf("detvideo = 0x%2x, lockstatus = 0x%2x!!!\n",
	// 		detvideo, lockstatus);

	if (((lockstatus & 0x80) == 0x00) &&
		(((lockstatus & 0x69) == 0x68) || ((detvideo & 0x07) < 0x6))) { // camera plug in
		// for  mode catch start
		if ((detvideo & 0x07) == 0x05) { //720p25fps
				signal_type = AHD_MODE_1280X720P25;
		} else if ((detvideo & 0x07) == 0x04) { //720p30fps
				signal_type = AHD_MODE_1280X720P30;
		} else if ((detvideo & 0x07) == 0x03) { //1080p25fps
			// printf("####### %s, %d\n", __func__, __LINE__);
				signal_type = AHD_MODE_1920X1080P25;
		} else if ((detvideo & 0x07) == 0x02) { //1080p30fps
				signal_type = AHD_MODE_1920X1080P30;
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "detect nothing! detvideo = 0x%2x, lockstatus = 0x%2x\n",
				detvideo, lockstatus);
			// printf("####### %s, %d\n", __func__, __LINE__);
			signal_type = AHD_MODE_NONE;
			return signal_type;
		}
	} else {
		// printf("####### %s, %d\n", __func__, __LINE__);
		signal_type = AHD_MODE_NONE;
		CVI_TRACE_SNS(CVI_DBG_INFO, "tp9950 has no signal!\n");
	}

	return signal_type;
}



CVI_S32 AHD_tp9963_set_mode(VI_PIPE ViPipe, CVI_S32 mode)
{
	switch (mode) {
	case AHD_MODE_1280X720P25:
		tp9963_set_720p_25(ViPipe);
		break;
	case AHD_MODE_1280X720P30:
		tp9963_set_720p_30(ViPipe);
		break;
	case AHD_MODE_1920X1080P25:
		tp9963_set_1080p_25(ViPipe);
		break;
	case AHD_MODE_1920X1080P30:
		tp9963_set_1080p_30(ViPipe);
		break;
	default:
		break;
	}
	return CVI_SUCCESS;
}

static CVI_U32 detect_cnt = 0;
CVI_S32 AHD_PR9963_detect_status(VI_PIPE ViPipe, CVI_S32 ahdOldType, CVI_S32 *ahdType)
{
	if (ViPipe > VI_MAX_PIPE_NUM - 1) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "invalid vipipe !!\n");
		return CVI_FAILURE;
	}
	CVI_S32 signal_type = -1;

	usleep(1000 * 1000);

	signal_type = AHD_tp9963_get_mode(ViPipe);
	if (ahdOldType == signal_type) {
		detect_cnt = 0;
		return CVI_FAILURE;
	}
	if (ahdOldType != signal_type && detect_cnt < 3) {
		detect_cnt++;
		return CVI_FAILURE;
	}
	*ahdType = signal_type;

	return CVI_SUCCESS;
}

SNS_AHD_OBJ_S stAhdTp9963Obj =
{
    .pfnAhdInit = AHD_tp9963_Init,
    .pfnAhdDeinit = AHD_tp9963_Deinit,
    .pfnGetAhdMode = AHD_tp9963_get_mode,
    .pfnSetAhdMode = AHD_tp9963_set_mode,
    // .pfnGetAhdMode = NULL,
    // .pfnSetAhdMode = NULL,
    .pfnSetAhdBusInfo = AHD_tp9963_set_bus_info,
	.pfnDetectAhdStatus = AHD_PR9963_detect_status,
};
