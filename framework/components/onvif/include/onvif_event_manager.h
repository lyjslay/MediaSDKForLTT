#ifndef _ONVIF_EVENT_MANAGER_H_
#define _ONVIF_EVENT_MANAGER_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#include <pthread.h>


// topics
#define ONVIF_EVENT_TOPIC_SIZE  (1)
#define ONVIF_EVENT_TOPIC_NS    "http://www.onvif.org/onvif/ver10/topics/topicns.xml"
// topic sets
#define ONVIF_EVENT_TS_SIZE     (2)
// expression
#define ONVIF_EVENT_EXP_SIZE    (2)
#define ONVIF_EVENT_EXPRESSION1 "http://docs.oasis-open.org/wsn/t-1/TopicExpression/Concrete"
#define ONVIF_EVENT_EXPRESSION2 "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet"
// MessageContentFilterDialect
#define ONVIF_EVENT_MCFD_SIZE   (1)
#define ONVIF_EVENT_MCFD1       "http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter"
// MessageContentSchemaLocation
#define ONVIF_EVENT_MCSL_SIZE   (1)
#define ONVIF_EVENT_MCSL1       "http://www.onvif.org/onvif/ver10/schema/onvif.xsd"
// action 
#define ONVIF_EVENT_PULL_ACTION "http://www.onvif.org/ver10/events/wsdl/EventPortType/CreatePullPointSubscriptionResponse"
#define ONVIF_EVENT_GET_ACTION  "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesResponse"

// common
#define ONVIF_EVENT_ADDR_LEN    (128)
#define ONVIF_EVENT_ITEM_LEN    (32)

typedef struct _OnvifEventSubscriber_S
{
    int id;
    int tick;
    struct _OnvifEventSubscriber_S *next;
} OnvifEventSubscriber_S;


typedef struct _OnvifEventManager_S
{
    pthread_t pid;
    int exit;
    pthread_mutex_t mutex;
    int max_sub_num;
    int num;
    OnvifEventSubscriber_S *head;
} OnvifEventManager_S;

// topic simple item
typedef struct _OnvifEventSimpleItem_S
{
    char name[ONVIF_EVENT_ITEM_LEN];
    char type[ONVIF_EVENT_ITEM_LEN];
} OnvifEventSimpleItem_S;

typedef struct _OnvifEventTopicSet_S
{
    char topic[ONVIF_EVENT_ITEM_LEN];
    OnvifEventSimpleItem_S source;
    OnvifEventSimpleItem_S data;
    OnvifEventSimpleItem_S key;
} OnvifEventTopicSet_S;

typedef struct _OnvifEventTopicSetList_S
{
    int size;
    OnvifEventTopicSet_S *topics;
} OnvifEventTopicSetList_S;

//TopicNamespaceLocation
typedef struct _OnvifEventTopicNS_S
{
    int size;
    char **spaces;
} OnvifEventTopicNS_S;

typedef struct _OnvifEventTopicExpDia_S
{
    int size;
    char **expressions;
} OnvifEventTopicExpDia_S;

typedef struct _OnvifEventMCFD_S
{
    int size;
    char **filters;
} OnvifEventMCFD_S;

typedef struct _OnvifEventMCSL_S
{
    int size;
    char **schemas;
} OnvifEventMCSL_S;

extern char* gOnvifEventTopics[];
extern OnvifEventTopicSetList_S gOnvifEventTopicSet;
extern OnvifEventTopicNS_S gOnvifEventTopicNS;
extern OnvifEventTopicExpDia_S gOnvifEventTED;
extern OnvifEventMCFD_S gOnvifEventMCFD;
extern OnvifEventMCSL_S gOnvifEventMCSL;

int onvif_event_manager_init();
int onvif_event_new_sub(const char *addr, int timeout);
int parse_datetime_type(char *InitialTerminationTime);
int parse_duration_type(char *InitialTerminationTime);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif