#!/bin/bash

SYSTEM_DIR=$1
rm -rf $SYSTEM_DIR/lib/libsns_gc*
rm -rf $SYSTEM_DIR/lib/libsns_imx*
rm -rf $SYSTEM_DIR/lib/libsns_sc*
rm -rf $SYSTEM_DIR/lib/libcipher.so

rm -rf $SYSTEM_DIR/ko/m2m-deinterlace.ko
rm -rf $SYSTEM_DIR/ko/efivarfs.ko

rm -rf $SYSTEM_DIR/etc/init.d/S02klogd
rm -rf $SYSTEM_DIR/etc/init.d/S20*
rm -rf $SYSTEM_DIR/etc/init.d/S40*
# rm -rf $SYSTEM_DIR/etc/localtime

rm -rf $SYSTEM_DIR/ko/3rd/aic8800_btlpm.ko
rm -rf $SYSTEM_DIR/ko/3rd/cvi_wiegand_gpio.ko
rm -rf $SYSTEM_DIR/ko/3rd/ts_gslX680.ko
rm -rf $SYSTEM_DIR/ko/cfb*
rm -rf $SYSTEM_DIR/ko/usb*

du -sh $SYSTEM_DIR/* |sort -rh
