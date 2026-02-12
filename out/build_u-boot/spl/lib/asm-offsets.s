	.arch armv8-a
	.file	"asm-offsets.c"
// GNU C11 (Linaro GCC 6.3-2017.05) version 6.3.1 20170404 (aarch64-linux-gnu)
//	compiled by GNU C version 4.8.4, GMP version 6.1.0, MPFR version 3.1.4, MPC version 1.0.3, isl version none
// GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
// options passed:  -nostdinc -I spl/include -I include
// -I /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include
// -I /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvitek
// -I /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include
// -I /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/.
// -I spl/.
// -I /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvitek
// -imultiarch aarch64-linux-gnu
// -iprefix /media/cvitek/yijun.liu01/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/../lib/gcc/aarch64-linux-gnu/6.3.1/
// -isysroot /media/cvitek/yijun.liu01/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/../aarch64-linux-gnu/libc
// -D __KERNEL__ -D __UBOOT__ -D CONFIG_SKIP_RAMDISK=y
// -D CONFIG_USE_DEFAULT_ENV=y -D CVICHIP=cv1811ha
// -D CVIBOARD=wevb_0007a_spinor -D CV1811HA_WEVB_0007A_SPINOR
// -D CONFIG_SPL_BUILD -D __ARM__ -D __LINUX_ARM_ARCH__=8 -D DO_DEPS_ONLY
// -D KBUILD_BASENAME="asm_offsets" -D KBUILD_MODNAME="asm_offsets"
// -isystem /media/cvitek/yijun.liu01/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/../lib/gcc/aarch64-linux-gnu/6.3.1/include
// -include /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/kconfig.h
// -MD spl/./lib/.asm-offsets.s.d
// /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/asm-offsets.c
// -mstrict-align -march=armv8-a -mlittle-endian -mabi=lp64
// -auxbase-strip spl/./lib/asm-offsets.s -g -Os -Wall -Wstrict-prototypes
// -Wno-format-security -Werror -Wno-pointer-sign -Wno-array-bounds
// -Wno-maybe-uninitialized -Wno-format-nonliteral
// -Wno-unused-but-set-variable -Werror=date-time -std=gnu11 -fno-builtin
// -ffreestanding -fshort-wchar -fno-strict-aliasing -fno-PIE
// -fno-delete-null-pointer-checks -fstack-usage -fno-stack-protector
// -ffunction-sections -fdata-sections -ffixed-r9 -fno-common -ffixed-x18
// -fverbose-asm
// options enabled:  -faggressive-loop-optimizations -falign-functions
// -falign-jumps -falign-labels -falign-loops -fauto-inc-dec
// -fbranch-count-reg -fcaller-saves -fchkp-check-incomplete-type
// -fchkp-check-read -fchkp-check-write -fchkp-instrument-calls
// -fchkp-narrow-bounds -fchkp-optimize -fchkp-store-bounds
// -fchkp-use-static-bounds -fchkp-use-static-const-bounds
// -fchkp-use-wrappers -fcombine-stack-adjustments -fcompare-elim
// -fcprop-registers -fcrossjumping -fcse-follow-jumps -fdata-sections
// -fdefer-pop -fdevirtualize -fdevirtualize-speculatively -fdwarf2-cfi-asm
// -fearly-inlining -feliminate-unused-debug-types
// -fexpensive-optimizations -fforward-propagate -ffunction-cse
// -ffunction-sections -fgcse -fgcse-lm -fgnu-runtime -fgnu-unique
// -fguess-branch-probability -fhoist-adjacent-loads -fident
// -fif-conversion -fif-conversion2 -findirect-inlining -finline
// -finline-atomics -finline-functions -finline-functions-called-once
// -finline-small-functions -fipa-cp -fipa-cp-alignment -fipa-icf
// -fipa-icf-functions -fipa-icf-variables -fipa-profile -fipa-pure-const
// -fipa-ra -fipa-reference -fipa-sra -fira-hoist-pressure
// -fira-share-save-slots -fira-share-spill-slots
// -fisolate-erroneous-paths-dereference -fivopts -fkeep-static-consts
// -fleading-underscore -flifetime-dse -flra-remat -flto-odr-type-merging
// -fmath-errno -fmerge-constants -fmerge-debug-strings
// -fmove-loop-invariants -fomit-frame-pointer -foptimize-sibling-calls
// -fpartial-inlining -fpeephole -fpeephole2 -fplt -fprefetch-loop-arrays
// -free -freg-struct-return -freorder-blocks -freorder-functions
// -frerun-cse-after-loop -fsched-critical-path-heuristic
// -fsched-dep-count-heuristic -fsched-group-heuristic -fsched-interblock
// -fsched-last-insn-heuristic -fsched-pressure -fsched-rank-heuristic
// -fsched-spec -fsched-spec-insn-heuristic -fsched-stalled-insns-dep
// -fschedule-fusion -fschedule-insns2 -fsection-anchors
// -fsemantic-interposition -fshow-column -fshrink-wrap -fsigned-zeros
// -fsplit-ivs-in-unroller -fsplit-wide-types -fssa-backprop -fssa-phiopt
// -fstdarg-opt -fstrict-overflow -fstrict-volatile-bitfields
// -fsync-libcalls -fthread-jumps -ftoplevel-reorder -ftrapping-math
// -ftree-bit-ccp -ftree-builtin-call-dce -ftree-ccp -ftree-ch
// -ftree-coalesce-vars -ftree-copy-prop -ftree-cselim -ftree-dce
// -ftree-dominator-opts -ftree-dse -ftree-forwprop -ftree-fre
// -ftree-loop-if-convert -ftree-loop-im -ftree-loop-ivcanon
// -ftree-loop-optimize -ftree-parallelize-loops= -ftree-phiprop -ftree-pre
// -ftree-pta -ftree-reassoc -ftree-scev-cprop -ftree-sink -ftree-slsr
// -ftree-sra -ftree-switch-conversion -ftree-tail-merge -ftree-ter
// -ftree-vrp -funit-at-a-time -fvar-tracking -fvar-tracking-assignments
// -fverbose-asm -fzero-initialized-in-bss -mfix-cortex-a53-835769
// -mfix-cortex-a53-843419 -mglibc -mlittle-endian
// -momit-leaf-frame-pointer -mpc-relative-literal-loads -mstrict-align

	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.section	.text.startup.main,"ax",@progbits
	.align	2
	.global	main
	.type	main, %function
main:
.LFB134:
	.file 1 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/asm-offsets.c"
	.loc 1 21 0
	.cfi_startproc
	.loc 1 23 0
#APP
// 23 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/asm-offsets.c" 1
	
.ascii "->GENERATED_GBL_DATA_SIZE 400 (sizeof(struct global_data) + 15) & ~15"	//
// 0 "" 2
	.loc 1 26 0
// 26 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/asm-offsets.c" 1
	
.ascii "->GENERATED_BD_INFO_SIZE 144 (sizeof(struct bd_info) + 15) & ~15"	//
// 0 "" 2
	.loc 1 29 0
// 29 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/asm-offsets.c" 1
	
.ascii "->GD_SIZE 400 sizeof(struct global_data)"	//
// 0 "" 2
	.loc 1 31 0
// 31 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/asm-offsets.c" 1
	
.ascii "->GD_BD 0 offsetof(struct global_data, bd)"	//
// 0 "" 2
	.loc 1 33 0
// 33 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/asm-offsets.c" 1
	
.ascii "->GD_MALLOC_BASE 288 offsetof(struct global_data, malloc_base)"	//
// 0 "" 2
	.loc 1 36 0
// 36 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/asm-offsets.c" 1
	
.ascii "->GD_RELOCADDR 120 offsetof(struct global_data, relocaddr)"	//
// 0 "" 2
	.loc 1 38 0
