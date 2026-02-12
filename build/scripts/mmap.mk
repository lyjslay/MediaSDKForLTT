.PHONY: memory-map

CVI_BOARD_MEMMAP_H_PATH := ${OUTPUT_DIR}/build_memmap/${CHIP_NAME}_${BOARD_NAME}/cvi_board_memmap.h
CVI_BOARD_MEMMAP_CONF_PATH := ${OUTPUT_DIR}/build_memmap/${CHIP_NAME}_${BOARD_NAME}/cvi_board_memmap.conf
CVI_BOARD_MEMMAP_LD_PATH:= ${OUTPUT_DIR}/build_memmap/${CHIP_NAME}_${BOARD_NAME}/cvi_board_memmap.ld

BOARD_MMAP_PATH := ${BOARD_DIR}/memmap.py
MMAP_CONV_PY := $(SRCTREE)/build/scripts/mmap_conv.py
$(info $(BOARD_MMAP_PATH) )

${CVI_BOARD_MEMMAP_H_PATH}: ${BOARD_MMAP_PATH} ${MMAP_CONV_PY}
	$(call print_target)
	mkdir -p $(dir $@)
	@${MMAP_CONV_PY} --type h $< $@

${CVI_BOARD_MEMMAP_CONF_PATH}: ${BOARD_MMAP_PATH} ${MMAP_CONV_PY}
	$(call print_target)
	@mkdir -p $(dir $@)
	@${MMAP_CONV_PY} --type conf $< $@

${CVI_BOARD_MEMMAP_LD_PATH}: ${BOARD_MMAP_PATH} ${MMAP_CONV_PY}
	$(call print_target)
	@mkdir -p $(dir $@)
	@${MMAP_CONV_PY} --type ld $< $@

ifeq ($(wildcard ${BOARD_MMAP_PATH}),)
memory-map:
else
memory-map: ${CVI_BOARD_MEMMAP_H_PATH} ${CVI_BOARD_MEMMAP_CONF_PATH} ${CVI_BOARD_MEMMAP_LD_PATH}
endif
