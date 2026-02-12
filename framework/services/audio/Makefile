include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_audio_service.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_audio_service.so

############################################
# Dependencies
############################################
MAPI_DIR	:= $(SRCTREE)/framework/mapi
INCS		+= -I$(MAPI_DIR)/include
LIBS		+= -L$(MAPI_DIR)/lib -lcvi_mapi

PERF_DIR	:= $(SRCTREE)/framework/components/perf
INCS		+= -I$(PERF_DIR)/include
LIBS		+= -L$(PERF_DIR)/lib -lcvi_perf

MQ_DIR		:= $(SRCTREE)/cpsl/common/mq
INCS		+= -I$(MQ_DIR)/include
LIBS		+= -L$(MQ_DIR)/lib -lcvi_mq

OSAL_DIR	:= $(SRCTREE)/cpsl/osal
INCS		+= -I$(OSAL_DIR)/include
LIBS		+= -L$(OSAL_DIR)/lib -lcvi_osal -lpthread

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include
LIBS		+= -L$(LOG_DIR)/lib -lcvi_log
############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk
