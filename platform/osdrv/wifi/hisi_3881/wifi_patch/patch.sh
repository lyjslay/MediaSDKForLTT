#!/bin/bash
#set -x
patch -p1 < ./wifi_patch/1_5d47e66.diff
patch -p1 < ./wifi_patch/2_f270dd0.diff
patch -p1 < ./wifi_patch/3_0cde96f.diff
patch -p1 < ./wifi_patch/4_a249357.diff
patch -p1 < ./wifi_patch/5_7229281.diff
patch -p1 < ./wifi_patch/6_fe399e3.diff
patch -p1 < ./wifi_patch/7_7cdc23e.diff
patch -p1 < ./wifi_patch/8_4626d83.diff
patch -p1 < ./wifi_patch/9_98aef84.diff
patch -p1 < ./wifi_patch/10_4646f91.diff
patch -p1 < ./wifi_patch/11_3d84dc8.diff
patch -p1 < ./wifi_patch/12_207f91b.diff
patch -p1 < ./wifi_patch/13_ab7f604.diff
patch -p1 < ./wifi_patch/14_b29ad51.diff
patch -p1 < ./wifi_patch/15_52ac110.diff