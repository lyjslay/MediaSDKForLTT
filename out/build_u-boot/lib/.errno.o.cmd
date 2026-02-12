cmd_lib/errno.o := aarch64-linux-gnu-gcc -Wp,-MD,lib/.errno.o.d  -nostdinc -isystem /media/cvitek/yijun.liu01/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/../lib/gcc/aarch64-linux-gnu/6.3.1/include -Iinclude  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvitek  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include -include /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/kconfig.h  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib -Ilib -D__KERNEL__ -D__UBOOT__ -DCONFIG_SKIP_RAMDISK=y -DCONFIG_USE_DEFAULT_ENV=y -DCVICHIP=cv1811ha -DCVIBOARD=wevb_0007a_spinor -DCV1811HA_WEVB_0007A_SPINOR -Wall -Wstrict-prototypes -Wno-format-security -fno-builtin -ffreestanding -std=gnu11 -fshort-wchar -fno-strict-aliasing -Werror -fno-PIE  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvitek -Os -fno-stack-protector -fno-delete-null-pointer-checks -Wno-pointer-sign -Wno-array-bounds -Wno-maybe-uninitialized -g -fstack-usage -Wno-format-nonliteral -Wno-unused-but-set-variable -Werror=date-time -D__ARM__ -fno-pic -mstrict-align -ffunction-sections -fdata-sections -fno-common -ffixed-r9 -fno-common -ffixed-x18 -pipe -march=armv8-a -D__LINUX_ARM_ARCH__=8    -DKBUILD_BASENAME='"errno"'  -DKBUILD_MODNAME='"errno"' -c -o lib/errno.o /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/errno.c

source_lib/errno.o := /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/errno.c

deps_lib/errno.o := \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/kconfig.h \
    $(wildcard include/config/booger.h) \
    $(wildcard include/config/foo.h) \
    $(wildcard include/config/spl/.h) \
    $(wildcard include/config/tpl/build.h) \
    $(wildcard include/config/spl/build.h) \
    $(wildcard include/config/spl/foo.h) \
    $(wildcard include/config/tpl/foo.h) \
    $(wildcard include/config/option.h) \
    $(wildcard include/config/acme.h) \
    $(wildcard include/config/spl/acme.h) \
    $(wildcard include/config/tpl/acme.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/errno.h \
    $(wildcard include/config/errno/str.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/errno.h \

lib/errno.o: $(deps_lib/errno.o)

$(deps_lib/errno.o):
