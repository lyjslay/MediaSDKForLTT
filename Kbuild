#
# Kbuild for top-level directory of the application
#
ifeq ($(CONFIG_RISCV), y)
	ARCH_TYPE := riscv
else ifeq ($(CONFIG_ARM), y)
	ARCH_TYPE := arm
endif

ifeq ($(CONFIG_TOOLCHAIN_GLIBC_RISCV64),y)
ROOTFS_ARCH := glibc_riscv64
MMF_TYPE	:= riscv_glibc
LP_TYPE		:= lp64d
SYSROOT_PATH:= sysroot-glibc-riscv64
else ifeq ($(CONFIG_TOOLCHAIN_MUSL_RISCV64),y)
ROOTFS_ARCH := musl_riscv64
MMF_TYPE	:= riscv_musl
LP_TYPE		:= lp64d
SYSROOT_PATH:= sysroot-musl-riscv64
else ifeq ($(CONFIG_TOOLCHAIN_GLIBC_ARM),y)
ROOTFS_ARCH := glibc_arm
MMF_TYPE	:= arm_glibc
LP_TYPE		:=
SYSROOT_PATH:= sysroot-glibc-linaro-2.23-2017.05-arm-linux-gnueabihf
endif

ifeq ($(CONFIG_ENABLE_ISP_PQ_TOOL), y)
	KBUILD_DEFINES += -DENABLE_ISP_PQ_TOOL
endif

include $(SRCTREE)/build/boards/Kbuild
include $(SRCTREE)/platform/Kbuild
include $(SRCTREE)/cpsl/mmf/Kbuild
include $(SRCTREE)/cpsl/hal/Kbuild
include $(SRCTREE)/applications/Kbuild
include $(SRCTREE)/framework/components/Kbuild
include $(SRCTREE)/framework/services/Kbuild
