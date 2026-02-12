TOPTARGETS := all clean install

SUBDIRS := $(notdir $(basename $(shell find . -maxdepth 1 -type d)))

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
