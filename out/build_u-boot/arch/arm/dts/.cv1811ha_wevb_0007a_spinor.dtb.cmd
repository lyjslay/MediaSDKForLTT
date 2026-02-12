cmd_arch/arm/dts/cv1811ha_wevb_0007a_spinor.dtb := mkdir -p arch/arm/dts/ ; (cat /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/cv1811ha_wevb_0007a_spinor.dts; ) > arch/arm/dts/.cv1811ha_wevb_0007a_spinor.dtb.pre.tmp; cc -E -Wp,-MD,arch/arm/dts/.cv1811ha_wevb_0007a_spinor.dtb.d.pre.tmp -nostdinc -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/include -Iinclude -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include -I/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/include -include /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/linux/kconfig.h -D__ASSEMBLY__ -undef -D__DTS__ -D__UBOOT__ -x assembler-with-cpp -o arch/arm/dts/.cv1811ha_wevb_0007a_spinor.dtb.dts.tmp arch/arm/dts/.cv1811ha_wevb_0007a_spinor.dtb.pre.tmp ; ./scripts/dtc/dtc -O dtb -o arch/arm/dts/cv1811ha_wevb_0007a_spinor.dtb -b 0 -i /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/  -Wno-unit_address_vs_reg -Wno-unit_address_format -Wno-avoid_unnecessary_addr_size -Wno-alias_paths -Wno-graph_child_address -Wno-graph_port -Wno-unique_unit_address -Wno-simple_bus_reg -Wno-pci_device_reg -Wno-pci_bridge -Wno-pci_device_bus_num  -a 0x8 -Wno-unit_address_vs_reg -Wno-unit_address_format -Wno-avoid_unnecessary_addr_size -Wno-alias_paths -Wno-graph_child_address -Wno-graph_port -Wno-unique_unit_address -Wno-simple_bus_reg -Wno-pci_device_reg -Wno-pci_bridge -Wno-pci_device_bus_num  -d arch/arm/dts/.cv1811ha_wevb_0007a_spinor.dtb.d.dtc.tmp arch/arm/dts/.cv1811ha_wevb_0007a_spinor.dtb.dts.tmp || (echo "Check /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/out/build_u-boot/arch/arm/dts/.cv1811ha_wevb_0007a_spinor.dtb.pre.tmp for errors" && false) ; sed "s:arch/arm/dts/.cv1811ha_wevb_0007a_spinor.dtb.pre.tmp:/media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/cv1811ha_wevb_0007a_spinor.dts:" arch/arm/dts/.cv1811ha_wevb_0007a_spinor.dtb.d.pre.tmp arch/arm/dts/.cv1811ha_wevb_0007a_spinor.dtb.d.dtc.tmp > arch/arm/dts/.cv1811ha_wevb_0007a_spinor.dtb.d

source_arch/arm/dts/cv1811ha_wevb_0007a_spinor.dtb := /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/cv1811ha_wevb_0007a_spinor.dts

deps_arch/arm/dts/cv1811ha_wevb_0007a_spinor.dtb := \
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
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/cv181x_base_arm.dtsi \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/include/dt-bindings/interrupt-controller/arm-gic.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/include/dt-bindings/interrupt-controller/irq.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/include/dt-bindings/gpio/gpio.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/include/dt-bindings/reset/cv181x-resets.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/include/dt-bindings/clock/cv181x-clock.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/include/dt-bindings/thermal/thermal.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/include/dt-bindings/dma/cv181x-dmamap.h \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/include/cvi_board_memmap.h \
    $(wildcard include/config/sys/text/base.h) \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/cv181x_base.dtsi \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/cv181x_asic_bga.dtsi \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/cv181x_asic_spinor.dtsi \
  /media/cvitek/yijun.liu01/car/mars/litiantai/pnd-ltt/MediaSDK/platform/u-boot-2021.10/arch/arm/dts/cv181x_default_memmap.dtsi \

arch/arm/dts/cv1811ha_wevb_0007a_spinor.dtb: $(deps_arch/arm/dts/cv1811ha_wevb_0007a_spinor.dtb)

$(deps_arch/arm/dts/cv1811ha_wevb_0007a_spinor.dtb):
