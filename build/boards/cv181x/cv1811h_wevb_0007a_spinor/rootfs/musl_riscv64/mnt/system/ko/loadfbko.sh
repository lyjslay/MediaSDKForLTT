#!/bin/sh
#
# Start to insert fb related modules
#

insmod /mnt/system/ko/cvi_fb.ko option=$1

exit $?
