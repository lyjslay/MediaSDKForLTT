TOPTARGETS := all clean install
.PHONY: $(TOPTARGETS)

# SUBDIRS := $(wildcard */.)
SUBDIRS := \
	modules \

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)