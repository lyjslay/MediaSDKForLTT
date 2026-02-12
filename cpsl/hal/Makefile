include $(SRCTREE)/build/base.mk

TOPTARGETS := all clean install

SUBDIRS := $(notdir $(basename $(shell find . -maxdepth 1 -type d)))
exclude_dirs += include lib
ifneq ($(CONFIG_ADC),y)
exclude_dirs += adc
endif
ifneq ($(CONFIG_GPIO),y)
exclude_dirs += gpio
endif
ifneq ($(CONFIG_SCREEN),y)
exclude_dirs += screen
endif
ifneq ($(CONFIG_WIFI),y)
exclude_dirs += wifi
endif
ifneq ($(CONFIG_GSENSOR),y)
exclude_dirs += gsensor
endif
ifneq ($(CONFIG_PWM),y)
exclude_dirs += pwm
endif
ifneq ($(CONFIG_LED),y)
exclude_dirs += led
endif
ifneq ($(CONFIG_KEY),y)
exclude_dirs += key
endif
ifneq ($(CONFIG_WATCHDOG),y)
exclude_dirs += watchdog
endif
ifneq ($(CONFIG_TOUCHPAD), y)
exclude_dirs += touchpad
endif
ifneq ($(CONFIG_GPS),y)
exclude_dirs += gps
endif
# ifneq ($(CONFIG_ACC),y)
# exclude_dirs += acc
# endif
SUBDIRS := $(filter-out $(exclude_dirs), $(SUBDIRS))

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
