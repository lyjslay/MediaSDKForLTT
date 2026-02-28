include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_upgrade.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_upgrade.so

############################################
# Dependencies
############################################
# MMF_DIR		:= $(SRCTREE)/cpsl/mmf
# INCS		+= -I$(MMF_DIR)/include
# INCS		+= -I$(MMF_DIR)/sample/common

FLASH_DIR   := $(SRCTREE)/cpsl/hal/flash
INCS        += -I$(FLASH_DIR)/include

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include

OSAL_DIR	:= $(SRCTREE)/cpsl/osal
INCS		+= -I$(OSAL_DIR)/include
############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk