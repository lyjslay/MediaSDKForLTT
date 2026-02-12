include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(OBJTREE)/bin
TARGET		:= $(TARGET_DIR)/sample_upgrade

############################################
# Dependencies
############################################
MMF_DIR		:= $(SRCTREE)/cpsl/mmf
INCS		+= -I$(MMF_DIR)/include
INCS		+= -I$(MMF_DIR)/sample/common

UPG_DIR		:= $(SRCTREE)/cpsl/common/upgrade
INCS		+= -I$(UPG_DIR)/include
LIBS		+= -L$(UPG_DIR)/lib -lcvi_upgrade

FLASH_DIR       := $(SRCTREE)/cpsl/hal/flash
INCS            += -I$(FLASH_DIR)/include
LIBS            += -L$(FLASH_DIR)/lib -lcvi_hal_flash

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include
LIBS		+= -L$(LOG_DIR)/lib -lcvi_log

############################################
# End of Dependencies
############################################

SEXT		:= c
SDIR		:= $(CURDIR)
include $(SRCTREE)/build/common_exe.mk
