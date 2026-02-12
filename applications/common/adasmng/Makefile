include $(SRCTREE)/applications/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_adasmng.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_adasmng.so

############################################
# End of Dependencies
############################################
INCS        += -I$(CURDIR)/include
CFLAGS      += -Wno-unused-parameter -Wno-unused-function

SEXT        := c
SDIR        := $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk
