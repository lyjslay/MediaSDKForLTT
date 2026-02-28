#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>

#include "pubsub.h"
#include "uthash.h"
#include "utlist.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef uint32_t u32;
typedef struct ps_queue_s {
    ps_msg_t **messages;
    size_t size;
    size_t count;
    size_t head;
    size_t tail;
    pthread_mutex_t mux;
    pthread_cond_t not_empty;
} ps_queue_t;

typedef struct subscriber_list_s {
    ps_subscriber_t *su;
    struct subscriber_list_s *next;
    struct subscriber_list_s *prev;
} subscriber_list_t;

typedef struct topic_map_s {
    u32 topic;
    subscriber_list_t *subscribers;
    ps_msg_t *sticky;
    UT_hash_handle hh;
} topic_map_t;

typedef struct subscriptions_list_s {
    topic_map_t *tm;
    struct subscriptions_list_s *next;
    struct subscriptions_list_s *prev;
} subscriptions_list_t;

struct ps_subscriber_s {
    subscriptions_list_t *subs;
    ps_subscriber_t *next;
    int32_t (*new_msg_cb)(void *argv, ps_msg_t *msg);
    bool    sync;
};

static pthread_mutex_t lock;
static topic_map_t *topic_map = NULL;
static uint32_t stat_live_msg;
static uint32_t stat_live_subscribers;
ps_subscriber_t subs_list_head;

typedef struct pubsub_thread_mem_s {
    pthread_t PubsubTid;
    ps_subscriber_t *su;
    ps_msg_t Pubsubmsg;
} pubsub_thread_mem_t;

#define PORT_LOCK pthread_mutex_lock(&lock);
#define PORT_UNLOCK pthread_mutex_unlock(&lock);
static int32_t push_subscriber_queue(ps_subscriber_t *su, ps_msg_t *msg);

#define LOCK(mutex)                  \
    do {                                      \
        (void)pthread_mutex_lock(&mutex); \
    } while (0)
#define UNLOCK(mutex)                  \
    do {                                        \
        (void)pthread_mutex_unlock(&mutex); \
    } while (0)
#define DESTROY(mutex)                  \
    do {                                         \
        (void)pthread_mutex_destroy(&mutex); \
    } while (0)

#define SAFE_FREE(p) \
    do {                        \
        if (NULL != (p)) {      \
            free(p);            \
            (p) = NULL;         \
        }                       \
    } while (0)

typedef struct tagpubsubqueue_S {
    pthread_mutex_t mutex;
    int32_t frontIdx;
    int32_t rearIdx;
    int32_t curLen;
    int32_t maxLen;
    int32_t nodeSize;
    void **node;
} pubsubqueue_s;

typedef struct tagpubsubqueuenode_S {
    ps_msg_t msg;
    ps_subscriber_t *su;
} pubsubqueuenode_s;

typedef void * pubsubqueuehdl;

/*create queue and malloc memory*/
static pubsubqueuehdl pubsub_queue_create(uint32_t nodeSize, uint32_t maxLen)
{
    pubsubqueue_s *queue = malloc(sizeof(pubsubqueue_s));

    if (!queue) {
        printf("malloc queue failed! \n");
        return NULL;
    }

    int32_t i = 0;
    queue->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    queue->frontIdx = 0;
    queue->rearIdx = 0;
    queue->curLen = 0;
    queue->maxLen = maxLen;
    queue->nodeSize = nodeSize;
    queue->node = (void **)malloc(sizeof(void *) * maxLen);

    for (i = 0; i < queue->maxLen; i++) {
        queue->node[i] = (void *)malloc(nodeSize);
    }

    return (pubsubqueuehdl)queue;
}

static void pubsub_queue_destroy(pubsubqueuehdl queueHdl)
{
    if (queueHdl == 0) {
        printf("queueHdl is NULL!\n");
        return;
    }

    pubsubqueue_s *queue = (pubsubqueue_s *)queueHdl;
    LOCK(queue->mutex);
    int32_t i = 0;

    for (i = 0; i < queue->maxLen; i++) {
        SAFE_FREE(queue->node[i]);
    }

    SAFE_FREE(queue->node);
    UNLOCK(queue->mutex);
    DESTROY(queue->mutex);
    SAFE_FREE(queue);
}

static void pubsub_queue_clear(pubsubqueuehdl queueHdl)
{
    if (queueHdl == 0) {
        printf("queueHdl is NULL!\n");
        return;
    }

    pubsubqueue_s *queue = (pubsubqueue_s *)queueHdl;
    LOCK(queue->mutex);
    queue->curLen = 0;
    queue->frontIdx = 0;
    queue->rearIdx = 0;
    UNLOCK(queue->mutex);
}

