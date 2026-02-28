#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/prctl.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

#include "cvi_timer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define DEBUG_LOG 0
#define EPOLL_SIZE 16

typedef struct cviTIMER_INFO CVI_TIMER_INFO_S, *CVI_TIMER_INFO_P;

struct cviTIMER_INFO {
    int32_t             epfd;
    int32_t             timeOut;
    CVI_TIMER_HANDLE_T    timerHdl[EPOLL_SIZE];
    struct epoll_event  events[EPOLL_SIZE];
    pthread_t           timerTskId;
    pthread_mutex_t     timerMutex;
    bool                bBlock;
    bool                bTimerRun;
    CVI_TIMER_INFO_P    next;
};

typedef struct cviTIMER {
    int32_t     tfd;
    CVI_TIMER_S timerConf;
}CVI_TIMER, *CVI_TIMER_P;

CVI_TIMER_INFO_P headList = NULL;

static int32_t CVI_Timer_GetTimerGrpHdl(CVI_TIMER_INFO_P *Hdl, int32_t grpHdl)
{
    CVI_TIMER_INFO_P p = NULL;
    for (p = headList; p != NULL; p = p->next) {
        if (p->epfd == grpHdl) {
            *Hdl = p;
            break;
        }
    }
    if (*Hdl == NULL) {
        printf("no find timer hdl!\n");
        return -1;
    }

    return 0;
}

static int32_t CVI_Timer_DeleteTimerGrpHdl(CVI_TIMER_INFO_P *Hdl, int32_t grpHdl)
{
    CVI_TIMER_INFO_P p = headList, pr = headList;

    if (headList == NULL) {
        printf("timer grp is empty!\n");
        return -1;
    }
    while (grpHdl != p->epfd && p->next != NULL) {
        pr = p;
        p = p->next;
    }
    if (grpHdl == p->epfd) {
        if (p == headList) {
            headList = p->next;
        } else {
            pr->next = p->next;
        }
        *Hdl = p;
    } else {
        printf("This Node has not been found");
    }

    return 0;
}

static void* CVI_Timer(void* pvParam)
{
    // int32_t ret = 0;
    int32_t nfds;
    CVI_TIMER_INFO_P Hdl = (CVI_TIMER_INFO_P)pvParam;

    prctl(PR_SET_NAME, __FUNCTION__, 0, 0, 0);
    #if DEBUG_LOG
    uint64_t curTimeUs = 0, preTimeUs = 0;
    #endif
    struct timespec curTime;
    while (Hdl->bTimerRun) {
        #if DEBUG_LOG
        clock_gettime(CLOCK_MONOTONIC, &curTime);
        curTimeUs = curTime.tv_sec * 1000000.0 + curTime.tv_nsec / 1000.0;
        #endif
        if (Hdl->bBlock == true) {
            nfds = epoll_wait(Hdl->epfd, Hdl->events, EPOLL_SIZE, -1);
        } else {
            nfds = epoll_wait(Hdl->epfd, Hdl->events, EPOLL_SIZE, Hdl->timeOut);
        }
        if (nfds == 0){
            #if DEBUG_LOG
            printf("wait time ===== (%"PRId64") us, nfds = %d\n", curTimeUs - preTimeUs, nfds);
            preTimeUs = curTimeUs;
            #endif
            continue;
        }
        for (int i = 0; i < nfds; i++) {
            if (Hdl->events[i].events & EPOLLIN) {
                uint64_t data;
                CVI_TIMER_P TimerHdl = (CVI_TIMER_P)Hdl->events[i].data.ptr;
                pthread_mutex_lock(&Hdl->timerMutex);
                if(TimerHdl == NULL){
                    pthread_mutex_unlock(&Hdl->timerMutex);
                    continue;
                }
                read(TimerHdl->tfd, &data, sizeof(uint64_t));
                #if DEBUG_LOG
                printf("wait time ===== (%"PRId64") us, nfds = %d\n", curTimeUs - preTimeUs, nfds);
                preTimeUs = curTimeUs;
                printf("i == %d tfd = %d\n", i, TimerHdl->tfd);
                #endif
                clock_gettime(CLOCK_MONOTONIC, &curTime);
                if (TimerHdl->timerConf.timer_proc != NULL) {
                    TimerHdl->timerConf.timer_proc(TimerHdl->timerConf.clientData, &curTime);
                }
                pthread_mutex_unlock(&Hdl->timerMutex);
            }
        }
    }

    return NULL;
}

