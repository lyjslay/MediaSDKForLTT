#ifndef _STACKTRACE_H_
#define _STACKTRACE_H_

// #include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include "cvi_log.h"

#define BACKTRACE_SIZE 64

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static inline void print_stacktrace(void) {
    void *array[BACKTRACE_SIZE];
    size_t size, i;
    char **strings;

    size = backtrace(array, BACKTRACE_SIZE);
    strings = backtrace_symbols(array, size);

    for (i = 0; i < size; i++) {
        CVI_LOGI("%p : %s", array[i], strings[i]);
    }

    free(strings);  // malloced by backtrace_symbols
}

static inline void handler(int32_t signo) {
    CVI_LOGI("\n=========>>>catch signal %d (%s) <<<=========", signo, (char *)strsignal(signo));

    CVI_LOGI("Dump stack start...");
    print_stacktrace();
    CVI_LOGI("Dump stack end...");

    signal(signo, SIG_DFL);

    raise(signo);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  // _STACKTRACE_H_
