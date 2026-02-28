#include <stdio.h>
#include <inttypes.h>

#include "cvi_perf.h"
#include "cvi_log.h"

#define MAX_NAME_LEN  (32)

typedef struct CVI_PERF_STAT_S {
    char name[MAX_NAME_LEN];
    uint64_t print_interval;
    uint64_t count;
    uint64_t total;
    uint64_t max;
    uint64_t min;
    uint64_t print_count;
} CVI_PERF_STAT_T;

void CVI_PERF_StatInit(CVI_PERF_STAT_HANDLE_T *hdl,
        const char *name, uint64_t print_interval) {
    CVI_PERF_STAT_T *s = (CVI_PERF_STAT_T *)malloc(sizeof(CVI_PERF_STAT_T));
    snprintf(s->name, MAX_NAME_LEN, "%s", name);
    s->print_interval = print_interval;

    CVI_LOG_ASSERT(print_interval > 0,
            "print_interval needs to be a positive number\n");
    s->count = 0;
    s->total = 0;
    s->max = 0;
    s->min = 0xffffffffffffffff;

    *hdl = (CVI_PERF_STAT_HANDLE_T)s;
}

void CVI_PERF_StatDeinit(CVI_PERF_STAT_HANDLE_T hdl) {
    CVI_PERF_STAT_T *s = (CVI_PERF_STAT_T *)hdl;
    free(s);
}

void CVI_PERF_StatAdd(CVI_PERF_STAT_HANDLE_T hdl, uint64_t val_us) {
    CVI_PERF_STAT_T *s = (CVI_PERF_STAT_T *)hdl;

    s->total += val_us;
    s->max = (val_us > s->max) ? val_us : s->max;
    s->min = (val_us < s->min) ? val_us : s->min;
    s->count ++;

    if (s->count == s->print_interval) {
        CVI_LOGI("[STAT](%s): cnt %"PRIu64", avg %2.2f ms, max %2.2f, min %2.2f\n",
            s->name, s->count,
            s->total / 1000.0f / s->count,
            s->max / 1000.0f, s->min / 1000.0f);

        s->total = 0;
        s->max = 0;
        s->min = 0xffffffffffffffff;
        s->count = 0;
    }
}

typedef struct CVI_PERF_MARK_S {
    CVI_PERF_STAT_HANDLE_T stat;
    uint64_t skip;
    uint64_t last_time_us;
} CVI_PERF_MARK_T;

void CVI_PERF_MarkInit(CVI_PERF_MARK_HANDLE_T *hdl,
        const char *name, uint64_t print_interval, uint64_t skip) {
    CVI_PERF_MARK_T *m = (CVI_PERF_MARK_T *)malloc(sizeof(CVI_PERF_MARK_T));
    CVI_PERF_StatInit(&m->stat, name, print_interval);
    m->skip = skip;
    if (m->skip == 0)
        m->skip = 1; // at least skip 1
    m->last_time_us = 0;
    *hdl = (CVI_PERF_MARK_HANDLE_T)m;
}

void CVI_PERF_MarkDeinit(CVI_PERF_MARK_HANDLE_T hdl) {
    CVI_PERF_MARK_T *m = (CVI_PERF_MARK_T *)hdl;
    CVI_PERF_StatDeinit(m->stat);
}

void CVI_PERF_MarkAdd(CVI_PERF_MARK_HANDLE_T hdl, uint64_t time_us) {
    CVI_PERF_MARK_T *m = (CVI_PERF_MARK_T *)hdl;
    if (!m->skip) {
        CVI_PERF_StatAdd(m->stat, time_us - m->last_time_us);
    } else {
        m->skip --;
    }
    m->last_time_us = time_us;
}
