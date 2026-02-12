include $(SRCTREE)/build/base.mk

.PHONY : clean all install
all:

clean:

install:
ifeq ($(INSTALL_PATH),)
	echo "please specify INSTALL_PATH"
	exit 1
endif
	mkdir -p $(INSTALL_PATH)
	mkdir -p $(INSTALL_PATH)/include/
	mkdir -p $(INSTALL_PATH)/lib/
	cp -a $(CURDIR)/include/* $(INSTALL_PATH)/include/
	cp -a $(CURDIR)/$(CVILIB_DIR)/* $(INSTALL_PATH)/lib/