int32_t CVI_Timer_Init(bool bBlock)
{
    CVI_TIMER_INFO_P Hdl = (CVI_TIMER_INFO_P)calloc(sizeof(CVI_TIMER_INFO_S), 1);
    int32_t epfd, ret = 0;
    epfd = epoll_create(EPOLL_SIZE);
    if (epfd < 0) {
        printf("epoll_create error!\n");
        return -1;
    }
    pthread_mutex_init(&Hdl->timerMutex, NULL);
    Hdl->epfd = epfd;
    Hdl->bBlock = bBlock;
    Hdl->bTimerRun = true;
    Hdl->timeOut = 300; //ms

    ret = pthread_create(&Hdl->timerTskId, NULL, CVI_Timer, (void *)Hdl);
    if (ret != 0) {
        printf("create CVI_Timer failed:%s\n", strerror(errno));
        return -1;
    }

    if (headList == NULL) {
        headList = Hdl;
        headList->next = NULL;
    } else {
        CVI_TIMER_INFO_P p = NULL;
        for (p = headList; p->next != NULL; p = p->next);
        p->next = Hdl;
    }

    return epfd;
}

int32_t CVI_Timer_DeInit(int32_t grpHdl)
{
    int32_t ret = 0;
    CVI_TIMER_INFO_P Hdl = NULL;
    ret = CVI_Timer_DeleteTimerGrpHdl(&Hdl, grpHdl);
    if (ret != 0) {
        printf("%s failed!\n", __func__);
        return -1;
    }
    // first close epfd ,when bBlock is true
    close(Hdl->epfd);

    // destroy timer thread
    if (Hdl->timerTskId != 0) {
        Hdl->bTimerRun = false;
        pthread_cancel(Hdl->timerTskId);
        pthread_join(Hdl->timerTskId, NULL);
    }

    pthread_mutex_destroy(&Hdl->timerMutex);
    free(Hdl);
    Hdl = NULL;
    printf("%s success\n", __func__);

    return 0;
}

CVI_TIMER_HANDLE_T CVI_Timer_Create(int32_t grpHdl, CVI_TIMER_S *timerConf)
{
    int32_t ret = 0;
    struct epoll_event event;
    CVI_TIMER_INFO_P Hdl = NULL;
    if (timerConf == NULL || grpHdl < 0) {
        printf("%s param illegal!\n", __func__);
        return NULL;
    }
    ret = CVI_Timer_GetTimerGrpHdl(&Hdl, grpHdl);
    if (ret != 0) {
        printf("%s failed!\n", __func__);
        return NULL;
    }

    CVI_TIMER_P TimerHdl = (CVI_TIMER_P)calloc(sizeof(CVI_TIMER), 1);

    //创建timerfd， CLOCK_MONOTONIC单调时间，TFD_NONBLOCK为非阻塞
    TimerHdl->tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (TimerHdl->tfd < 0) {
	    printf("timerfd_create error!");
        free(TimerHdl);
        return NULL;
    }
    memcpy(&TimerHdl->timerConf, timerConf, sizeof(CVI_TIMER_S));

    struct timespec startTime, intervalTime;
    uint64_t nsec = ((uint64_t)TimerHdl->timerConf.interval_ms)*1000*1000;
    startTime.tv_sec = (nsec/1000000000LL);
    startTime.tv_nsec = nsec%1000000000LL;
    if (TimerHdl->timerConf.periodic == true) {
        intervalTime.tv_sec = (nsec/1000000000LL);
        intervalTime.tv_nsec = nsec%1000000000LL;
    } else {
        intervalTime.tv_sec = 0;
        intervalTime.tv_nsec = 0;
    }
    #if DEBUG_LOG
    printf("intervalTime.tv_sec ===== (%"PRId64") s,(%"PRId64")ns\n", (nsec/1000000000LL), nsec%1000000000LL);
    #endif
    struct itimerspec newValue;
    newValue.it_value = startTime;
    newValue.it_interval = intervalTime;

    //第二个参数为 0是相对时间，为1是绝对时间
    if (timerfd_settime(TimerHdl->tfd, 0, &newValue, NULL) < 0) {
        printf("timerfd_settime error!");
        close(TimerHdl->tfd);
        free(TimerHdl);
        return NULL;
    }

    event.data.ptr = (CVI_TIMER_HANDLE_T)TimerHdl;
    event.events = EPOLLIN;
    pthread_mutex_lock(&Hdl->timerMutex);
    if (epoll_ctl(Hdl->epfd, EPOLL_CTL_ADD, TimerHdl->tfd, &event) < 0) {
        printf("epoll_ctl error!");
        close(TimerHdl->tfd);
        free(TimerHdl);
        pthread_mutex_unlock(&Hdl->timerMutex);
        return NULL;
    }

    for (int32_t i = 0; i < EPOLL_SIZE; i++) {
        if (Hdl->timerHdl[i] == NULL) {
            Hdl->timerHdl[i] = TimerHdl;
            break;
        }
    }
    pthread_mutex_unlock(&Hdl->timerMutex);

    return (CVI_TIMER_HANDLE_T)TimerHdl;
}

