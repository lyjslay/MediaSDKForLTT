include $(SRCTREE)/build/base.mk

YOC_FILE := $(CURDIR)/.yoc

ifeq ("$(wildcard $(YOC_FILE))","")
$(error ".yoc is not present, please do 'yoc init' first!!")
endif
#
HOST_TOOLS := $(CURDIR)/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin
CC := $(HOST_TOOLS)/riscv64-unknown-elf-gcc
CXX := $(HOST_TOOLS)/riscv64-unknown-elf-g++
AR := $(HOST_TOOLS)/riscv64-unknown-elf-ar
STRIP := $(HOST_TOOLS)/riscv64-unknown-elf-strip
export CC CXX AR STRIP
export PATH := $(HOST_TOOLS):$(PATH)
#
TOPSUBDIRS := $(CONFIG_ALIOS_SOLUTION)
#
.PHONY:all clean install $(TOPSUBDIRS)

#
all: $(TOPSUBDIRS)
#
$(TOPSUBDIRS):
	$(MAKE) -C solutions/$@ $(patsubst $@,,$(MAKECMDGOALS))
#
clean: 
	$(MAKE) -C solutions/$(TOPSUBDIRS) clean
#
install:


