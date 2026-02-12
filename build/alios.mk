.PHONY: alios_partition_dep
.PHONY: alios-tool-clean
.PHONY: alios-loader-clean

ALIOS_SOLUTIONS_DIR=${ALIOS_DIR}/solutions/${CONFIG_ALIOS_SOLUTION}
CVI_BOARD_PINMUX_PATH=${ALIOS_DIR}/solutions/${CONFIG_ALIOS_SOLUTION}/application/common/src/cvi_board_pinmux.c
CVI_AHD_INFO_PATH=${ALIOS_DIR}/solutions/${CONFIG_ALIOS_SOLUTION}/application/media/src/cvi_ahd_info.h

alios-build: memory-map
	$(call print_target)
	${Q}ln -snrf ${CVI_BOARD_MEMMAP_H_PATH} ${ALIOS_DIR}/components/aos/include/
	${Q}ln -snrf ${CVI_BOARD_MEMMAP_LD_PATH} ${ALIOS_DIR}/solutions/yoc/
	${Q}ln -snrf ${BOARD_DIR}/alios/cvi_board_pinmux.c ${CVI_BOARD_PINMUX_PATH}
	${Q}ln -snrf ${BOARD_DIR}/alios/cvi_ahd_info.h ${CVI_AHD_INFO_PATH}
	# ${Q}ln -sf package.yaml.turnkey_${CHIP_NAME} ${ALIOS_SOLUTIONS_DIR}/package.yaml.turnkey
	${Q}cd ${ALIOS_DIR};echo "y" | yoc init;cd -
	${Q}$(MAKE) -C ${ALIOS_SOLUTIONS_DIR} BOARD=""
ifeq ($(CONFIG_COMPRESS_RTOS_BIN), y)
	${Q}cp ${ALIOS_SOLUTIONS_DIR}/yoc_lzma.bin ${OUTPUT_DIR}/yoc.bin
else
	${Q}cp ${ALIOS_SOLUTIONS_DIR}/yoc.bin ${OUTPUT_DIR}/
endif
	${Q}cp ${OUTPUT_DIR}/yoc.bin ${OUTPUT_DIR}/raw_image/yoc.bin

alios_clean:
	$(call print_target)
	${Q}$(MAKE) -C ${ALIOS_SOLUTIONS_DIR} clean

alios: alios-build

alios_build_burn_tool%: export KBUILD_OUTPUT=${BUILD_UBOOT_DIR}/burn_tool
# alios_build_burn_tool%: export RELEASE=${RELEASE_VERSION}
# alios_build_burn_tool%: export CVIBOARD=${BOARD}
alios_build_burn_tool%: export CONFIG_SKIP_RAMDISK:=y
# alios_build_burn_tool%: export CONFIG_USE_DEFAULT_ENV:=${CONFIG_USE_DEFAULT_ENV}
# alios_build_burn_tool%: export MULTI_FIP=$(if ${CONFIG_MULTI_FIP},1,0)
# alios_build_burn_tool%: export CROSS_COMPILE=$(patsubst "%",%,$(CONFIG_CROSS_COMPILE))
# alios_build_burn_tool%: export ARCH=$(patsubst "%",%,$(ARCH_TYPE))
alios_build_burn_tool%: export ARCH=riscv

ALIOS_BURN_TOOL_BUILD_PATH:
	mkdir -p ${BUILD_UBOOT_DIR}/burn_tool

#ALIOS_PARTITION_DEP := ${UBOOT_DIR}/include/imgs.h ${UBOOT_DIR}/include/cvipart.h
ALIOS_PARTITION_DEP := alios_partition_dep

${ALIOS_PARTITION_DEP}: ${FLASH_CONFIG_YAML}
	$(call print_target)
	${Q}python3 $(CURDIR)/tools/common/image_tool/alios/mkcvipart_alios.py ${FLASH_CONFIG_YAML} ${UBOOT_DIR}/include
	${Q}python3 $(CURDIR)/tools/common/image_tool/alios/mk_imgHeader_alios.py ${FLASH_CONFIG_YAML} ${UBOOT_DIR}/include

