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

du -sh $SYSTEM_DIR/* |sort -rh