int32_t CVI_Timer_Destroy(int32_t grpHdl, CVI_TIMER_HANDLE_T tmrHdle)
{
    int32_t ret = 0, i = 0;
    struct epoll_event event;
    CVI_TIMER_INFO_P Hdl = NULL;
    CVI_TIMER_P TimerHdl = NULL;
    if (tmrHdle == NULL || grpHdl < 0) {
        printf("%s param illegal!\n", __func__);
        return -1;
    }
    ret = CVI_Timer_GetTimerGrpHdl(&Hdl, grpHdl);
    if (ret != 0) {
        printf("%s failed!\n", __func__);
        return -1;
    }
    TimerHdl = (CVI_TIMER_P)tmrHdle;
    event.data.ptr = (CVI_TIMER_HANDLE_T)TimerHdl;
    event.events = EPOLLIN;
    pthread_mutex_lock(&Hdl->timerMutex);
    if (epoll_ctl(Hdl->epfd, EPOLL_CTL_DEL, TimerHdl->tfd, &event) < 0) {
        printf("epoll_ctl error!\n");
        pthread_mutex_unlock(&Hdl->timerMutex);
        return -1;
    }
    for (i = 0; i < EPOLL_SIZE; i++) {
        if (Hdl->timerHdl[i] == TimerHdl) {
            break;
        }
    }
    close(TimerHdl->tfd);
    free(TimerHdl);
    TimerHdl = NULL;
    Hdl->timerHdl[i] = NULL;
    pthread_mutex_unlock(&Hdl->timerMutex);
    return 0;
}

int32_t CVI_Timer_Reset(int32_t grpHdl, CVI_TIMER_HANDLE_T tmrHdle, struct timespec *timeVal, uint32_t timeLen)
{
    int32_t ret = 0;
    (void)timeVal;
    struct epoll_event event;
    CVI_TIMER_INFO_P Hdl = NULL;
    CVI_TIMER_P TimerHdl = (CVI_TIMER_P)tmrHdle;
    ret = CVI_Timer_GetTimerGrpHdl(&Hdl, grpHdl);
    if (ret != 0) {
        printf("%s failed!\n", __func__);
        return -1;
    }
    struct timespec startTime, intervalTime;
    uint64_t nsec = ((uint64_t)timeLen)*1000*1000;
    startTime.tv_sec = (nsec/1000000000LL);
    startTime.tv_nsec = nsec%1000000000LL;
    if (TimerHdl->timerConf.periodic == true) {
        intervalTime.tv_sec = (nsec/1000000000LL);
        intervalTime.tv_nsec = nsec%1000000000LL;
    } else {
        intervalTime.tv_sec = 0;
        intervalTime.tv_nsec = 0;
    }
    #if DEBUG_LOG
    printf("intervalTime.tv_sec ===== (%"PRId64") s,(%"PRId64")ns\n", (nsec/1000000000LL), nsec%1000000000LL);
    #endif
    struct itimerspec newValue;
    newValue.it_value = startTime;
    newValue.it_interval = intervalTime;

    //第二个参数为 0是相对时间，为1是绝对时间
    if (timerfd_settime(TimerHdl->tfd, 0, &newValue, NULL) < 0) {
        printf("timerfd_settime error!");
        return -1;
    }

    event.data.ptr = (CVI_TIMER_HANDLE_T)TimerHdl;
    event.events = EPOLLIN;
    if (epoll_ctl(Hdl->epfd, EPOLL_CTL_MOD, TimerHdl->tfd, &event) < 0) {
        printf("epoll_ctl error!");
        return -1;
    }

    return 0;
}

int32_t CVI_Timer_SetTickValue(int32_t grpHdl, uint32_t u32TickVal_us)
{
    int32_t ret = 0;
    CVI_TIMER_INFO_P Hdl = NULL;
    ret = CVI_Timer_GetTimerGrpHdl(&Hdl, grpHdl);
    if (ret != 0) {
        printf("%s failed!\n", __func__);
        return -1;
    }
    Hdl->timeOut = u32TickVal_us/1000; //ms

    return 0;
}

int32_t CVI_Timer_CleanUp(int32_t grpHdl)
{
    int32_t ret = 0;
    CVI_TIMER_INFO_P Hdl = NULL;
    ret = CVI_Timer_GetTimerGrpHdl(&Hdl, grpHdl);
    if (ret != 0) {
        printf("%s failed!\n", __func__);
        return -1;
    }
    for (int32_t i = 0; i < EPOLL_SIZE; i++) {
        if(Hdl->timerHdl[i] != NULL) {
            CVI_TIMER_P TimerHdl =  (CVI_TIMER_P)Hdl->timerHdl[i];
            if (epoll_ctl(Hdl->epfd, EPOLL_CTL_DEL, TimerHdl->tfd, NULL) < 0) {
                printf("epoll_ctl error!\n");
                return -1;
            }
            close(TimerHdl->tfd);
            free(TimerHdl);
            Hdl->timerHdl[i] = NULL;
        }
    }
    return 0;
}

// int32_t CVI_Timer_SetPeriodicAttr(CVI_TIMER_HANDLE_T tmrHdle, bool periodic)
// {
//     return 0;
// }

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */