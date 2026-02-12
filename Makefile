SRCTREE := $(CURDIR)
OBJTREE	:= $(CURDIR)/out/obj
INSTALL_PATH ?= $(CURDIR)/out/install
KCONFIG_FILE ?= $(SRCTREE)/.config
NPROC ?= $(shell expr $(shell nproc) - 1)
$(info "NPROC = $(NPROC)")
export SRCTREE OBJTREE INSTALL_PATH KCONFIG_FILE NPROC

ifeq ("$(wildcard $(KCONFIG_FILE))","")
ifeq ("$(filter menuconfig %defconfig clean% %clean, $(MAKECMDGOALS))","")
# if no configuration file is present and defconfig or clean
# is not a named target, run defconfig then menuconfig to get the initial config
$(error ".config is not present, make xxx_defconfig or menuconfig first")
endif
endif

include $(SRCTREE)/Kbuild
include $(SRCTREE)/build/base.mk

#######################################################################
# Target
#######################################################################

TOPTARGETS := all clean install distclean mmf_install prepare_all u-boot kernel system boot \
				rootfs param burn_images ota_images raw_images prepare_middleware prepare_init \
				prepare_alios alios_clean prepare_fsbl prepare_version applications_build cpsl_build framework_build \
				applications_clean cpsl_clean framework_clean

TOPSUBDIRS := \
	cpsl \
	framework \
	applications

