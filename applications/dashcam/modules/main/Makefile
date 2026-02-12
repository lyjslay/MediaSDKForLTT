include $(SRCTREE)/applications/build/base.mk
include $(SRCTREE)/applications/build/lib.mk

TARGET_DIR	:= $(OBJTREE)/bin
TARGET		:= $(TARGET_DIR)/cardv_app

CFLAGS		+= -Wno-unused-parameter

# for stack trace
CFLAGS		+= -rdynamic  -funwind-tables -ffunction-sections

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_exe.mk
