include $(SRCTREE)/applications/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_mode.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_mode.so

############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include
CFLAGS		+= -Wno-unused-parameter -Wno-unused-function

SEXT		:= c
SDIR		:= $(CURDIR)/src

ifneq ($(CONFIG_SERVICES_PHOTO),y)
exclude_src	+= $(SDIR)/cvi_photomode.c
endif

SRCS_TEMP	= $(shell find $(SDIR) -type f -name "*.$(SEXT)")
SRCS		= $(filter-out $(exclude_src), $(SRCS_TEMP))

include $(SRCTREE)/build/common_lib.mk
