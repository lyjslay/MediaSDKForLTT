include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_recorder.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_recorder.so

############################################
# Dependencies
############################################
MUXER_DIR	:= $(SRCTREE)/framework/components/muxer
INCS		+= -I$(MUXER_DIR)/include

RINGBUFFER_DIR := $(SRCTREE)/framework/components/ringbuffer
INCS		+= -I$(RINGBUFFER_DIR)/include

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include

OSAL_DIR	:= $(SRCTREE)/cpsl/osal
INCS		+= -I$(OSAL_DIR)/include

############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include
CXXFLAGS	+= -g

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk
