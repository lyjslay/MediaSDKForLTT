# CVI_RECORDER

CVI_RECORDER accept encoded audio and video data, muxing them as mp4, ts or mov file.

## Prerequisites

* ffmpeg

## How to build

```bash
$ mkdir build && cd build
$ cmake ..
$ make -j4
$ make install
# cvi_recorder shared library and header file will be install in the install directory
```

## How to test

1. build
```bash
$ cd example
$ make
```

2. execute
   
```bash
$ ./recorder_test
```
