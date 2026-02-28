include $(SRCTREE)/build/base.mk
include $(SRCTREE)/applications/build/inc.mk

CPRE := @
ifeq ($(V),1)
CPRE :=
VERB := --verbose
endif
.PHONY: prepare all clean

YOC_SDKPATH=$(SRCTREE)/platform/alios/solutions/yoc/yoc_sdk
ALIOS_PATH = $(SRCTREE)/platform/alios
MK_GENERATED_IMGS_PATH:=generated
ALGORITHMS_PATH:=$(SRCTREE)/platform/alios/components/chip_d1/d1_flash.elf
HOST_TOOLS := $(SRCTREE)/platform/alios/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin
CC = $(HOST_TOOLS)/riscv64-unknown-elf-gcc
CXX = $(HOST_TOOLS)/riscv64-unknown-elf-g++
AR = $(HOST_TOOLS)/riscv64-unknown-elf-ar
STRIP = $(HOST_TOOLS)/riscv64-unknown-elf-strip
OBJDUMP=$(HOST_TOOLS)/riscv64-unknown-elf-objdump
OBJCOPY=$(HOST_TOOLS)/riscv64-unknown-elf-objcopy
export PATH := $(HOST_TOOLS):$(PATH)

ifeq ($(CONFIG_RISCV), y)
	KBUILD_DEFINES += -DARCH_RISCV
else ifeq ($(CONFIG_ARM), y)
	KBUILD_DEFINES += -DARCH_ARM
endif

CLFAG		=-c -MP -MMD -Os -Wno-main -march=rv64imafdcxtheadc -mabi=lp64d -mcmodel=medany -mtune=c906 -c -g -ffunction-sections -fdata-sections -Wall -Werror
CCFLAG		=-MP -MMD -Os -Wno-main -march=rv64imafdcxtheadc -mabi=lp64d -mcmodel=medany -mtune=c906 -Wl,--gc-sections -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys -nostartfiles --specs=nosys.specs -Wl,-Map="yoc.map" -Wl,--whole-archive
CCFLAG		+=  $(KBUILD_DEFINES)
CLFAG  		+=  $(KBUILD_DEFINES) -DCONFIG_KERNEL_RHINO -DCONFIG_CPU_C906 -DCONFIG_CSI_V2 -D__ALIOS__ -DCVI_PARAM_MEDIA_CNT=$(MEDIA_PARAM_CNT)
ifeq ($(CHIP_TYPE), cv181x)
CLFAG		+= -D__CV181X__
else ifeq ($(CHIP_TYPE), cv180x)
CLFAG		+= -D__CV180X__
endif

OBJFALG		=	-Wl,--no-whole-archive
LD_SCRIPT	=	$(SRCTREE)/platform/alios/components/chip_cv181x/gcc_flash.ld
LIBSMMFDIR	=	-L$(SRCTREE)/platform/alios/components/cvi_mmf_sdk/$(CHIP_TYPE)
LIBSDIR		=	$(YOC_SDKPATH)/lib

LIBS=	-lcvi_mw_bin -lcvi_mw_isp_bin -lcvi_mw_isp_awb -lcvi_mw_isp_af -lcvi_mw_isp_ae \
		-lcvi_mw_isp_algo -lcvi_mw_isp -lcvi_mw_vi -lcvi_mw_vo -lcvi_mw_vpss -lcvi_mw_gdc \
		-lcvi_mw_rgn -lcvi_mw_ldc -lcvi_mw_venc -lcvi_mw_vdec -lcvi_mw_mipi_tx \
		-lcvi_mw_audio -lcvi_mw_sys -lcvi_link -lcvi_msg -lcvi_osdrv_base -lcvi_osdrv_cif -lcvi_osdrv_ipcm \
		-lcvi_osdrv_ldc -lcvi_osdrv_rgn -lcvi_osdrv_sys -lcvi_osdrv_tpu -lcvi_osdrv_vcodec -lcvi_osdrv_vi \
		-lcvi_osdrv_vo -lcvi_osdrv_vpss -lrhino -lrhino_arch -laos \
		-lcli -lkv -lulog -lcsi -ldebug -laoshal -lc_stub -luservice -lpartition -lkey_mgr -lsec_crypto \
		-lposix -lvfs -lcv181xc_evb -lchip_cv181x -lminialsa -ldrv_snd_cv181x -lcvi_sensor


ifeq ($(CONFIG_SCREEN),y)
LIBS += -lcvi_hal_screen
endif

ifeq ($(CONFIG_PWM),y)
LIBS += -lcvi_hal_pwm
endif

ifeq ($(CONFIG_GPIO),y)
LIBS += -lcvi_hal_gpio
endif

ifeq ($(CONFIG_ENABLE_ALIOS_TOOL), y)
LIBS+= -lcvi_test
endif

