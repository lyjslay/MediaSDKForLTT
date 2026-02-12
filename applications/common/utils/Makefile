include $(SRCTREE)/applications/build/base.mk

TOPTARGETS := all clean install

#SUBDIRS := $(notdir $(basename $(shell find . -maxdepth 1 -type d)))
SUBDIRS := timedtask md5

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
