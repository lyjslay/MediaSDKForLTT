cmd_spl/arch/arm/lib/cache.o := aarch64-linux-gnu-gcc -Wp,-MD,spl/arch/arm/lib/.cache.o.d  -nostdinc -isystem /media/cvitek/yijun.liu01/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/../lib/gcc/aarch64-linux-gnu/6.3.1/include -Ispl/include -Iinclude  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvitek  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include -include /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/kconfig.h  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/lib -Ispl/arch/arm/lib -D__KERNEL__ -D__UBOOT__ -DCONFIG_SKIP_RAMDISK=y -DCONFIG_USE_DEFAULT_ENV=y -DCVICHIP=cv1811ha -DCVIBOARD=wevb_0007a_spinor -DCV1811HA_WEVB_0007A_SPINOR -DCONFIG_SPL_BUILD -Wall -Wstrict-prototypes -Wno-format-security -fno-builtin -ffreestanding -std=gnu11 -fshort-wchar -fno-strict-aliasing -Werror -fno-PIE  -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvitek -Os -fno-stack-protector -fno-delete-null-pointer-checks -Wno-pointer-sign -Wno-array-bounds -Wno-maybe-uninitialized -g -fstack-usage -Wno-format-nonliteral -Wno-unused-but-set-variable -Werror=date-time -ffunction-sections -fdata-sections -fno-stack-protector -D__ARM__ -mstrict-align -ffunction-sections -fdata-sections -fno-common -ffixed-r9 -fno-common -ffixed-x18 -pipe -march=armv8-a -D__LINUX_ARM_ARCH__=8    -DKBUILD_BASENAME='"cache"'  -DKBUILD_MODNAME='"cache"' -c -o spl/arch/arm/lib/cache.o /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/lib/cache.c

source_spl/arch/arm/lib/cache.o := /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/lib/cache.c

