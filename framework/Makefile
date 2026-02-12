include $(SRCTREE)/build/base.mk

TOPTARGETS := all clean install

# SUBDIRS := $(notdir $(basename $(shell find . -maxdepth 1 -type d)))
SUBDIRS := \
	components \
	mapi \
	services

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
