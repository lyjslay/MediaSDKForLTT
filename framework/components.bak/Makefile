include $(SRCTREE)/build/base.mk

TOPTARGETS := all clean install


components-$(CONFIG_COMPONENTS_MEMORY_ALLOCATOR)               += mem_alloc
components-$(CONFIG_COMPONENTS_RINGBUFFER)           += ringbuffer
components-$(CONFIG_COMPONENTS_MUXER)               += muxer
components-$(CONFIG_COMPONENTS_DEMUXER)             += demuxer
components-$(CONFIG_COMPONENTS_THUMBNAIL_EXTRACTOR)       += thumbnail_extractor
components-$(CONFIG_COMPONENTS_FILE_RECOVER)           += file_recover
components-$(CONFIG_COMPONENTS_PLAYER)             += player
components-$(CONFIG_COMPONENTS_RECORDER)             += recorder
components-$(CONFIG_COMPONENTS_ONVIF)            += onvif
components-$(CONFIG_COMPONENTS_PERF)            += perf

SUBDIRS += $(components-y)
$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
