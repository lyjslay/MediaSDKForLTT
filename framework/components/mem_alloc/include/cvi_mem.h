#ifndef CVI_MEM_H
#define CVI_MEM_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

void *CVI_MEM_Allocate(size_t size, const char *name);
void CVI_MEM_Free(void *vir_addr);

void *CVI_MEM_AllocateVb(size_t size);
void CVI_MEM_VbFree(void *vir_addr);
/** return 1 if vir_addr was allocated by CVI_MEM_AllocateVb, 0 otherwise */
int CVI_MEM_IsVbAddress(void *vir_addr);

#ifdef __cplusplus
}
#endif
#endif
