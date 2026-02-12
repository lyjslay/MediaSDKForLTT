#ifndef __PS_PARAM_H__
#define __PS_PARAM_H__

#include "cvi_player_service.h"

typedef CVI_PLAYER_SERVICE_PARAM_S* PS_PARAM_HANDLE;

int32_t ps_load_default_params(PS_PARAM_HANDLE param);

#endif // __PS_PARAM_H__