deps_spl/arch/arm/lib/cache.o := \
    $(wildcard include/config/sys/cacheline/size.h) \
    $(wildcard include/config/sys/noncached/memory.h) \
    $(wildcard include/config/spl/sys/dcache/off.h) \
    $(wildcard include/config/spl/sys/thumb/build.h) \
    $(wildcard include/config/spl/sys/icache/off.h) \
    $(wildcard include/config/sys/mem/reserve/secure.h) \
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
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/common.h \
  include/config.h \
    $(wildcard include/config/boarddir.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/config_uncmd_spl.h \
    $(wildcard include/config/spl/dm.h) \
    $(wildcard include/config/dm/serial.h) \
    $(wildcard include/config/dm/i2c.h) \
    $(wildcard include/config/dm/spi.h) \
    $(wildcard include/config/dm/stdio.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/configs/cv181x-asic.h \
    $(wildcard include/config/env/offset.h) \
    $(wildcard include/config/env/offset/redund.h) \
    $(wildcard include/config/env/size.h) \
    $(wildcard include/config/env/is/in/spi/flash.h) \
    $(wildcard include/config/env/is/in/mmc.h) \
    $(wildcard include/config/env/is/in/nand.h) \
    $(wildcard include/config/env/sect/size.h) \
    $(wildcard include/config/bootcommand.h) \
    $(wildcard include/config/armv8/switch/to/el1.h) \
    $(wildcard include/config/remake/elf.h) \
    $(wildcard include/config/sys/resvionsz.h) \
    $(wildcard include/config/sys/bootmapsz.h) \
    $(wildcard include/config/sys/sdram/base.h) \
    $(wildcard include/config/sys/init/sp/addr.h) \
    $(wildcard include/config/sys/load/addr.h) \
    $(wildcard include/config/sys/bootm/len.h) \
    $(wildcard include/config/sys/malloc/len.h) \
    $(wildcard include/config/cons/index.h) \
    $(wildcard include/config/sys/ns16550/com1.h) \
    $(wildcard include/config/sys/ns16550/serial.h) \
    $(wildcard include/config/sys/ns16550/reg/size.h) \
    $(wildcard include/config/sys/ns16550/mem32.h) \
    $(wildcard include/config/sys/ns16550/clk.h) \
    $(wildcard include/config/baudrate.h) \
    $(wildcard include/config/menu/show.h) \
    $(wildcard include/config/spi/flash.h) \
    $(wildcard include/config/spi/flash/cvsfc.h) \
    $(wildcard include/config/cmd/mtdparts.h) \
    $(wildcard include/config/mtd/partitions.h) \
    $(wildcard include/config/flash/cfi/mtd.h) \
    $(wildcard include/config/sys/max/flash/banks.h) \
    $(wildcard include/config/spi/flash/mtd.h) \
    $(wildcard include/config/nand/support.h) \
    $(wildcard include/config/cmd/nand.h) \
    $(wildcard include/config/sys/max/nand/device.h) \
    $(wildcard include/config/nand/flash/cvsnfc.h) \
    $(wildcard include/config/sys/max/nand/chips.h) \
    $(wildcard include/config/sys/nand/self/init.h) \
    $(wildcard include/config/rbtree.h) \
    $(wildcard include/config/lzo.h) \
    $(wildcard include/config/cmd/ubi.h) \
    $(wildcard include/config/cmd/ubifs.h) \
    $(wildcard include/config/mtd/ubi/wl/threshold.h) \
    $(wildcard include/config/mtd/ubi/beb/limit.h) \
    $(wildcard include/config/sys/nand/max/chips.h) \
    $(wildcard include/config/sys/nand/base.h) \
    $(wildcard include/config/cvsnfc/max/chip.h) \
    $(wildcard include/config/cvsnfc/reg/base/address.h) \
    $(wildcard include/config/cvsnfc/buffer/base/address.h) \
    $(wildcard include/config/cvsnfc/hardware/pagesize/ecc.h) \
    $(wildcard include/config/sys/nand/base/list.h) \
    $(wildcard include/config/spl/nand/support.h) \
    $(wildcard include/config/nand/flash/cvsnfc/spl.h) \
    $(wildcard include/config/sys/cbsize.h) \
    $(wildcard include/config/sys/pbsize.h) \
    $(wildcard include/config/sys/prompt.h) \
    $(wildcard include/config/sys/bargsize.h) \
    $(wildcard include/config/sys/maxargs.h) \
    $(wildcard include/config/env/overwrite.h) \
    $(wildcard include/config/mmc/uhs/support.h) \
    $(wildcard include/config/mmc/hs200/support.h) \
    $(wildcard include/config/mmc/supports/tuning.h) \
    $(wildcard include/config/usb/dwc2.h) \
    $(wildcard include/config/usb/dwc2/reg/addr.h) \
    $(wildcard include/config/usb.h) \
    $(wildcard include/config/usb/gadget.h) \
    $(wildcard include/config/usb/gadget/dwc2/otg.h) \
    $(wildcard include/config/usb/function/fastboot.h) \
    $(wildcard include/config/usb/gadget/download.h) \
    $(wildcard include/config/g/dnl/manufacturer.h) \
    $(wildcard include/config/g/dnl/vendor/num.h) \
    $(wildcard include/config/g/dnl/product/num.h) \
    $(wildcard include/config/ipaddr.h) \
    $(wildcard include/config/netmask.h) \
    $(wildcard include/config/gatewayip.h) \
    $(wildcard include/config/serverip.h) \
    $(wildcard include/config/use/default/env.h) \
    $(wildcard include/config/bootlogo.h) \
    $(wildcard include/config/skip/ramdisk.h) \
    $(wildcard include/config/rootfs/rw.h) \
    $(wildcard include/config/emmc/support.h) \
    $(wildcard include/config/build/for/debug.h) \
    $(wildcard include/config/extra/env/settings.h) \
    $(wildcard include/config/nandbootcommand.h) \
    $(wildcard include/config/norbootcommand.h) \
    $(wildcard include/config/emmcbootcommand.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/../../../board/cvitek/cv181x/cv181x_reg.h \
    $(wildcard include/config/dw/wdt/base.h) \
    $(wildcard include/config/dw/wdt/clock/khz.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvi_board_memmap.h \
    $(wildcard include/config/sys/text/base.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvipart.h \
    $(wildcard include/config/sys/os/base.h) \
    $(wildcard include/config/sys/redundand/environment.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvitek/cvi_panels/cvi_panel_diffs.h \
    $(wildcard include/config/display/cvitek/i80/sw.h) \
    $(wildcard include/config/display/cvitek/i80/hw.h) \
    $(wildcard include/config/display/cvitek/lvds.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/config.h \
    $(wildcard include/config/sys/boot/ramdisk/high.h) \
    $(wildcard include/config/arch/ls1021a.h) \
    $(wildcard include/config/cpu/pxa27x.h) \
    $(wildcard include/config/cpu/monahans.h) \
    $(wildcard include/config/cpu/pxa25x.h) \
    $(wildcard include/config/fsl/layerscape.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/kconfig.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/config_fallbacks.h \
    $(wildcard include/config/spl.h) \
    $(wildcard include/config/spl/pad/to.h) \
    $(wildcard include/config/spl/max/size.h) \
    $(wildcard include/config/sys/baudrate/table.h) \
    $(wildcard include/config/cmd/kgdb.h) \
    $(wildcard include/config/spl/dm/i2c.h) \
    $(wildcard include/config/sys/i2c/legacy.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/errno.h \
    $(wildcard include/config/errno/str.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/errno.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/time.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/typecheck.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/types.h \
    $(wildcard include/config/uid16.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/posix_types.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/stddef.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/posix_types.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/types.h \
    $(wildcard include/config/arm64.h) \
    $(wildcard include/config/phys/64bit.h) \
    $(wildcard include/config/dma/addr/t/64bit.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/asm-generic/int-ll64.h \
  /media/cvitek/yijun.liu01/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/lib/gcc/aarch64-linux-gnu/6.3.1/include/stdbool.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/printk.h \
    $(wildcard include/config/loglevel.h) \
    $(wildcard include/config/log.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/log.h \
    $(wildcard include/config/log/max/level.h) \
    $(wildcard include/config/spl/log.h) \
    $(wildcard include/config/spl/log/max/level.h) \
    $(wildcard include/config/panic/hang.h) \
    $(wildcard include/config/log/error/return.h) \
    $(wildcard include/config/logf/file.h) \
    $(wildcard include/config/logf/line.h) \
    $(wildcard include/config/logf/func.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/stdio.h \
    $(wildcard include/config/tpl/serial/support.h) \
    $(wildcard include/config/spl/serial/support.h) \
  /media/cvitek/yijun.liu01/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/lib/gcc/aarch64-linux-gnu/6.3.1/include/stdarg.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/stack/validation.h) \
    $(wildcard include/config/kasan.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/compiler_types.h \
    $(wildcard include/config/have/arch/compiler/h.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/spl/optimize/inlining.h) \
    $(wildcard include/config/cc/has/asm/inline.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/compiler_attributes.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/compiler-gcc.h \
    $(wildcard include/config/retpoline.h) \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linker_lists.h \
    $(wildcard include/config/linker/list/align.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/dm/uclass-id.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/bitops.h \
    $(wildcard include/config/sandbox.h) \
    $(wildcard include/config/sandbox/bits/per/long.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/asm-generic/bitsperlong.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/kernel.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/bitops.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/asm-generic/bitops/__ffs.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/asm-generic/bitops/__fls.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/asm-generic/bitops/fls.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/asm-generic/bitops/fls64.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/proc-armv/system.h \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/list.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/poison.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/string.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/string.h \
    $(wildcard include/config/spl/use/arch/memcpy.h) \
    $(wildcard include/config/spl/use/arch/memset.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/linux_string.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/u-boot.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/asm-generic/u-boot.h \
    $(wildcard include/config/arm.h) \
    $(wildcard include/config/mpc8xx.h) \
    $(wildcard include/config/e500.h) \
    $(wildcard include/config/mpc86xx.h) \
    $(wildcard include/config/m68k.h) \
    $(wildcard include/config/mpc83xx.h) \
    $(wildcard include/config/cpm2.h) \
    $(wildcard include/config/extra/clock.h) \
    $(wildcard include/config/nr/dram/banks.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/u-boot-arm.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/display_options.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/vsprintf.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cpu_func.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/malloc.h \
    $(wildcard include/config/spl/sys/malloc/simple.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/cache.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/system.h \
    $(wildcard include/config/armv8/psci.h) \
    $(wildcard include/config/armv7/lpae.h) \
    $(wildcard include/config/cpu/v7a.h) \
    $(wildcard include/config/sys/arm/cache/writethrough.h) \
    $(wildcard include/config/sys/arm/cache/writealloc.h) \
    $(wildcard include/config/sys/arm/cache/writeback.h) \
    $(wildcard include/config/armv7/psci.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/barriers.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/global_data.h \
    $(wildcard include/config/fsl/esdhc.h) \
    $(wildcard include/config/fsl/esdhc/imx.h) \
    $(wildcard include/config/u/qe.h) \
    $(wildcard include/config/at91family.h) \
    $(wildcard include/config/resv/ram.h) \
    $(wildcard include/config/arch/omap2plus.h) \
    $(wildcard include/config/fsl/lsch3.h) \
    $(wildcard include/config/sys/fsl/has/dp/ddr.h) \
    $(wildcard include/config/arch/imx8.h) \
    $(wildcard include/config/arch/imx8ulp.h) \
    $(wildcard include/config/lto.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/asm-generic/global_data.h \
    $(wildcard include/config/pci.h) \
    $(wildcard include/config/lcd.h) \
    $(wildcard include/config/video.h) \
    $(wildcard include/config/dm/video.h) \
    $(wildcard include/config/post.h) \
    $(wildcard include/config/board/types.h) \
    $(wildcard include/config/spl/pre/console/buffer.h) \
    $(wildcard include/config/dm.h) \
    $(wildcard include/config/spl/of/platdata/driver/rt.h) \
    $(wildcard include/config/spl/of/platdata/rt.h) \
    $(wildcard include/config/timer.h) \
    $(wildcard include/config/spl/of/live.h) \
    $(wildcard include/config/spl/multi/dtb/fit.h) \
    $(wildcard include/config/trace.h) \
    $(wildcard include/config/spl/sys/malloc/f/len.h) \
    $(wildcard include/config/pci/bootdelay.h) \
    $(wildcard include/config/console/record.h) \
    $(wildcard include/config/bootstage.h) \
    $(wildcard include/config/spl/bloblist.h) \
    $(wildcard include/config/spl/handoff.h) \
    $(wildcard include/config/translation/offset.h) \
    $(wildcard include/config/spl/wdt.h) \
    $(wildcard include/config/generate/acpi/table.h) \
    $(wildcard include/config/spl/generate/smbios/table.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/fdtdec.h \
    $(wildcard include/config/of/prior/stage.h) \
    $(wildcard include/config/of/board.h) \
    $(wildcard include/config/of/separate.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/libfdt.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/libfdt_env.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/byteorder.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/byteorder/little_endian.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/byteorder/swab.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/byteorder/generic.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/../../scripts/dtc/libfdt/libfdt.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/../../scripts/dtc/libfdt/libfdt_env.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/../../scripts/dtc/libfdt/fdt.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/pci.h \
    $(wildcard include/config/sys/pci/64bit.h) \
    $(wildcard include/config/dm/pci/compat.h) \
    $(wildcard include/config/mpc85xx.h) \
    $(wildcard include/config/pci/sriov.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/pci_ids.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/dm/pci.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/membuff.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/build_bug.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/asm-offsets.h \
  spl/include/generated/generic-asm-offsets.h \

spl/arch/arm/lib/cache.o: $(deps_spl/arch/arm/lib/cache.o)

$(deps_spl/arch/arm/lib/cache.o):
