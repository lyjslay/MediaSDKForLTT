include $(SRCTREE)/applications/build/base.mk

all: prepare_include core ini2bin_host ini2bin_board bin2flash

OUTPUT_DIR=$(SRCTREE)/out
include $(SRCTREE)/build/scripts/mmap.mk

prepare_include: memory-map
	ln -snrf ${CVI_BOARD_MEMMAP_H_PATH}/ core/include/

core:
	echo $(PARAM_FULL_PATH)
	$(MAKE) -C core $(MAKECMDGOALS)

ini2bin_host:

	$(MAKE) -C ini2bin RUNTIME_ENV=pc $(MAKECMDGOALS)
	@cp $(OBJTREE)/bin/ini2bin_host $(PARAM_FULL_PATH)/
	mkdir -p $(SRCTREE)/out/param_bin
# 	@cd $(PARAM_FULL_PATH); ./ini2bin_host; cp app_cfg* $(SRCTREE)/out/param_bin -rf; rm app_cfg* ini2bin_host; cd -
	echo "Load param form $(PARAM_FULL_PATH)"
	cd $(PARAM_FULL_PATH); ./ini2bin_host; cp app_cfg* $(SRCTREE)/out/param_bin -rf; cd -

ini2bin_board:
	$(MAKE) -C ini2bin RUNTIME_ENV=board $(MAKECMDGOALS)

bin2flash:
	$(MAKE) -C bin2flash $(MAKECMDGOALS)

install:
	$(MAKE) install -C core $(MAKECMDGOALS)
	$(MAKE) install -C ini2bin RUNTIME_ENV=board $(MAKECMDGOALS)
	$(MAKE) install -C bin2flash $(MAKECMDGOALS)

clean:
	$(MAKE) clean -C core $(MAKECMDGOALS)
	$(MAKE) clean -C bin2flash $(MAKECMDGOALS)
	$(MAKE) clean -C ini2bin RUNTIME_ENV=pc $(MAKECMDGOALS)
	$(MAKE) clean -C ini2bin RUNTIME_ENV=board $(MAKECMDGOALS)


.PHONY: all clean install core sample_flash ini2bin_host ini2bin_board bin2flash sample_readcfg
