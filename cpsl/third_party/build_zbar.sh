#!/bin/bash
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

echo $CROSS_COMPILE_3RD
echo $SCRIPT_DIR
CCPREFIX=$CROSS_COMPILE_3RD

if [ "$RISCV" = "y" ]; then
ARCH=riscv64
else
ARCH=arm
fi

pushd $BUILD_DIR

./configure --host=${ARCH} \
			CC=${CCPREFIX}gcc \
			CXX=${CCPREFIX}g++ \
			AR=${CCPREFIX}ar \
			NM=nm CROSS_COMPILE=${CCPREFIX} \
			--program-prefix=${INSTALL_DIR} \
			--prefix=${INSTALL_DIR} \
			--enable-shared \
			--without-imagemagick \
			--without-python \
			--without-gtk \
			--without-qt \
			--without-xshm  \
			--disable-video

make -j4
make install
cp $INSTALL_DIR/* $SCRIPT_DIR/zbar-0.10/ -rf
popd
