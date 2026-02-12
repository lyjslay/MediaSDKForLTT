SRCTREE ?= ../../../
include $(SRCTREE)/build/base.mk

PROJECT_NAME := cvi_image_viewer

.PHONY: clean all install

all:
	cmake $(CURDIR) -G Ninja -Bbuild/$(CVILIB_DIR) \
	-DCMAKE_C_COMPILER=$(CROSS_COMPILE)gcc \
	-DCMAKE_CXX_COMPILER=$(CROSS_COMPILE)g++ \
	-D$(PROJECT_NAME)_BUILD_STATIC_LIB=on \
	-DCMAKE_CFLAGS=$(CMAKE_CFLAGS)
	ninja -C $(CURDIR)/build/$(CVILIB_DIR)
	ninja -C $(CURDIR)/build/$(CVILIB_DIR) install
	mkdir -p $(CURDIR)/$(CVILIB_DIR)
	cp -a $(CURDIR)/install/lib/lib$(PROJECT_NAME)* $(CURDIR)/$(CVILIB_DIR)/

clean:
	rm $(CURDIR)/build -rf
	rm $(CURDIR)/install -rf

$(INSTALL_PATH)/lib:
	mkdir -p $@

install: | $(INSTALL_PATH)/lib
	cp -a $(CURDIR)/$(CVILIB_DIR)/lib$(PROJECT_NAME)* $(INSTALL_PATH)/lib/

