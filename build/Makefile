include $(CONFIG_FILE)

CUR_DIR := $(shell pwd)
include $(CUR_DIR)/base.mk

CROSS_COMPILE ?= arm-linux-gnueabihf-

#######################################################################
# DIR Config
#######################################################################
OUTPUT_DIR ?= $(CURDIR)/out
TOOLS_DIR := $(CURDIR)/tools
RAWIMAGE_DIR := $(OUTPUT_DIR)/raw_image
BUILD_UBOOT_DIR   := $(OUTPUT_DIR)/build_u-boot
BUILD_FSBL_DIR    := $(OUTPUT_DIR)/build_fsbl
BUILD_OPENSBI_DIR := $(OUTPUT_DIR)/build_opensbi
BUILD_RAMDISK_DIR := $(OUTPUT_DIR)/build_ramdisk
BUILD_KERNEL_DIR  := $(OUTPUT_DIR)/build_kernel
KERNEL_HEADER_DIR := $(OUTPUT_DIR)/kernel_headers
KERNEL_MODULE_DIR := $(OUTPUT_DIR)/kernel_modules
BUILD_OSDRV_DIR   := $(OUTPUT_DIR)/build_osdrv
OSDRV_MODULE_DIR  := $(OUTPUT_DIR)/kernel_osdrv
BUILD_MW_DIR      := $(OUTPUT_DIR)/build_middleware
BUILD_FIP_DIR     := $(OUTPUT_DIR)/build_fip
BUILD_GPT_DIR     := $(OUTPUT_DIR)/build_gpt
BUILD_BOOT_DIR    := $(OUTPUT_DIR)/build_boot
BUILD_CFG_DIR     := $(OUTPUT_DIR)/build_cfg
BUILD_ROOTFS_DIR  := $(OUTPUT_DIR)/build_rootfs
BUILD_SYSTEM_DIR  := $(OUTPUT_DIR)/build_system

#######################################################################
# target config
#######################################################################
TARGETS += fip.bin
TARGETS += boot.$(STORAGE_TYPE)
TARGETS += rootfs.$(STORAGE_TYPE)
ifneq ($(CONFIG_ROOTFS_WITH_APP),y)
TARGETS += system.$(STORAGE_TYPE)
endif
ifeq ($(STORAGE_TYPE),emmc)
TARGETS += gpt.img
else ifeq ($(STORAGE_TYPE),spinand)
else ifeq ($(STORAGE_TYPE),spinor)
else
$(error $(STORAGE_TYPE) invalid, [emmc | spinand | spinor])
endif
TARGETS += app_cfg.bin app_cfg_def.bin
ifeq ($(CONFIG_UBOOT_BOOTLOGO),y)
SDK_PARTITION_XML := ${BOARD_DIR}/partition/partition_${STORAGE_TYPE}.xml
else
SDK_PARTITION_XML := ${BOARD_DIR}/partition/partition_nologo_${STORAGE_TYPE}.xml
endif
ifeq ($(STORAGE_TYPE),spinor)
ifneq ($(CONFIG_ROOTFS_WITH_APP),y)
ifeq ($(CONFIG_UBOOT_BOOTLOGO),y)
SDK_PARTITION_XML = ${BOARD_DIR}/partition/partition_sys_${STORAGE_TYPE}.xml
else
SDK_PARTITION_XML = ${BOARD_DIR}/partition/partition_sys_nologo_${STORAGE_TYPE}.xml
endif

endif
endif

ifeq ($(CONFIG_TOOLCHAIN_GLIBC_ARM64),y)
packages_arch := arm64
else ifeq ($(CONFIG_TOOLCHAIN_GLIBC_ARM),y)
packages_arch := arm
OVERLAY_PATH := $(CHIP_TYPE)_32bit
else ifeq ($(CONFIG_TOOLCHAIN_UCLIBC_ARM),y)
packages_arch := uclibc
else ifeq ($(CONFIG_TOOLCHAIN_GLIBC_RISCV64),y)
packages_arch := glibc_riscv64
OVERLAY_PATH := $(CHIP_TYPE)_$(ROOTFS_ARCH)
else ifeq ($(CONFIG_TOOLCHAIN_MUSL_RISCV64),y)
packages_arch := musl_riscv64
OVERLAY_PATH := $(CHIP_TYPE)_$(ROOTFS_ARCH)
endif
COMMON_PATH := common_$(packages_arch)

#######################################################################
# SDK Config
#######################################################################
# try to find MEDIASDK_DIR, based on MediaSDK.version file
ifneq ($(wildcard $(CURDIR)/../MediaSDK.version),)
MEDIASDK_DIR := $(CURDIR)/..
endif

ifneq ($(MEDIASDK_DIR),)

$(info Build with MediaSDK)

UBOOT_DIR := $(MEDIASDK_DIR)/platform/u-boot-2021.10
# FREERTOS_DIR := $(MEDIASDK_DIR)/platform/freertos
ALIOS_DIR := $(MEDIASDK_DIR)/platform/alios
KERNEL_DIR := $(MEDIASDK_DIR)/platform/linux_5.10
OSDRV_DIR := $(MEDIASDK_DIR)/platform/osdrv
RAMDISK_DIR := $(MEDIASDK_DIR)/platform/ramdisk
FSBL_DIR := $(MEDIASDK_DIR)/platform/fsbl
OPENSBI_DIR := $(MEDIASDK_DIR)/platform/opensbi
MW_DIR := $(MEDIASDK_DIR)/cpsl/mmf

else

$(info Build with SDK source)

ifndef SDK_DIR
$(error SDK_DIR is undefined)
endif
ifeq ($(BUILD_SDK), 1)
$(info SDK_DIR $(SDK_DIR))
else ifeq ($(wildcard $(SDK_DIR)),)
$(error $(SDK_DIR) not found.)
endif

UBOOT_DIR := $(SDK_DIR)/u-boot-2021.10
FREERTOS_DIR := $(SDK_DIR)/freertos
KERNEL_DIR := $(SDK_DIR)/linux_5.10
RAMDISK_DIR := $(SDK_DIR)/ramdisk
FSBL_DIR := $(SDK_DIR)/fsbl
OPENSBI_DIR := $(SDK_DIR)/opensbi
MW_DIR := $(SDK_DIR)/middleware

# SDK targets (for BUILD_SDK=1 only)
$(SDK_DIR):
	$(MAKE) -f Makefile.sdk \
    	OUTPUT_DIR=$(OUTPUT_DIR) \
    	SDK_DIR=$(SDK_DIR) \
    	SDK_SRC_DIR=$(SDK_SRC_DIR) \
    	all

$(UBOOT_DIR) : $(SDK_DIR)
$(KERNEL_DIR) : $(SDK_DIR)
$(RAMDISK_DIR) : $(SDK_DIR)
$(MW_DIR) : $(SDK_DIR)

endif

UBOOT_CVI_BOARD_INIT_PATH := $(UBOOT_DIR)/board/cvitek/cvi_board_init.c
UBOOT_CVITEK_PATH := $(UBOOT_DIR)/include/cvitek/cvitek.h
UBOOT_DEFCONFIG := $(BRAND)_$(CHIP_NAME)_$(BOARD_NAME)_defconfig

################################################################################
# Output directory
################################################################################
${OUTPUT_DIR}/elf:
	mkdir -p $@

################################################################################
# Generate global memmory map for all source repo
################################################################################
include scripts/mmap.mk

################################################################################
# arm-trusted-firmware and bld
################################################################################
include scripts/atf.mk

################################################################################
# rtos targets
################################################################################
include scripts/rtos.mk
include alios.mk
################################################################################
# fip targets
################################################################################
include scripts/fip_v2.mk

#######################################################################
# intermediate targets
#######################################################################
define uboot_compress_action
	echo "Compress u-boot with LZMA"
	lzma --force ${BUILD_UBOOT_DIR}/u-boot.bin
	echo -n 'LZMA' | cat - ${BUILD_UBOOT_DIR}/u-boot.bin.lzma > \
		${BUILD_UBOOT_DIR}/u-boot.bin
endef

define print_target
	@printf '\033[1;36;40m  [TARGET] $@ \033[0m\n'
endef

u-boot-dts:
	$(call print_target)
	find ${CURDIR}/boards/${CHIP_TYPE} \
		\( -path "*linux/*.dts*" -o -path "*dts_${ARCH_TYPE}/*.dts*" \) \
		-exec cp {} ${UBOOT_DIR}/arch/${ARCH_TYPE}/dts/ \;
	find ${CURDIR}/boards/default -name *.dts* -exec cp {} ${UBOOT_DIR}/arch/${ARCH_TYPE}/dts/ \;

u-boot-build: memory-map
u-boot-build: u-boot-dts
	$(call print_target)
	mkdir -p $(KBUILD_OUTPUT)
	mkdir -p $(RAWIMAGE_DIR)

	python3 ${TOOLS_DIR}/common/image_tool/mkcvipart.py ${SDK_PARTITION_XML} ${UBOOT_DIR}/include
	python3 ${TOOLS_DIR}/common/image_tool/mk_imgHeader.py ${SDK_PARTITION_XML} ${UBOOT_DIR}/include

	sed -i "s/CONFIG_BOOTDELAY=.*/CONFIG_BOOTDELAY=$(CONFIG_UBOOT_BOOTDELAY)/g" \
		$(BOARD_DIR)/u-boot/${UBOOT_DEFCONFIG}
	sed -i "s/CONFIG_BOOTLOGO=.*/CONFIG_BOOTLOGO=$(CONFIG_UBOOT_BOOTLOGO)/g" \
		$(BOARD_DIR)/u-boot/${UBOOT_DEFCONFIG}

	sed -i "s/CONFIG_BOOTAHD=.*/CONFIG_BOOTAHD=$(CONFIG_UBOOT_BOOTAHD)/g" \
		$(BOARD_DIR)/u-boot/${UBOOT_DEFCONFIG}

	cp -f $(BOARD_DIR)/u-boot/${UBOOT_DEFCONFIG} $(BUILD_UBOOT_DIR)/.config

	ln -snrf ${CVI_BOARD_MEMMAP_H_PATH} ${UBOOT_DIR}/include/
	rm -f ${UBOOT_CVI_BOARD_INIT_PATH}
	ln -s ${BOARD_DIR}/u-boot/cvi_board_init.c ${UBOOT_CVI_BOARD_INIT_PATH}
	rm -f ${UBOOT_CVITEK_PATH}
	# ln -s ${BOARD_DIR}/u-boot/cvitek.h ${UBOOT_CVITEK_PATH}
	$(MAKE) STORAGE_TYPE=$(STORAGE_TYPE) -j${NPROC} -C ${UBOOT_DIR} olddefconfig
	$(MAKE) STORAGE_TYPE=$(STORAGE_TYPE) -j${NPROC} -C ${UBOOT_DIR} all
	cat ${BUILD_UBOOT_DIR}/u-boot.bin > ${BUILD_UBOOT_DIR}/u-boot-raw.bin
# ifeq ($(STORAGE_TYPE),spinor)
ifeq ($(CONFIG_UBOOT_SPL_CUSTOM),y)
	cat ${BUILD_UBOOT_DIR}/spl/u-boot-spl.bin > ${BUILD_UBOOT_DIR}/u-boot-raw_spl.bin
	# cat ${BUILD_UBOOT_DIR}/u-boot.bin > ${BUILD_UBOOT_DIR}/u-boot-raw_spl.bin
endif
# endif

u-boot: export KBUILD_OUTPUT=${BUILD_UBOOT_DIR}
u-boot: export RELEASE=0
u-boot: export CVIBOARD=${BOARD_NAME}
u-boot: export CROSS_COMPILE=$(patsubst "%",%,$(CROSS_COMPILE_UBOOT))
u-boot: export CONFIG_USE_DEFAULT_ENV:=y
u-boot: export CONFIG_SKIP_RAMDISK:=y
u-boot: export CHIP=$(CHIP_NAME)
u-boot: export CONFIG_MMC_SKIP_TUNING:=${CONFIG_MMC_SKIP_TUNING}
u-boot: export CONFIG_SUP_LARGE_PART_SIZE=${SUP_LARGE_PART_SIZE}
u-boot: export ARCH=$(patsubst "%",%,$(ARCH_TYPE))
u-boot: export MULTI_FIP=n
u-boot: u-boot-dep

################################################################################
# kernel targets
################################################################################
# configure kernel defconfig
ifeq ($(CONFIG_BUILD_FOR_DEBUG),y)
KERNEL_CONFIG_NAME := cvitek_$(CHIP_NAME)_$(BOARD_NAME)_defconfig
else
#TODO add release defconfig
KERNEL_CONFIG_NAME := cvitek_$(CHIP_NAME)_$(BOARD_NAME)_defconfig
# KERNEL_CONFIG_NAME := cvitek_$(CHIP_NAME)_$(BOARD_NAME)_rls_defconfig
endif

KERNEL_VERSION ?= -tag-$(shell git -C ${KERNEL_DIR} describe --exact-match HEAD 2>/dev/null)

KERNEL_OUTPUT_CONFIG_PATH := ${BUILD_KERNEL_DIR}/.config
KERNEL_DEFAULT_CONFIG_PATH := $(CURDIR)/boards/${CHIP_TYPE}/$(CHIP_NAME)_$(BOARD_NAME)/linux/${KERNEL_CONFIG_NAME}

# Make a copy to .config~
${KERNEL_OUTPUT_CONFIG_PATH}: ${KERNEL_DEFAULT_CONFIG_PATH}
	$(call print_target)
	echo "Kernel's defconfig is updated. Use it."
	mkdir -p $(dir ${KERNEL_OUTPUT_CONFIG_PATH})
	cmp -s ${KERNEL_DEFAULT_CONFIG_PATH} ${KERNEL_OUTPUT_CONFIG_PATH} || \
		cp -vb ${KERNEL_DEFAULT_CONFIG_PATH} ${KERNEL_OUTPUT_CONFIG_PATH}

kerne%: export LOCALVERSION=${KERNEL_VERSION}
kerne%: export CVIBOARD=${BOARD_NAME}
kerne%: export ARCH=$(patsubst "%",%,$(ARCH_TYPE))

kernel: ${KERNEL_OUTPUT_CONFIG_PATH}
kernel: $(KERNEL_DIR)
	echo "build $@"
	mkdir -p $(BUILD_KERNEL_DIR)

	cd $(KERNEL_DIR) && \
	$(MAKE) distclean && \
	$(MAKE) -j${NPROC} CROSS_COMPILE=$(CROSS_COMPILE_KERNEL) ARCH=$(ARCH_TYPE) O=$(BUILD_KERNEL_DIR) olddefconfig && \
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE_KERNEL) ARCH=$(ARCH_TYPE) O=$(BUILD_KERNEL_DIR) \
		${KERNEL_OUTPUT_CONFIG_PATH} && \
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE_KERNEL) ARCH=$(ARCH_TYPE) O=$(BUILD_KERNEL_DIR) \
		LOCALVERSION=$(KERNEL_LOCALVERSION) -j$(NPROC) Image modules && \
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE_KERNEL) ARCH=$(ARCH_TYPE) O=$(BUILD_KERNEL_DIR) \
		INSTALL_MOD_PATH=$(KERNEL_MODULE_DIR) modules modules_install && \
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE_KERNEL) ARCH=$(ARCH_TYPE) O=$(BUILD_KERNEL_DIR) \
		INSTALL_HDR_PATH=$(KERNEL_HEADER_DIR)/usr headers_install

	# copy vip uapi header files to linux user headers
	cp $(KERNEL_DIR)/drivers/staging/android/uapi/ion.h \
		$(KERNEL_HEADER_DIR)/usr/include/linux/
	cp $(KERNEL_DIR)/drivers/staging/android/uapi/ion_cvitek.h \
		$(KERNEL_HEADER_DIR)/usr/include/linux/
	cp $(KERNEL_DIR)/include/uapi/linux/dma-buf.h \
		$(KERNEL_HEADER_DIR)/usr/include/linux/

$(BUILD_BOOT_DIR)/workspace:
	mkdir -p $(BUILD_BOOT_DIR)/workspace

DTS_DEFATUL_PATHS := \
	${CURDIR}/boards/default/dts/${CHIP_TYPE} \
	${CURDIR}/boards/default/dts/${CHIP_TYPE}_$(patsubst "%",%,$(ARCH_TYPE))
DTS_DEFATUL_PATHS := $(wildcard ${DTS_DEFATUL_PATHS})

