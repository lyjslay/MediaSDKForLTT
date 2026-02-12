include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_qrcode_service.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_qrcode_service.so

############################################
# Dependencies
############################################
MAPI_DIR	:= $(SRCTREE)/framework/mapi
INCS		+= -I$(MAPI_DIR)/include

ifeq ($(CONFIG_ZBAR), y)
ZBAR_DIR    := $(SRCTREE)/cpsl/third_party/zbar-0.10
INCS        += -I$(ZBAR_DIR)/include
endif

MMF_DIR		:= $(SRCTREE)/cpsl/mmf
INCS		+= -I$(MMF_DIR)/include

OSAL_DIR	:= $(SRCTREE)/cpsl/osal
INCS		+= -I$(OSAL_DIR)/include

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include


############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk
