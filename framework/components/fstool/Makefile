SRCTREE ?= ../../../
PROJECT_NAME := cvi_fstool

include $(SRCTREE)/build/base.mk

PREBUILD_LIB_PATH = $(CURDIR)/prebuild/prebuild.tar.gz

.PHONY: clean all uzip_prebuild_lib install

#all: uzip_prebuild_lib
all:

uzip_prebuild_lib:
	tar -zxf $(PREBUILD_LIB_PATH)

clean:

$(INSTALL_PATH)/lib:
	mkdir -p $@

install: | $(INSTALL_PATH)/lib
	#cp $(CURDIR)/lib/lib$(PROJECT_NAME)* $(INSTALL_PATH)/lib/
