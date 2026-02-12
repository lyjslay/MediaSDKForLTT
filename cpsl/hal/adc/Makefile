include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_hal_adc.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_hal_adc.so

INCS		+= -I$(CURDIR)/include
INCS		+= -I$(SRCTREE)/cpsl/common/sysutils/include
INCS		+= -I$(SRCTREE)/cpsl/common/log/include
INCS		+= -I$(SRCTREE)/applications/common/utils/include
CFLAGS		+= -Wno-unused-result

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk
