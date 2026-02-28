/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: oam log interface's header file.(in rom).
 * Author: Hisilicon
 * Create: 2018-08-04
 */

#ifndef __OAM_LOG_H__
#define __OAM_LOG_H__

#include "hi_types.h"
#include "oal_err_wifi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  10 º¯ÊýÉùÃ÷
*****************************************************************************/
hi_u32 oam_log_level_set(hi_u32 log_level);

hi_void oal_print_nlogs(
        const hi_char* pfile_name,
        const hi_char* pfuc_name,
        hi_u16         us_line_no,
        void*          pfunc_addr,
        hi_u8          uc_vap_id,
        hi_u8          en_feature_id,
        hi_u8          clog_level,
        hi_u8          uc_param_cnt,
        hi_char*       fmt,
        size_t p1, size_t p2, size_t p3, size_t p4);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif /* end of oam_log.h */