static int32_t pubsub_queue_getlen(pubsubqueuehdl queueHdl)
{
    if (queueHdl == 0) {
        printf("queueHdl is NULL!\n");
        return -1;
    }

    pubsubqueue_s *queue = (pubsubqueue_s *)queueHdl;
    return (int32_t)queue->curLen;
}

static int32_t pubsub_queue_push(pubsubqueuehdl queueHdl, const void *node)
{
    if (queueHdl == 0) {
        printf("queueHdl is NULL!\n");
        return -1;
    }

    pubsubqueue_s *queue = (pubsubqueue_s *)queueHdl;
    LOCK(queue->mutex);

    if (queue->curLen >= queue->maxLen) {
        UNLOCK(queue->mutex);
        printf("queue is full!\n");
        return -2;
    }

    if (node) {
        memcpy(queue->node[queue->rearIdx], node, queue->nodeSize);
    }

    queue->curLen++;
    queue->rearIdx = (queue->rearIdx + 1) % queue->maxLen;
    UNLOCK(queue->mutex);
    return 0;
}

static int32_t pubsub_queue_pop(pubsubqueuehdl queueHdl, void *node)
{
    if (queueHdl == 0) {
        printf("queueHdl is NULL!\n");
        return -1;
    }

    pubsubqueue_s *queue = (pubsubqueue_s *)queueHdl;
    LOCK(queue->mutex);

    if (queue->curLen == 0) {
        UNLOCK(queue->mutex);
        printf("queue is empity!\n");
        return -1;
    }

    if (node) {
        memcpy(node, queue->node[queue->frontIdx], queue->nodeSize);
    }

    queue->curLen--;
    queue->frontIdx = (queue->frontIdx + 1) % queue->maxLen;
    UNLOCK(queue->mutex);
    return 0;
}

static int32_t inited = 0;
static pubsubqueuehdl squeueHdl = NULL;
static sem_t pubsubsem;
static pthread_t pubsubqueuepth;
static bool pubsubrun = false;

static void* pubsub_queue_proc(void* pvParam)
{
    int32_t s32Ret = 0;
    (void)pvParam;
    prctl(PR_SET_NAME, "pubsub", 0, 0, 0);

    while (pubsubrun) {
        while((0 != sem_wait(&pubsubsem)) && (errno == EINTR));
        if (0 == pubsub_queue_getlen(squeueHdl)) {
            continue;
        }
        pubsubqueuenode_s node = {0};
        s32Ret = pubsub_queue_pop(squeueHdl, &node);
        if (0 != s32Ret) {
            printf("Queue_Pop failed! \n");
            continue;
        }
        if (push_subscriber_queue(node.su, &(node.msg)) != 0) {
            printf("subscriber callback failed! \n");
        }
    }

    return NULL;
}

void ps_init(void) {
    if (inited == 0) {
        pthread_mutex_init(&lock, NULL);
        subs_list_head.next = NULL;
        inited = 1;
        pubsubrun = true;
        squeueHdl = pubsub_queue_create(sizeof(pubsubqueuenode_s), 64);
        if (squeueHdl == NULL) {
            printf("pubsub_queue_create failed!\n");
            return;
        }
        sem_init(&pubsubsem, 0, 0);
        pthread_attr_t thread_attr;
        pthread_attr_init(&thread_attr);
        pthread_attr_setstacksize(&thread_attr, (1024 * 512));
        if (0 != pthread_create(&pubsubqueuepth, &thread_attr, pubsub_queue_proc, NULL)) {
            printf("create async msg thread fail:%s\n", strerror(errno));
            pthread_attr_destroy(&thread_attr);
            return;
        }
        pthread_attr_destroy(&thread_attr);
    }
}

void ps_msg_set_topic(ps_msg_t *msg, const u32 topic) {
    if (msg != NULL) {
        msg->topic = topic;
    }
}

ps_msg_t *ps_ref_msg(ps_msg_t *msg) {
    return msg;
}

void ps_unref_msg(ps_msg_t *msg) {
    if (msg == NULL) return;
    __sync_sub_and_fetch(&stat_live_msg, 1);
}

int32_t ps_stats_live_msg(void) { return __sync_fetch_and_add(&stat_live_msg, 0); }

static int32_t free_topic_if_empty(topic_map_t *tm) {
    if (tm && tm->subscribers == NULL) {
        if (topic_map != NULL) HASH_DEL(topic_map, tm);
        if(tm)
            free(tm);
        tm = NULL;
    }
    return 0;
}
static topic_map_t *fetch_topic(const u32 topic) {
    topic_map_t *tm = NULL;
    HASH_FIND_INT(topic_map, &topic, tm);
    return tm;
}

