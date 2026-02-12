cmd_spl/arch/arm/lib/crt0_aarch64_efi.o := aarch64-linux-gnu-gcc -Wp,-MD,spl/arch/arm/lib/.crt0_aarch64_efi.o.d  -nostdinc -isystem /media/cvitek/yijun.liu01/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/../lib/gcc/aarch64-linux-gnu/6.3.1/include -Ispl/include -Iinclude  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvitek  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include -include /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/kconfig.h -D__KERNEL__ -D__UBOOT__ -DCONFIG_SKIP_RAMDISK=y -DCONFIG_USE_DEFAULT_ENV=y -DCVICHIP=cv1811ha -DCVIBOARD=wevb_0007a_spinor -DCV1811HA_WEVB_0007A_SPINOR -DCONFIG_SPL_BUILD -D__ASSEMBLY__ -fno-PIE -g -D__ARM__ -mstrict-align -ffunction-sections -fdata-sections -fno-common -ffixed-r9 -fno-common -ffixed-x18 -pipe -march=armv8-a -D__LINUX_ARM_ARCH__=8   -c -o spl/arch/arm/lib/crt0_aarch64_efi.o /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/lib/crt0_aarch64_efi.S

source_spl/arch/arm/lib/crt0_aarch64_efi.o := /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/lib/crt0_aarch64_efi.S

deps_spl/arch/arm/lib/crt0_aarch64_efi.o := \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/kconfig.h \
    $(wildcard include/config/booger.h) \
    $(wildcard include/config/foo.h) \
    $(wildcard include/config/spl/.h) \
    $(wildcard include/config/tpl/build.h) \
    $(wildcard include/config/spl/build.h) \
    $(wildcard include/config/spl/foo.h) \
    $(wildcard include/config/tpl/foo.h) \
    $(wildcard include/config/spl/option.h) \
    $(wildcard include/config/spl/acme.h) \
    $(wildcard include/config/acme.h) \
    $(wildcard include/config/tpl/acme.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/asm-generic/pe.h \

spl/arch/arm/lib/crt0_aarch64_efi.o: $(deps_spl/arch/arm/lib/crt0_aarch64_efi.o)

$(deps_spl/arch/arm/lib/crt0_aarch64_efi.o):
