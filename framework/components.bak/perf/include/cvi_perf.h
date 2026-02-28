
#ifndef _CVI_PERF_H
#define _CVI_PERF_H

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *CVI_PERF_STAT_HANDLE_T;

void CVI_PERF_StatInit(CVI_PERF_STAT_HANDLE_T *hdl,
        const char *name, uint64_t print_interval);
void CVI_PERF_StatDeinit(CVI_PERF_STAT_HANDLE_T hdl);
void CVI_PERF_StatAdd(CVI_PERF_STAT_HANDLE_T hdl, uint64_t val_us);

typedef void *CVI_PERF_MARK_HANDLE_T;

void CVI_PERF_MarkInit(CVI_PERF_MARK_HANDLE_T *hdl,
        const char *name, uint64_t print_interval, uint64_t skip);
void CVI_PERF_MarkDeinit(CVI_PERF_MARK_HANDLE_T hdl);
void CVI_PERF_MarkAdd(CVI_PERF_MARK_HANDLE_T hdl, uint64_t time_us);

#ifdef __cplusplus
}
#endif

#endif
