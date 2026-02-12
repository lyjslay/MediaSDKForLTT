include $(SRCTREE)/build/base.mk

.PHONY : all clean

all:
ifeq ($(CONFIG_WIFI_HI3881),y)
	@if [ -d hisi_3881 ];then cd hisi_3881; make;fi
endif
ifeq ($(CONFIG_WIFI_AP6201BM),y)
	@if [ -d sdio_ap6201bm ];then cd sdio_ap6201bm; make;fi
endif
ifeq ($(CONFIG_WIFI_RTL8189),y)
	@if [ -d sdio_rtl8189fs ];then cd sdio_rtl8189fs; make;fi
endif
ifeq ($(CONFIG_WIFI_RTL8188),y)
	@if [ -d usb_rtl8188f ];then cd usb_rtl8188f; make;fi
endif
ifeq ($(CONFIG_WIFI_AIC8800),y)
	@if [ -d aic8800 ];then cd aic8800; make;fi
endif

clean:
	@if [ -d hisi_3881 ];then cd hisi_3881;  make clean;fi
	@if [ -d sdio_ap6201bm ];then cd sdio_ap6201bm;  make clean;fi
	@if [ -d sdio_rtl8189fs ];then cd sdio_rtl8189fs;  make clean;fi
	@if [ -d usb_rtl8188f ];then cd usb_rtl8188f;  make clean;fi