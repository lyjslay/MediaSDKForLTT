include $(SRCTREE)/build/base.mk

TOPTARGETS := all clean install

SUBDIRS := $(notdir $(basename $(shell find . -maxdepth 1 -type d)))
exclude_dirs = onvif
SUBDIRS := $(filter-out $(exclude_dirs), $(SUBDIRS))

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
