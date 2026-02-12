cmd_u-boot.elf := aarch64-linux-gnu-ld.bfd u-boot-elf.o -o u-boot.elf -T u-boot-elf.lds --defsym="_start"=0x83d00000 -Ttext=0x83d00000