static topic_map_t *create_topic(const u32 topic) {
    topic_map_t *tm = NULL;
    tm = calloc(1, sizeof(*tm));
    tm->topic = topic;
    HASH_ADD_KEYPTR(hh, topic_map, &tm->topic, sizeof(tm->topic), tm);
    return tm;
}

static topic_map_t *fetch_topic_create_if_not_exist(const u32 topic) {
    topic_map_t *tm = NULL;
    tm = fetch_topic(topic);
    if (tm == NULL) {
        tm = create_topic(topic);
    }
    return tm;
}

int32_t register_topic(const u32 topic) {
    int32_t ret = 0;
    PORT_LOCK
    if (fetch_topic_create_if_not_exist(topic) == NULL) {
        ret = -1;
    }
    PORT_UNLOCK
    return ret;
}

int32_t unregister_topic(const u32 topic) {
    int32_t ret = 0;
    topic_map_t *tm = NULL;
    PORT_LOCK
    tm = fetch_topic(topic);
    if (tm != NULL) {
        HASH_DEL(topic_map, tm);
        tm = NULL;
    }
    PORT_UNLOCK
    return ret;
}

ps_subscriber_t *to_ps_subscriber(void *argv) {
    return (ps_subscriber_t *)argv;
}

static int32_t push_subscriber_queue(ps_subscriber_t *su, ps_msg_t *msg) {
    if (su->new_msg_cb != NULL) {
        su->new_msg_cb((void *)su, msg);
    }

    return 0;
}

static void add_new_subscriber(ps_subscriber_t *su) {
    ps_subscriber_t *ptr = &subs_list_head;
    while (ptr->next != NULL) {
        ptr = ptr->next;
    }
    ptr->next = su;
}

static void list_del_subscriber(ps_subscriber_t *su) {
    ps_subscriber_t *ptr = &subs_list_head;
    ps_subscriber_t *ptr_next = ptr->next;
    while (ptr_next != NULL) {
        if (ptr_next == su) {
            ptr->next = ptr_next->next;
            break;
        }
        ptr = ptr_next;
        ptr_next = ptr_next->next;
    }
}

static void list_del_subscribers(void) {
    ps_subscriber_t *ptr = subs_list_head.next;
    ps_subscriber_t *ptr_next = ptr->next;
    while (ptr_next != NULL) {
        if (ptr != NULL) {
            free(ptr);
        }
        ptr = ptr_next;
        ptr_next = ptr->next;
    }
    if (ptr != NULL) {
        free(ptr);
    }
}
ps_subscriber_t *ps_create_subscriber(void) {
    ps_subscriber_t *su = calloc(1, sizeof(ps_subscriber_t));
    if (!su) return NULL;
    __sync_add_and_fetch(&stat_live_subscribers, 1);
    add_new_subscriber(su);

    return su;
}

void ps_free_subscriber(ps_subscriber_t *su) {
    ps_unsubscribe_all(su);
    list_del_subscriber(su);

    if (su) {
        free(su);
        su = NULL;
    }
    __sync_sub_and_fetch(&stat_live_subscribers, 1);
}
void ps_set_cb(ps_subscriber_t *su,
               int32_t (*new_msg_cb)(void *argv, ps_msg_t *msg)) {
    su->new_msg_cb = new_msg_cb;
}

void ps_set_sync(ps_subscriber_t *su, bool en) {
    su->sync = en;
}

void clean_all_subscribers(void) {
    inited = 0;
    pubsubrun = false;
    sem_post(&pubsubsem);
    sem_destroy(&pubsubsem);
    pthread_join(pubsubqueuepth, NULL);
    pubsub_queue_destroy(squeueHdl);
    list_del_subscribers();
}

int32_t ps_stats_live_subscribers(void) {
    return __sync_fetch_and_add(&stat_live_subscribers, 0);
}

// int32_t ps_subscribe_many(ps_subscriber_t *su, u32 *subs) {
//     int32_t n = 0;
//     size_t idx = 0;
//     for (idx = 0; idx < sizeof(subs) / sizeof(uint32_t); idx++) {
//         if (ps_subscribe(su, subs[idx]) == 0) n++;
//     }
//     return n;
// }

int32_t ps_subscribe(ps_subscriber_t *su, const u32 topic) {
    int32_t ret = 0;
    topic_map_t *tm;
    subscriber_list_t *sl;
    subscriptions_list_t *subs;

    PORT_LOCK
    tm = fetch_topic_create_if_not_exist(topic);
    DL_SEARCH_SCALAR(tm->subscribers, sl, su, su);
    if (sl != NULL) {
        ret = -1;
        goto exit_fn;
    }
    sl = calloc(1, sizeof(*sl));
    sl->su = su;
    DL_APPEND(tm->subscribers, sl);
    subs = calloc(1, sizeof(*subs));
    subs->tm = tm;
    DL_APPEND(su->subs, subs);

exit_fn:
    PORT_UNLOCK
    return ret;
}

