include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_hal_touchpad.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_hal_touchpad.so

SYSUTILS_DIR:= $(SRCTREE)/cpsl/common/sysutils
INCS        += -I$(SYSUTILS_DIR)/include

INCS		+= -I$(CURDIR)/include
CFLAGS		+= -Wno-unused-result

ifeq ($(CONFIG_SCREEN_OTA7290B_MIPI11INCH), y)
CFLAGS   += -DCONFIG_SCREEN_OTA7290B_MIPI11INCH
else ifeq ($(CONFIG_SCREEN_ICN9707), y)
CFLAGS   += -DCONFIG_SCREEN_ICN9707
else ifeq ($(CONFIG_SCREEN_JD9165A), y)
CFLAGS    += -DCONFIG_SCREEN_JD9165A
else ifeq ($(CONFIG_SCREEN_LTD600), y)
CFLAGS    += -DCONFIG_SCREEN_LTD600
endif

ifeq ($(CONFIG_TOUCHPAD_GT9XX), y)
TOUCHPAD_TYPE := gt9xx
endif

SEXT		:= c
SDIR		:= $(CURDIR)/src/$(TOUCHPAD_TYPE)
include $(SRCTREE)/build/common_lib.mk
