#ifndef _ONVIF_DEBUG_H_
#define _ONVIF_DEBUG_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <stdio.h>


#define ONVIF_DEBUG 1

#if ONVIF_DEBUG
#define LOG_FUNC_IN(fmt, args...)   printf("\033[1m\033[35m[In ]--->%s():%dL" fmt "\033[0m\n", __func__, __LINE__, ##args);
#define LOG_FUNC_OUT(fmt, args...)  printf("\033[1m\033[35m[Out]--->%s():%dL" fmt "\033[0m\n", __func__, __LINE__, ##args);
#else
#define LOG_FUNC_IN(fmt, args...)
#define LOG_FUNC_OUT(fmt, args...) 
#endif

#if ONVIF_DEBUG
#define log_err(format, args...)    printf("\033[1m\033[31m[ERR|%s|%d] " format "\033[0m\n", __FUNCTION__, __LINE__, ##args);
#define log_inf(format, args...)    printf("\033[1m\033[32m[INF|%s|%d] " format "\033[0m\n", __FUNCTION__, __LINE__, ##args);     
#define log_war(format, args...)    printf("\033[1m\033[33m[WAR|%s|%d] " format "\033[0m\n", __FUNCTION__, __LINE__, ##args);  
#define log_dbg(format, args...)    printf("\033[1m\033[34m[DBG|%s|%d] " format "\033[0m\n", __FUNCTION__, __LINE__, ##args);    
#define log_com(format, args...)    printf("[COM|%s|%d] " format "\n", __FUNCTION__, __LINE__, ##args);
#else
#define log_err(format, args...)
#define log_inf(format, args...)
#define log_war(format, args...)
#define log_dbg(format, args...)
#define log_com(format, args...)
#endif   


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif