include $(SRCTREE)/build/base.mk
include $(SRCTREE)/applications/build/inc.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_system.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_system.so

############################################
# End of Dependencies
############################################
CFLAGS		+= -Wno-unused-parameter

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk
