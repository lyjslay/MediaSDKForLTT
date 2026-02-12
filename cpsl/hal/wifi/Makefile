include $(SRCTREE)/build/base.mk

wifi-y += src/common
wifi-$(CONFIG_WIFI_RTL8189)              += src/rtl8189fs
wifi-$(CONFIG_WIFI_AP6201BM)             += src/ap6201bm
wifi-$(CONFIG_WIFI_HI3881)               += src/hi3881
wifi-$(CONFIG_WIFI_RTL8188)              += src/rtl8188fu
wifi-$(CONFIG_WIFI_AIC8800)              += src/aic8800

SEXT := c
SDIR := $(CURDIR)
ODIR := $(patsubst $(SRCTREE)/%,$(OBJTREE)/%,$(SDIR)/$(CVILIB_DIR))
SRCS := $(foreach dir,$(wifi-y),$(wildcard $(dir)/*.$(SEXT)))
OBJS := $(addprefix $(ODIR)/,$(SRCS:.c=.o))
DEPS := $(addprefix $(ODIR)/,$(SRCS:.c=.d))

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_hal_wifi.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_hal_wifi.so

INCS		+= -I$(CURDIR)/include
INCS		+= -I$(CURDIR)/src/common
INCS		+= -I$(SRCTREE)/cpsl/common/sysutils/include
INCS		+= -I$(SRCTREE)/cpsl/hal/gpio/include
CFLAGS		+= -Wno-unused-result

CFLAGS      += $(INCS)
CFLAGS      += -fPIC

.PHONY : clean all install
all : $(TARGET_A) $(TARGET_SO)

$(ODIR)/%.o : $(SDIR)/%.$(SEXT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET_DIR) :
	mkdir -p $@

$(TARGET_A) : $(OBJS) | $(TARGET_DIR)
	$(AR) $(ARFLAGS) $@ $^

$(TARGET_SO) : $(OBJS) | $(TARGET_DIR)
	$(CC) $(LDFLAGS_SO) -o $@ $^

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO)

$(INSTALL_PATH)/lib :
	mkdir -p $@

install: $(TARGET_A) $(TARGET_SO) | $(INSTALL_PATH)/lib
ifeq ($(CONFIG_WIFI_HI3881), y)
	mkdir -p $(INSTALL_PATH)/vendor
	cp src/hi3881/wifi_cfg $(INSTALL_PATH)/vendor
	cp src/hi3881/hi3881_fw.bin $(INSTALL_PATH)/vendor
endif
	cp $^ $(INSTALL_PATH)/lib

ifeq ($(CONFIG_WIFI_AIC8800), y)
	mkdir -p $(INSTALL_PATH)/bin/aic8800
	cp -r src/aic8800/aic8800/* $(INSTALL_PATH)/bin/aic8800
endif

-include $(DEPS)
