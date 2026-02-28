#!/bin/bash

SOAP_PATH="./gsoap-2.8/" 
echo $SOAP_PATH

/opt/gsoap/bin/soapcpp2 -2 -c -L -d ./autogen -I  ./gsoap-2.8/gsoap:./gsoap-2.8/gsoap/import:./gsoap-2.8/gsoap/custom:./gsoap-2.8/gsoap/extras:./gsoap-2.8/gsoap/plugin -x ./autogen/onvif.h

cp gsoap-2.8/gsoap/dom.c autogen/
cp gsoap-2.8/gsoap/stdsoap2.c autogen/
cp gsoap-2.8/gsoap/stdsoap2.h autogen/
cp gsoap-2.8/gsoap/custom/duration.* autogen/
cp gsoap-2.8/gsoap/plugin/wsaapi.* autogen/
cp gsoap-2.8/gsoap/plugin/threads.*  autogen/
cp gsoap-2.8/gsoap/plugin/mecevp.* autogen/
cp gsoap-2.8/gsoap/plugin/smdevp.*  autogen/
cp gsoap-2.8/gsoap/plugin/wsseapi.h  autogen/
cp gsoap-2.8/gsoap/plugin/wsseapi.c  autogen/
cp gsoap-2.8/gsoap/custom/struct_timeval.* autogen/
