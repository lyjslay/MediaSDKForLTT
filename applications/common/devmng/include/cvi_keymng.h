/**
* @file    cvi_keymng.h
* @brief   product keymng struct and interface
* @version   1.0

*/

#ifndef _CVI_KEYMNG_H
#define _CVI_KEYMNG_H

#include "cvi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     KEYMNG */
/** @{ */  /** <!-- [KEYMNG] */

/** macro define */
#define CVI_KEYMNG_KEY_NUM_EACH_GRP (4)                        /**<key number in group-key*/

#define CVI_KEYMNG_EINVAL            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_KEYMNG,CVI_EINVAL)    /**<parm invlid*/
#define CVI_KEYMNG_EINTER            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_KEYMNG,CVI_EINTER)          /**<intern error*/
#define CVI_KEYMNG_ENOINIT           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_KEYMNG,CVI_ENOINIT)  /**< no initialize*/
#define CVI_KEYMNG_EINITIALIZED      CVI_APPCOMM_ERR_ID(CVI_APP_MOD_KEYMNG,CVI_EINITIALIZED) /**< already initialized */
#define CVI_KEYMNG_EREGISTEREVENT    CVI_APPCOMM_ERR_ID(CVI_APP_MOD_KEYMNG,CVI_ERRNO_CUSTOM_BOTTOM)            /**<thread creat or join error*/
#define CVI_KEYMNG_ETHREAD           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_KEYMNG,CVI_ERRNO_CUSTOM_BOTTOM+1)           /**<thread creat or join error*/

typedef enum cviEVENT_KEYMNG_E
{
    CVI_EVENT_KEYMNG_SHORT_CLICK = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_KEYMNG, 0),   /**<short key click event*/
    CVI_EVENT_KEYMNG_LONG_CLICK,    /**<long key click event*/
    CVI_EVENT_KEYMNG_HOLD_DOWN,     /**<key hold up event*/
    CVI_EVENT_KEYMNG_HOLD_UP,       /**<key hold down event*/
    CVI_EVENT_KEYMNG_GROUP,     /**<group key event*/
    CVI_EVENT_KEYMNG_BUIT
} CVI_EVENT_KEYMNG_E;


/** key index enum */
typedef enum cviKEYMNG_KEY_IDX_E
{
    CVI_KEYMNG_KEY_IDX_0 = 0,
    CVI_KEYMNG_KEY_IDX_1,
    CVI_KEYMNG_KEY_IDX_2,
    CVI_KEYMNG_KEY_IDX_3,
    CVI_KEYMNG_KEY_IDX_4,
    CVI_KEYMNG_KEY_IDX_5,
    CVI_KEYMNG_KEY_IDX_6,
    CVI_KEYMNG_KEY_IDX_BUTT,
} CVI_KEYMNG_KEY_IDX_E;

/** key type enum */
typedef enum cviKEYMNG_KEY_TYPE_E
{
    CVI_KEYMNG_KEY_TYPE_CLICK = 0, /**<support click and longclick event */
    CVI_KEYMNG_KEY_TYPE_HOLD,      /**<support keydown and keyup event */
} CVI_KEYMNG_KEY_TYPE_E;

/** click key attribute */
typedef struct cviKEYMNG_KEY_CLICK_ATTR_S
{
    bool bLongClickEnable; /**<ture: support click and longclick event; false: only support click event */
    uint32_t  u32LongClickTime_msec; /**<long click check time, valid when longclick enabled */
} CVI_KEYMNG_KEY_CLICK_ATTR_S;

/** hold key attribute */
typedef struct cviKEYMNG_KEY_HOLD_ATTR_S
{
} CVI_KEYMNG_KEY_HOLD_ATTR_S;

/** key attribute */
typedef struct cviKEYMNG_KEY_ATTR_S
{
    CVI_KEYMNG_KEY_TYPE_E enType;   /**<click type or hold type*/
    int32_t s32Id; /**<key id */
    union tagKEYMNG_KEY_ATTR_U
    {
        CVI_KEYMNG_KEY_CLICK_ATTR_S stClickKeyAttr;   /**<click attr type */
        CVI_KEYMNG_KEY_HOLD_ATTR_S stHoldKeyAttr;     /**<hold attr type */
    } unAttr;
} CVI_KEYMNG_KEY_ATTR_S;

/** key configure */
typedef struct cviKEYMNG_KEY_CFG_S
{
    uint32_t u32KeyCnt;     /**<key count*/
    CVI_KEYMNG_KEY_ATTR_S astKeyAttr[CVI_KEYMNG_KEY_IDX_BUTT];
} CVI_KEYMNG_KEY_CFG_S;


/** group-key configure */
typedef struct cviKEYMNG_GRP_KEY_CFG_S
{
    bool bEnable;    /**<ture: support group key event; false: not support */
    uint32_t au32GrpKeyIdx[CVI_KEYMNG_KEY_NUM_EACH_GRP]; /**<only support two keys group at present*/
} CVI_KEYMNG_GRP_KEY_CFG_S;

/** keymng configure */
typedef struct cviKEYMNG_CFG_S
{
    CVI_KEYMNG_KEY_CFG_S stKeyCfg;
    CVI_KEYMNG_GRP_KEY_CFG_S stGrpKeyCfg;
} CVI_KEYMNG_CFG_S;

/**
* @brief    register keymng event
* @return 0 success,non-zero error code.
*/
int32_t CVI_KEYMNG_RegisterEvent(void);

/**
* @brief    keymng initialization, create key event check task
* @param[in] pstCfg: keymng configure, including key/grpkey configure
* @return 0 success,non-zero error code.
*/
int32_t CVI_KEYMNG_Init(CVI_KEYMNG_CFG_S KeyCfg);

/**
* @brief    keymng deinitialization, destroy key event check task
* @return 0 success,non-zero error code.
*/
int32_t CVI_KEYMNG_DeInit(void);

/** @}*/  /** <!-- ==== KEYMNG End ====*/


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */

#endif /* #ifdef _CVI_KEYMNG_H */

