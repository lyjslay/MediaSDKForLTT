#!/bin/sh

export LD_LIBRARY_PATH="/lib:/lib/3rd:/lib/arm-linux-gnueabihf:/usr/lib:/usr/local/lib:/mnt/system/lib:/mnt/system/usr/lib:/mnt/system/usr/lib/3rd:/mnt/data/lib"
export PATH="/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin:/mnt/system/usr/bin:/mnt/system/usr/sbin:/mnt/data/bin:/mnt/data/sbin"

dmesg -n 1

# extract environment variables
# eval "$(fw_printenv ahdstatus pwmstatus)"
# [ -n "$ahdstatus" ] && export ahdstatus=$ahdstatus
# # [ -n "$pwmstatus" ] && export pwmstatus=$pwmstatus

echo '/mnt/system/core_%e.%p' | tee /proc/sys/kernel/core_pattern
ulimit -c unlimited
dmesg -n 4

# vm for fflush
echo 50 > /proc/sys/vm/dirty_writeback_centisecs
echo 3 > /proc/sys/vm/dirty_background_ratio
echo 30 > /proc/sys/vm/dirty_ratio
echo 10000 > /proc/sys/vm/vfs_cache_pressure

#/mnt/system/bin/cardv_app < /dev/null &
insmod /mnt/system/ko/cvi_fb.ko
insmod /mnt/system/ko/3rd/gt9xx.ko
mount /dev/mmcblk0p1 /mnt/sd

exit $?
