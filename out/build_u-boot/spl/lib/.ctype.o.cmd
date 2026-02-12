cmd_spl/lib/ctype.o := aarch64-linux-gnu-gcc -Wp,-MD,spl/lib/.ctype.o.d  -nostdinc -isystem /media/cvitek/yijun.liu01/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/../lib/gcc/aarch64-linux-gnu/6.3.1/include -Ispl/include -Iinclude  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvitek  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include -include /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/kconfig.h  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib -Ispl/lib -D__KERNEL__ -D__UBOOT__ -DCONFIG_SKIP_RAMDISK=y -DCONFIG_USE_DEFAULT_ENV=y -DCVICHIP=cv1811ha -DCVIBOARD=wevb_0007a_spinor -DCV1811HA_WEVB_0007A_SPINOR -DCONFIG_SPL_BUILD -Wall -Wstrict-prototypes -Wno-format-security -fno-builtin -ffreestanding -std=gnu11 -fshort-wchar -fno-strict-aliasing -Werror -fno-PIE  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvitek -Os -fno-stack-protector -fno-delete-null-pointer-checks -Wno-pointer-sign -Wno-array-bounds -Wno-maybe-uninitialized -g -fstack-usage -Wno-format-nonliteral -Wno-unused-but-set-variable -Werror=date-time -ffunction-sections -fdata-sections -fno-stack-protector -D__ARM__ -mstrict-align -ffunction-sections -fdata-sections -fno-common -ffixed-r9 -fno-common -ffixed-x18 -pipe -march=armv8-a -D__LINUX_ARM_ARCH__=8    -DKBUILD_BASENAME='"ctype"'  -DKBUILD_MODNAME='"ctype"' -c -o spl/lib/ctype.o /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/ctype.c

source_spl/lib/ctype.o := /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/ctype.c

deps_spl/lib/ctype.o := \
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
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/ctype.h \

spl/lib/ctype.o: $(deps_spl/lib/ctype.o)

$(deps_spl/lib/ctype.o):
