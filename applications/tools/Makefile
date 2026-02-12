include $(SRCTREE)/applications/build/base.mk

TOPTARGETS := all clean install

# SUBDIRS := $(wildcard */.)
ifeq ($(CONFIG_SCREEN), y)
SUBDIRS 	+= awtkres
endif
ifeq ($(CONFIG_BOARD_DEBUG), y)
SUBDIRS		+= cc_tools
# SUBDIRS		+= test_scripts
endif
SUBDIRS		+= bmp2bitmap

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)