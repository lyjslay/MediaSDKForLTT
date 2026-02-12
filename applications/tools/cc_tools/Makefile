include $(SRCTREE)/applications/build/base.mk

TARGET_DIR	:= $(OBJTREE)/bin
TARGET		:= $(TARGET_DIR)/cc_tools

############################################
# Dependencies
############################################
MQ_DIR		:= $(SRCTREE)/cpsl/common/mq
INCS		+= -I$(MQ_DIR)/include
LIBS		+= -L$(MQ_DIR)/$(CVILIB_DIR) -lcvi_mq

OSAL_DIR	:= $(SRCTREE)/cpsl/osal
INCS		+= -I$(OSAL_DIR)/include
LIBS		+= -L$(OSAL_DIR)/$(CVILIB_DIR) -lcvi_osal

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include
LIBS		+= -L$(LOG_DIR)/$(CVILIB_DIR) -lcvi_log

DYN_LIBS	+= -lm -lstdc++ -pthread -ldl
############################################
# End of Dependencies
############################################
CFLAGS		+= -Wno-unused-parameter

SEXT		:= c
SDIR		:= $(CURDIR)
include $(SRCTREE)/build/common_exe.mk
