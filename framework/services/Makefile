include $(SRCTREE)/build/base.mk

TOPTARGETS := all clean install


services-$(CONFIG_SERVICES_AUDIO) += audio
services-$(CONFIG_SERVICES_IMAGE_VIEWER) += image_viewer
services-$(CONFIG_SERVICES_LIVEVIEW)    += liveview
services-$(CONFIG_SERVICES_PLAYER) += player
services-$(CONFIG_SERVICES_RECORD)  += recorder
services-$(CONFIG_SERVICES_RTSP)   += rtsp_ser
services-$(CONFIG_SERVICES_STORAGE) += storage
services-$(CONFIG_SERVICES_PHOTO)  += photo
services-$(CONFIG_SERVICES_SPEECH)  += speech
services-$(CONFIG_SERVICES_ADAS)  += adas
services-$(CONFIG_SERVICES_QRCODE)  += qrcode
services-$(CONFIG_SERVICES_SUBVIDEO)  += vcap_ser

SUBDIRS += $(services-y)

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
