# Onvif_Server

# Getting Started

## Host Env
`Linux ubuntu 4.15.0-117-generic #118~16.04.1-Ubuntu SMP Sat Sep 5 23:35:06 UTC 2020 x86_64 x86_64 x86_64 GNU/Linux`

## Install OpenSSL for Host
```bash
# 1. download and untar
mv openssl-1.1.0i.tar.gz onvif/tools/
cd onvif/tools/
tar xvf openssl-1.1.0i.tar.gz
# 2. compile 
cd openssl-1.1.0i/
./config --prefix=/usr/local/openssl
make -j4
# 3. install
sudo make install
```

## Install OpenSSL for arm
```bash
# 1. compile
cd openssl-1.1.0i/
mkdir install


make clean
make -j4
# 2. install
make install
```

## Install gsoap tool
- dependent
```bash
sudo apt install flex bison openssl libssl-dev
# maybe you should install openssl manually. install dir: /usr/local/openssl
```
- download gsoap open source  
`http://sourceforge.net/projects/gsoap2`
```bash
### 1. unzip src code
mv gsoap_2.8.106.zip onvif/tools/
cd onvif/tools/
unzip gsoap_2.8.106.zip
### 2. build
cd gsoap-2.8/
./configure --with-openssl=/usr/include/openssl --prefix=/opt/gsoap
make -j4
sudo make install
### 3. modify the typemap.dat, Uncomment the line below
vim ./gsoap/typemap.dat
# - # xsd__duration = #import "custom/duration.h" | xsd__duration
# + xsd__duration = #import "custom/duration.h" | xsd__duration
### 4. modify the wsa5.h
vim ./gsoap/import/wsa5.h
# - int SOAP_ENV__Fault
# + int SOAP_ENV__Fault_alex
```

## Auto generate the soap code
```bash
### 1. init the environment
cd ../
mkdir autogen
### 2. auto generate onvif.h
./autogen_onvif.sh
### 3. add a include ("wsse.h") in onvif.h
vim ./autogen/onvif.h
# + #import "wsse.h"
### 4. aotu generate soapServer
./autogen_soap.sh
### 5. copy dependent files manually
# has being do in autogen_soap.sh
### 6. rm unused files (unnecessary)
cd ./autogen/
cp wsdd.nsmap wsdd.nsmap.bk
rm *.nsmap
mv wsdd.nsmap.bk wsdd.nsmap
rm onvif.h
### 7. generate libs
cd onvif/tools/
make
```

## Build onvif server
### Dependent
- minIni   
`this will auto build, when build onvif.`

- sqlite3
```bash
# 1. get sqlite3
download the sqlite3 src (sqlite-autoconf-3330000.tar.gz)
# 2. build
mkdir -p $(TOP_DIR)/cpsl/third_party/sqlite3/
tar xvf sqlite-autoconf-3330000.tar.gz
cd sqlite-autoconf-3330000
./configure --prefix=$(TOP_DIR)/cpsl/third_party/sqlite3/ --host=arm-linux CC=aarch64-linux-gnu-gcc
make -j4
# 3. install
make install
# 4. install to framework/install
cp cpsl/third_party/sqlite3/include/* framework/install/include/
cp cpsl/third_party/sqlite3/lib/lib* framework/install/lib/
```

### Build
```bash
make
```

### Install
```c
make install
```

# sample
see sample code in samples/onvif/
