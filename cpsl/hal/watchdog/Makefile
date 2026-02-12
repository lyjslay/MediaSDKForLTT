# Module build makefile
# CUR_ROOT    := $(shell pwd)
# REF_ROOT    ?= $(CUR_ROOT)/../..
# OS_TYPE     := Linux
# include $(REF_ROOT)/build/base.mak

# MODULE_ROOT := $(CUR_ROOT)
# MODULE_NAME := hal$(CUR_DIR_NAME)

# SRC_ROOT    := $(MODULE_ROOT)/src
# SRC_DIR     := $(SRC_ROOT)
# OBJ_ROOT    := $(REF_OUT_HAL_OBJ)/$(OS_TYPE)/$(MODULE_NAME)
# LIB_ROOT    := $(REF_OUT_HAL_LIB)/$(OS_TYPE)

# include $(REF_ROOT)/build/modbuild.mak

include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_hal_watchdog.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_hal_watchdog.so

INCS		+= -I$(CURDIR)/include
INCS		+= -I$(SRCTREE)/cpsl/mmf/include
INCS		+= -I$(SRCTREE)/cpsl/common/log/include
INCS		+= -I$(SRCTREE)/cpsl/hal/gpio/include
INCS		+= -I$(SRCTREE)/cpsl/common/sysutils/include
INCS		+= -I$(SRCTREE)/cpsl/hal/watchdog/include
INCS		+= -I$(SRCTREE)/cpsl/hal/key/include
INCS		+= -I$(SRCTREE)/cpsl/common/sysutils/include
INCS		+= -I$(SRCTREE)/cpsl/mmf/include
INCS		+= -I$(SRCTREE)/applications/common/utils/include

CFLAGS		+= -Wno-unused-result

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk