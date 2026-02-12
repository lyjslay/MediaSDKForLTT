
#include "cvi_hal_gsensor.h"
#include "da380.h"
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define I2C_BUS_PREF "/dev/i2c-"
#define USAGE_MESSAGE                                   \
    "Usage:\n"                                          \
    "  %s r [addr] [register]   "                       \
    "to read value from [register]\n"                   \
    "  %s w [addr] [register] [value]   "               \
    "to write a value [value] to register [register]\n" \
    ""

#define abs(x) (((x) < 0) ? -(x) : (x))
#define IDC_SADDR_G_Sensor_W 0x27
#define IDC_SADDR_G_Sensor_R 0x27
// #define IDC_STYPE_G_Sensor IDC2

static int32_t i2c_file = 0;

/*return value: 0: is ok    other: is failed*/
static int32_t set_i2c_register(int32_t file, unsigned char addr, unsigned char reg,
			    unsigned char value)
{
	unsigned char outbuf[2];
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[1];

	messages[0].addr = addr;
	messages[0].flags = 0;
	messages[0].len = sizeof(outbuf);
	messages[0].buf = outbuf;

	/* The first byte indicates which register we'll write */
	outbuf[0] = reg;

	/*
	 * The second byte indicates the value to write.  Note that for many
	 * devices, we can write multiple, sequential registers at once by
	 * simply making outbuf bigger.
	 */
	outbuf[1] = value;

	/* Transfer the i2c packets to the kernel and verify it worked */
	packets.msgs = messages;
	packets.nmsgs = 1;
	if (ioctl(file, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		return 1;
	}

	return 0;
}

static int32_t get_i2c_register(int32_t file, unsigned char addr, unsigned char reg,
			    unsigned char *val)
{
	unsigned char inbuf, outbuf;
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[2];

	/*
	 * In order to read a register, we first do a "dummy write" by writing
	 * 0 bytes to the register we want to read from.  This is similar to
	 * the packet in set_i2c_register, except it's 1 byte rather than 2.
	 */
	outbuf = reg;
	messages[0].addr = addr;
	messages[0].flags = 0;
	messages[0].len = sizeof(outbuf);
	messages[0].buf = &outbuf;

	/* The data will get returned in this structure */
	messages[1].addr = addr;
	messages[1].flags = I2C_M_RD /* | I2C_M_NOSTART*/;
	messages[1].len = sizeof(inbuf);
	messages[1].buf = &inbuf;

	/* Send the request to the kernel and get the result back */
	packets.msgs = messages;
	packets.nmsgs = 2;
	if (ioctl(file, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		return 1;
	}
	*val = inbuf;

	return 0;
}

int32_t i2c_read_byte_data(unsigned char addr, unsigned char *data)
{
	int32_t ret = 0;
	ret = get_i2c_register(i2c_file, IDC_SADDR_G_Sensor_R, addr, data);
	return ret;
}

/*return value: 0: is ok    other: is failed*/
int32_t i2c_write_byte_data(unsigned char addr, unsigned char data)
{
	int32_t ret = 0;
	ret = set_i2c_register(i2c_file, IDC_SADDR_G_Sensor_W, addr, data);
	return ret;
}

int32_t da380_register_read(unsigned char addr, unsigned char *data)
{
	int32_t res = 0;

	res = i2c_read_byte_data(addr, data);
	if (res != 0) {
		return res;
	}

	return res;
}

int32_t da380_register_write(unsigned char addr, unsigned char data)
{
	int32_t res = 0;

	res = i2c_write_byte_data(addr, data);
	if (res != 0) {
		return res;
	}

	return res;
}

int32_t gsensor_i2c_bus_init(int32_t busnum)
{
	char busname[15];
	sprintf(busname, "%s%d", I2C_BUS_PREF, busnum);
	if ((i2c_file = open(busname, O_RDWR)) < 0) {
		perror("Unable to open i2c control file");
		return -1;
	}
	return 0;
}
int32_t gsensor_i2c_bus_deinit(void)
{
	close(i2c_file);
	return 0;
}

//映射active interrupt to INT pin
void da380_int_map1(unsigned char map_int)
{
    switch (map_int) {
    case 0x04://active interrupt to INT
        da380_register_write(INT_MAP1, 0x04);
        break;
    }
}


///获取当前三轴加速度值
int32_t gsensor_read_data(int32_t *acc_x, int32_t *acc_y, int32_t *acc_z)
{
///三轴加速度

    // uint16_t acc_x, acc_y, acc_z;
    uint8_t acc_x_lsb, acc_x_msb;
    uint8_t acc_y_lsb, acc_y_msb;
    uint8_t acc_z_lsb, acc_z_msb;

    // uint8_t acc_x_msb;
    // uint8_t acc_y_msb;
    // uint8_t acc_z_msb;

    da380_register_read(ACC_X_LSB, &acc_x_lsb);
    da380_register_read(ACC_X_MSB, &acc_x_msb);
    *acc_x = (((acc_x_lsb | (acc_x_msb << 8))) >> 4) & 0xFFF;
    // *acc_x = acc_x_msb;

    da380_register_read(ACC_Y_LSB, &acc_y_lsb);
    da380_register_read(ACC_Y_MSB, &acc_y_msb);
    *acc_y = (((acc_y_lsb | (acc_y_msb << 8))) >> 4) & 0xFFF;
    // *acc_y = acc_y_msb;

    da380_register_read(ACC_Y_LSB, &acc_z_lsb);
    da380_register_read(ACC_Y_MSB, &acc_z_msb);
    *acc_z = (((acc_z_lsb | (acc_z_msb << 8))) >> 4) & 0xFFF;
    // *acc_z = acc_z_msb;

    return 0;
}

//工作分辨率选着
void da380_resolution_range(unsigned char range)
{
    switch (range) {
    case G_SENSITY_HIGH:
        da380_register_write(RESOLUTION_RANGE, 0x0C);//14bit +/-2g     对应分辨率4096 LSB/g   高
        break;
    case G_SENSITY_MEDIUM:
        da380_register_write(RESOLUTION_RANGE, 0x0D);//14bit +/-4g     对应分辨率2048 LSB/g   中
        break;
    case G_SENSITY_LOW:
        /* da380_register_write(RESOLUTION_RANGE, 0x0F);//14bit +/-16g    对应分辨率512 LSB/g */
        da380_register_write(RESOLUTION_RANGE, 0x0E);//14bit +/-16g    对应分辨率512 LSB/g
        break;
    default :
        da380_register_write(RESOLUTION_RANGE, 0x0C);//14bit +/-2g     对应分辨率4096 LSB/g   高
        break;
    }
}


//工作模式选着
void da380_work_mode(unsigned char work_mode)
{
    switch (work_mode) {
    case G_NORMAL_MODE:
        da380_register_write(MODE_BW, 0X1E);//normal mode
        break;
    case G_LOW_POWER_MODE:
        da380_register_write(MODE_BW, 0X5E);//low power mode
        break;
    case G_SUSPEND_MODE:
        da380_register_write(MODE_BW, 0X9E);//suspend mode
        break;
    }
}

/**返回0 id正确*/
char da380_id_check(void)
{
    unsigned char chipid = 0x00;

    // chipid = da380_sensor_get_data(CHIPID_DA380);
    da380_register_read(CHIPID_DA380, &chipid);

    printf("CHIPID_da380: %x\n", chipid);

    if (chipid != 0x13) {
        printf("not da380 \n");
        return -1;
    }

    return 0;
}

/**返回0 id正确*/
int8_t da380_check(void)
{
    da380_register_write(RESET_DA380, 0x20);
    for (uint8_t i = 0; i < 5; i++) {
        // delay(10000);
        usleep(10 * 1000);
        if (!da380_id_check()) {
            return 0;
        }
    }
    return -1;
}

int32_t gsensor_init(void)//config RESOLUTION_RANGE ,MODE_BW,INT_MAP1, ACTIVE_DUR, ACTIVE_THS
{


    /* da380_register_write(RESET_DA380, 0x20); */
//    init_i2c_io();

    /* os_time_dly(10); */
    da380_check();
    da380_resolution_range(G_SENSITY_LOW);
    da380_register_write(ODR_AXIS,     0x06);//enable X/Y/Z axis,1000Hz
    da380_work_mode(G_LOW_POWER_MODE);//normal mode, 500Hz
    da380_register_write(SWAP_POLARITY, 0x00); //remain the polarity of X/Y/Z-axis

    /*INT中断配置， 分别有三种使用方式使能z轴，使能z和y轴，或者使能z,y和x，这三种方式*/

//    da380_register_write(INT_SET1,     0x04);//disable orient interrupt.enable the active interrupt for the  z,axis
//    da380_register_write(INT_SET1,     0x07);//disable orient interrupt.enable the active interrupt for the  z, y and x,axis
    da380_register_write(INT_SET1,     0x27);//disable orient interrupt.enable the active interrupt for the  z, y and x,axis
    da380_register_write(INT_SET2,     0x00);//disable the new data interrupt and the freefall interupt
    da380_int_map1(0x04);             //mapping active interrupt to INT
    da380_register_write(INT_MAP2,     0x00);//doesn't mappint new data interrupt to INT
    da380_register_write(INT_CONFIG,   0x01);//push-pull output for INT ,selects active level high for pin INT
    da380_register_write(INT_LTACH,    0x0E);///Burgess_151210
    da380_register_write(FREEFALL_DUR, 0x09);//freefall duration time = (freefall_dur + 1)*2ms
    da380_register_write(FREEFALL_THS, 0x30);//default is 375mg
    da380_register_write(FREEFALL_HYST, 0x01);
    da380_register_write(ACTIVE_DUR,   0x11);//Active duration time = (active_dur + 1) ms
    da380_register_write(ACTIVE_THS,   0x8F);
    da380_register_write(TAP_DUR,      0x04);//
    da380_register_write(TAP_THS,      0x0a);
    da380_register_write(ORIENT_HYST,  0x18);
    da380_register_write(Z_BLOCK,      0x08);
    da380_register_write(SELF_TEST,    0x00);//close self_test
    da380_register_write(CUSTOM_OFF_X, 0x00);
    da380_register_write(CUSTOM_OFF_Y, 0x00);
    da380_register_write(CUSTOM_OFF_Z, 0x00);
    da380_register_write(CUSTOM_FLAG,  0x00);
    da380_register_write(CUSTOM_CODE,  0X00);
    da380_register_write(Z_ROT_HODE_TM, 0x09);
    da380_register_write(Z_ROT_DUR,    0xFF);
    da380_register_write(ROT_TH_H,     0x45);
    da380_register_write(ROT_TH_L,     0x35);

    return 0;
}

extern int da380_pin_int_interrupt();
/**提供给二级子菜单回调函数使用*/
int gsensor_set_sensitity(CVI_HAL_GSENSOR_SENSITITY_E enSensitity)
{
    if (enSensitity == CVI_HAL_GSENSOR_SENSITITY_HIGH) {
        da380_work_mode(G_NORMAL_MODE);//normal mode
        da380_resolution_range(G_SENSITY_HIGH);
        da380_register_write(INT_CONFIG, 0x01);//push-pull output for INT ,selects active level high for pin INT
        da380_register_write(ACTIVE_THS, 0x3F);//0x4F);
    }

    if (enSensitity == CVI_HAL_GSENSOR_SENSITITY_MIDDLE) {
        da380_work_mode(G_NORMAL_MODE);//normal mode
        da380_resolution_range(G_SENSITY_MEDIUM);
        da380_register_write(INT_CONFIG, 0x01);//push-pull output for INT ,selects active level high for pin INT
        da380_register_write(ACTIVE_THS, 0x5F);//0x8F);
    }

    if (enSensitity == CVI_HAL_GSENSOR_SENSITITY_LOW) {
        da380_work_mode(G_NORMAL_MODE);//normal mode
        da380_resolution_range(G_SENSITY_LOW);
        da380_register_write(INT_CONFIG, 0x01);//push-pull output for INT ,selects active level high for pin INT
        da380_register_write(ACTIVE_THS, 0xaF);//0xFF);
    }

    if (enSensitity == CVI_HAL_GSENSOR_SENSITITY_OFF) {
        /* da380_register_write(RESET_DA380, 0x20); */
        gsensor_init();
        da380_resolution_range(G_SENSITY_LOW);
        da380_register_write(ACTIVE_THS, 0x00);//0xFF);
        da380_register_write(INT_CONFIG, 0x00);//no output for INT
        da380_work_mode(G_SUSPEND_MODE);//暂停
    }

    // if (gsid == G_SENSOR_LOW_POWER_MODE) { //低功耗
	// 	/* da380_register_write(RESET_DA380, 0x20); */
	// 	/* delay(5000); */
	// 	/* da380_init(); */
    //     da380_work_mode(G_LOW_POWER_MODE);
    //     da380_resolution_range(G_SENSITY_MEDIUM);
    //     da380_register_write(INT_CONFIG,   0x01);//push-pull output for INT ,selects active level high for pin INT
    //     /* da380_resolution_range(G_SENSITY_HIGH); */
    //     da380_register_write(ACTIVE_THS, 0xAF);//0xFF);
    //     da380_register_write(INT_LTACH, 0x0F);//latched
    // }
    return 0;
}

/**
返回值 TRUE  重力传感器触发
       FALSE  重力传感器未触发
*/
int32_t gsensor_read_int_status()
{
    unsigned char date_tmp;
    // date_tmp = da380_sensor_get_data(MOTION_FLAG);
    // date_tmp = da380_register_read(MOTION_FLAG);
    da380_register_read(MOTION_FLAG, &date_tmp);
    if (date_tmp & 0x24) {
        return 0;
    }

    return -1;
}
/**加速度超过与设定的阀值，产生中断。为当前视频文件上锁*/
int gsensor_open_interrupt()
{

    if (gsensor_read_int_status()) {
        return 0;
    }
    // return -EINVAL;
    return -1;
}


CVI_HAL_GSENSOR_OBJ_S gsensorObj = {
	.i2c_bus_init = gsensor_i2c_bus_init,
	.i2c_bus_deinit = gsensor_i2c_bus_deinit,
	.init = gsensor_init,
	// .deinit = da380_deinit,
	.read_data = gsensor_read_data,
	.set_sensitity = gsensor_set_sensitity,
	.read_int_status = gsensor_read_int_status,
	.open_interrupt = gsensor_open_interrupt,
	.read_interrupt = gsensor_read_int_status,
};



