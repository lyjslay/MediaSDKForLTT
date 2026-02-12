include $(SRCTREE)/build/base.mk

TOPTARGETS := all clean install

# SUBDIRS := $(notdir $(basename $(shell find . -maxdepth 1 -type d)))
SUBDIRS := \
	mmf \
	common \
	osal \
	hal \
	third_party \
	tpu

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
