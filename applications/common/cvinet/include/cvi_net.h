#include "thttpd.h"
#include "libhttpd.h"
#include "config.h"
#include "cvi_appcomm.h"
#ifndef _CVI_NET_H_
#define _CVI_NET_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define CGI_CMD_MAX_LEN     (32)
#define CGI_VAL_MAX_LEN     (32)
#define CGI_REG_MAX         (64)
#define MAX_CGI_INTER       (64)
#define CMDKEYWORDSIZE      (74)

typedef int32_t (*CVI_NET_CGI_CALLBACK)(void *param, const char *cmd, const char *val);
typedef int32_t (*CVI_NET_SYSCALL_CMD_TO_CALLBACK)(httpd_conn *pram, char *sptr);
typedef int32_t (*SYSCALL_CMD_TO_APP)(httpd_conn *pram, char * fun, char *sptr);

typedef struct _WIFIAPPMAPTO_S {
	int32_t cmd;
	CVI_NET_SYSCALL_CMD_TO_CALLBACK callback;
} CVI_NET_WIFIAPPMAPTO_S;

typedef struct _WIFIAPPMAPTO_CGI_S {
	char cmd[CGI_CMD_MAX_LEN];
	CVI_NET_SYSCALL_CMD_TO_CALLBACK callback;
} CVI_NET_WIFIAPPMAPTO_CGI_S;

typedef struct _WIFIAPPMAPTO_APP_S {
	char cmd[CGI_CMD_MAX_LEN];
	SYSCALL_CMD_TO_APP callback;
} CVI_NET_WIFIAPPMAPTO_APP_S;

int32_t CVI_NET_Init(void);
int32_t CVI_NET_DeInit(void);
int32_t CVI_NET_AddCgiResponse(int32_t len, void *param, char *pszfmt, ...);
int32_t CVI_NET_RegisterCgiCmd(CVI_NET_WIFIAPPMAPTO_S *cgi_cmd);
int32_t CVI_NET_RegisterCgiCmd_CGI(CVI_NET_WIFIAPPMAPTO_CGI_S *cgi_cmd);
int32_t CVI_RegisterCgiCmd_App(CVI_NET_WIFIAPPMAPTO_APP_S *cgi_cmd);
int32_t CVI_NET_AddResponse(void *param, char *pszfmt, int32_t len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* _CVI_NET_H_ */
