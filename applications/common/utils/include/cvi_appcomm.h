#ifndef __CVI_APPCOMM_H__
#define __CVI_APPCOMM_H__

#include <stdint.h>
#include <stdbool.h>
#include "cvi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define CVI_KOMOD_PATH "/mnt/system/ko"
#define CVI_UVC_SCRIPTS_PATH "/etc"
/** Invalid FD */
#define CVI_APPCOMM_FD_INVALID_VAL (-1)

/** General String Length */
#define CVI_APPCOMM_COMM_STR_LEN (64)

/** Path Maximum Length */
#define CVI_APPCOMM_MAX_PATH_LEN (128)

/** FileName Maximum Length */
#define CVI_APPCOMM_MAX_FILENAME_LEN (64)

/** Pointer Check */
#define CVI_APPCOMM_CHECK_POINTER(p, errcode)    \
    do {                                        \
        if (!(p)) {                             \
            CVI_LOGE("pointer[%s] is NULL\n", #p); \
            return errcode;                     \
        }                                       \
    } while (0)

/** Expression Check */
#define CVI_APPCOMM_CHECK_EXPR(expr, errcode)  \
    do {                                      \
        if (!(expr)) {                        \
            CVI_LOGE("expr[%s] false\n", #expr); \
            return errcode;                   \
        }                                     \
    } while (0)

/** Expression Check With ErrInformation */
#define CVI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(expr, errcode, errstring) \
    do {                                                             \
        if (!(expr)) {                                               \
            CVI_LOGE("[%s] failed\n", errstring);                       \
            return errcode;                                          \
        }                                                            \
    } while (0)

/** Return Result Check */
#define CVI_APPCOMM_CHECK_RETURN(ret, errcode)       \
    do {                                            \
        if (0 != ret) {                    \
            CVI_LOGE("Error Code: [0x%08X]\n\n", ret); \
            return errcode;                         \
        }                                           \
    } while (0)

/** Return Result Check With ErrInformation */
#define CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(ret, errcode, errstring) \
    do {                                                              \
        if (0 != ret) {                                      \
            CVI_LOGE("[%s] failed[0x%08X]\n", errstring, ret);           \
            return errcode;                                           \
        }                                                             \
    } while (0)

/** Expression Check Without Return */
#define CVI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(expr, errstring) \
    do {                                                      \
        if ((expr)) {                                        \
            CVI_LOGE("[%s] failed\n", errstring);                \
        }                                                     \
    } while (0)

/** Range Check */
#define CVI_APPCOMM_CHECK_RANGE(value, min, max) (((value) <= (max) && (value) >= (min)) ? 1 : 0)

/** Memory Safe Free */
#define CVI_APPCOMM_SAFE_FREE(p) \
    do {                        \
        if (NULL != (p)) {      \
            free(p);            \
            (p) = NULL;         \
        }                       \
    } while (0)

/** Value Align */
#define CVI_APPCOMM_ALIGN(value, base) (((value) + (base)-1) / (base) * (base))

/** strcmp enum string and value */
#define CVI_APPCOMM_STRCMP_ENUM(enumStr, enumValue) strncmp(enumStr, #enumValue, CVI_APPCOMM_COMM_STR_LEN)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef MAX
#define MAX(a, b) (((a) < (b)) ? (b) : (a))
#endif

#ifndef MIN
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#endif

#ifndef SWAP
#define SWAP(a, b) (a = (a) + (b), b = (a) - (b), a = (a) - (b))
#endif

/** Mutex Lock */
#define CVI_MUTEX_INIT_LOCK(mutex)                   \
    do {                                            \
        (void)pthread_mutex_init(&mutex, NULL); \
    } while (0)
#define CVI_MUTEX_LOCK(mutex)                  \
    do {                                      \
        (void)pthread_mutex_lock(&mutex); \
    } while (0)
#define CVI_MUTEX_UNLOCK(mutex)                  \
    do {                                        \
        (void)pthread_mutex_unlock(&mutex); \
    } while (0)
#define CVI_MUTEX_DESTROY(mutex)                  \
    do {                                         \
        (void)pthread_mutex_destroy(&mutex); \
    } while (0)

/** Cond */
#define CVI_COND_INIT(cond)                                               \
    do {                                                                 \
        pthread_condattr_t condattr;                                     \
        (void)pthread_condattr_init(&condattr);                      \
        (void)pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC); \
        (void)pthread_cond_init(&cond, &condattr);                   \
        (void)pthread_condattr_destroy(&condattr);                   \
    } while (0)
#define CVI_COND_TIMEDWAIT(cond, mutex, usec)                  \
    do {                                                      \
        struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };   \
        (void)clock_gettime(CLOCK_MONOTONIC, &ts);        \
        ts.tv_sec += (usec / 1000000LL);                      \
        ts.tv_nsec += (usec * 1000LL % 1000000000LL);         \
        (void)pthread_cond_timedwait(&cond, &mutex, &ts); \
    } while (0)
