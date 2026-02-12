include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcJSON.a
TARGET_SO	:= $(TARGET_DIR)/libcJSON.so

INCS		+= -I$(CURDIR)/include
CFLAGS		+= -Wno-unused-result

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk