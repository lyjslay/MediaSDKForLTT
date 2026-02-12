qstrip = $(strip $(subst ",,$(1)))

ifeq ($(ARCH_TYPE),riscv)
ARCH_FSBL := riscv
else
ARCH_FSBL := aarch64
endif

opensbi: export CROSS_COMPILE=$(CROSS_COMPILE_UBOOT)
opensbi: u-boot-build
	$(call print_target)
	${Q}$(MAKE) -j${NPROC} -C ${OPENSBI_DIR} PLATFORM=generic \
	    FW_PAYLOAD_PATH=${BUILD_UBOOT_DIR}/u-boot-raw.bin \
	    FW_FDT_PATH=${BUILD_UBOOT_DIR}/arch/riscv/dts/${CHIP_NAME}_${BOARD_NAME}.dtb

opensbi-clean:
	$(call print_target)
	${Q}$(MAKE) -C ${OPENSBI_DIR} PLATFORM=generic distclean

ifeq ($(call qstrip,${ARCH_TYPE}),riscv)
fsbl-build: opensbi
endif
fsbl-build: alios
fsbl%: export BLCP_2ND_PATH=${ALIOS_DIR}/solutions/${CONFIG_ALIOS_SOLUTION}/yoc.bin
fsbl%: export RTOS_ENABLE_FREERTOS=${CONFIG_ENABLE_FREERTOS}
fsbl%: export FSBL_SECURE_BOOT_SUPPORT=${CONFIG_FSBL_SECURE_BOOT_SUPPORT}
fsbl%: export ARCH=$(ARCH_FSBL)
ifeq ($(CONFIG_OD_CLK_SEL),y)
fsbl%: export OD_CLK_SEL=${CONFIG_OD_CLK_SEL}
else ifeq ($(CONFIG_VC_CLK_OVERDRIVE),y)
fsbl%: export VC_CLK_OVERDRIVE=${CONFIG_VC_CLK_OVERDRIVE}
endif
fsbl%: export COMPRESS_RTOS_BIN=${CONFIG_COMPRESS_RTOS_BIN}
fsbl%: export C906L_PARTITION_EXIST=1
ifeq (${CONFIG_BUILD_FOR_DEBUG},y)
fsbl%: export LOG_LEVEL=4
else
fsbl%: export LOG_LEVEL=1
endif

fsbl-build: u-boot-build memory-map
	$(call print_target)
	${Q}mkdir -p ${BUILD_FSBL_DIR}
	#TODO clean rm this dir
	${Q}mkdir -p ${FSBL_DIR}/build
	${Q}ln -snrf -t ${FSBL_DIR}/build ${CVI_BOARD_MEMMAP_H_PATH}
	${Q}ln -snrf -t ${FSBL_DIR}/build ${UBOOT_DIR}/include/cvipart.h
	${Q}$(MAKE) -j${NPROC} -C ${FSBL_DIR} O=${BUILD_FSBL_DIR} BLCP_2ND_PATH=${BLCP_2ND_PATH} LOG_LEVEL=${LOG_LEVEL}\
		LOADER_2ND_PATH=${BUILD_UBOOT_DIR}/u-boot-raw.bin CHIP_ARCH=${CHIP_TYPE} CROSS_COMPILE=$(CROSS_COMPILE_UBOOT) \
		BOOT_CPU=$(ARCH_FSBL) DDR_CFG=$(CONFIG_DDR_CFG) STORAGE_TYPE=${STORAGE_TYPE} FORCE_BOOT_FROM_FLASH=y
	${Q}cp ${BUILD_FSBL_DIR}/fip.bin ${OUTPUT_DIR}/
# ifeq ($(STORAGE_TYPE),spinor)
ifeq ($(CONFIG_UBOOT_SPL_CUSTOM),y)
	${Q}$(MAKE) -j${NPROC} -C ${FSBL_DIR} O=${BUILD_FSBL_DIR} BLCP_2ND_PATH=${BLCP_2ND_PATH} \
		LOADER_2ND_PATH=${BUILD_UBOOT_DIR}/u-boot-raw_spl.bin CHIP_ARCH=${CHIP_TYPE} CROSS_COMPILE=$(CROSS_COMPILE_UBOOT) \
		BOOT_CPU=$(ARCH_FSBL) DDR_CFG=$(CONFIG_DDR_CFG) STORAGE_TYPE=${STORAGE_TYPE} LOG_LEVEL=${LOG_LEVEL}
	${Q}cp ${BUILD_FSBL_DIR}/fip.bin ${OUTPUT_DIR}/fip_spl.bin
else
	${Q}$(MAKE) -C ${FSBL_DIR} clean O=${BUILD_FSBL_DIR} CHIP_ARCH=${CHIP_TYPE} CROSS_COMPILE=$(CROSS_COMPILE_UBOOT) \
		BOOT_CPU=$(ARCH_FSBL) DDR_CFG=$(CONFIG_DDR_CFG)
	${Q}$(MAKE) -j${NPROC} -C ${FSBL_DIR} O=${BUILD_FSBL_DIR} BLCP_2ND_PATH=${BLCP_2ND_PATH} \
		LOADER_2ND_PATH=${BUILD_UBOOT_DIR}/u-boot-raw.bin CHIP_ARCH=${CHIP_TYPE} CROSS_COMPILE=$(CROSS_COMPILE_UBOOT) \
		BOOT_CPU=$(ARCH_FSBL) DDR_CFG=$(CONFIG_DDR_CFG) STORAGE_TYPE=${STORAGE_TYPE} LOG_LEVEL=${LOG_LEVEL}
	${Q}cp ${BUILD_FSBL_DIR}/fip.bin ${OUTPUT_DIR}/fip_spl.bin
endif
	cp ${OUTPUT_DIR}/fip_spl.bin ${OUTPUT_DIR}/raw_image/
# endif
	${Q}python3 $(CURDIR)/tools/common/image_tool/raw2cimg.py ${OUTPUT_DIR}/yoc.bin ${OUTPUT_DIR} ${SDK_PARTITION_XML}

fsbl-clean:
	$(call print_target)
	${Q}$(MAKE) -C ${FSBL_DIR} clean O=${BUILD_FSBL_DIR}

u-boot-dep: fsbl-build ${OUTPUT_DIR}/elf
	$(call print_target)
ifeq ($(call qstrip,${ARCH_TYPE}),riscv)
	${Q}cp ${OPENSBI_DIR}/build/platform/generic/firmware/fw_payload.bin ${OUTPUT_DIR}/fw_payload_uboot.bin
	${Q}cp ${OPENSBI_DIR}/build/platform/generic/firmware/fw_payload.elf ${OUTPUT_DIR}/elf/fw_payload_uboot.elf
endif

ifeq ($(call qstrip,${ARCH_TYPE}),riscv)
u-boot-clean: opensbi-clean
endif
u-boot-clean: fsbl-clean