BURNTOOL_OUTPUT_CONFIG_PATH := ${BUILD_UBOOT_DIR}/burn_tool/.config
BURNTOOL_DEFAULT_CONFIG_PATH := ${BOARD_DIR}/u-boot/${BRAND}_$(CHIP_NAME)_$(BOARD_NAME)_burntool_defconfig

${BURNTOOL_OUTPUT_CONFIG_PATH}: ${BURNTOOL_DEFAULT_CONFIG_PATH}
	$(call print_target)
	${Q}echo "u-boot's defconfig is updated. Use it."
	${Q}mkdir -p $(dir ${BURNTOOL_OUTPUT_CONFIG_PATH})
	${Q}cmp -s ${BURNTOOL_DEFAULT_CONFIG_PATH} ${BURNTOOL_OUTPUT_CONFIG_PATH} || \
		${Q}cp -vb ${BURNTOOL_DEFAULT_CONFIG_PATH} ${BURNTOOL_OUTPUT_CONFIG_PATH}

alios_build_burn_tool: ${ALIOS_BURN_TOOL_BUILD_PATH}
alios_build_burn_tool: ${ALIOS_PARTITION_DEP} ${BURNTOOL_OUTPUT_CONFIG_PATH}
	$(call print_target)
	${Q}rm -f ${UBOOT_CVI_BOARD_INIT_PATH}
	${Q}ln -s ${BOARD_DIR}/u-boot/cvi_board_init.c ${UBOOT_CVI_BOARD_INIT_PATH}
	${Q}rm -f ${UBOOT_CVITEK_PATH}
	${Q}ln -s ${BOARD_DIR}/u-boot/cvitek.h ${UBOOT_CVITEK_PATH}
	${Q}$(MAKE) STORAGE_TYPE=$(STORAGE_TYPE) -j${NPROC} -C ${UBOOT_DIR} olddefconfig
	${Q}$(MAKE) STORAGE_TYPE=$(STORAGE_TYPE) -j${NPROC} -C ${UBOOT_DIR} all
	${Q}cat ${BUILD_UBOOT_DIR}/burn_tool/u-boot.bin > ${BUILD_UBOOT_DIR}/burn_tool/u-boot-raw.bin

alios_build_burn_tool_dep: fip-pre-merge alios_build_burn_tool
	$(call print_target)
	${Q}python3 ${TOOLS_PATH}/${CHIP_ARCH_L}/pack_fip/pack_fip.py ${FIP_PRE_BIN_DIR}/fip_pre.bin \
		--add-bl33 ${BUILD_UBOOT_DIR}/burn_tool/u-boot.bin --output ${OUTPUT_DIR}/fip.bin

alios-tool: alios_build_burn_tool_dep

alios-tool-menuconfig: export KBUILD_OUTPUT=${BUILD_UBOOT_DIR}/burn_tool
alios-tool-menuconfig: ${BURNTOOL_OUTPUT_CONFIG_PATH}
	$(call print_target)
	${Q}$(MAKE) -j${NPROC} -C ${UBOOT_DIR} menuconfig
	${Q}$(MAKE) -j${NPROC} -C ${UBOOT_DIR} savedefconfig

alios-tool-clean: export KBUILD_OUTPUT=${BUILD_UBOOT_DIR}/burn_tool
alios-tool-clean:
	$(call print_target)
	${Q}$(MAKE) -j${NPROC} -C ${UBOOT_DIR} distclean

alios-loader-clean: export KBUILD_OUTPUT=${BUILD_UBOOT_DIR}
alios-loader-clean:
	$(call print_target)
	${Q}$(MAKE) -j${NPROC} -C ${UBOOT_DIR} distclean
	rm -f ${BUILD_UBOOT_DIR}/u-boot.bin.lzma
