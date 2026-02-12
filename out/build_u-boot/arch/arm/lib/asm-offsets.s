	.arch armv8-a
	.file	"asm-offsets.c"
// GNU C11 (Linaro GCC 6.3-2017.05) version 6.3.1 20170404 (aarch64-linux-gnu)
//	compiled by GNU C version 4.8.4, GMP version 6.1.0, MPFR version 3.1.4, MPC version 1.0.3, isl version none
// GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
// options passed:  -nostdinc -I include
// -I /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include
// -I /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvitek
// -I /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include
// -I /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/.
// -I .
// -I /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvitek
// -imultiarch aarch64-linux-gnu
// -iprefix /media/cvitek/yijun.liu01/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/../lib/gcc/aarch64-linux-gnu/6.3.1/
// -isysroot /media/cvitek/yijun.liu01/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/../aarch64-linux-gnu/libc
// -D __KERNEL__ -D __UBOOT__ -D CONFIG_SKIP_RAMDISK=y
// -D CONFIG_USE_DEFAULT_ENV=y -D CVICHIP=cv1811ha
// -D CVIBOARD=wevb_0007a_spinor -D CV1811HA_WEVB_0007A_SPINOR -D __ARM__
// -D __LINUX_ARM_ARCH__=8 -D DO_DEPS_ONLY -D KBUILD_BASENAME="asm_offsets"
// -D KBUILD_MODNAME="asm_offsets"
// -isystem /media/cvitek/yijun.liu01/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/../lib/gcc/aarch64-linux-gnu/6.3.1/include
// -include /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/kconfig.h
// -MD arch/arm/lib/.asm-offsets.s.d
// /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/lib/asm-offsets.c
// -mstrict-align -march=armv8-a -mlittle-endian -mabi=lp64
// -auxbase-strip arch/arm/lib/asm-offsets.s -g -Os -Wall
// -Wstrict-prototypes -Wno-format-security -Werror -Wno-pointer-sign
// -Wno-array-bounds -Wno-maybe-uninitialized -Wno-format-nonliteral
// -Wno-unused-but-set-variable -Werror=date-time -std=gnu11 -fno-builtin
// -ffreestanding -fshort-wchar -fno-strict-aliasing -fno-stack-protector
// -fno-delete-null-pointer-checks -fstack-usage -fno-pic
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
.LFB100:
	.file 1 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/lib/asm-offsets.c"
	.loc 1 24 0
	.cfi_startproc
	.loc 1 202 0
#APP
// 202 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/lib/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_RES_X0_OFFS 0 offsetof(struct arm_smccc_res, a0)"	//
// 0 "" 2
	.loc 1 203 0
// 203 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/lib/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_RES_X2_OFFS 16 offsetof(struct arm_smccc_res, a2)"	//
// 0 "" 2
	.loc 1 204 0
// 204 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/lib/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_QUIRK_ID_OFFS 0 offsetof(struct arm_smccc_quirk, id)"	//
// 0 "" 2
	.loc 1 205 0
// 205 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/lib/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_QUIRK_STATE_OFFS 8 offsetof(struct arm_smccc_quirk, state)"	//
// 0 "" 2
	.loc 1 209 0
#NO_APP
	mov	w0, 0	//,
	ret
	.cfi_endproc
.LFE100:
	.size	main, .-main
	.text
