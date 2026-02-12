include $(SRCTREE)/build/base.mk

screen-y += comm/src
screen-y += mipidsi/src
screen-$(CONFIG_SCREEN_HX8394)              += hx8394_mipi
screen-$(CONFIG_SCREEN_OTA5182)             += ota5182
screen-$(CONFIG_SCREEN_ST7796S)             += st7796s
screen-$(CONFIG_SCREEN_ST7789)              += st7789
screen-$(CONFIG_SCREEN_ST7796S_RGB16BIT)    += st7796s_rgb16bit
screen-$(CONFIG_SCREEN_OTA7290B)            += ota7290b
screen-$(CONFIG_SCREEN_ILI8961)             += ili8961
screen-$(CONFIG_SCREEN_OTA7290B_MIPI11INCH) += ota7290b_mipi11inch
screen-$(CONFIG_SCREEN_ICNL9707)            += icnl9707
screen-$(CONFIG_SCREEN_ICN9707)            	+= icn9707
screen-$(CONFIG_SCREEN_ST7701S)            	+= st7701s
screen-$(CONFIG_SCREEN_NV3049E)            	+= nv3049e
screen-$(CONFIG_SCREEN_EK79007)            	+= ek79007
screen-$(CONFIG_SCREEN_GC9503V)            	+= gc9503v
screen-$(CONFIG_SCREEN_GC9503V_W820)        += gc9503v_w820
screen-$(CONFIG_SCREEN_GC9503V_W960)        += gc9503v_w960
screen-$(CONFIG_SCREEN_I80ST7789)           += i80st7789
screen-$(CONFIG_SCREEN_JD9165A)             += jd9165a
screen-$(CONFIG_SCREEN_FL7707N)             += fl7707n
screen-$(CONFIG_SCREEN_ST7789P3SPI)         += st7789p3_spi
screen-$(CONFIG_SCREEN_LTD600)         += LTT600

#SCREEN_DRIVER_DIR    := hx8394_mipi
ifeq ($(CONFIG_SCREEN_HX8394), y)
SCREEN_DRIVER_DIR    := hx8394_mipi
else ifeq ($(CONFIG_SCREEN_ICN9707), y)
SCREEN_DRIVER_DIR    := icn9707
else ifeq ($(CONFIG_SCREEN_OTA7290B_MIPI11INCH), y)
SCREEN_DRIVER_DIR    := ota7290b_mipi11inch
else ifeq ($(CONFIG_SCREEN_ST7701S), y)
SCREEN_DRIVER_DIR    := st7701s
else ifeq ($(CONFIG_SCREEN_GC9503V), y)
SCREEN_DRIVER_DIR    := gc9503v
else ifeq ($(CONFIG_SCREEN_GC9503V_W820), y)
SCREEN_DRIVER_DIR    := gc9503v_w820
else ifeq ($(CONFIG_SCREEN_GC9503V_W960), y)
SCREEN_DRIVER_DIR    := gc9503v_w960
else ifeq ($(CONFIG_SCREEN_NV3049E), y)
SCREEN_DRIVER_DIR    := nv3049e
else ifeq ($(CONFIG_SCREEN_EK79007), y)
SCREEN_DRIVER_DIR    := ek79007
else ifeq ($(CONFIG_SCREEN_I80ST7789), y)
SCREEN_DRIVER_DIR    := i80st7789
else ifeq ($(CONFIG_SCREEN_JD9165A), y)
SCREEN_DRIVER_DIR    := jd9165a
else ifeq ($(CONFIG_SCREEN_FL7707N), y)
SCREEN_DRIVER_DIR    := fl7707n
else ifeq ($(CONFIG_SCREEN_ST7789P3SPI), y)
SCREEN_DRIVER_DIR    := st7789p3_spi
else ifeq ($(CONFIG_SCREEN_LTD600), y)
SCREEN_DRIVER_DIR    := LTT600
endif

SEXT := c
SDIR := $(CURDIR)
ODIR := $(patsubst $(SRCTREE)/%,$(OBJTREE)/%,$(SDIR)/$(CVILIB_DIR))
SRCS := $(foreach dir,$(screen-y),$(wildcard $(dir)/*.$(SEXT)))
OBJS := $(addprefix $(ODIR)/,$(SRCS:.c=.o))
DEPS := $(addprefix $(ODIR)/,$(SRCS:.c=.d))
# $(info SDIR = $(SDIR))
# $(info ODIR = $(ODIR))
# $(info SRCS = $(SRCS))
# $(info OBJS = $(OBJS))

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_hal_screen.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_hal_screen.so



INCS		+= -I$(CURDIR)/../gpio/include
INCS		+= -I$(CURDIR)/../pwm/include
INCS        += -I$(CURDIR) -I$(CURDIR)/include
INCS		+= -I$(CURDIR)/comm/include
INCS		+= -I$(CURDIR)/mipidsi/include
INCS		+= -I$(SRCTREE)/cpsl/mmf/include
INCS		+= -I$(SRCTREE)/cpsl/hal/screen/mipidsi/include

CFLAGS      += $(INCS)
CFLAGS      += -fPIC
CFLAGS		+= -Wno-unused-parameter -Wno-unused-function -Wno-error
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
	cp $^ $(INSTALL_PATH)/lib
	# mkdir -p $(SRCTREE)/platform/u-boot-2021.10/drivers/video/cvitek/panels/
	# cp $(CURDIR)/$(SCREEN_DRIVER_DIR)/* $(SRCTREE)/platform/u-boot-2021.10/drivers/video/cvitek/panels/ -rf
	# platform/alios/solutions/cardv/application/media/include/hal_screen.h
	# cp $(CURDIR)/$(SCREEN_DRIVER_DIR)/*.h $(SRCTREE)/platform/alios/solutions/cardv/application/media/include/ -rf
	 cp $(CURDIR)/$(SCREEN_DRIVER_DIR)/hal_screen.* $(SRCTREE)/platform/u-boot-2021.10/drivers/video/cvitek/panels/ -rf
-include $(DEPS)
