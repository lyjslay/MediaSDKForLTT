include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_muxer.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_muxer.so

############################################
# Dependencies
############################################
FFMPEG_DIR	:= $(SRCTREE)/cpsl/third_party/ffmpeg
INCS		+= -I$(FFMPEG_DIR)/include

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include

############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include


SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk
