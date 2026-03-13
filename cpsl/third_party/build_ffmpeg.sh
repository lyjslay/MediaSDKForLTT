#!/bin/bash
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

CONFIG_OTHER=""
echo $CROSS_COMPILE_3RD
CCPREFIX=$CROSS_COMPILE_3RD
ARCH=riscv64
CONFIG_OTHER="--enable-pic --cpu=rv64imafd"
sed -i "45s/__off_t/off_t/g" ${SCRIPT_DIR}/ffmpeg/libavformat/file.c


pushd $BUILD_DIR
$SCRIPT_DIR/ffmpeg/configure \
    --enable-cross-compile \
    --cross-prefix=${CCPREFIX} \
    --arch=${ARCH} \
    --target-os=linux \
    --disable-avdevice \
    --disable-avfilter \
    --disable-swscale \
    --enable-swresample \
    --disable-everything \
	--enable-shared \
    --enable-muxer=mp4 \
    --enable-muxer=mov \
    --enable-muxer=mpegts \
    --enable-demuxer=matroska \
    --enable-demuxer=avi \
    --enable-demuxer=flv \
    --enable-demuxer=mpegps \
    --enable-demuxer=mxf \
    --enable-demuxer=asf \
    --enable-demuxer=rm \
    --enable-demuxer=mov \
    --enable-demuxer=mpegts \
    --enable-demuxer=image2 \
    --enable-demuxer=mjpeg \
    --enable-demuxer=pcm_s16le \
    --enable-demuxer=aac \
    --enable-demuxer=mp3 \
    --enable-demuxer=wav \
    --enable-decoder=mjpeg \
    --enable-decoder=hevc \
    --enable-decoder=h264 \
    --enable-decoder=mpeg4 \
    --enable-decoder=mp3 \
    --enable-decoder=wmv3 \
    --enable-decoder=wmav2 \
    --enable-decoder=rv40 \
    --enable-decoder=cook \
    --enable-decoder=pcm_s16le \
    --enable-decoder=aac \
    --enable-protocol=file \
	--enable-muxer=aac \
	--enable-encoder=aac \
    --enable-parser=aac \
    --enable-parser=h264 \
    --enable-parser=hevc \
    $CONFIG_OTHER \
    --prefix=$INSTALL_DIR
## extra flags
# --enable-gpl
# --enable-libx264
# --enable-nonfree
# --enable-libaacplus
# --extra-cflags="-I/my/path/were/i/keep/built/arm/stuff/include"
# --extra-ldflags="-L/my/path/were/i/keep/built/arm/stuff/lib"
# --extra-libs=-ldl
make -j4
make install
cp $INSTALL_DIR/* $SCRIPT_DIR/ffmpeg -rf
popd
