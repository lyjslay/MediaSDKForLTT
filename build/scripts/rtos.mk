rtos: memory-map
	$(call print_target)
	cd ${FREERTOS_DIR}/cvitek && ./build_${CHIP_TYPE}.sh

rtos-clean:
	$(call print_target)
	cd ${FREERTOS_DIR}/cvitek && rm -rf build install
