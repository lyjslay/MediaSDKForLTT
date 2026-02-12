include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_photo_service.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_photo_service.so

############################################
# Dependencies
############################################
MAPI_DIR	:= $(SRCTREE)/framework/mapi
INCS		+= -I$(MAPI_DIR)/include

AUDIO_DIR   := $(SRCTREE)/framework/services/audio
INCS		+= -I$(AUDIO_DIR)/include

RECR_DIR	:= $(SRCTREE)/framework/components/recorder
INCS		+= -I$(RECR_DIR)/include

MUXER_DIR	:= $(SRCTREE)/framework/components/muxer
INCS		+= -I$(MUXER_DIR)/include

DTCF_DIR	:= $(SRCTREE)/cpsl/common/dtcf
INCS		+= -I$(DTCF_DIR)/include

FFMPEG_DIR	:= $(SRCTREE)/cpsl/third_party/ffmpeg
INCS		+= -I$(FFMPEG_DIR)/include

PERF_DIR	:= $(SRCTREE)/framework/components/perf
INCS		+= -I$(PERF_DIR)/include

EXIF_DIR	:= $(SRCTREE)/cpsl/common/exif
INCS		+= -I$(EXIF_DIR)/include

MMF_DIR		:= $(SRCTREE)/cpsl/mmf
INCS		+= -I$(MMF_DIR)/include

MQ_DIR		:= $(SRCTREE)/cpsl/common/mq
INCS		+= -I$(MQ_DIR)/include

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
