#ifndef __CS_PARAM_H__
#define __CS_PARAM_H__

#include "rs_define.h"
#include "cvi_record_service.h"

typedef CVI_RECORD_SERVICE_PARAM_S rs_param_t, *rs_param_handle_t;

int32_t rs_param_load_default(rs_param_handle_t param);

#endif  // __CS_PARAM_H__
