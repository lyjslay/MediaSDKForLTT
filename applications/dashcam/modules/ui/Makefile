
include $(SRCTREE)/applications/build/base.mk

TOPTARGETS := all clean install

# SUBDIRS := $(wildcard */.)
SUBDIRS := $(CFG_PDT_SUB)/$(UI_PACKET)

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
