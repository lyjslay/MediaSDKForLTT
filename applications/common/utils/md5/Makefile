include $(SRCTREE)/applications/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_md5.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_md5.so

############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include
CFLAGS		+= -Wno-unused-parameter

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk