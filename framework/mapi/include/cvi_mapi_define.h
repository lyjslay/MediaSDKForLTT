#ifndef __CVI_MAPI_DEFINE_H__
#define __CVI_MAPI_DEFINE_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CVI_MAPI_SUCCESS           ((int)(0))
#define CVI_MAPI_ERR_FAILURE       ((int)(-1001))
#define CVI_MAPI_ERR_NOMEM         ((int)(-1002))
#define CVI_MAPI_ERR_TIMEOUT       ((int)(-1003))
#define CVI_MAPI_ERR_INVALID       ((int)(-1004))

typedef void * CVI_MAPI_HANDLE_T;

#ifndef UNUSED
# define UNUSED(x) x=x
#endif

#ifdef __cplusplus
}
#endif

#endif
