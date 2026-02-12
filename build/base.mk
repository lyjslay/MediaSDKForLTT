-include $(KCONFIG_FILE)
include $(SRCTREE)/Kbuild
include $(SRCTREE)/MediaSDK.version

CROSS_COMPILE	?= $(CONFIG_CROSS_COMPILE:"%"=%)
CROSS_COMPILE_KERNEL	?= $(CONFIG_CROSS_COMPILE_KERNEL:"%"=%)
CROSS_COMPILE_UBOOT	?= $(CONFIG_CROSS_COMPILE_UBOOT:"%"=%)
CMAKE_CFLAGS ?= $(CONFIG_CMAKE_CFLAGS)

# Make variables (CC, etc...)
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CXX		= $(CROSS_COMPILE)g++
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
AWK		= awk
PERL		= perl
PYTHON		= python
CHECK		= sparse

# Compile flags
OPT_FLAGS	= -O3
DEP_FLAGS	= -MMD

CFLAGS		+= -g -Wall -Wextra -Werror -std=gnu99 -D_GNU_SOURCE -Wno-stringop-truncation \
			-Wno-format-truncation -Wno-type-limits -Wno-enum-conversion -Wno-implicit-fallthrough \
			-Wno-stringop-overflow -Wno-sizeof-pointer-memaccess
CXXFLAGS	+= -g -Wall -Wextra -Werror -std=gnu++11 -D_GNU_SOURCE -Wno-stringop-truncation

CFLAGS		+= $(OPT_FLAGS) $(DEP_FLAGS) $(KBUILD_DEFINES)
CXXFLAGS	+= $(OPT_FLAGS) $(DEP_FLAGS) $(KBUILD_DEFINES)

#support large file manipulate
ifeq ($(CONFIG_32BIT), y)
CFLAGS		+= -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CXXFLAGS	+= -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
endif

ifeq ($(CONFIG_TOOLCHAIN_GLIBC_ARM64),y)
CVILIB_DIR := lib_arm64
else ifeq ($(CONFIG_TOOLCHAIN_GLIBC_ARM),y)
CVILIB_DIR := lib_arm
else ifeq ($(CONFIG_TOOLCHAIN_UCLIBC_ARM),y)
CVILIB_DIR := lib_uclibc
else ifeq ($(CONFIG_TOOLCHAIN_GLIBC_RISCV64),y)
CVILIB_DIR := lib_glibc_riscv64
else ifeq ($(CONFIG_TOOLCHAIN_MUSL_RISCV64),y)
CVILIB_DIR := lib_musl_riscv64
endif

ARFLAGS		= rcs
LDFLAGS_SO	= -shared -fPIC -Wl,--gc-sections

MMF_DIR		:= $(SRCTREE)/cpsl/mmf
INCS        += -I${MMF_DIR}/include
INCS        += -I${MMF_DIR}/include/isp/$(CHIP_TYPE)
INCS        += -I${MMF_DIR}/component/isp/sensor/sensor_cfg

# Config file path
PARAM_PATH_PREFIX = $(SRCTREE)/applications/$(CFG_PDT)/modules/param/inicfgs

BRAND=cvitek
BOARD_NAME=$(BOARD_TYPE)_$(STORAGE_TYPE)
BOARD_DIR=$(SRCTREE)/build/boards/$(CHIP_TYPE)/$(CHIP_NAME)_$(BOARD_NAME)

ifeq ($(CONFIG_APP_DASHCAM), y)
PARAM_FULL_PATH=$(PARAM_PATH_PREFIX)/$(CHIP_TYPE)/$(CFG_PDT_SUB)/$(PARAM_PATH_SENSOR0)$(PARAM_PATH_SENSOR1)$(PARAM_PATH_SENSOR2)$(PARAM_PATH_RESO)_$(UI_WIDTH)_$(UI_HEIGHT)_$(BOARD_DDR)
MEDIA_PARAM_CNT   = $(shell grep -nr "config_media_cam*" $(PARAM_FULL_PATH)/config_access_entry.ini |grep -v comm |wc -l)
CFLAGS 		+= -DCVI_PARAM_MEDIA_CNT=$(MEDIA_PARAM_CNT)
MAX_DEV_NUM = $(shell grep "dev_num" $(PARAM_FULL_PATH)/config_devmng.ini | awk '{print $$3}')
CFLAGS 		+= -DMAX_DEV_INSTANCES=$(MAX_DEV_NUM)
MAX_SENSOR_NUM = $(shell grep "cam_num" $(PARAM_FULL_PATH)/config_devmng.ini | awk '{print $$3}')
CFLAGS 		+= -DMAX_CAMERA_INSTANCES=$(MAX_SENSOR_NUM)
KBUILD_DEFINES += -DMAX_CAMERA_INSTANCES=$(MAX_SENSOR_NUM) -DMAX_DEV_INSTANCES=$(MAX_DEV_NUM)
endif


