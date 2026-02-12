include $(SRCTREE)/applications/build/base.mk

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_devmng.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_devmng.so

INCS		+= -I$(CURDIR)/include
CFLAGS		+= -Wno-unused-parameter -Wno-unused-function

SEXT		:= c
SDIR		:= $(CURDIR)/src

ifneq ($(CONFIG_ADC),y)
exclude_src	+= $(SDIR)/cvi_gaugemng.c
endif
ifneq ($(CONFIG_KEY),y)
exclude_src	+= $(SDIR)/cvi_keymng.c
endif
ifneq ($(CONFIG_LED),y)
exclude_src	+= $(SDIR)/cvi_ledmng.c
endif
ifneq ($(CONFIG_GSENSOR),y)
exclude_src	+= $(SDIR)/cvi_gsensormng.c
endif
ifneq ($(CONFIG_WIFI),y)
exclude_src	+= $(SDIR)/cvi_wifimng.c
endif
ifneq ($(CONFIG_WATCHDOG),y)
exclude_src	+= $(SDIR)/cvi_watchdogmng.c
endif

SRCS_TEMP	= $(shell find $(SDIR) -type f -name "*.$(SEXT)")
SRCS		= $(filter-out $(exclude_src), $(SRCS_TEMP))

include $(SRCTREE)/build/common_lib.mk
