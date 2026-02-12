#!/bin/sh
#
# Start to insert fb related modules
#
insmod /mnt/system/ko/cfbcopyarea.ko
insmod /mnt/system/ko/cfbfillrect.ko
insmod /mnt/system/ko/cfbimgblt.ko
insmod /mnt/system/ko/cvi_fb.ko

exit $?