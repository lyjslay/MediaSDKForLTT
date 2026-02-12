#define GPIO_SPKEN_GRP 4
#define GPIO_SPKEN_NUM 2

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)


void _GPIOSetValue(u8 gpio_grp, u8 gpio_num, u8 level)
{
	csi_error_t ret;
	csi_gpio_t gpio = {0};

	ret = csi_gpio_init(&gpio, gpio_grp);
	if(ret != CSI_OK) {
		printf("csi_gpio_init failed\r\n");
		return;
	}
	// gpio write
	ret = csi_gpio_dir(&gpio , GPIO_PIN_MASK(gpio_num), GPIO_DIRECTION_OUTPUT);

	if(ret != CSI_OK) {
		printf("csi_gpio_dir failed\r\n");
		return;
	}
	csi_gpio_write(&gpio , GPIO_PIN_MASK(gpio_num), level);
	//printf("test pin end and success.\r\n");
}

void _AudioPinmux(void)
{
    PINMUX_CONFIG(PWR_GPIO2, PWR_GPIO_2);
    PLATFORM_SpkMute(1);
}

void _UartPinmux()
{
	// uart1 pinmux
    PINMUX_CONFIG(IIC0_SCL, UART1_TX);
	PINMUX_CONFIG(IIC0_SDA, UART1_RX);
}

void _SensorPinmux()
{
    // //Sensor Pinmux
	PINMUX_CONFIG(PAD_MIPI_TXP1, IIC2_SCL);
	PINMUX_CONFIG(PAD_MIPI_TXM1, IIC2_SDA);
	PINMUX_CONFIG(PAD_MIPI_TXP0, XGPIOC_13);
	PINMUX_CONFIG(PAD_MIPI_TXP0, CAM_MCLK0);
	PINMUX_CONFIG(PAD_MIPI_TXM0, CAM_MCLK1);
}

void _MipiRxPinmux(void)
{

}

void _MipiTxPinmux(void)
{

}

void _VBatPinmux(void)
{
    //pinmux init vbat 
    // PINMUX_CONFIG(PWR_VBAT_DET, PWR_VBAT_DET);//PWR_ADC3
    // PINMUX_CONFIG(PWR_GPIO1, PWR_GPIO_1);//adc5
    // mmio_write_32(0x05027030, 0x40);
}

void _BoardPinmux(void)
{
    //pinmux init 
    _UartPinmux();
    _MipiRxPinmux();
    _MipiTxPinmux();
    _SensorPinmux();
    _AudioPinmux();
	_VBatPinmux();
}