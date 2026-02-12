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
	// PINMUX_CONFIG(IIC0_SCL, UART1_TX);
	// PINMUX_CONFIG(IIC0_SDA, UART1_RX);
    PINMUX_CONFIG(JTAG_CPU_TMS, UART1_TX);
	PINMUX_CONFIG(JTAG_CPU_TCK, UART1_RX);
}

void _SensorPinmux()
{
    //Sensor Pinmux
    PINMUX_CONFIG(CAM_PD0, XGPIOA_1);
    PINMUX_CONFIG(IIC3_SCL, IIC3_SCL);
    PINMUX_CONFIG(IIC3_SDA, IIC3_SDA);
    PINMUX_CONFIG(CAM_RST0, XGPIOA_2);
    PINMUX_CONFIG(CAM_MCLK0, CAM_MCLK0);
    _GPIOSetValue(0, 1, 1);
    _GPIOSetValue(0, 2, 1);

    PINMUX_CONFIG(ADC3, IIC4_SCL);
	PINMUX_CONFIG(ADC2, IIC4_SDA);
	PINMUX_CONFIG(USB_VBUS_EN, XGPIOB_5);
	PINMUX_CONFIG(PAD_MIPIRX5N, XGPIOC_0);
    PINMUX_CONFIG(CAM_MCLK1, CAM_MCLK1);
    _GPIOSetValue(1, 5, 1);
    _GPIOSetValue(2, 0, 1);
#if CONFIG_BOARD_CV181XH
#else
    // PINMUX_CONFIG(PAD_MIPI_TXP1, IIC2_SCL);
    // PINMUX_CONFIG(PAD_MIPI_TXM1, IIC2_SDA);
    // PINMUX_CONFIG(PAD_MIPI_TXP0, XGPIOC_13);
    // PINMUX_CONFIG(PAD_MIPI_TXM0, CAM_MCLK1);
#endif
}

void _MipiRxPinmux(void)
{
//mipi rx pinmux
#if 0//need porting for phobos
    PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
    PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2);
#endif
}

void _MipiTxPinmux(void)
{
// //mipi tx pinmux
// #if CONFIG_PANEL_ILI9488
// 	PINMUX_CONFIG(PAD_MIPI_TXM1, XGPIOC_14);
// 	PINMUX_CONFIG(PAD_MIPI_TXP1, XGPIOC_15);
// 	PINMUX_CONFIG(PAD_MIPI_TXM2, XGPIOC_16);
// 	PINMUX_CONFIG(PAD_MIPI_TXP2, XGPIOC_17);
// 	PINMUX_CONFIG(IIC0_SCL, XGPIOA_28);
// #elif (CONFIG_PANEL_HX8394)
// #if CONFIG_BOARD_CV181XC
// 	PINMUX_CONFIG(PAD_MIPI_TXM0, XGPIOC_12);
// 	PINMUX_CONFIG(PAD_MIPI_TXP0, XGPIOC_13);
// 	PINMUX_CONFIG(PAD_MIPI_TXM1, XGPIOC_14);
// 	PINMUX_CONFIG(PAD_MIPI_TXP1, XGPIOC_15);
// 	PINMUX_CONFIG(PAD_MIPI_TXM2, XGPIOC_16);
// 	PINMUX_CONFIG(PAD_MIPI_TXP2, XGPIOC_17);
// 	PINMUX_CONFIG(JTAG_CPU_TCK, XGPIOA_18);
// 	PINMUX_CONFIG(JTAG_CPU_TMS, XGPIOA_19);
// 	PINMUX_CONFIG(SPK_EN, XGPIOA_15);
// #elif (CONFIG_BOARD_CV181XH)
	PINMUX_CONFIG(PAD_MIPI_TXM0, XGPIOC_12);
	PINMUX_CONFIG(PAD_MIPI_TXP0, XGPIOC_13);
	PINMUX_CONFIG(PAD_MIPI_TXM1, XGPIOC_14);
	PINMUX_CONFIG(PAD_MIPI_TXP1, XGPIOC_15);
	PINMUX_CONFIG(PAD_MIPI_TXM2, XGPIOC_16);
	PINMUX_CONFIG(PAD_MIPI_TXP2, XGPIOC_17);
	PINMUX_CONFIG(PAD_MIPI_TXM3, XGPIOC_20);
	PINMUX_CONFIG(PAD_MIPI_TXP3, XGPIOC_21);
	PINMUX_CONFIG(PAD_MIPI_TXM4, XGPIOC_18);
	PINMUX_CONFIG(PAD_MIPI_TXP4, XGPIOC_19);
// #endif
// #endif
}

void _VBatPinmux(void)
{
    //pinmux init vbat 
    PINMUX_CONFIG(PWR_VBAT_DET, PWR_VBAT_DET);//PWR_ADC3
    PINMUX_CONFIG(PWR_GPIO1, PWR_GPIO_1);//adc5
    mmio_write_32(0x05027030, 0x40);
}

void _BoardPinmux(void)
{
	PINMUX_CONFIG(PWR_GPIO0, PWM_8); //panel PWM pinmux
    //pinmux init 
    _UartPinmux();
    _MipiRxPinmux();
    _MipiTxPinmux();
    _SensorPinmux();
    _AudioPinmux();
	_VBatPinmux();
}