include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_mapi.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_mapi.so

############################################
# Dependencies
############################################


LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include

OSAL_DIR	:= $(SRCTREE)/cpsl/osal
INCS		+= -I$(OSAL_DIR)/include

GPIO_DIR	:= $(SRCTREE)/cpsl/hal/gpio
INCS		+= -I$(GPIO_DIR)/include
############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include
INCS		+= -I$(CURDIR)/src/aac
CFLAGS		+= -Wno-unused-parameter

SEXT		:= c
SDIR		= $(CURDIR)/src
#SDIR		+= $(CURDIR)/src/aac
include $(SRCTREE)/build/common_lib.mk