// 38 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/asm-offsets.c" 1
	
.ascii "->GD_RELOC_OFF 160 offsetof(struct global_data, reloc_off)"	//
// 0 "" 2
	.loc 1 40 0
// 40 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/asm-offsets.c" 1
	
.ascii "->GD_START_ADDR_SP 152 offsetof(struct global_data, start_addr_sp)"	//
// 0 "" 2
	.loc 1 42 0
// 42 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/asm-offsets.c" 1
	
.ascii "->GD_NEW_GD 168 offsetof(struct global_data, new_gd)"	//
// 0 "" 2
	.loc 1 44 0
// 44 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/asm-offsets.c" 1
	
.ascii "->GD_ENV_ADDR 72 offsetof(struct global_data, env_addr)"	//
// 0 "" 2
	.loc 1 47 0
#NO_APP
	mov	w0, 0	//,
	ret
	.cfi_endproc
.LFE134:
	.size	main, .-main
	.text
.Letext0:
	.file 2 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/asm-generic/int-ll64.h"
	.file 3 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/types.h"
	.file 4 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/errno.h"
	.file 5 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/string.h"
	.file 6 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/u-boot-arm.h"
	.file 7 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/env_internal.h"
	.file 8 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/libfdt_env.h"
	.file 9 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/../../scripts/dtc/libfdt/fdt.h"
	.file 10 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/libfdt.h"
	.file 11 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/fdtdec.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.4byte	0x24c
	.2byte	0x4
	.4byte	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.4byte	.LASF41
	.byte	0xc
	.4byte	.LASF42
	.4byte	.LASF43
	.4byte	.Ldebug_ranges0+0
	.8byte	0
	.4byte	.Ldebug_line0
	.uleb128 0x2
	.4byte	.LASF15
	.byte	0x4
	.byte	0x11
	.4byte	0x34
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x4
	.4byte	0x5e
	.4byte	0x4b
	.uleb128 0x5
	.4byte	0x50
	.byte	0
	.byte	0
	.uleb128 0x6
	.4byte	0x3b
	.uleb128 0x7
	.byte	0x8
	.byte	0x7
	.4byte	.LASF0
	.uleb128 0x7
	.byte	0x1
	.byte	0x8
	.4byte	.LASF1
	.uleb128 0x6
	.4byte	0x57
	.uleb128 0x8
	.4byte	.LASF44
	.byte	0x4
	.byte	0x1f
	.4byte	0x4b
	.string	""
	.uleb128 0x7
	.byte	0x8
	.byte	0x7
	.4byte	.LASF2
	.uleb128 0x7
	.byte	0x2
	.byte	0x7
	.4byte	.LASF3
	.uleb128 0x7
	.byte	0x8
	.byte	0x5
	.4byte	.LASF4
	.uleb128 0x9
	.byte	0x8
	.4byte	0x57
	.uleb128 0x7
	.byte	0x4
	.byte	0x7
	.4byte	.LASF5
	.uleb128 0x7
	.byte	0x8
	.byte	0x5
	.4byte	.LASF6
	.uleb128 0x7
	.byte	0x1
	.byte	0x6
	.4byte	.LASF7
	.uleb128 0xa
	.4byte	.LASF10
	.byte	0x2
	.byte	0x13
	.4byte	0xaa
	.uleb128 0x7
	.byte	0x1
	.byte	0x8
	.4byte	.LASF8
	.uleb128 0x6
	.4byte	0xaa
	.uleb128 0x7
	.byte	0x2
	.byte	0x5
	.4byte	.LASF9
	.uleb128 0xa
	.4byte	.LASF11
	.byte	0x2
	.byte	0x19
	.4byte	0x8a
	.uleb128 0x7
	.byte	0x8
	.byte	0x7
	.4byte	.LASF12
	.uleb128 0xb
	.string	"u8"
	.byte	0x2
	.byte	0x24
	.4byte	0x9f
	.uleb128 0xa
	.4byte	.LASF13
	.byte	0x3
	.byte	0x5b
	.4byte	0x6f
	.uleb128 0xa
	.4byte	.LASF14
	.byte	0x3
	.byte	0x90
	.4byte	0xbd
	.uleb128 0x2
	.4byte	.LASF16
	.byte	0x5
	.byte	0xb
	.4byte	0x84
	.uleb128 0x2
	.4byte	.LASF17
	.byte	0x6
	.byte	0x12
	.4byte	0xd9
	.uleb128 0x2
	.4byte	.LASF18
	.byte	0x6
	.byte	0x13
	.4byte	0xd9
	.uleb128 0x2
	.4byte	.LASF19
	.byte	0x6
	.byte	0x14
	.4byte	0xd9
	.uleb128 0x2
	.4byte	.LASF20
	.byte	0x6
	.byte	0x15
	.4byte	0xd9
	.uleb128 0x2
	.4byte	.LASF21
	.byte	0x6
	.byte	0x16
	.4byte	0xd9
	.uleb128 0x2
	.4byte	.LASF22
	.byte	0x6
	.byte	0x17
	.4byte	0xd9
	.uleb128 0x2
	.4byte	.LASF23
	.byte	0x6
	.byte	0x18
	.4byte	0xd9
	.uleb128 0x7
	.byte	0x10
	.byte	0x4
	.4byte	.LASF24
	.uleb128 0x4
	.4byte	0xb1
	.4byte	0x159
	.uleb128 0xc
	.byte	0
	.uleb128 0x6
	.4byte	0x14e
	.uleb128 0x2
	.4byte	.LASF25
	.byte	0x7
	.byte	0x75
	.4byte	0x159
	.uleb128 0xa
	.4byte	.LASF26
	.byte	0x8
	.byte	0x12
	.4byte	0xe4
	.uleb128 0xd
	.4byte	.LASF45
	.byte	0x28
	.byte	0x9
	.byte	0xc
	.4byte	0x1f9
	.uleb128 0xe
	.4byte	.LASF27
	.byte	0x9
	.byte	0xd
	.4byte	0x169
	.byte	0
	.uleb128 0xe
	.4byte	.LASF28
	.byte	0x9
	.byte	0xe
	.4byte	0x169
	.byte	0x4
	.uleb128 0xe
	.4byte	.LASF29
	.byte	0x9
	.byte	0xf
	.4byte	0x169
	.byte	0x8
	.uleb128 0xe
	.4byte	.LASF30
	.byte	0x9
	.byte	0x10
	.4byte	0x169
	.byte	0xc
	.uleb128 0xe
	.4byte	.LASF31
	.byte	0x9
	.byte	0x11
	.4byte	0x169
	.byte	0x10
	.uleb128 0xe
	.4byte	.LASF32
	.byte	0x9
	.byte	0x12
	.4byte	0x169
	.byte	0x14
	.uleb128 0xe
	.4byte	.LASF33
	.byte	0x9
	.byte	0x13
	.4byte	0x169
	.byte	0x18
	.uleb128 0xe
	.4byte	.LASF34
	.byte	0x9
	.byte	0x16
	.4byte	0x169
	.byte	0x1c
	.uleb128 0xe
	.4byte	.LASF35
	.byte	0x9
	.byte	0x19
	.4byte	0x169
	.byte	0x20
	.uleb128 0xe
	.4byte	.LASF36
	.byte	0x9
	.byte	0x1c
	.4byte	0x169
	.byte	0x24
	.byte	0
	.uleb128 0x2
	.4byte	.LASF37
	.byte	0xa
	.byte	0xb
	.4byte	0x204
	.uleb128 0x9
	.byte	0x8
	.4byte	0x174
	.uleb128 0x7
	.byte	0x1
	.byte	0x2
	.4byte	.LASF38
	.uleb128 0x4
	.4byte	0xcf
	.4byte	0x21c
	.uleb128 0xc
	.byte	0
	.uleb128 0x2
	.4byte	.LASF39
	.byte	0xb
	.byte	0x73
	.4byte	0x211
	.uleb128 0x2
	.4byte	.LASF40
	.byte	0xb
	.byte	0x74
	.4byte	0x211
	.uleb128 0xf
	.4byte	.LASF46
	.byte	0x1
	.byte	0x14
	.4byte	0x34
	.8byte	.LFB134
	.8byte	.LFE134-.LFB134
	.uleb128 0x1
	.byte	0x9c
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.4byte	0x2c
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x8
	.byte	0
	.2byte	0
	.2byte	0
	.8byte	.LFB134
	.8byte	.LFE134-.LFB134
	.8byte	0
	.8byte	0
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.8byte	.LFB134
	.8byte	.LFE134
	.8byte	0
	.8byte	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF44:
	.string	"error_message"
