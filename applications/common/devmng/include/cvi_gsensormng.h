#ifndef __CVI_GSENSORMNG_H__
#define __CVI_GSENSORMNG_H__

#include "cvi_appcomm.h"
#include "cvi_hal_gsensor.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define pi 3.141592
#define ROLLOVER_MAX_DEGREE 75
#define ROLLOVER_MIN_DEGREE 25

#define CVI_GSENSORMNG_EINVAL            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_GSENSORMNG,CVI_EINVAL)   /**<parm error*/
#define CVI_GSENSORMNG_EINTER            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_GSENSORMNG,CVI_EINTER)        /**<intern error*/
#define CVI_GSENSORMNG_ENOINIT           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_GSENSORMNG,CVI_ENOINIT)  /**< no initialize*/
#define CVI_GSENSORMNG_EINITIALIZED      CVI_APPCOMM_ERR_ID(CVI_APP_MOD_GSENSORMNG,CVI_EINITIALIZED) /**< already initialized */
#define CVI_GSENSORMNG_EREGISTEREVENT    CVI_APPCOMM_ERR_ID(CVI_APP_MOD_GSENSORMNG,CVI_ERRNO_CUSTOM_BOTTOM)            /**<thread creat or join error*/
#define CVI_GSENSORMNG_ETHREAD           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_GSENSORMNG,CVI_ERRNO_CUSTOM_BOTTOM+1)            /**<thread creat or join error*/

typedef enum cviEVENT_GSENSORMNG_E
{
    CVI_EVENT_GSENSORMNG_COLLISION = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_GSENSORMNG, 0), /**<collision occur event*/
    CVI_EVENT_GSENSORMNG_BUIT
} CVI_EVENT_GSENSORMNG_E;

/* @brief sensitity enum*/
// typedef enum cviGSENSORMNG_SENSITITY_E
// {
//     CVI_GSENSORMNG_SENSITITY_OFF = 0,/**<gsensor off*/
//     CVI_GSENSORMNG_SENSITITY_LOW, /**<low sensitity*/
//     CVI_GSENSORMNG_SENSITITY_MIDDLE, /**<middle sensitity*/
//     CVI_GSENSORMNG_SENSITITY_HIGH, /**<high sensitity*/
//     CVI_GSENSORMNG_SENSITITY_BUIT
// } CVI_GSENSORMNG_SENSITITY_E;

typedef struct cviGSENSORMNG_VALUE_S
{
    int32_t s32XDirValue; /**<x direction value,unit acceleration of gravity g*/
    int32_t s32YDirValue; /**<y direction value,unit acceleration of gravity g*/
    int32_t s32ZDirValue; /**<z direction value,unit acceleration of gravity g*/
} CVI_GSENSORMNG_VALUE_S;

/* @brief gesensor chip work attr*/
typedef struct cviGSENSORMNG_ATTR_S
{
    uint32_t u32SampleRate; /**<sample rate,0 mean Adopt default,not config,unit kps*/
} CVI_GSENSORMNG_ATTR_S;

typedef struct cviGSENSORMNG_CFG_S
{
    int32_t gsensor_level;
    int32_t gsensor_enable;
    CVI_HAL_GSENSOR_SENSITITY_E enSensitity;
    CVI_GSENSORMNG_ATTR_S stAttr;
} CVI_GSENSORMNG_CFG_S;

int32_t CVI_GSENSORMNG_RegisterEvent(void);
int32_t CVI_GSENSORMNG_Init(const CVI_GSENSORMNG_CFG_S* pstCfg);
int32_t CVI_GSENSORMNG_SetSensitity(CVI_HAL_GSENSOR_SENSITITY_E enSensitity);
void CVI_GSENSORMNG_MenuSetSensitity(CVI_HAL_GSENSOR_SENSITITY_E enSensitity);
int32_t CVI_GSENSORMNG_GetAttr(CVI_GSENSORMNG_ATTR_S* pstAttr);
int32_t CVI_GSENSORMNG_SetAttr(const CVI_GSENSORMNG_ATTR_S* pstAttr);
int32_t CVI_GSENSORMNG_OpenInterrupt(int32_t IntNum);
int32_t CVI_GSENSORMNG_DeInit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_GSENSORMNG_H__ */