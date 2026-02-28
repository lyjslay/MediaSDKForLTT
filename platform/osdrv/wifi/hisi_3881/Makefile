################################################################################
# Main Makefile for 3881 linux
# History
# Author     Date          Version
# hisilion   2020-3-29     V1.0
################################################################################
PWD := $(shell pwd)
KDIR=$(KERNEL_DIR)
CROSS_COMPILE=$(CROSS_COMPILE)
ARCH=$(ARCH)
HISILICON_PLATFORM=

CURDIR := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
SYSDIR ?= $(CURDIR)
PRJ_ROOT=$(CURDIR)
COMPLIE_ROOT := $(CURDIR)
WIFI_DRIVER_DIR=$(PRJ_ROOT)/driver

export PRJ_ROOT
export SYSDIR
export WIFI_DRIVER_DIR
export COMPLIE_ROOT

MAKE = make

TAGETS_BUILD :=
TAGETS_CLEAN :=

.PHONY: all linux_driver tools sample clean_prepare clean linux_driver_clean

all: linux_driver

clean:clean_prepare linux_driver_clean tools_clean

clean_prepare:
	$(RM) -rf $(SYSDIR)/build/build_tmp
	$(RM) -rf $(SYSDIR)/output

linux_driver:
	$(MAKE) ARCH=${ARCH} -C ${WIFI_DRIVER_DIR} WIFI_DRIVER_DIR=${WIFI_DRIVER_DIR} CROSS_COMPILE=${CROSS_COMPILE} HISILICON_PLATFORM=${HISILICON_PLATFORM} KDIR=${KDIR} clean; \
	$(MAKE) ARCH=${ARCH} -C ${WIFI_DRIVER_DIR} WIFI_DRIVER_DIR=${WIFI_DRIVER_DIR} CROSS_COMPILE=${CROSS_COMPILE} HISILICON_PLATFORM=${HISILICON_PLATFORM} KDIR=${KDIR} -j16
	cp ${WIFI_DRIVER_DIR}/*.ko  $(CURDIR)/

tools:
	$(MAKE) ARCH=${ARCH} -C components/linux CROSS_COMPILE=${CROSS_COMPILE} HISILICON_PLATFORM=${HISILICON_PLATFORM} tools


linux_driver_clean:
	$(MAKE) ARCH=${ARCH} -C ${WIFI_DRIVER_DIR} WIFI_DRIVER_DIR=${WIFI_DRIVER_DIR} CROSS_COMPILE=${CROSS_COMPILE} HISILICON_PLATFORM=${HISILICON_PLATFORM} KDIR=${KDIR} clean
	rm -rf ${CUR_DIR}/*.ko
tools_clean:
	$(MAKE) ARCH=${ARCH} -C components/linux CROSS_COMPILE=${CROSS_COMPILE} HISILICON_PLATFORM=${HISILICON_PLATFORM} clean
