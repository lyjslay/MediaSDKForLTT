include $(SRCTREE)/build/base.mk
CPRE := @
ifeq ($(V),1)
CPRE :=
VERB := --verbose
endif
.PHONY: prepare all clean install flashall flash sdk
HOST_TOOLS = $(SRCTREE)/platform/alios/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin
export PATH := $(HOST_TOOLS):$(PATH)

all: prepare
	@echo "Build Solution by $(BOARD) "
	$(CPRE) if [ -e $(CURDIR)/prebuild.sh ]; then bash $(CURDIR)/prebuild.sh; fi
	$(CPRE) scons $(VERB) --board=$(BOARD) -j4
	$(CPRE) yoc sdk

flashall:
	$(CPRE) product flash ${MK_GENERATED_IMGS_PATH}/images.zip -a -x gdbinit -f ${ALGORITHMS_PATH}

flash:
	$(CPRE) product flash ${MK_GENERATED_IMGS_PATH}/images.zip -w prim -x gdbinit -f ${ALGORITHMS_PATH}

sdk:
	$(CPRE) yoc sdk

clean: prepare
	$(CPRE) scons -c
ifeq ($(OS), Windows_NT)
	$(CPRE) if exist yoc_sdk rmdir /s /q yoc_sdk
	$(CPRE) if exist binary rmdir /s /q binary
	$(CPRE) if exist out rmdir /s /q out
	$(CPRE) if exist generated rmdir /s /q generated
	$(CPRE) if exist yoc.elf del /f /q yoc.elf
	$(CPRE) if exist yoc.map del /f /q yoc.map
else
	$(CPRE) rm -rf yoc_sdk binary out yoc.elf yoc.map generated
endif
	$(CPRE) if [ -e cvi_board_memmap.ld ]; then rm -f cvi_board_memmap.ld; fi
	$(CPRE) if [ -e .sconsign.dblite ]; then rm -f .sconsign.dblite; fi
	$(CPRE) if [ -e package.yaml ]; then rm -f package.yaml; fi

prepare:
	cp package_$(CHIP_TYPE).yaml -rf package.yaml
	# make -C $(SRCTREE)/cpsl/mmf/component/isp/sensor all

install:
	$(CPRE) yoc sdk