kernel-dts: memory-map
kernel-dts: $(BUILD_KERNEL_DIR)
	$(call print_target)
	ln -snrf ${CVI_BOARD_MEMMAP_H_PATH} ${KERNEL_DIR}/scripts/dtc/include-prefixes/
	find ${KERNEL_DIR}/arch/$(ARCH_TYPE)/boot/dts/${BRAND}/ -type l -delete
	find ${DTS_DEFATUL_PATHS} -name *.dts* -exec ln -sf {} ${KERNEL_DIR}/arch/${ARCH}/boot/dts/${BRAND}/ \;
	find ${CURDIR}/boards/${CHIP_TYPE} \
		\( -path "*linux/*.dts*" -o -path "*dts_$(ARCH_TYPE)/*.dts*" \) \
		-exec ln -sf {} ${KERNEL_DIR}/arch/$(ARCH_TYPE)/boot/dts/${BRAND}/ \;
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE_KERNEL) ARCH=$(ARCH_TYPE) -j${NPROC} -C $(BUILD_KERNEL_DIR) dtbs

OSDRV_DIRS := $(wildcard $(OSDRV_DIR)/*)
OSDRV_TARGETS := $(addprefix osdrv_target_,${OSDRV_DIRS})

${OSDRV_TARGETS}: osdrv_target_%: kernel
	echo "build $* $(notdir $*)"
	mkdir -p $(BUILD_OSDRV_DIR)
	mkdir -p $(OSDRV_MODULE_DIR)

	# O= option does not work for kernel module build
	# copy the source into BUILD_OSDRV_DIR
	cp -a $(OSDRV_DIR)/* $(BUILD_OSDRV_DIR)
	cd $(BUILD_OSDRV_DIR)/$(notdir $*) && \
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH_TYPE) INSTALL_DIR=$(BUILD_OSDRV_DIR) \
		KERNEL_DIR=$(BUILD_KERNEL_DIR) -j$(NPROC) all

osdrv: kernel $(OSDRV_TARGETS)
	echo "build $@"
	find $(BUILD_OSDRV_DIR) -name "*.ko" -type f -exec cp {} $(OSDRV_MODULE_DIR)/ \;

ramdisk: $(RAMDISK_DIR)
	echo "build $@"
	mkdir -p $(BUILD_RAMDISK_DIR)
	rm -rf $(BUILD_RAMDISK_DIR)/*

	mkdir -p $(BUILD_RAMDISK_DIR)/target
	mkdir -p $(BUILD_RAMDISK_DIR)/configs

	if [ -d $(RAMDISK_DIR)/initramfs/$(ROOTFS_ARCH) ]; then \
		cp -rf $(RAMDISK_DIR)/initramfs/$(ROOTFS_ARCH)/* $(BUILD_RAMDISK_DIR)/target/; \
	fi

	if [ -d $(RAMDISK_DIR)/rootfs/overlay/$(OVERLAY_PATH) ]; then \
		cp -a $(RAMDISK_DIR)/rootfs/overlay/$(OVERLAY_PATH)/* $(BUILD_RAMDISK_DIR)/target/; \
	fi

	# copy config with priority: project > chip
	if [ -d $(RAMDISK_DIR)/configs/$(CHIP_TYPE) ]; then \
		cp $(RAMDISK_DIR)/configs/$(CHIP_TYPE)/* $(BUILD_RAMDISK_DIR)/configs; \
	fi

middleware: $(MW_DIR)
	echo "build $@"
	mkdir -p $(BUILD_MW_DIR)
	rm -rf $(BUILD_MW_DIR)/*

	mkdir -p $(BUILD_MW_DIR)/usr/bin
	mkdir -p $(BUILD_MW_DIR)/usr/lib
	mkdir -p $(BUILD_MW_DIR)/ko

	# cd $(MW_DIR)/sample && \
	# $(MAKE) KERNEL_DIR=$(KERNEL_HEADER_DIR)/usr clean && \
	# $(MAKE) KERNEL_DIR=$(KERNEL_HEADER_DIR)/usr \
	# 	CHIP=$(CHIP) SUBTYPE=$(SUBTYPE) SDK_VER=$(SDK_ARCH) all && \
	# find ./ -type f \( -iname "sample_*" ! -iname "*.*" \) -exec cp '{}' $(BUILD_MW_DIR)/usr/bin \;

	#cd $(MW_DIR)/component/isp && \
	#$(MAKE) KERNEL_DIR=$(KERNEL_HEADER_DIR)/usr clean && \
	#$(MAKE) KERNEL_DIR=$(KERNEL_HEADER_DIR)/usr \
	#	CHIP=$(CHIP) SUBTYPE=$(SUBTYPE) SDK_VER=$(SDK_ARCH) all

	# copy bin
	# cp -f $(MW_DIR)/bin/* $(BUILD_MW_DIR)/usr/bin/
	# copy lib
	cp -a $(MW_DIR)/lib/*.so* $(BUILD_MW_DIR)/usr/lib/
	cp -a $(MW_DIR)/lib/3rd/*.so* $(BUILD_MW_DIR)/usr/lib/
	# copy ko
	cp -rf $(MW_DIR)/ko/* $(BUILD_MW_DIR)/ko/

#######################################################################
# final targets
#######################################################################

all: $(TARGETS)
.PHONY : $(TARGETS) all clean u-boot kernel ramdisk middleware osdrv ota_images

# fip.bin: export UBOOT_DIR=/home/user/work/framework-182x-port2/externals/u-boot
# fip.bin: export BUILD_UBOOT_DIR=${UBOOT_DIR}/out
fip.bin: u-boot
	echo "build $@"
	mkdir -p $(RAWIMAGE_DIR)

app_cfg.bin:
	echo "build $@"
	mkdir -p $(RAWIMAGE_DIR)
	make -C $(SRCTREE)/applications/dashcam/modules/param/ all
	cp $(OUTPUT_DIR)/param_bin/app_cfg.bin $(RAWIMAGE_DIR)/
	python3 $(TOOLS_DIR)/common/image_tool/raw2cimg.py \
		$(OUTPUT_DIR)/param_bin/app_cfg.bin \
		$(OUTPUT_DIR) $(SDK_PARTITION_XML)

app_cfg_def.bin:
	echo "build $@"
	mkdir -p $(RAWIMAGE_DIR)
	make -C $(SRCTREE)/applications/dashcam/modules/param/ all
	cp $(OUTPUT_DIR)/param_bin/app_cfg_def.bin $(RAWIMAGE_DIR)/
	python3 $(TOOLS_DIR)/common/image_tool/raw2cimg.py \
		$(OUTPUT_DIR)/param_bin/app_cfg_def.bin  \
		$(OUTPUT_DIR) $(SDK_PARTITION_XML)

gpt.img:
	echo "build $@"
	mkdir -p $(BUILD_GPT_DIR)
	rm -rf $(BUILD_GPT_DIR)/*

	$(MAKE) -C $(TOOLS_DIR)/common/emmc_tool gpt.img \
		PARTITION_XML=$(SDK_PARTITION_XML) \
		INSTALL_DIR=$(BUILD_GPT_DIR)
	python3 $(TOOLS_DIR)/common/image_tool/raw2cimg.py $(BUILD_GPT_DIR)/gpt.img \
		$(OUTPUT_DIR) $(SDK_PARTITION_XML)

# boot.$(STORAGE_TYPE): export KERNEL_COMPRESS=lzma
ifneq ($(CONFIG_KERNEL_UNCOMPRESSED),y)
boot.$(STORAGE_TYPE): export KERNEL_COMPRESS=$(patsubst "%",%,$(CONFIG_KERNEL_COMPRESS))
endif
boot.$(STORAGE_TYPE): $(BUILD_BOOT_DIR)/workspace
boot.$(STORAGE_TYPE): ramdisk kernel kernel-dts
	echo "build $@"
	mkdir -p $(RAWIMAGE_DIR)

	#
	# FIXME:
	# the reason is onekernel_fixed_files.txt.sqsh hard-coded rootfs as ../target/
	#
	cp -a $(BUILD_RAMDISK_DIR)/target $(BUILD_BOOT_DIR)
	cp -a $(BUILD_RAMDISK_DIR)/configs $(BUILD_BOOT_DIR)

	# copy multi.its for *.itb layout, to workspace
	mv $(BUILD_BOOT_DIR)/configs/multi.its $(BUILD_BOOT_DIR)/workspace/multi.its.tmp

	# copy kernel image, and dtb file
	cp $(BUILD_KERNEL_DIR)/arch/$(ARCH_TYPE)/boot/Image $(BUILD_BOOT_DIR)/workspace

	# copy dtb file
	cp $(BUILD_KERNEL_DIR)/arch/$(ARCH_TYPE)/boot/dts/${BRAND}/*.dtb $(BUILD_BOOT_DIR)/workspace

ifeq ($(CONFIG_KERNEL_UNCOMPRESSED),y)
	# pack boot
	cd $(BUILD_BOOT_DIR)/workspace && \
	python3 "${CURDIR}/scripts/boards_scan.py" --skip_ramdisk --gen_single_board_its --chip_name "${CHIP_NAME}" --board_name "${BOARD_NAME}" && \
	mv multi.its.tmp multi.its && \
	$(RAMDISK_DIR)/tools/gen_init_cpio $(BUILD_BOOT_DIR)/configs/onekernel_fixed_files.txt.sqsh > boot.cpio && \
	sed -i "s/data = \/incbin\/(\".\/Image.gz\");/data = \/incbin\/(\".\/Image\");/g" multi.its && \
	sed -i "s/compression = \"gzip\";/compression = \"none\";/" multi.its && \
	gzip -9 -f -k boot.cpio > boot.cpio.gz && \
	sed -ie '26,38d' multi.its && \
	sed -i "s/arch = \"arm64\";/arch = \"$(patsubst "%",%,$(ARCH_TYPE))\";/g" multi.its && \
	sed -i "s/load = <0x0 0x.*>;/load = <0x0 $(CONFIG_KERNEL_ENTRY_HACK_ADDR)>;/g" multi.its && \
	sed -i "s/entry = <0x0 0x.*>;/entry = <0x0 $(CONFIG_KERNEL_ENTRY_HACK_ADDR)>;/g" multi.its && \
	$(TOOLS_DIR)/common/prebuild/mkimage -f multi.its -k ${RAMDISK_DIR}/keys -r boot.itb
else
	# pack boot
	cd $(BUILD_BOOT_DIR)/workspace && \
	python3 "${CURDIR}/scripts/boards_scan.py" --skip_ramdisk --gen_single_board_its --chip_name "${CHIP_NAME}" --board_name "${BOARD_NAME}" && \
	mv multi.its.tmp multi.its && \
	$(RAMDISK_DIR)/tools/gen_init_cpio $(BUILD_BOOT_DIR)/configs/onekernel_fixed_files.txt.sqsh > boot.cpio && \
	${Q}${KERNEL_COMPRESS} -9 -f -k Image && \
	gzip -9 -f -k boot.cpio && \
	sed -i "s/data = \/incbin\/(\".\/Image.gz\");/data = \/incbin\/(\".\/Image.${KERNEL_COMPRESS}\");/g" multi.its && \
	sed -i "s/compression = \"gzip\";/compression = \"${KERNEL_COMPRESS}\";/" multi.its && \
	sed -ie '26,38d' multi.its && \
	sed -i "s/arch = \"arm64\";/arch = \"$(patsubst "%",%,$(ARCH_TYPE))\";/g" multi.its && \
	sed -i "s/load = <0x0 0x.*>;/load = <0x0 $(CONFIG_KERNEL_ENTRY_HACK_ADDR)>;/g" multi.its && \
	sed -i "s/entry = <0x0 0x.*>;/entry = <0x0 $(CONFIG_KERNEL_ENTRY_HACK_ADDR)>;/g" multi.its && \
	$(TOOLS_DIR)/common/prebuild/mkimage -f multi.its -k ${RAMDISK_DIR}/keys -r boot.itb
endif

	cp $(BUILD_BOOT_DIR)/workspace/boot.itb $(BUILD_BOOT_DIR)/workspace/boot.$(STORAGE_TYPE)
	cp $(BUILD_BOOT_DIR)/workspace/boot.$(STORAGE_TYPE) $(RAWIMAGE_DIR)
	python3 $(TOOLS_DIR)/common/image_tool/raw2cimg.py \
		$(BUILD_BOOT_DIR)/workspace/boot.$(STORAGE_TYPE) \
		$(OUTPUT_DIR) $(SDK_PARTITION_XML)

ifeq ($(CONFIG_UBOOT_BOOTLOGO),y)
	# the name of logo.jpg is hard-coded
	python3 $(TOOLS_DIR)/common/image_tool/raw2cimg.py \
		$(BOARD_DIR)/logo/logo.jpg \
		$(OUTPUT_DIR) $(SDK_PARTITION_XML)
	cp $(BOARD_DIR)/logo/logo.jpg $(RAWIMAGE_DIR)
endif

rootfs.$(STORAGE_TYPE): ramdisk middleware osdrv
	mkdir -p $(BUILD_ROOTFS_DIR)
	mkdir -p $(RAWIMAGE_DIR)
	rm -rf $(BUILD_ROOTFS_DIR)/*

	mkdir -p $(BUILD_ROOTFS_DIR)/rootfs

	if [ -d $(RAMDISK_DIR)/rootfs/$(COMMON_PATH)/ ]; then \
		cp -a --remove-destination $(RAMDISK_DIR)/rootfs/$(COMMON_PATH)/* $(BUILD_ROOTFS_DIR)/rootfs/; \
	fi

	if [ -d $(RAMDISK_DIR)/rootfs/overlay/$(OVERLAY_PATH) ]; then \
		cp -r $(RAMDISK_DIR)/rootfs/overlay/$(OVERLAY_PATH)/* $(BUILD_ROOTFS_DIR)/rootfs; \
	fi

	$(foreach t,$(TARGET_PACKAGES),\
	${Q}cd $(MEDIASDK_DIR)/platform/ramdisk/rootfs/public/$(t)/$(packages_arch)/ && \
	${Q}find . \( ! -type d ! -name "*.a" ! -path "*include*" ! -name ".gitkeep" \) \
		-printf 'Copy Package file $(MEDIASDK_DIR)/platform/ramdisk/rootfs/public/$(t)/$(packages_arch)/%p\n' \
		-exec cp -a --parents '{}' $(BUILD_ROOTFS_DIR)/rootfs/ \; ; )
	$(CURDIR)/tools/common/rootfs_tool/prepare_rootfs.sh $(BUILD_ROOTFS_DIR)/rootfs

ifeq ($(CONFIG_ENABLE_ISP_PQ_TOOL), y)
	if [ -d $(MEDIASDK_DIR)/platform/ramdisk/rootfs/public/envtools/$(packages_arch) ]; then \
		cp -rf $(MEDIASDK_DIR)/platform/ramdisk/rootfs/public/envtools/$(packages_arch)/* $(BUILD_ROOTFS_DIR)/rootfs; \
	fi
endif

	# Generate S10_automount
	python3 $(CURDIR)/tools/common/image_tool/create_automount.py $(SDK_PARTITION_XML) $(BUILD_ROOTFS_DIR)/rootfs/etc/init.d/
	# Generate /etc/fw_env.config
	python3 $(CURDIR)/tools/common/image_tool/mkcvipart.py $(SDK_PARTITION_XML) $(BUILD_ROOTFS_DIR)/rootfs/etc/ --fw_env

	if [ -d $(BUILD_ROOTFS_DIR)/rootfs/system ]; then \
		cp -rf $(BUILD_ROOTFS_DIR)/rootfs/system/* $(BUILD_ROOTFS_DIR)/rootfs/mnt/system; \
	fi

	# overlay board sel
ifeq ($(CONFIG_WIFI),y)
	if [ -d $(CURDIR)/boards/${CHIP_TYPE}/$(CHIP_NAME)_$(BOARD_NAME)/rootfs/musl_wifi/ ]; then \
		cp -rf $(CURDIR)/boards/${CHIP_TYPE}/$(CHIP_NAME)_$(BOARD_NAME)/rootfs/musl_wifi/* $(BUILD_ROOTFS_DIR)/rootfs/; \
	fi
endif
	if [ -d $(CURDIR)/boards/${CHIP_TYPE}/$(CHIP_NAME)_$(BOARD_NAME)/rootfs/$(ROOTFS_ARCH) ]; then \
		cp -rf $(CURDIR)/boards/${CHIP_TYPE}/$(CHIP_NAME)_$(BOARD_NAME)/rootfs/$(ROOTFS_ARCH)/* $(BUILD_ROOTFS_DIR)/rootfs/; \
	fi
ifeq ($(CONFIG_ROOTFS_WITH_APP),y)
	cp -a $(BUILD_MW_DIR)/ko $(BUILD_ROOTFS_DIR)/rootfs/mnt/system
	find $(KERNEL_MODULE_DIR) -name '*.ko' \
		-exec cp {} $(BUILD_ROOTFS_DIR)/rootfs/mnt/system/ko/ \;

	# copy osdrv
	mkdir -p $(BUILD_ROOTFS_DIR)/rootfs/mnt/system/ko/3rd
	find $(OSDRV_MODULE_DIR) -name '*.ko' \
		-exec cp {} $(BUILD_ROOTFS_DIR)/rootfs/mnt/system/ko/3rd/ \;

	cp -rdf $(MEDIASDK_DIR)/out/install/bin $(BUILD_ROOTFS_DIR)/rootfs/mnt/system
	# rm ini bin
	rm -rf $(BUILD_ROOTFS_DIR)/rootfs/mnt/system/bin/bin2flash
	rm -rf $(BUILD_ROOTFS_DIR)/rootfs/mnt/system/bin/ini2bin_board

ifneq ($(CONFIG_STATIC), y)
	mkdir -p $(BUILD_ROOTFS_DIR)/rootfs/mnt/system/usr
	cp -rdf $(MEDIASDK_DIR)/out/install/lib $(BUILD_ROOTFS_DIR)/rootfs/mnt/system/usr/
endif

else
	rm -rf $(BUILD_ROOTFS_DIR)/rootfs/mnt/system/*
endif
	#clean lib
	$(CURDIR)/boards/${CHIP_TYPE}/$(CHIP_NAME)_$(BOARD_NAME)/rootfs_script/clean_rootfs.sh $(BUILD_ROOTFS_DIR)/rootfs

	# strip all .so and remove .a used files.
	find $(BUILD_ROOTFS_DIR)/rootfs -executable -type f ! -name "*.sh" ! -name "*.ko" ! -path "*etc*" -printf 'striping %p\n' -exec $(CROSS_COMPILE)strip --strip-all {} \;
	find $(BUILD_ROOTFS_DIR)/rootfs -name "*.ko" -printf 'striping %p\n' -exec $(CROSS_COMPILE)strip --strip-unneeded {} \;
	find $(BUILD_ROOTFS_DIR)/rootfs -name "*.so*" -printf 'striping %p\n' -exec ${CROSS_COMPILE}strip --strip-all {} \;

	rm -rf $(BUILD_ROOTFS_DIR)/rootfs/system

ifeq ($(STORAGE_TYPE),spinor)
	mksquashfs $(BUILD_ROOTFS_DIR)/rootfs $(BUILD_ROOTFS_DIR)/rootfs.sqsh -root-owned -comp xz
	mv $(BUILD_ROOTFS_DIR)/rootfs.sqsh $(BUILD_ROOTFS_DIR)/rootfs.$(STORAGE_TYPE)
	cp $(BUILD_ROOTFS_DIR)/rootfs.$(STORAGE_TYPE) $(RAWIMAGE_DIR)
else
	mksquashfs $(BUILD_ROOTFS_DIR)/rootfs $(BUILD_ROOTFS_DIR)/rootfs.emmc \
		 -root-owned -comp xz -e mnt/cfg/*
endif

ifeq ($(STORAGE_TYPE),spinand)
	# hard-coded using rootfs.emmc as input
	python3 $(TOOLS_DIR)/common/spinand_tool/mkubiimg.py --ubionly \
		$(SDK_PARTITION_XML) \
		ROOTFS $(BUILD_ROOTFS_DIR)/rootfs.emmc $(BUILD_ROOTFS_DIR)/rootfs.spinand \
		-b $(CONFIG_NANDFLASH_BLOCKSIZE) -p $(CONFIG_NANDFLASH_PAGESIZE)
endif

	python3 $(TOOLS_DIR)/common/image_tool/raw2cimg.py \
		$(BUILD_ROOTFS_DIR)/rootfs.${STORAGE_TYPE} \
		$(OUTPUT_DIR) $(SDK_PARTITION_XML)


system.$(STORAGE_TYPE): ramdisk middleware osdrv
	echo "build $@"
	mkdir -p $(BUILD_SYSTEM_DIR)/system
	mkdir -p $(RAWIMAGE_DIR)
	rm -rf $(BUILD_SYSTEM_DIR)/system/*
	mkdir -p $(BUILD_SYSTEM_DIR)/system/bin/App
	cp $(MEDIASDK_DIR)/../App $(BUILD_SYSTEM_DIR)/system/bin -rf
	if [ -d $(RAMDISK_DIR)/rootfs/overlay/$(OVERLAY_PATH) ]; then \
		cp -r $(RAMDISK_DIR)/rootfs/overlay/$(OVERLAY_PATH)/system $(BUILD_SYSTEM_DIR)/; \
	fi

	cp -a $(BUILD_MW_DIR)/ko $(BUILD_SYSTEM_DIR)/system
	find $(KERNEL_MODULE_DIR) -name '*.ko' \
		-exec cp {} $(BUILD_SYSTEM_DIR)/system/ko/ \;
	# !!! this is lyj add to reduce flash /mnt/system for more lib*.so spcae
	# rm -f $(BUILD_SYSTEM_DIR)/system/ko/cv181x_ive.ko

	# copy osdrv
	mkdir -p $(BUILD_SYSTEM_DIR)/system/ko/3rd
	find $(OSDRV_MODULE_DIR) -name '*.ko' \
		-exec cp {} $(BUILD_SYSTEM_DIR)/system/ko/3rd/ \;
	# !!! aic8800 in out/install/bin is no use, and ko can also delete
	cp -rdf $(MEDIASDK_DIR)/out/install/bin $(BUILD_SYSTEM_DIR)/system
	rm -rf $(BUILD_SYSTEM_DIR)/system/bin/aic8800
	rm -f $(BUILD_SYSTEM_DIR)/system/ko/3rd/aic8800_*.ko

ifeq ($(CONFIG_STATIC), y)

else
	mkdir -p $(BUILD_SYSTEM_DIR)/system/usr
	cp -rdf $(MEDIASDK_DIR)/out/install/lib $(BUILD_SYSTEM_DIR)/system/usr/
endif

	if [ -d $(CURDIR)/boards/${CHIP_TYPE}/$(CHIP_NAME)_$(BOARD_NAME)/rootfs/$(ROOTFS_ARCH)/mnt/system ]; then \
		cp -rf $(CURDIR)/boards/${CHIP_TYPE}/$(CHIP_NAME)_$(BOARD_NAME)/rootfs/$(ROOTFS_ARCH)/mnt/system/* $(BUILD_SYSTEM_DIR)/system/; \
	fi

ifeq ($(CONFIG_SPEECH_CARDV), y)
		cp -rf $(CURDIR)/boards/${CHIP_TYPE}/$(CHIP_NAME)_$(BOARD_NAME)/rootfs/speech_model/sound_cardv_sr16k_v5_cv181x.cvimodel $(BUILD_SYSTEM_DIR)/system/;
endif

ifeq ($(CONFIG_SERVICES_ADAS), y)
		cp -rf $(CURDIR)/boards/${CHIP_TYPE}/$(CHIP_NAME)_$(BOARD_NAME)/rootfs/adas_model/vehicle_det.cvimodel $(BUILD_SYSTEM_DIR)/system/;
		cp -rf $(CURDIR)/boards/${CHIP_TYPE}/$(CHIP_NAME)_$(BOARD_NAME)/rootfs/adas_model/lane_det.cvimodel $(BUILD_SYSTEM_DIR)/system/;
endif

	#clean lib
	$(CURDIR)/boards/${CHIP_TYPE}/$(CHIP_NAME)_$(BOARD_NAME)/rootfs_script/clean_rootfs.sh $(BUILD_SYSTEM_DIR)/system

ifeq ($(CONFIG_BOARD_DEBUG),y)
	mkdir -p $(BUILD_SYSTEM_DIR)/system/bin/param
	cp -rdf $(PARAM_FULL_PATH)/* $(BUILD_SYSTEM_DIR)/system/bin/param/
	mv $(BUILD_SYSTEM_DIR)/system/bin/bin2flash $(BUILD_SYSTEM_DIR)/system/bin/param/
	mv $(BUILD_SYSTEM_DIR)/system/bin/ini2bin_board $(BUILD_SYSTEM_DIR)/system/bin/param/
else
	rm -rf $(BUILD_SYSTEM_DIR)/system/bin/bin2flash
	rm -rf $(BUILD_SYSTEM_DIR)/system/bin/ini2bin_board
	rm -rf $(BUILD_SYSTEM_DIR)/system/bin/cc_tools
	rm -rf $(BUILD_SYSTEM_DIR)/system/bin/test_scripts
endif

	# strip all .so and remove .a used files.
	find $(BUILD_SYSTEM_DIR)/system -executable -type f ! -name "*.sh" ! -name "*.ko" ! -path "*etc*" -printf 'striping %p\n' -exec $(CROSS_COMPILE)strip --strip-all {} \;
	find $(BUILD_SYSTEM_DIR)/system -name "*.ko" -printf 'striping %p\n' -exec $(CROSS_COMPILE)strip --strip-unneeded {} \;
	find $(BUILD_SYSTEM_DIR)/system -name "*.so*" -printf 'striping %p\n' -exec ${CROSS_COMPILE}strip --strip-all {} \;
	find $(BUILD_SYSTEM_DIR)/system -name "*.a" -printf 'rm %p\n' -exec rm -rf {} \;

ifeq ($(STORAGE_TYPE),emmc)
	make_ext4fs -l 2G -L SYSTEM $(BUILD_SYSTEM_DIR)/system.emmc $(BUILD_SYSTEM_DIR)/system
	# Resize the image
	#e2fsck -f -y $(BUILD_SYSTEM_DIR)/system.emmc
	resize2fs -M $(BUILD_SYSTEM_DIR)/system.emmc
else ifeq ($(STORAGE_TYPE),spinand)
	python3 $(TOOLS_DIR)/common/spinand_tool/mkubiimg.py \
		$(SDK_PARTITION_XML) \
		SYSTEM $(BUILD_SYSTEM_DIR)/system $(BUILD_SYSTEM_DIR)/system.spinand \
		-b $(CONFIG_NANDFLASH_BLOCKSIZE) -p $(CONFIG_NANDFLASH_PAGESIZE)
else ifeq ($(STORAGE_TYPE),spinor)
	# $(TOOLS_DIR)/common/spinor_tool/mkfs.jffs2 -e 0x10000 -d $(BUILD_SYSTEM_DIR)/system \
	# -o $(BUILD_SYSTEM_DIR)/system.spinor -n -X $(JFFS2_COMPRESSION) -m size
	python3 $(TOOLS_DIR)/common/spinor_tool/mkjffs2img.py \
		$(SDK_PARTITION_XML) \
		SYSTEM $(BUILD_SYSTEM_DIR)/system $(BUILD_SYSTEM_DIR)/system.spinor 65536 lzo
else
$(error $(STORAGE_TYPE) invalid, [emmc | spinand | spinor])
endif
	cp $(BUILD_SYSTEM_DIR)/system.${STORAGE_TYPE} $(RAWIMAGE_DIR)
	python3 $(TOOLS_DIR)/common/image_tool/raw2cimg.py \
		$(BUILD_SYSTEM_DIR)/system.${STORAGE_TYPE} \
		$(OUTPUT_DIR) $(SDK_PARTITION_XML)

ota_images:
	python3 ${TOOLS_DIR}/common/spinand_tool/packcimg.py ${OUTPUT_DIR} --ver \
		${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_NUMBER} -o \
		${OUTPUT_DIR}/upgrade_${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_NUMBER}.bin --xml ${SDK_PARTITION_XML}

raw_images:
	python3 ${TOOLS_DIR}/common/image_tool/pack_images.py \
		${CHIP_TYPE} ${SDK_PARTITION_XML} ${OUTPUT_DIR}/raw_image ${OUTPUT_DIR}/

clean:
	rm -rf $(OUTPUT_DIR)
