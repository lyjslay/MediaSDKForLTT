#ifndef _ONVIF_SERVER_INTERFACE_H_
#define _ONVIF_SERVER_INTERFACE_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <stdio.h>
#include "onvif_device_interface.h"


#define ALLOC_STRUCT(val, type) {\
       val = (type *) soap_malloc(soap, sizeof(type)); \
    if(val == NULL) \
    {\
        printf("malloc err\r\n");\
        return SOAP_FAULT;\
    }\
    memset(val, 0, sizeof(type)); \
}

#define ALLOC_STRUCT_NUM(val, type, num) {\
       val = (type *) soap_malloc(soap, sizeof(type)*num); \
    if(val == NULL) \
    {\
        printf("malloc err\r\n");\
        return SOAP_FAULT;\
    }\
    memset(val, 0, sizeof(type)*num); \
}


#define CHECK_USERNAME_TOKEN { \
        int ret = onvif_check_security(soap); \
        if(ret != SOAP_OK) return ret; \
    }

#define WSSE_SECURITY_DELETE { \
        int ret = onvif_clear_wsse_head(soap); \
        if(ret != SOAP_OK) { return ret; }\
    }
    
// printf("CHECK_USERNAME_TOKEN: %d\n", ret);




#define ONVIF_FRAME_WIDTH       (1920)
#define ONVIF_FRAME_HEIGHT      (1080)
#define ONVIF_SESSION_TIME_OUT  (6000) //6S
#define ONVIF_IP_ADDR_LENGTH    (128)
 
//port
#define ONVIF_UDP_PORT          (3702)
#define ONVIF_TCP_PORT          (8081)
#define ONVIF_MULTICAST_PORT    (8082)
#define ONVIF_SNAP_PORT         (8083)
//ip
#define ONVIF_UDP_IP "239.255.255.250"
#define ONVIF_TCP_IP "192.168.1.168"
//vido size
#define ONVIF_VIDEO_WIDTH       (1920)
#define ONVIF_VIDEO_HEIGHT      (1080) 

// #define CMD_LENGTH      128
// #define INFO_LENGTH     512
#define ONVIF_MAX_TOKEN_LEN     (64) 
#define	ETHERNET_ADAPT  "eth0" 
#define MAX_SCOPE               (10)

#define USER_NAME_LENGTH        (24)
#define USER_PSWD_LENGTH        (24)





#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif 