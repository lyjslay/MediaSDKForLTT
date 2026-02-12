/**
* @file    cvi_hal_key.h
* @brief   product hal key interface
*/
#ifndef __CVI_HAL_KEY_H__
#define __CVI_HAL_KEY_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#include "cvi_hal_gpio.h"

/** \addtogroup     HAL_KEY */

/** macro define */
#define CVI_HAL_EUNKNOW           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_EUNKNOWN) /* unknow error*/
#define CVI_HAL_EINTER            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_EINTER)   /* intern error*/
#define CVI_HAL_EINVAL            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_EINVAL)   /* parm invalid*/
#define CVI_HAL_ENOINIT           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_ENOINIT)  /* no initialize*/
#define CVI_HAL_ENOREG            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_ELOST)    /* no registered*/

#define CVI_HAL_EREGRED           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_EEXIST)   /* already registered*/
#define CVI_HAL_EINITIALIZED      CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_EINITIALIZED)         /* already initialized */
#define CVI_HAL_ENOSTART          CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_ERRNO_CUSTOM_BOTTOM)  /* no start*/
#define CVI_HAL_ESTARTED          CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_ERRNO_CUSTOM_BOTTOM+1)/* already started*/
#define CVI_HAL_ENOSTOP           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_ERRNO_CUSTOM_BOTTOM+2)/* no stop*/

#define CVI_HAL_EGPIO             CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_ERRNO_CUSTOM_BOTTOM+3)/* gpio cfg or read error */
#define CVI_HAL_EINVOKESYS        CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_ERRNO_CUSTOM_BOTTOM+4)/* invoke system function error */
#define CVI_HAL_EIPCMSG           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_ERRNO_CUSTOM_BOTTOM+5)/* ipcmsg error */
#define CVI_HAL_TIMEOUT           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_ERRNO_CUSTOM_BOTTOM+6)/* timeout error */
#define CVI_HAL_EAGAIN            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_HAL,CVI_ERRNO_CUSTOM_BOTTOM+7)/* Call is interrupted */

#define HAL_FD_INITIALIZATION_VAL (-1)
#define HAL_MMAP_LENGTH (0x500)

/* @brief key status enum*/
typedef enum cviHAL_KEY_STATE_E
{
    CVI_HAL_KEY_STATE_DOWN = 0,/**<key down state*/
    CVI_HAL_KEY_STATE_UP, /**<key up state*/
    CVI_HAL_KEY_STATE_BUIT
} CVI_HAL_KEY_STATE_E;

/* @brief key index enum*/
typedef enum cviHAL_KEY_IDX_E
{
    CVI_HAL_KEY_IDX_0 = 0, /**<key index 0*/
    CVI_HAL_KEY_IDX_1,
    CVI_HAL_KEY_IDX_2,
    CVI_HAL_KEY_IDX_3,
    CVI_HAL_KEY_IDX_4,
    CVI_HAL_KEY_IDX_5,
    CVI_HAL_KEY_IDX_6,
    CVI_HAL_KEY_IDX_BUIT
} CVI_HAL_KEY_IDX_E;

/* @Key GPIO list*/
typedef struct cviGPIO_ID_E {
	CVI_HAL_KEY_IDX_E id;
	int32_t gpioid;
} CVI_GPIO_ID_E;

/**
* @brief    hal key initialization, open gpio device and mmap CRG regist address
* @return 0 success,non-zero error code.
*/
int32_t CVI_HAL_KEY_Init(void);

/**
* @brief    get hal key state
* @param[in] enKeyIdx: key index
* @param[out] penKeyState:key state
* @return 0 success,non-zero error code.
*/
int32_t CVI_HAL_KEY_GetState(CVI_HAL_KEY_IDX_E enKeyIdx, CVI_HAL_KEY_STATE_E* penKeyState);

/**
* @brief    hal key deinitialization, close gpio device and ummap CRG regist address
* @return 0 success,non-zero error code.
*/
int32_t CVI_HAL_KEY_Deinit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_HAL_KEY_H__*/
