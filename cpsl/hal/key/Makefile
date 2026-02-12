include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_hal_key.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_hal_key.so

INCS		+= -I$(CURDIR)/include
INCS		+= -I$(SRCTREE)/cpsl/mmf/include
INCS		+= -I$(SRCTREE)/cpsl/common/log/include

INCS		+= -I$(CURDIR)/include
INCS		+= -I$(SRCTREE)/cpsl/hal/gpio/include
INCS		+= -I$(SRCTREE)/cpsl/hal/adc/include

INCS		+= -I$(CURDIR)/include
INCS		+= -I$(SRCTREE)/cpsl/common/sysutils/include
CFLAGS		+= -Wno-unused-result

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk
