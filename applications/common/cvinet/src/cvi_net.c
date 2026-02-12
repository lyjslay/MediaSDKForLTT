#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include "cvi_net.h"
#include "cvi_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static cvi_osal_task_handle_t g_webserver_thread;

pthread_mutex_t g_cgi_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_cgi_mutex_cgi = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_cgi_mutex_app = PTHREAD_MUTEX_INITIALIZER;
CVI_NET_WIFIAPPMAPTO_S g_custom_list[CMDKEYWORDSIZE];
CVI_NET_WIFIAPPMAPTO_CGI_S g_custom_list_cgi[CMDKEYWORDSIZE];
CVI_NET_WIFIAPPMAPTO_APP_S g_custom_list_app[CMDKEYWORDSIZE];

static int32_t cvi_wifi_cmd_proc(httpd_conn *hc, int32_t cmd, char *str);
static int32_t cvi_wifi_cmd_proc_cgi(httpd_conn *hc, char *cmd, char *str);
static int32_t cvi_wifi_cmd_proc_app(httpd_conn *hc, char *cmd, char *str, char *val);

int32_t CVI_NET_AddCgiResponse(int32_t len, void *param, char *pszfmt, ...)
{
    httpd_conn *hc = (httpd_conn *)param;

    if (NULL == hc || NULL == pszfmt) {
        return -1;
    }
    int32_t ret;
    char response[len];
    memset(response, 0, sizeof(response));
    va_list stVal;
    va_start(stVal, pszfmt);
    ret = vsprintf(response, pszfmt, stVal);
    va_end(stVal);

    add_customresponse(hc, response, ret);
    return ret;
}

int32_t CVI_NET_AddResponse(void *param, char *pszfmt, int32_t len)
{
    httpd_conn *hc = (httpd_conn *)param;

    if (NULL == hc || NULL == pszfmt) {
        return -1;
    }

    add_customresponse(hc, pszfmt, len);
    return 0;
}

static int32_t cvi_cgi_handle(httpd_conn *hc)
{
    char *origfilenameA = NULL;
    char *origfilenameB = NULL;
    char *sptr = NULL;
    char *urlbuff;
    char szStrNeed[128] = {0};
    int32_t pcmd = 0;

    if (hc == NULL) {
        CVI_LOGI("pstRequest & pstReponse can not be null\n\n\n");
        return -1;
    }

    urlbuff = CVI_THTTPGETURL();
    if (NULL == urlbuff) {
        CVI_LOGI("url can not be null\n\n\n");
        return -1;
    }

    if ((NULL != strstr(urlbuff, "str")) ||
        (NULL != strstr(urlbuff, "par"))) {
        origfilenameA = strchr(urlbuff, '&');
        origfilenameB = (origfilenameA + 1);
        memcpy(szStrNeed, origfilenameB, strchr(origfilenameB, '&') - origfilenameB);
        origfilenameA = strrchr(urlbuff, '=');
        sptr = (origfilenameA + 1);
        origfilenameB = (strrchr(szStrNeed, '=') + 1);
        sscanf(origfilenameB,"%d", &pcmd);
        cvi_wifi_cmd_proc(hc, pcmd, sptr);
    } else {
        if ((NULL != strstr(urlbuff, "MOV")) ||
            (NULL != strstr(urlbuff, "JPG"))) {
            memcpy(szStrNeed, urlbuff, (strrchr(urlbuff, '?') - urlbuff));
            origfilenameA = strrchr(urlbuff, '=');
            origfilenameB = (origfilenameA + 1);
            sscanf(origfilenameB,"%d", &pcmd);
            cvi_wifi_cmd_proc(hc, pcmd, szStrNeed);
        } else {
            origfilenameA = strrchr(urlbuff, '=');
            origfilenameB = (origfilenameA + 1);
            sscanf(origfilenameB,"%d", &pcmd);
            cvi_wifi_cmd_proc(hc, pcmd, sptr);
        }
    }

    return 0;
}

static int32_t cvi_cgi_handle_cgibin(httpd_conn *hc)
{
    char *origfilenameA = NULL;
    char *origfilenameB = NULL;
    char *sptr = NULL;
    char *urlbuff;
    char szStrNeed[128] = {0};
    char *pcmd = NULL;
    char *from = NULL;

    if (hc == NULL) {
        CVI_LOGI("pstRequest & pstReponse can not be null\n\n\n");
        return -1;
    }

    urlbuff = CVI_THTTPGETURL();
    if (NULL == urlbuff) {
        CVI_LOGI("url can not be null\n\n\n");
        return -1;
    }

    if (NULL != strstr(urlbuff, "set") || NULL != strstr(urlbuff, "get")) {
        origfilenameA = strchr(urlbuff, '&');
        origfilenameB = (origfilenameA + 1);
        if(strchr(origfilenameB, '&') != NULL){
            memcpy(szStrNeed, origfilenameB, strchr(origfilenameB, '&') - origfilenameB);
            origfilenameA = strchr(origfilenameB, '&');
            sptr = (strchr(origfilenameA, '=') + 1);
            pcmd = (strrchr(szStrNeed, '=') + 1);
        }else{
            pcmd = (strrchr(urlbuff, '=') + 1);
        }
    } else {
        if(strstr(urlbuff, "action=dir") != NULL){
            from = strrchr(urlbuff, '=') + 1;
            if(strcmp(from, "0") == 0)
            {
                pcmd = "dir";
                origfilenameA = (strchr(urlbuff, '&') + 1);
                memcpy(szStrNeed, origfilenameA, strchr(origfilenameA, '&') - origfilenameA);
                sptr = (strrchr(szStrNeed, '=') + 1);
            }else{
                pcmd = "NULL";
            }
        }else if(strstr(urlbuff, "action=reardir") != NULL){
           from = strrchr(urlbuff, '=') + 1;
            if(strcmp(from, "0") == 0)
            {
                pcmd = "dir";
                origfilenameA = (strchr(urlbuff, '&') + 1);
                memcpy(szStrNeed, origfilenameA, strchr(origfilenameA, '&') - origfilenameA);
                sptr = (strrchr(szStrNeed, '=') + 1);
                strcat(sptr, "_rear");
            }else{
                pcmd = "NULL";
            }
        }else if(strstr(urlbuff, "action=del") != NULL){
            pcmd = "DEL";
            sptr = (strrchr(urlbuff, '=') + 1);
        }else if(strstr(urlbuff, "cammenu") != NULL){
            pcmd = "cammenu";
        }else if(strstr(urlbuff, "thumb") != NULL){
            pcmd = "thumb";
            sptr = (strstr(urlbuff, "thumb")) + 5;
        }else{
            pcmd = "NULL";
        }
    }
    cvi_wifi_cmd_proc_cgi(hc, pcmd, sptr);

    return 0;
}

static int32_t cvi_handle_app(httpd_conn *hc)
{
    char *origfilenameA = NULL;
    char *origfilenameB = NULL;
    char szStrNeed[128] = {0};
    char *urlbuff;
    char pcmd[64] = {0};
    char *fun = NULL;
    char *sptr = NULL;
    // pcmd = malloc(32);
    if (hc == NULL) {
        CVI_LOGI("pstRequest & pstReponse can not be null\n\n\n");
        return -1;
    }

    urlbuff = CVI_THTTPGETURL();
    if (NULL == urlbuff) {
        CVI_LOGI("url can not be null\n\n\n");
        return -1;
    }

    origfilenameA = urlbuff+5;
    if (NULL == strchr(origfilenameA, '?')){
        CVI_LOGD("%s : origfilenameA is %s\n ",__func__ , origfilenameA);
        cvi_wifi_cmd_proc_app(hc, origfilenameA, fun, sptr);
    }else if(NULL == strchr(origfilenameA, '&')){
        memcpy(szStrNeed, origfilenameA, strchr(origfilenameA, '?') - origfilenameA);
        // pcmd = szStrNeed;
        memcpy(pcmd, szStrNeed, strlen(szStrNeed));
        if(pcmd != NULL) {CVI_LOGD("%s : pcmd is %s\n ",__func__ , pcmd);}
        fun = strchr(origfilenameA, '?')+1;
        if(fun != NULL) {CVI_LOGD("%s : fun is %s\n ",__func__ , fun);}
        cvi_wifi_cmd_proc_app(hc, pcmd, fun, sptr);
    }else{
        memcpy(szStrNeed, origfilenameA, strchr(origfilenameA, '?') - origfilenameA);
        // pcmd = szStrNeed;
        memcpy(pcmd, szStrNeed, strlen(szStrNeed));
        if(pcmd != NULL) {CVI_LOGD("%s : pcmd is %s\n ",__func__ , pcmd);}
        origfilenameB = strchr(origfilenameA, '?')+1;
        memset(szStrNeed, 0, sizeof(szStrNeed));
        memcpy(szStrNeed, origfilenameB, strchr(origfilenameB, '&') - origfilenameB);
        fun = szStrNeed;
        if(fun != NULL) {CVI_LOGD("%s : fun is %s\n ",__func__ , fun);}
        sptr = strchr(origfilenameB,'&')+1;
        if(sptr != NULL) {CVI_LOGD("%s : sptr is %s\n ",__func__ , sptr);}
        CVI_LOGD("enter pcmd is %s, fun is %s, sptr = %s\n", pcmd, fun, sptr);
        cvi_wifi_cmd_proc_app(hc, pcmd, fun, sptr);
    }
    // free(pcmd);
    return 0;
}

static int32_t cvi_cgi_handle_all(httpd_conn *hc)
{
    int32_t result = 0;
    CVI_LOGD("net url = %s\n", hc->encodedurl);
#ifdef NETPROTOCOL_XML
    if (NULL == strstr(hc->encodedurl, XML_PATTERN)) {
        httpd_send_err( hc, 404, "Not Found", "", "The requested URL '%.80s' was not found on this server.\n", hc->encodedurl );
	    return -1;
    } else {
        cvi_cgi_handle(hc);
    }
#elif defined (NETPROTOCOL_CGI)
    if (NULL == strstr(hc->encodedurl, CGI_PATTERN) &&
        NULL == strstr(hc->encodedurl, THUMB_PATTERN) &&
        NULL == strstr(hc->encodedurl, MENU_PATTERN)) {
        httpd_send_err( hc, 404, "Not Found", "", "The requested URL '%.80s' was not found on this server.\n", hc->encodedurl );
	    return -1;
    } else {
        cvi_cgi_handle_cgibin(hc);
    }
#else
    if (NULL == strstr(hc->encodedurl, "app")) {
        httpd_send_err( hc, 404, "Not Found", "", "The requested URL '%.80s' was not found on this server.\n", hc->encodedurl );
	    return -1;
    } else {
        cvi_handle_app(hc);
    }
#endif
    return result;
}

int32_t CVI_NET_Init(void)
{
    CVI_LOGI("enter: %s\n", __func__);
    cvi_net_terminate(0);
    int32_t s32Ret = 0;
    cvi_osal_task_attr_t ta;
    ta.name = "thttpd";
    ta.entry = thttpd_start_main;
    ta.param = NULL;
    ta.priority = CVI_OSAL_PRI_NORMAL;
    ta.detached = false;
    s32Ret = cvi_osal_task_create(&ta, &g_webserver_thread);
    CVI_THTTPD_RegisterCgiHandle(cvi_cgi_handle_all);
    return s32Ret;
}

int32_t CVI_NET_DeInit(void)
{
    CVI_LOGI("enter: %s\n", __func__);
    cvi_net_terminate(1);
    cvi_osal_task_join(g_webserver_thread);
    cvi_osal_task_destroy(&g_webserver_thread);
    return 0;
}

int32_t CVI_NET_RegisterCgiCmd(CVI_NET_WIFIAPPMAPTO_S *cgi_cmd)
{
    int32_t i = 0;
    pthread_mutex_lock(&g_cgi_mutex);
    for (i = 0; i < CMDKEYWORDSIZE; i++) {
        if (NULL == g_custom_list[i].callback) {
            memcpy(&g_custom_list[i], cgi_cmd, sizeof(CVI_NET_WIFIAPPMAPTO_S));
            break;
        }
    }

    if (i >= CMDKEYWORDSIZE) {
        pthread_mutex_unlock(&g_cgi_mutex);
        CVI_LOGI("command list is full\n");
        return -1;
    }
    pthread_mutex_unlock(&g_cgi_mutex);
    return 0;
}

int32_t CVI_NET_RegisterCgiCmd_CGI(CVI_NET_WIFIAPPMAPTO_CGI_S *cgi_cmd)
{
    int32_t i = 0;
    pthread_mutex_lock(&g_cgi_mutex_cgi);
    for (i = 0; i < CMDKEYWORDSIZE; i++) {
        if (NULL == g_custom_list_cgi[i].callback) {
            memcpy(&g_custom_list_cgi[i], cgi_cmd, sizeof(CVI_NET_WIFIAPPMAPTO_CGI_S));
            break;
        }
    }

    if (i >= CMDKEYWORDSIZE) {
        pthread_mutex_unlock(&g_cgi_mutex_cgi);
        CVI_LOGI("command list is full\n");
        return -1;
    }
    pthread_mutex_unlock(&g_cgi_mutex_cgi);
    return 0;
}

int32_t CVI_RegisterCgiCmd_App(CVI_NET_WIFIAPPMAPTO_APP_S *cgi_cmd)
{
    int32_t i = 0;
    pthread_mutex_lock(&g_cgi_mutex_app);
    for (i = 0; i < CMDKEYWORDSIZE; i++) {
        if (NULL == g_custom_list_app[i].callback) {
            memcpy(&g_custom_list_app[i], cgi_cmd, sizeof(CVI_NET_WIFIAPPMAPTO_APP_S));
            break;
        }
    }

    if (i >= CMDKEYWORDSIZE) {
        pthread_mutex_unlock(&g_cgi_mutex_app);
        CVI_LOGI("command list is full\n");
        return -1;
    }
    pthread_mutex_unlock(&g_cgi_mutex_app);
    return 0;
}

static int32_t cvi_wifi_cmd_proc(httpd_conn *hc, int32_t cmd, char *str)
{
    CVI_NET_SYSCALL_CMD_TO_CALLBACK pFunCustomCmdProc = NULL;

    for (int32_t i = 0; i < CMDKEYWORDSIZE; i++) {
        if (g_custom_list[i].cmd == cmd) {
            pFunCustomCmdProc = g_custom_list[i].callback;
            if (NULL != pFunCustomCmdProc) {
                pFunCustomCmdProc(hc, str);
            }
            break;
        }

        if (CMDKEYWORDSIZE <= i) {
            CVI_LOGI("cmd fail\n");
            return -1;
        }
    }

    return 0;

}

static int32_t cvi_wifi_cmd_proc_cgi(httpd_conn *hc, char *cmd, char *str)
{
    CVI_NET_SYSCALL_CMD_TO_CALLBACK pFunCustomCmdProc = NULL;

    if(strstr(cmd, "MJPEG") != NULL){
        cmd = strchr(cmd, '.')+1;
    }
    for (int32_t i = 0; i < CMDKEYWORDSIZE; i++) {
        if (strcmp(g_custom_list_cgi[i].cmd ,cmd) == 0) {
            pFunCustomCmdProc = g_custom_list_cgi[i].callback;
            if (NULL != pFunCustomCmdProc) {
                pFunCustomCmdProc(hc, str);
            }
            break;
        }

        if (CMDKEYWORDSIZE <= i) {
            CVI_LOGI("cmd fail\n");
            return -1;
        }
    }

    return 0;
}

static int32_t cvi_wifi_cmd_proc_app(httpd_conn *hc, char *cmd, char *fun, char *str)
{
    SYSCALL_CMD_TO_APP pFunCustomCmdProc = NULL;
    for (int32_t i = 0; i < CMDKEYWORDSIZE; i++) {
        if (strcmp(g_custom_list_app[i].cmd ,cmd) == 0) {
            pFunCustomCmdProc = g_custom_list_app[i].callback;
            if (NULL != pFunCustomCmdProc) {
                pFunCustomCmdProc(hc, fun, str);
            }
            break;
        }

        if (CMDKEYWORDSIZE <= i) {
            CVI_LOGI("cmd fail\n");
            return -1;
        }
    }

    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