.Letext0:
	.file 2 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/errno.h"
	.file 3 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/string.h"
	.file 4 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include/asm/u-boot-arm.h"
	.file 5 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/env_internal.h"
	.file 6 "/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/types.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.4byte	0x158
	.2byte	0x4
	.4byte	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.4byte	.LASF22
	.byte	0xc
	.4byte	.LASF23
	.4byte	.LASF24
	.4byte	.Ldebug_ranges0+0
	.8byte	0
	.4byte	.Ldebug_line0
	.uleb128 0x2
	.4byte	.LASF11
	.byte	0x2
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
	.4byte	.LASF25
	.byte	0x2
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
	.uleb128 0x7
	.byte	0x1
	.byte	0x8
	.4byte	.LASF8
	.uleb128 0x6
	.4byte	0x9f
	.uleb128 0x7
	.byte	0x2
	.byte	0x5
	.4byte	.LASF9
	.uleb128 0x7
	.byte	0x8
	.byte	0x7
	.4byte	.LASF10
	.uleb128 0xa
	.4byte	.LASF26
	.byte	0x6
	.byte	0x5b
	.4byte	0x6f
	.uleb128 0x2
	.4byte	.LASF12
	.byte	0x3
	.byte	0xb
	.4byte	0x84
	.uleb128 0x2
	.4byte	.LASF13
	.byte	0x4
	.byte	0x12
	.4byte	0xb9
	.uleb128 0x2
	.4byte	.LASF14
	.byte	0x4
	.byte	0x13
	.4byte	0xb9
	.uleb128 0x2
	.4byte	.LASF15
	.byte	0x4
	.byte	0x14
	.4byte	0xb9
	.uleb128 0x2
	.4byte	.LASF16
	.byte	0x4
	.byte	0x15
	.4byte	0xb9
	.uleb128 0x2
	.4byte	.LASF17
	.byte	0x4
	.byte	0x16
	.4byte	0xb9
	.uleb128 0x2
	.4byte	.LASF18
	.byte	0x4
	.byte	0x17
	.4byte	0xb9
	.uleb128 0x2
	.4byte	.LASF19
	.byte	0x4
	.byte	0x18
	.4byte	0xb9
	.uleb128 0x7
	.byte	0x10
	.byte	0x4
	.4byte	.LASF20
	.uleb128 0x4
	.4byte	0xa6
	.4byte	0x12e
	.uleb128 0xb
	.byte	0
	.uleb128 0x6
	.4byte	0x123
	.uleb128 0x2
	.4byte	.LASF21
	.byte	0x5
	.byte	0x75
	.4byte	0x12e
	.uleb128 0xc
	.4byte	.LASF27
	.byte	0x1
	.byte	0x17
	.4byte	0x34
	.8byte	.LFB100
	.8byte	.LFE100-.LFB100
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
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0xc
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
	.8byte	.LFB100
	.8byte	.LFE100-.LFB100
	.8byte	0
	.8byte	0
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.8byte	.LFB100
	.8byte	.LFE100
	.8byte	0
	.8byte	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF16:
	.string	"_datarelrolocal_start_ofs"
.LASF5:
	.string	"unsigned int"
.LASF27:
	.string	"main"
.LASF24:
	.string	"/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/out/build_u-boot"
.LASF7:
	.string	"signed char"
.LASF26:
	.string	"ulong"
.LASF2:
	.string	"long unsigned int"
.LASF13:
	.string	"IRQ_STACK_START"
.LASF10:
	.string	"long long unsigned int"
.LASF17:
	.string	"_datarellocal_start_ofs"
.LASF12:
	.string	"___strtok"
.LASF8:
	.string	"unsigned char"
.LASF1:
	.string	"char"
.LASF23:
	.string	"/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/lib/asm-offsets.c"
.LASF22:
	.ascii	"GNU C11 6.3.1 20170404 -mstrict-align -march=armv8-"
	.string	"a -mlittle-endian -mabi=lp64 -g -Os -std=gnu11 -fno-builtin -ffreestanding -fshort-wchar -fno-strict-aliasing -fno-stack-protector -fno-delete-null-pointer-checks -fstack-usage -fno-pic -ffunction-sections -fdata-sections -ffixed-r9 -fno-common -ffixed-x18"
.LASF25:
	.string	"error_message"
.LASF6:
	.string	"long long int"
.LASF14:
	.string	"FIQ_STACK_START"
.LASF3:
	.string	"short unsigned int"
.LASF19:
	.string	"IRQ_STACK_START_IN"
.LASF4:
	.string	"long int"
.LASF11:
	.string	"errno"
.LASF20:
	.string	"long double"
.LASF9:
	.string	"short int"
.LASF18:
	.string	"_datarelro_start_ofs"
.LASF15:
	.string	"_datarel_start_ofs"
.LASF0:
	.string	"sizetype"
.LASF21:
	.string	"default_environment"
	.ident	"GCC: (Linaro GCC 6.3-2017.05) 6.3.1 20170404"
	.section	.note.GNU-stack,"",@progbits
