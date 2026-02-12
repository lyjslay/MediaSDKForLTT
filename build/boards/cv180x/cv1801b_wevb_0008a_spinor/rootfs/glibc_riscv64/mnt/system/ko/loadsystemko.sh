#!/bin/sh
${CVI_SHOPTS}
#
# Start to insert kernel modules
#
insmod /mnt/system/ko/cv180x_sys.ko
insmod /mnt/system/ko/cv180x_base.ko
insmod /mnt/system/ko/cv180x_clock_cooling.ko
insmod /mnt/system/ko/cvi_ipcm.ko
insmod /mnt/system/ko/cv180x_rtc.ko

#

echo 3 > /proc/sys/vm/drop_caches
dmesg -n 4

#usb hub control
#/etc/uhubon.sh host

exit $?
