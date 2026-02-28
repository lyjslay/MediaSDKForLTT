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

#ifdef __cplusplus
}
#endif
#endif
