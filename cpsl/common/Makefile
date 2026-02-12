include $(SRCTREE)/build/base.mk

TOPTARGETS := all clean install

SUBDIRS := $(notdir $(basename $(shell find . -maxdepth 1 -type d)))

# solve dependency
ordered_dirs = log
no_make_dirs = exif
filtered_dirs := $(filter-out $(ordered_dirs), $(SUBDIRS))
filtered_dirs := $(filter-out $(no_make_dirs), $(filtered_dirs))
SUBDIRS := $(ordered_dirs) $(filtered_dirs)

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
