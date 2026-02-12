SRCTREE ?= ../../../
include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_player_service.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_player_service.so

############################################
# Dependencies
############################################
MAPI_DIR	:= $(SRCTREE)/framework/mapi
INCS		+= -I$(MAPI_DIR)/include

PLAYER_DIR	:= $(SRCTREE)/framework/components/player
INCS		+= -I$(PLAYER_DIR)/include

MEMORY_ALLOCATOR_DIR := $(SRCTREE)/framework/components/memory_allocator
INCS		+= -I$(MEMORY_ALLOCATOR_DIR)/include

DEMUXER_DIR	:= $(SRCTREE)/framework/components/demuxer
INCS		+= -I$(DEMUXER_DIR)/include

FILE_RECOVER_DIR:= $(SRCTREE)/framework/components/file_recover
INCS		+= -I$(FILE_RECOVER_DIR)/include

FFMPEG_DIR	:= $(SRCTREE)/cpsl/third_party/ffmpeg
INCS		+= -I$(FFMPEG_DIR)/include

PERF_DIR	:= $(SRCTREE)/framework/components/perf
INCS		+= -I$(PERF_DIR)/include

HAL_DIR		:= $(SRCTREE)/cpsl/hal
INCS		+= -I$(HAL_DIR)/include
INCS		+= -I$(HAL_DIR)/screen/include

MQ_DIR		:= $(SRCTREE)/cpsl/common/mq
INCS		+= -I$(MQ_DIR)/include

SIGSLOT_DIR	:= $(SRCTREE)/cpsl/common/signal_slot
INCS		+= -I$(SIGSLOT_DIR)/include

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
