# CVI_PLAYER_SERVICE_TEST

Run unit test for cvi playe service

## Prerequisites

* googletest

## How to build

```bash
$ make && make install
```

## How to test

1. copy test video to install/bin, and rename to test.mov

2. execute binary in install/bin on device

```bash
$ ./cvi_player_services_test
```

## How to edit

### Edit test input file name:

Edit unit_test.cpp test_input variable.

### Edit stress loop count:

Edit unit_test.cpp StressTest count.