all: mmf_install
all: $(KCONFIG_FILE) $(TOPSUBDIRS)
install: $(KCONFIG_FILE) $(TOPSUBDIRS)
clean: $(TOPSUBDIRS) alios_clean
	rm -rf $(OBJTREE)
	rm -rf $(INSTALL_PATH)
	rm -rf platform/fsbl/build
	rm -rf platform/opensbi/build
	rm -rf out/build_fsbl
	rm -rf out/*.$(STORAGE_TYPE)
	rm -rf out/*.bin
	rm -rf out/*.jpg

mmf_install:
	mkdir -p $(SRCTREE)/cpsl/mmf
	rm -rf $(SRCTREE)/cpsl/mmf/lib
	rm -rf $(SRCTREE)/cpsl/mmf/ko
	cp -rf $(SRCTREE)/cpsl/mmf_type/$(CHIP_TYPE)/$(MMF_TYPE)/* $(SRCTREE)/cpsl/mmf/

$(TOPSUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

cpsl_build:
	$(MAKE) -C cpsl all
	$(MAKE) -C cpsl install

cpsl_clean:
	$(MAKE) -C cpsl clean

framework_build:
	$(MAKE) -C framework all
	$(MAKE) -C framework install

framework_clean:
	$(MAKE) -C framework clean

applications_build:
	$(MAKE) -C applications all
	$(MAKE) -C applications install

applications_clean:
	$(MAKE) -C applications clean

.PHONY: $(TOPTARGETS) $(TOPSUBDIRS)

distclean:
	$(MAKE) clean
	rm .config
	rm -rf out

kernel: $(KCONFIG_FILE)
	mkdir -p out
	$(MAKE) -C build OUTPUT_DIR=$(CURDIR)/out \
		CUSTOMER=$(CONFIG_CUSTOMER) BOARD_DEF=$(CONFIG_BOARD) \
		kernel

u-boot:
	mkdir -p out
	$(MAKE) -C build OUTPUT_DIR=$(CURDIR)/out \
		CUSTOMER=$(CONFIG_CUSTOMER) BOARD_DEF=$(CONFIG_BOARD) \
		u-boot

system:
	mkdir -p out
	$(MAKE) -C build OUTPUT_DIR=$(CURDIR)/out \
		CUSTOMER=$(CONFIG_CUSTOMER) BOARD_DEF=$(CONFIG_BOARD) \
		system.$(STORAGE_TYPE)

boot:
	mkdir -p out
	$(MAKE) -C build OUTPUT_DIR=$(CURDIR)/out \
		CUSTOMER=$(CONFIG_CUSTOMER) BOARD_DEF=$(CONFIG_BOARD) \
		boot.$(STORAGE_TYPE)

rootfs:
	mkdir -p out
	$(MAKE) -C build OUTPUT_DIR=$(CURDIR)/out \
		CUSTOMER=$(CONFIG_CUSTOMER) BOARD_DEF=$(CONFIG_BOARD) \
		rootfs.$(STORAGE_TYPE)

param:
	mkdir -p out
	$(MAKE) -C build OUTPUT_DIR=$(CURDIR)/out \
		app_cfg.bin
	$(MAKE) -C build OUTPUT_DIR=$(CURDIR)/out \
		app_cfg_def.bin

burn_images:
	mkdir -p out
	$(MAKE) -C build OUTPUT_DIR=$(CURDIR)/out \
		CUSTOMER=$(CONFIG_CUSTOMER) BOARD_DEF=$(CONFIG_BOARD) \
		all

ota_images:
	$(MAKE) -C build OUTPUT_DIR=$(CURDIR)/out ota_images

alios:
	$(MAKE) -C build OUTPUT_DIR=$(CURDIR)/out u-boot

alios_clean:
	$(MAKE) -C build OUTPUT_DIR=$(CURDIR)/out alios_clean

raw_images:
	$(MAKE) -C build OUTPUT_DIR=$(CURDIR)/out raw_images

prepare_all:
	if [ -f prepare.sh ];then chmod +x ./prepare.sh;./prepare.sh all \
		$(CHIP_TYPE) $(CHIP_NAME) $(BOARD_NAME) $(ARCH_TYPE) $(CONFIG_BRANCH_NAME:"%"=%);fi

prepare_kernel:
	if [ -f prepare.sh ];then chmod +x ./prepare.sh;./prepare.sh kernel \
		$(CHIP_TYPE) $(CHIP_NAME) $(BOARD_NAME) $(ARCH_TYPE);fi

prepare_ramdisk:
	if [ -f prepare.sh ];then chmod +x ./prepare.sh;./prepare.sh ramdisk \
		$(CHIP_TYPE) $(CHIP_NAME) $(BOARD_NAME) $(ARCH_TYPE);fi

prepare_u-boot:
	if [ -f prepare.sh ];then chmod +x ./prepare.sh;./prepare.sh u-boot \
		$(CHIP_TYPE) $(CHIP_NAME) $(BOARD_NAME) $(ARCH_TYPE) $(CONFIG_BRANCH_NAME:"%"=%);fi

prepare_middleware:
	if [ -f prepare.sh ];then chmod +x ./prepare.sh;./prepare.sh middleware \
		$(CHIP_TYPE) $(CHIP_NAME) $(BOARD_NAME) $(ARCH_TYPE);fi

prepare_init:
	if [ -f prepare.sh ];then chmod +x ./prepare.sh;./prepare.sh init \
		$(CONFIG_BRANCH_NAME:"%"=%);fi

prepare_update:
	if [ -f prepare.sh ];then chmod +x ./prepare.sh;./prepare.sh update \
		$(CONFIG_BRANCH_NAME:"%"=%);fi

prepare_alios:
	if [ -f prepare.sh ];then chmod +x ./prepare.sh;./prepare.sh alios \
		$(CHIP_TYPE) $(CHIP_NAME) $(BOARD_NAME) $(ARCH_TYPE) $(CONFIG_BRANCH_NAME:"%"=%);fi

prepare_fsbl:
	if [ -f prepare.sh ];then chmod +x ./prepare.sh;./prepare.sh fsbl \
		$(CHIP_TYPE) $(CHIP_NAME) $(BOARD_NAME);fi

prepare_version:
	if [ -f prepare.sh ];then chmod +x ./prepare.sh;./prepare.sh version $(VERSION_PATH);fi

menuconfig:
	$(MAKE) -f Makefile.kbuild $@
%_defconfig:
	$(MAKE) -f Makefile.kbuild $@


