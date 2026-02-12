include $(SRCTREE)/applications/build/base.mk

TARGET_DIR	:= $(OBJTREE)/bin

ifeq ($(RUNTIME_ENV), pc)
CROSS_COMPILE	=
TARGET	:= $(TARGET_DIR)/ini2bin_host
else
TARGET	:= $(TARGET_DIR)/ini2bin_board
endif

############################################
# Dependencies
############################################
DYN_LIBS	+= -lm -lstdc++ -pthread -ldl

############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include
CFLAGS		+= -Wno-unused-parameter

# for stack trace
CFLAGS		+= -rdynamic  -funwind-tables -ffunction-sections

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_exe.mk
