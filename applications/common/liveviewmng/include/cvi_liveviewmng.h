#ifndef __CVI_LIVEVIEWMNG_H__
#define __CVI_LIVEVIEWMNG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdint.h>

int32_t CVI_LIVEVIEWMNG_Switch(uint32_t val);
int32_t CVI_LIVEVIEWMNG_MoveUp(int32_t wndid);
int32_t CVI_LIVEVIEWMNG_MoveDown(int32_t wndid);
int32_t CVI_LIVEVIEWMNG_Mirror(uint32_t val);
int32_t CVI_LIVEVIEWMNG_Filp(uint32_t val);
int32_t CVI_LIVEVIEWMNG_AdjustFocus(int32_t wndid , char* ratio);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_RECORDMNG_H__ */