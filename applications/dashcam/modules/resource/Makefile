include $(SRCTREE)/applications/build/base.mk

all:
	echo $(SRCTREE)
	$(SRCTREE)/applications/tools/bmp2bitmap/./bmp2bitmap -f $(CURDIR)/bmp/cvitek.bmp -b 0xffff; mv cvi_bitmap.h $(SRCTREE)/applications/$(CFG_PDT)/modules/media/include/

clean:
	echo $(SRCTREE)

install:
	# cp -a $(CURDIR)/voice $(INSTALL_PATH)/bin/

-include $(DEPS)