int32_t ps_unsubscribe(ps_subscriber_t *su, const u32 topic) {
    int32_t ret = 0;
    topic_map_t *tm;
    subscriber_list_t *sl;
    subscriptions_list_t *subs;

    PORT_LOCK
    tm = fetch_topic(topic);
    if (tm == NULL) {
        ret = -1;
        goto exit_fn;
    }
    DL_SEARCH_SCALAR(tm->subscribers, sl, su, su);
    if (sl == NULL) {
        ret = -1;
        goto exit_fn;
    }
    DL_DELETE(tm->subscribers, sl);
    if (sl) {
        free(sl);
        sl = NULL;
    }
    free_topic_if_empty(tm);
    DL_SEARCH_SCALAR(su->subs, subs, tm, tm);
    if (subs != NULL) {
        DL_DELETE(su->subs, subs);
        free(subs);
        subs = NULL;
    }
exit_fn:
    PORT_UNLOCK
    return ret;
}

// int32_t ps_unsubscribe_many(ps_subscriber_t *su, u32 *subs) {
//     int32_t n = 0;

//     size_t idx = 0;
//     for (idx = 0; idx < sizeof(subs) / sizeof(uint32_t); idx++) {
//         if (ps_unsubscribe(su, subs[idx++]) == 0) n++;
//     }

//     return n;
// }

int32_t ps_unsubscribe_all(ps_subscriber_t *su) {
    subscriptions_list_t *s, *ps;
    subscriber_list_t *sl;
    size_t count = 0;

    PORT_LOCK
    s = su->subs;
    while (s != NULL) {
        if (s->tm) {
            DL_SEARCH_SCALAR(s->tm->subscribers, sl, su, su);
            if (sl != NULL) {
                DL_DELETE(s->tm->subscribers, sl);
                free(sl);
                sl = NULL;
                free_topic_if_empty(s->tm);
            }
        }
        ps = s;
        s = s->next;
        if (ps) {
            free(ps);
            ps = NULL;
        }
        count++;
    }
    su->subs = NULL;
    PORT_UNLOCK
    return count;
}

int32_t ps_num_subs(ps_subscriber_t *su) {
    int32_t count;
    subscriptions_list_t *elt;
    DL_COUNT(su->subs, elt, count);
    return count;
}

int32_t ps_get_topic_sticky_msg(u32 topic, ps_msg_t *msg) {
    topic_map_t *tm = NULL;
    int32_t ret = 0;
    PORT_LOCK
    tm = fetch_topic(topic);

    if (tm == NULL) {
        ret = -1;
    } else {
        if (tm->sticky != NULL) {
            memcpy(msg, tm->sticky, sizeof(ps_msg_t));
        } else {
            ret = -2;
        }
    }
    PORT_UNLOCK
    return ret;
}

ps_msg_t *ps_dup_msg(ps_msg_t *msg) {
    if (msg == NULL) return NULL;
    ps_msg_t *dmsg = calloc(1, sizeof(ps_msg_t));
    memcpy(dmsg, msg, sizeof(ps_msg_t));
    return dmsg;
}

int32_t ps_publish(ps_msg_t *msg)
{
    topic_map_t *tm = NULL;
    size_t ret = 0;
    subscriber_list_t *sl = NULL;
    if (!msg) return 0;

    u32 topic = msg->topic;

    PORT_LOCK
    tm = fetch_topic(topic);
    if (tm == NULL) {
        tm = create_topic(topic);
    }
    if (tm->sticky != NULL) {
        ps_unref_msg(tm->sticky);
    }
    tm->sticky = ps_ref_msg(msg);

    PORT_UNLOCK
    if (tm != NULL) {
        DL_FOREACH(tm->subscribers, sl) {
            if (sl->su->sync == true) {
                if (push_subscriber_queue(sl->su, msg) != 0) {
                    ret = -1;
                }
            } else {
                pubsubqueuenode_s node;
                memcpy(&node.msg, msg, sizeof(ps_msg_t));
                node.su = sl->su;
                ret = pubsub_queue_push(squeueHdl, &node);
                sem_post(&pubsubsem);
            }
        }
    }
    return ret;
}

int32_t ps_subs_count(uint32_t topic) {
    topic_map_t *tm = NULL;
    subscriber_list_t *sl = NULL;
    size_t count = 0;

    PORT_LOCK
    tm = fetch_topic(topic);
    if (tm != NULL) {
        DL_FOREACH(tm->subscribers, sl) { count++; }
    }
    PORT_UNLOCK
    return count;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