.LASF42:
	.string	"/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/lib/asm-offsets.c"
.LASF45:
	.string	"fdt_header"
.LASF38:
	.string	"_Bool"
.LASF11:
	.string	"__u32"
.LASF40:
	.string	"__dtb_dt_spl_begin"
.LASF43:
	.string	"/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/out/build_u-boot"
.LASF15:
	.string	"errno"
.LASF29:
	.string	"off_dt_struct"
.LASF26:
	.string	"fdt32_t"
.LASF27:
	.string	"magic"
.LASF31:
	.string	"off_mem_rsvmap"
.LASF17:
	.string	"IRQ_STACK_START"
.LASF8:
	.string	"unsigned char"
.LASF36:
	.string	"size_dt_struct"
.LASF2:
	.string	"long unsigned int"
.LASF3:
	.string	"short unsigned int"
.LASF13:
	.string	"ulong"
.LASF18:
	.string	"FIQ_STACK_START"
.LASF14:
	.string	"__be32"
.LASF16:
	.string	"___strtok"
.LASF10:
	.string	"__u8"
.LASF37:
	.string	"working_fdt"
.LASF22:
	.string	"_datarelro_start_ofs"
.LASF32:
	.string	"version"
.LASF41:
	.ascii	"GNU C11 6.3.1 20170404 -mstrict-align -march=armv8-"
	.string	"a -mlittle-endian -mabi=lp64 -g -Os -std=gnu11 -fno-builtin -ffreestanding -fshort-wchar -fno-strict-aliasing -fno-PIE -fno-delete-null-pointer-checks -fstack-usage -fno-stack-protector -ffunction-sections -fdata-sections -ffixed-r9 -fno-common -ffixed-x18"
.LASF5:
	.string	"unsigned int"
.LASF19:
	.string	"_datarel_start_ofs"
.LASF12:
	.string	"long long unsigned int"
.LASF46:
	.string	"main"
.LASF39:
	.string	"__dtb_dt_begin"
.LASF0:
	.string	"sizetype"
.LASF23:
	.string	"IRQ_STACK_START_IN"
.LASF6:
	.string	"long long int"
.LASF25:
	.string	"default_environment"
.LASF1:
	.string	"char"
.LASF9:
	.string	"short int"
.LASF35:
	.string	"size_dt_strings"
.LASF33:
	.string	"last_comp_version"
.LASF30:
	.string	"off_dt_strings"
.LASF21:
	.string	"_datarellocal_start_ofs"
.LASF4:
	.string	"long int"
.LASF24:
	.string	"long double"
.LASF7:
	.string	"signed char"
.LASF20:
	.string	"_datarelrolocal_start_ofs"
.LASF34:
	.string	"boot_cpuid_phys"
.LASF28:
	.string	"totalsize"
	.ident	"GCC: (Linaro GCC 6.3-2017.05) 6.3.1 20170404"
	.section	.note.GNU-stack,"",@progbits