TARGET=yoc.elf
TARGETBIN=yoc.bin
TARGETASM=yoc.asm
INS = -I$(YOC_SDKPATH)/include
INS += -I$(YOC_SDKPATH)/include/devices/
INS += -I$(SRCTREE)/cpsl/mmf/component/isp/sensor/sensor_cfg
INS += -I$(SRCTREE)/cpsl/hal/screen/include
INS += -I$(SRCTREE)/cpsl/hal/pwm/include
INS += -I$(SRCTREE)/cpsl/hal/gpio/include
INS += -I$(CURDIR)/application/common/include
#INS += -I$(CURDIR)/application/hal/gpio/include
INS += -I$(CURDIR)/application/media/include

SEXT := c
SDIR := $(CURDIR)
ODIR := $(patsubst $(SRCTREE)/%,$(OBJTREE)/%,$(SDIR))
SRCS := $(shell find application -type f -name "*.$(SEXT)")
OBJS := $(addprefix $(ODIR)/,$(SRCS:.c=.o))
DEPS := $(addprefix $(ODIR)/,$(SRCS:.c=.d))


all: prepare $(TARGET)
	@echo "Build Solution by $(BOARD) "
ifeq ($(CONFIG_COMPRESS_RTOS_BIN), y)
	@cp yoc.bin yoc_bak.bin
	@lzma -zf yoc_bak.bin && mv yoc_bak.bin.lzma yoc_lzma.bin
endif
	@echo YoC SDK Done
	# @echo [INFO] Create bin files
	# @echo [INFO] Create imtb bin file
	# $(CPRE) product image $(MK_GENERATED_IMGS_PATH)/images.zip -i $(MK_GENERATED_IMGS_PATH)/data -l -p
	# $(CPRE) product image $(MK_GENERATED_IMGS_PATH)/images.zip -e $(MK_GENERATED_IMGS_PATH) -x

$(TARGET) : $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) -o $@ $(CCFLAG) $(LIBS) $^ $(OBJFALG) -T $(LD_SCRIPT) -L$(ALIOS_PATH)/solutions/yoc/script -L$(LIBSDIR) $(LIBSMMFDIR)
	$(OBJCOPY) -O binary $@ $(TARGETBIN)
	$(OBJDUMP) -d $@ > $(TARGETASM)

$(ODIR)/%.o : $(SDIR)/%.$(SEXT)
	mkdir -p $(dir $@)
	$(CC) -o $@ -c $(CLFAG) $(INS) $(INCS) $^

clean:
ifeq ($(CONFIG_ENABLE_ALIOS_TOOL), y)
	make -C $(SRCTREE)/platform/alios/components/cvi_test clean
endif
	make -C $(SRCTREE)/cpsl/mmf/component/isp/sensor/ -f Makefile.alios clean
	make -C $(SRCTREE)/platform/alios/solutions/yoc clean
	$(CPRE) rm -rf yoc_sdk binary out yoc.elf yoc.map generated yoc.asm yoc.bin
	$(CPRE) if [ -e cvi_board_memmap.ld ]; then rm -f cvi_board_memmap.ld; fi
	$(CPRE) if [ -e .sconsign.dblite ]; then rm -f .sconsign.dblite; fi
	@rm -f $(OBJS) $(DEPS) $(TARGET) $(TARGETBIN) $(TARGETASM)

prepare:
	make -C $(SRCTREE)/platform/alios/solutions/yoc all
	$(info CURDIR = $(CURDIR))
ifeq ($(CONFIG_ENABLE_ALIOS_TOOL), y)
	make -C $(SRCTREE)/platform/alios/components/cvi_test clean
	make -C $(SRCTREE)/platform/alios/components/cvi_test all
	make -C $(SRCTREE)/platform/alios/components/cvi_test install
endif
	make -C $(SRCTREE)/cpsl/mmf/component/isp/sensor/ -f Makefile.alios clean
	make -C $(SRCTREE)/cpsl/mmf/component/isp/sensor/ -f Makefile.alios all
	make -C $(SRCTREE)/cpsl/mmf/component/isp/sensor/ -f Makefile.alios install
ifeq ($(CONFIG_SCREEN),y)
	make -C $(SRCTREE)/cpsl/hal/screen -f Makefile.alios clean
	make -C $(SRCTREE)/cpsl/hal/screen -f Makefile.alios all
	make -C $(SRCTREE)/cpsl/hal/screen -f Makefile.alios install
endif
ifeq ($(CONFIG_GPIO),y)
	make -C $(SRCTREE)/cpsl/hal/gpio -f Makefile.alios clean
	make -C $(SRCTREE)/cpsl/hal/gpio -f Makefile.alios all
	make -C $(SRCTREE)/cpsl/hal/gpio -f Makefile.alios install
endif
ifeq ($(CONFIG_PWM),y)
	make -C $(SRCTREE)/cpsl/hal/pwm -f Makefile.alios clean
	make -C $(SRCTREE)/cpsl/hal/pwm -f Makefile.alios all
	make -C $(SRCTREE)/cpsl/hal/pwm -f Makefile.alios install
endif
	cp $(SRCTREE)/platform/alios/solutions/yoc/cvi_board_memmap.ld -rf .