#define CVI_COND_TIMEDWAIT_WITH_RETURN(cond, mutex, usec, ret) \
    do {                                                      \
        struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };   \
        (void)clock_gettime(CLOCK_MONOTONIC, &ts);        \
        ts.tv_sec += (usec / 1000000LL);                      \
        ts.tv_nsec += (usec * 1000LL % 1000000000LL);         \
        ret = pthread_cond_timedwait(&cond, &mutex, &ts);     \
    } while (0)

#define CVI_COND_WAIT(cond, mutex)                   \
    do {                                            \
        (void)pthread_cond_wait(&cond, &mutex); \
    } while (0)
#define CVI_COND_SIGNAL(cond)                  \
    do {                                      \
        (void)pthread_cond_signal(&cond); \
    } while (0)
#define CVI_COND_DESTROY(cond)                  \
    do {                                       \
        (void)pthread_cond_destroy(&cond); \
    } while (0)

/*************************************************************************
  typedef
*************************************************************************/

typedef int32_t CVI_ERRNO;

/*************************************************************************
  common error code
*************************************************************************/

#define CVI_ERRNO_COMMON_BASE  0
#define CVI_ERRNO_COMMON_COUNT 256

#define CVI_EUNKNOWN            (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 1)  /*  */
#define CVI_EOTHER              (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 2)  /*  */
#define CVI_EINTER              (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 3)  /*  */
#define CVI_EVERSION            (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 4)  /*  */
#define CVI_EPAERM              (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 5)  /*  */
#define CVI_EINVAL              (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 6)  /*  */
#define CVI_ENOINIT             (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 7)  /*  */
#define CVI_ENOTREADY           (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 8)  /*  */
#define CVI_ENORES              (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 9)  /*  */
#define CVI_EEXIST              (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 10) /*  */
#define CVI_ELOST               (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 11) /*  */
#define CVI_ENOOP               (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 12) /*  */
#define CVI_EBUSY               (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 13) /*  */
#define CVI_EIDLE               (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 14) /*  */
#define CVI_EFULL               (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 15) /*  */
#define CVI_EEMPTY              (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 16) /*  */
#define CVI_EUNDERFLOW          (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 17) /*  */
#define CVI_EOVERFLOW           (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 18) /*  */
#define CVI_EACCES              (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 19) /*  */
#define CVI_EINTR               (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 20) /*  */
#define CVI_ECONTINUE           (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 21) /*  */
#define CVI_EOVER               (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 22) /*  */
#define CVI_ERRNO_COMMON_BOTTOM (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + 23) /*  */

/*************************************************************************
  custom error code
*************************************************************************/

#define CVI_ERRNO_BASE          (CVI_ERRNO)(CVI_ERRNO_COMMON_BASE + CVI_ERRNO_COMMON_COUNT)
#define CVI_EINITIALIZED        (CVI_ERRNO)(CVI_ERRNO_BASE + 1) /*  */
#define CVI_ERRNO_CUSTOM_BOTTOM (CVI_ERRNO)(CVI_ERRNO_BASE + 2) /*  */

#define CVI_COND_INIT(cond)                                               \
    do {                                                                 \
        pthread_condattr_t condattr;                                     \
        (void)pthread_condattr_init(&condattr);                      \
        (void)pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC); \
        (void)pthread_cond_init(&cond, &condattr);                   \
        (void)pthread_condattr_destroy(&condattr);                   \
    } while (0)
#define CVI_COND_TIMEDWAIT(cond, mutex, usec)                  \
    do {                                                      \
        struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };   \
        (void)clock_gettime(CLOCK_MONOTONIC, &ts);        \
        ts.tv_sec += (usec / 1000000LL);                      \
        ts.tv_nsec += (usec * 1000LL % 1000000000LL);         \
        (void)pthread_cond_timedwait(&cond, &mutex, &ts); \
    } while (0)
#define CVI_COND_TIMEDWAIT_WITH_RETURN(cond, mutex, usec, ret) \
    do {                                                      \
        struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };   \
        (void)clock_gettime(CLOCK_MONOTONIC, &ts);        \
        ts.tv_sec += (usec / 1000000LL);                      \
        ts.tv_nsec += (usec * 1000LL % 1000000000LL);         \
        ret = pthread_cond_timedwait(&cond, &mutex, &ts);     \
    } while (0)

#define CVI_COND_WAIT(cond, mutex)                   \
    do {                                            \
        (void)pthread_cond_wait(&cond, &mutex); \
    } while (0)
#define CVI_COND_SIGNAL(cond)                  \
    do {                                      \
        (void)pthread_cond_signal(&cond); \
    } while (0)
#define CVI_COND_DESTROY(cond)                  \
    do {                                       \
        (void)pthread_cond_destroy(&cond); \
    } while (0)

/** App Event BaseId : [28bit~31bit] unique */
#define CVI_APPCOMM_EVENT_BASEID (0x10000000L)

/** App Event ID Rule [ --base[4bit]--|--module[8bit]--|--event[20bit]--]
    * module : module enum value [CVI_APP_MOD_E]
    * event_code : event code in specified module, unique in module
 */
#define CVI_APPCOMM_EVENT_ID(module, event) \
    ((uint32_t)((CVI_APPCOMM_EVENT_BASEID) | ((module) << 20) | (event)))


/** App Error BaseId : [28bit~31bit] unique */
#define CVI_APPCOMM_ERR_BASEID (0x80000000L)

/** App Error Code Rule [ --base[4bit]--|--module[8bit]--|--error[20bit]--]
    * module : module enum value [CVI_APP_MOD_E]
    * event_code : event code in specified module, unique in module
 */
#define CVI_APPCOMM_ERR_ID(module, err) \
    ((int32_t)((CVI_APPCOMM_ERR_BASEID) | ((module) << 20) | (err)))

/** App Module ID */
typedef enum cviAPP_MOD_E {
    CVI_APP_MOD_MEDIA = 0,
    CVI_APP_MOD_RECMNG,
    CVI_APP_MOD_PHOTOMNG,
    CVI_APP_MOD_FILEMNG,
    CVI_APP_MOD_LIVESVR,
    CVI_APP_MOD_USBMNG,
    CVI_APP_MOD_STORAGEMNG,
    CVI_APP_MOD_KEYMNG,
    CVI_APP_MOD_GAUGEMNG,
    CVI_APP_MOD_GSENSORMNG,
    CVI_APP_MOD_LEDMNG,
    CVI_APP_MOD_SCENE,
    CVI_APP_MOD_WEBSRV,
    CVI_APP_MOD_CONFACCESS,
    CVI_APP_MOD_PM,
    CVI_APP_MOD_SYSTEM,
    CVI_APP_MOD_RAWCAP,
    CVI_APP_MOD_UPGRADE,
    CVI_APP_MOD_OSD,
    CVI_APP_MOD_VIDEODETECT,
    CVI_APP_MOD_ISP_IR,

    CVI_APP_MOD_MODEMNG,
    CVI_APP_MOD_PARAM,
    CVI_APP_MOD_NETCTRL,
    CVI_APP_MOD_UI,
    CVI_APP_MOD_GPSMNG,
    CVI_APP_MOD_ACCMNG,
    CVI_APP_MOD_USBCTRL,
    CVI_APP_MOD_ALGADAPTER_ADAS,
    CVI_APP_MOD_VIDEOANALYSIS_ADAS,
    CVI_APP_MOD_VIDEOPROCESS,
    CVI_APP_MOD_VOLMNG,
    CVI_APP_MOD_PLAYBACKMNG,
    CVI_APP_MOD_SPEECHMNG,
    CVI_APP_MOD_ADASMNG,
    CVI_APP_MOD_BUTT
} CVI_APP_MOD_E;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif