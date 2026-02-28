#include "onvif_server.h"
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "wsdd.nsmap"
#include "soapH.h"
#include "onvif_server_interface.h"
#include "onvif_device_interface.h"
#include "onvif_debug.h"
#include "onvif_event_manager.h"


static SOAP_SOCKET onvif_soap_bind(struct soap *pSoap, const char *pIp, int flag)
{
    SOAP_SOCKET sockFD = SOAP_INVALID_SOCKET;
    if (flag) {
        sockFD = soap_bind(pSoap, ONVIF_UDP_IP, pSoap->port, 10);
        if (soap_valid_socket(sockFD)) {
            printf("%s:%d flag = %d, sockFD = %d, pSoap->master = %d\n", 
                __func__, __LINE__, flag, sockFD, pSoap->master);
        } else {
            printf("%s:%d flag = %d\n", __func__, __LINE__, flag);
            soap_print_fault(pSoap, stderr);
        }
    } else {
        sockFD = soap_bind(pSoap, pIp, pSoap->port, 10);
        if (soap_valid_socket(sockFD)) {
            printf("%s:%d flag = %d, sockFD = %d, pSoap->master = %d\n", 
                __func__, __LINE__, flag, sockFD, pSoap->master);
        } else {
            printf("%s:%d flag = %d\n", __func__, __LINE__, flag);
            soap_print_fault(pSoap, stderr);
        }
    }
    return sockFD;
}

static void *thread_onvif_discover(void *arg) 
{ 
    printf("[%d][%s][%s] start \n", __LINE__, __TIME__, __func__);

    struct soap UDPserverSoap;
    struct ip_mreq mcast;

    soap_init1(&UDPserverSoap, SOAP_IO_UDP | SOAP_XML_IGNORENS);
    soap_set_namespaces(&UDPserverSoap,  namespaces);

    printf("[%d][%s][%s] UDPserverSoap.version = %d \n", __LINE__, __TIME__, __func__, UDPserverSoap.version);

    int m = soap_bind(&UDPserverSoap, NULL, ONVIF_UDP_PORT, 10);
    if(!soap_valid_socket(m)) {
        soap_print_fault(&UDPserverSoap, stderr);
        exit(1);
    }
    log_inf("socket bind success %d", m);

    mcast.imr_multiaddr.s_addr = inet_addr(ONVIF_UDP_IP);
    mcast.imr_interface.s_addr = htonl(INADDR_ANY);
    //开启route
    system("route add -net 224.0.0.0 netmask 224.0.0.0 eth0");
    //IP_ADD_MEMBERSHIP 用于加入某个多播组，之后就可以向这个多播组发送数据或者从多播组接收数据
    if(setsockopt(UDPserverSoap.master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0) {
        log_inf("setsockopt error! error code = %d,err string = %s", errno, strerror(errno));
        return 0;
    }

    int udp_fd = -1;
    while(!gSystemStatus.exit_discover) {
        log_inf("[%s|%d] wait for accept connect", __FUNCTION__, __LINE__);
        udp_fd = soap_accept(&UDPserverSoap);
        log_inf("socket connect, udp_fd: %d\n", udp_fd);
        if (!soap_valid_socket(udp_fd)) {
            soap_print_fault(&UDPserverSoap, stderr);
            exit(1);
        }
        log_inf("-----------------------------------\n");

        if( soap_serve(&UDPserverSoap) != SOAP_OK ) {
            soap_print_fault(&UDPserverSoap, stderr);
            log_inf("soap_print_fault");
        }

        log_inf("IP = %u.%u.%u.%u", 
            ((UDPserverSoap.ip)>>24)&0xFF, ((UDPserverSoap.ip)>>16)&0xFF, 
            ((UDPserverSoap.ip)>>8)&0xFF,(UDPserverSoap.ip)&0xFF);

        soap_destroy(&UDPserverSoap);
        soap_end(&UDPserverSoap);
    }
    //分离运行时的环境
    printf("[%s] exit\n", __FUNCTION__);
    soap_done(&UDPserverSoap);
    pthread_exit(0);
}

 
static void *thread_onvif_webservices(void *arg) 
{
    struct soap tcpSoap;
    soap_init(&tcpSoap);
    tcpSoap.port = ONVIF_TCP_PORT;
    tcpSoap.bind_flags = SO_REUSEADDR;
    //tcpSoap.accept_timeout = tcpSoap.recv_timeout = tcpSoap.send_timeout = 5;
    soap_set_namespaces(&tcpSoap, namespaces);

    int tcpsocket = onvif_soap_bind(&tcpSoap, gOnvifDevInfo.ip, 0);
    if (!soap_valid_socket(tcpsocket)) {
        log_inf("tcpsocket onvif_soap_bind failed!\n");
        soap_print_fault(&tcpSoap, stderr);
        exit(1);
    }

    int tcp_fd = -1;
    log_inf("[%s|%d] wait for accept connect", __FUNCTION__, __LINE__);
    while(!gSystemStatus.exit_webservices) {
        tcp_fd = soap_accept(&tcpSoap);
        // log_inf("[%d] socket connect, tcp_fd: %d IP:%u.%u.%u.%u", __LINE__, tcp_fd, ((tcpSoap.ip)>>24)&0xFF, ((tcpSoap.ip)>>16)&0xFF, ((tcpSoap.ip)>>8)&0xFF,(tcpSoap.ip)&0xFF);
        
        if (!soap_valid_socket(tcp_fd)) {
            soap_print_fault(&tcpSoap, stderr);
            exit(1);
        }

        log_inf("[Req] tcp_fd: %d IP:%u.%u.%u.%u",
            tcp_fd, ((tcpSoap.ip)>>24)&0xFF, ((tcpSoap.ip)>>16)&0xFF, ((tcpSoap.ip)>>8)&0xFF,(tcpSoap.ip)&0xFF);

        if( soap_serve(&tcpSoap) != SOAP_OK ) {
            soap_print_fault(&tcpSoap, stderr);
            printf("soap_print_fault\n");
        }

        // printf("IP = %u.%u.%u.%u\n", ((tcpSoap.ip)>>24)&0xFF, ((tcpSoap.ip)>>16)&0xFF, ((tcpSoap.ip)>>8)&0xFF,(tcpSoap.ip)&0xFF);
        soap_destroy(&tcpSoap);
        soap_end(&tcpSoap);
    }

    //分离运行时的环境
    printf("[%s] exit\n", __FUNCTION__);
    soap_done(&tcpSoap);
    pthread_exit(0);
}


static void *thread_hello_bye(void *arg)
{
    struct soap send_soap;
    soap_init1(&send_soap, SOAP_IO_UDP|SOAP_XML_IGNORENS);
    soap_set_namespaces(&send_soap,  namespaces);

    while(!gSystemStatus.exit_hellobye) {
        printf("wait for hellobye sem\n");
        sem_wait(&gSystemStatus.hello_bye_sem);
        printf("gSystemStatus.hello_bye: %d\n", gSystemStatus.hello_bye);
        if (1 == gSystemStatus.hello_bye) {
            sleep(1);
            if (SOAP_OK != __wsdd__Hello(&send_soap, NULL)) {
                printf("ERROR! Say Hello\n");
            }
        } else if (2 == gSystemStatus.hello_bye) {
            sleep(1);
            if (SOAP_OK != __wsdd__Bye(&send_soap, NULL)) {
                printf("ERROR! Say Bye\n");
            }
            printf("is reboot!\n");
            system("reboot");
        } else {
            printf("error operation, hello_bye: %d\n", gSystemStatus.hello_bye);
        }
        // reset
        gSystemStatus.hello_bye = 0;
    }
    printf("[%s] exit\n", __FUNCTION__);
    pthread_exit(0);
}

int ONVIF_StartServer()
{
    onvif_register_callback();
    onvif_device_init();
    onvif_event_manager_init();
    pthread_create(&gSystemStatus.discover, NULL, thread_onvif_discover, NULL);
    pthread_create(&gSystemStatus.webservices, NULL, thread_onvif_webservices, NULL);
    pthread_create(&gSystemStatus.hellobye, NULL, thread_hello_bye, NULL);
    printf("ONVIF_StartServer finish\n");
 
    return 0;
}


int ONVIF_StopServer()
{
    printf("ONVIF_StopServer\n"); 

    gSystemStatus.exit_discover = 1;
    gSystemStatus.exit_webservices = 1;
    gSystemStatus.exit_hellobye = 1; 

    pthread_cancel(gSystemStatus.hellobye);
    pthread_cancel(gSystemStatus.discover);
    pthread_cancel(gSystemStatus.webservices);

    pthread_join(gSystemStatus.hellobye, 0);
    pthread_join(gSystemStatus.discover, 0);
    pthread_join(gSystemStatus.webservices, 0);
    return 0;
}

int __main(int argc,char ** argv)
{ 
    printf("argc %d, argv %p\n", argc, *argv);
    onvif_register_callback();
    onvif_device_init();
    printf("init finish\n");
    pthread_t discover = 0;
    pthread_t webservice = 0;
    pthread_t hellobye = 0;
    pthread_create(&discover, NULL, thread_onvif_discover, NULL);
    pthread_create(&webservice, NULL, thread_onvif_webservices, NULL);
    pthread_create(&hellobye, NULL, thread_hello_bye, NULL);

    pthread_join(discover, 0);
    pthread_join(webservice, 0);
    pthread_join(hellobye, 0);
    return 0;
}








