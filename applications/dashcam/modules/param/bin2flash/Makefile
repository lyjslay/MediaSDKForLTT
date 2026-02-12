include $(SRCTREE)/applications/build/base.mk

TARGET_DIR	:= $(OBJTREE)/bin
TARGET	:= $(TARGET_DIR)/bin2flash

############################################
# Dependencies
############################################
PARAM_DIR	:=  $(SRCTREE)/applications/dashcam/modules/param/core
LIBS		+= -L$(PARAM_DIR)/$(CVILIB_DIR) -lcvi_param

FLASH_DIR	:= $(SRCTREE)/cpsl/hal/flash
LIBS		+= -L$(FLASH_DIR)/$(CVILIB_DIR) -lcvi_hal_flash

LOG_DIR     := $(SRCTREE)/cpsl/common/log
LIBS		+= -L$(LOG_DIR)/$(CVILIB_DIR) -lcvi_log

DYN_LIBS	+= -lm -lstdc++ -pthread -ldl
############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include
CFLAGS		+= -Wno-unused-parameter

# for stack trace
CFLAGS		+= -rdynamic  -funwind-tables -ffunction-sections

MEDIA_PARAM_CNT   = $(shell grep -nr "config_media_cam*" $(PARAM_FULL_PATH)/config_access_entry.ini |grep -v comm |wc -l)
CFLAGS 		+= -DCVI_PARAM_MEDIA_CNT=$(MEDIA_PARAM_CNT)

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_exe.mk
