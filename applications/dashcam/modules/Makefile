include $(SRCTREE)/applications/build/base.mk

TOPTARGETS := all clean install
.PHONY: $(TOPTARGETS)

# SUBDIRS := $(wildcard */.)
SUBDIRS := \
	param \
	resource \
	usbctrl \
	media \
	ui \
	mode \
	netctrl \
	main

SUBDIRS := $(filter-out ui, $(SUBDIRS))

#ifeq ($(CONFIG_SCREEN), y)
SUBDIRS := ui $(SUBDIRS)
#endif

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)