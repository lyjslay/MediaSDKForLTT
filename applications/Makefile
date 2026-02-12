include $(SRCTREE)/build/base.mk

TOPTARGETS := all clean install


app-y += tools
app-$(CONFIG_APP_DASHCAM)		+= common
app-$(CONFIG_APP_DASHCAM)		+= dashcam


SUBDIRS += $(app-y)

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)