include $(SRCTREE)/applications/build/base.mk

TOPTARGETS := all clean install

#SUBDIRS := $(notdir $(basename $(shell find . -maxdepth 1 -type d)))
SUBDIRS := utils devmng cmdmng filemng recordmng storagemng cvinet \
			system usb gpsmng tempermng powercontrol


ifeq ($(CONFIG_SERVICES_LIVEVIEW), y)
SUBDIRS += liveviewmng
endif

ifeq ($(CONFIG_SERVICES_LIVEVIEW), y)
SUBDIRS += volmng
endif

ifeq ($(CONFIG_SERVICES_PLAYER), y)
SUBDIRS += playbackmng
endif

ifeq ($(CONFIG_VIDEO_MD), y)
SUBDIRS += videomd
endif

ifeq ($(CONFIG_ISP_IR_CUT), y)
SUBDIRS += ispircut
endif

ifeq ($(CONFIG_SERVICES_PHOTO), y)
SUBDIRS += photomng
endif

ifeq ($(CONFIG_SERVICES_SPEECH), y)
SUBDIRS += speechmng
endif

ifeq (${CONFIG_SERVICES_ADAS}, y)
SUBDIRS += adasmng
endif

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
