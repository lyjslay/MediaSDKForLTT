#include "onvif_event_manager.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include "onvif_debug.h"


char* gOnvifEventTopics[] =
{
#if 1
    //md alarm
    "<tns1:RuleEngine wstop:topic=\"true\">"
    "<CellMotionDetector wstop:topic=\"true\">"
    "<Motion wstop:topic=\"true\">"
    "<tt:MessageDescription IsProperty=\"true\">"
    "<tt:Source>"
    "<tt:SimpleItemDescription Name=\"VideoSourceConfigurationToken\""
    " Type=\"tt:ReferenceToken\"/>"
    "<tt:SimpleItemDescription Name=\"VideoAnalyticsConfigurationToken\" "
    "Type=\"tt:ReferenceToken\"/>"
    "<tt:SimpleItemDescription Name=\"Rule\" Type=\"xs:string\"/>"
    "</tt:Source>"
    "<tt:Data>"
    "<tt:SimpleItemDescription Name=\"IsMotion\" Type=\"xs:boolean\"/>"
    "</tt:Data>"
    "</tt:MessageDescription>"
    "</Motion>"
    "</CellMotionDetector>"
    "<LineDetector wstop:topic=\"true\">\
    <Crossed wstop:topic=\"true\">\
    <tt:MessageDescription IsProperty=\"true\">\
    <tt:Source>\
    <tt:SimpleItemDescription Name=\"VideoSourceConfigurationToken\"\
    Type=\"tt:ReferenceToken\" />\
    <tt:SimpleItemDescription Name=\"VideoAnalyticsConfigurationToken\"\
    Type=\"tt:ReferenceToken\" />\
    <tt:SimpleItemDescription Name=\"Rule\" Type=\"xs:string\" />\
    </tt:Source>\
    <tt:Data>\
    <tt:SimpleItemDescription Name=\"ObjectId\" Type=\"xs:integer\" />\
    </tt:Data>\
    </tt:MessageDescription>\
    </Crossed>\
    </LineDetector>"
    "</tns1:RuleEngine>",
#endif

#if 0
    //md alarm
    "<tns1:RuleEngine>"
    "<LineDetector>"
    "<Crossed wstop:topic=\"true\">"
    "<tns1:Motion wstop:topic=\"true\">"
    "<tt:MessageDescription>"
    "<tt:Source>"
    "<tt:SimpleItemDescription Name=\"VideoSourceConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
    "<tt:SimpleItemDescription Name=\"VideoAnalyticsConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
    "</tt:Source>"
    "<tt:Data>"
    "<tt:SimpleItemDescription Name=\"ObjectId\" Type=\"xs:integer\"/>"
    "</tt:Data>"
    "</tt:MessageDescription>"
    "</tns1:Motion>"
    "</tns1:Crossed>"
    "</LineDetector>"
    "</tns1:RuleEngine>",
#endif
#if 0
    //od alarm
    "<tns1:RuleEngine wstop:topic=\"true\">"
    "<tns1:TamperDetector wstop:topic=\"true\">"
    "<tns1:Tamper wstop:topic=\"true\">"
    "<tt:MessageDescription IsProperty=\"true\">"
    "<tt:Source>"
    "<tt:SimpleItemDescription Name=\"VideoSourceConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
    "<tt:SimpleItemDescription Name=\"VideoAnalyticsConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
    "<tt:SimpleItemDescription Name=\"Rule\" Type=\"xs:string\"/>"
    "</tt:Source>"
    "<tt:Data>"
    "<tt:SimpleItemDescription Name=\"IsTamper\" Type=\"xs:boolean\"/>"
    "</tt:Data>"
    "</tt:MessageDescription>"
    "</tns1:Tamper>"
    "</tns1:TamperDetector>"
    "</tns1:RuleEngine>",
#endif

#if 1
    //io alarm
    "<tns1:Device wstop:topic=\"true\">"
    "<Trigger wstop:topic=\"true\">"
    "<DigitalInput wstop:topic=\"true\">"
    "<tt:MessageDescription IsProperty=\"true\">"
    "<tt:Source>"
    "<tt:SimpleItemDescription Name=\"IOInputToken\" Type=\"tt:ReferenceToken\"/>"
    "</tt:Source>"
    "<tt:Data>"
    "<tt:SimpleItemDescription Name=\"LogicalState\" Type=\"xs:boolean\"/>"
    "</tt:Data>"
    "</tt:MessageDescription>"
    "</DigitalInput>"
    "</Trigger>"
    "</tns1:Device>"
#endif
};


OnvifEventTopicNS_S gOnvifEventTopicNS;
OnvifEventTopicSetList_S gOnvifEventTopicSet;
OnvifEventTopicExpDia_S gOnvifEventTED;
OnvifEventMCFD_S gOnvifEventMCFD;
OnvifEventMCSL_S gOnvifEventMCSL;
static OnvifEventManager_S gOnvifEventManager;

static OnvifEventSubscriber_S *onvif_event_find_sub(const char *addr)
{
    OnvifEventSubscriber_S *pSubscribe = NULL;
    if(addr == NULL) {
        log_war("addr is NULL");
        goto error;
    }

    if(gOnvifEventManager.exit == 1) {
        log_err("manager is exit");
        goto error;
    }

    if(gOnvifEventManager.head == NULL) {
        log_err("head is NULL");
        goto error;
    }

    pthread_mutex_lock(&gOnvifEventManager.mutex);
    pSubscribe = gOnvifEventManager.head;
    while(1) {
        if(pSubscribe == NULL) {
            break;
        } 
        pSubscribe = pSubscribe->next;
    }
    pthread_mutex_unlock(&gOnvifEventManager.mutex);
    return pSubscribe;
error:
    return NULL;
}


static int onvif_event_delete_sub(int id)
{
    if(id <= 0) {
        log_war("id <= 0");
        return -1;
    }
    OnvifEventSubscriber_S *pSubscribe = gOnvifEventManager.head;
    OnvifEventSubscriber_S *pre = NULL;
    if(pSubscribe->id == id) {
        log_inf("delete current head");
        gOnvifEventManager.head = gOnvifEventManager.head->next;
    } else {
        while(pSubscribe) {
            pre = pSubscribe;
            pSubscribe = pSubscribe->next;
            if((pSubscribe == NULL) || pSubscribe->id == id) {
                log_inf("got sub: %d", pSubscribe->id);
                break;
            }
        }
        if(pSubscribe == NULL) {
            log_err("can not find sub: %d", id);
            return -1;
        } else {
            pre->next = pSubscribe->next;
        }
    }
    // delete
    if(pSubscribe != NULL) {
        free(pSubscribe);
    }
    gOnvifEventManager.num--;
}

static void *run(void *param)
{
    log_inf("start");
    OnvifEventSubscriber_S *pSubscribe = gOnvifEventManager.head;
    while(!gOnvifEventManager.exit) {
        pthread_mutex_lock(&gOnvifEventManager.mutex);

        while(pSubscribe) {
            pSubscribe->tick--;
            if(1) {
                // if have no warning , do nothing
            } else {
                // if have warning, notify the client, TODO
            }

            if(pSubscribe->tick <= 0) {
                log_inf("sub: %d tick: %d, will be delet", pSubscribe->id, pSubscribe->tick);
                onvif_event_delete_sub(pSubscribe->id);
            }
        }
        pthread_mutex_unlock(&gOnvifEventManager.mutex);
        sleep(1);
    }
    log_inf("exit");
    return 0;
}

static int onvif_event_manager_start()
{
    int ret = 0;
    gOnvifEventManager.exit = 0;
    pthread_mutex_init(&gOnvifEventManager.mutex, NULL);
    ret = pthread_create(&gOnvifEventManager.pid, NULL, run, NULL);
    if(ret != 0) {
        perror("pthread_create error: ");
        return -1;
    }
    return 0;
}


int onvif_event_manager_init()
{
    // init topic namespaces
    log_inf("enter");
    gOnvifEventTopicNS.size = ONVIF_EVENT_TOPIC_SIZE;
    gOnvifEventTopicNS.spaces = (char **)malloc(ONVIF_EVENT_TOPIC_SIZE * sizeof(char *));
    if(gOnvifEventTopicNS.spaces == NULL) {
        printf("Error: malloc spaces failed\n");
        goto error;
    }
    gOnvifEventTopicNS.spaces[0] = (char *)malloc(ONVIF_EVENT_ADDR_LEN);
    strncpy(gOnvifEventTopicNS.spaces[0], ONVIF_EVENT_TOPIC_NS, ONVIF_EVENT_ADDR_LEN);

    // init expression
    gOnvifEventTED.size = ONVIF_EVENT_EXP_SIZE;
    gOnvifEventTED.expressions = (char **)malloc(ONVIF_EVENT_EXP_SIZE * sizeof(char *));
    if(gOnvifEventTED.expressions == NULL) {
        printf("Error: malloc expressions failed\n");
        goto error;
    }
    gOnvifEventTED.expressions[0] = (char *)malloc(ONVIF_EVENT_ADDR_LEN);
    gOnvifEventTED.expressions[1] = (char *)malloc(ONVIF_EVENT_ADDR_LEN);
    strncpy(gOnvifEventTED.expressions[0], ONVIF_EVENT_EXPRESSION1, ONVIF_EVENT_ADDR_LEN);
    strncpy(gOnvifEventTED.expressions[1], ONVIF_EVENT_EXPRESSION2, ONVIF_EVENT_ADDR_LEN);

    // int filters
    gOnvifEventMCFD.size = ONVIF_EVENT_MCFD_SIZE;
    gOnvifEventMCFD.filters = (char **)malloc(ONVIF_EVENT_MCFD_SIZE * sizeof(char *));
    if(gOnvifEventMCFD.filters == NULL) {
        printf("Error: malloc filters failed\n");
        goto error;
    }
    gOnvifEventMCFD.filters[0] = (char *)malloc(ONVIF_EVENT_ADDR_LEN);
    strncpy(gOnvifEventMCFD.filters[0], ONVIF_EVENT_MCFD1, ONVIF_EVENT_ADDR_LEN);

    // init schemas
    gOnvifEventMCSL.size = ONVIF_EVENT_MCSL_SIZE;
    gOnvifEventMCSL.schemas = (char **)malloc(ONVIF_EVENT_MCSL_SIZE * sizeof(char *));
    if(gOnvifEventMCSL.schemas == NULL) {
        printf("Error: malloc schemas failed\n");
        goto error;
    }
    gOnvifEventMCSL.schemas[0] = (char *)malloc(ONVIF_EVENT_ADDR_LEN);
    strncpy(gOnvifEventMCSL.schemas[0], ONVIF_EVENT_MCSL1, ONVIF_EVENT_ADDR_LEN);

    // init topic sets
    gOnvifEventTopicSet.size = ONVIF_EVENT_TS_SIZE;
    gOnvifEventTopicSet.topics = 
        (OnvifEventTopicSet_S *)malloc(ONVIF_EVENT_TS_SIZE * sizeof(OnvifEventTopicSet_S));
    memset(gOnvifEventTopicSet.topics, 0, sizeof(OnvifEventTopicSet_S)*ONVIF_EVENT_TS_SIZE);
    strncpy(gOnvifEventTopicSet.topics[0].topic, "tt:MotionAlarm", ONVIF_EVENT_ITEM_LEN - 1);
    strncpy(gOnvifEventTopicSet.topics[0].source.name, "VideoSourceToken", ONVIF_EVENT_ITEM_LEN - 1);
    strncpy(gOnvifEventTopicSet.topics[0].source.type, "tt:ReferenceToken", ONVIF_EVENT_ITEM_LEN - 1);
    strncpy(gOnvifEventTopicSet.topics[0].data.name, "State", ONVIF_EVENT_ITEM_LEN - 1);
    strncpy(gOnvifEventTopicSet.topics[0].data.type, "xsd:boolean", ONVIF_EVENT_ITEM_LEN - 1);
    strncpy(gOnvifEventTopicSet.topics[0].key.name, "any", ONVIF_EVENT_ITEM_LEN - 1);
    strncpy(gOnvifEventTopicSet.topics[0].key.type, "any", ONVIF_EVENT_ITEM_LEN - 1);
    memcpy(gOnvifEventTopicSet.topics[1].topic, "tt:IoAlarm", ONVIF_EVENT_ITEM_LEN - 1);
    memcpy(gOnvifEventTopicSet.topics[1].source.name, "Source", ONVIF_EVENT_ITEM_LEN - 1);
    memcpy(gOnvifEventTopicSet.topics[1].source.type, "tt:ReferenceToken", ONVIF_EVENT_ITEM_LEN - 1);
    memcpy(gOnvifEventTopicSet.topics[1].data.name, "State", ONVIF_EVENT_ITEM_LEN - 1);
    memcpy(gOnvifEventTopicSet.topics[1].data.type, "xsd:boolean", ONVIF_EVENT_ITEM_LEN - 1);
    memcpy(gOnvifEventTopicSet.topics[1].key.name, "any", ONVIF_EVENT_ITEM_LEN - 1);
    memcpy(gOnvifEventTopicSet.topics[1].key.type, "any", ONVIF_EVENT_ITEM_LEN - 1);

    onvif_event_manager_start();
    return 0;
error:
    return -1;
}


int onvif_event_new_sub(const char* addr, int timeout)
{
    int ret = 0;
    OnvifEventSubscriber_S *pSubscribe = NULL;

    pSubscribe = onvif_event_find_sub(addr);
    if(pSubscribe != NULL) {
        ret = pSubscribe->id;
        log_war("find the same sub, id: %d", ret);
        goto end;
    }

    pthread_mutex_lock(&gOnvifEventManager.mutex);
    gOnvifEventManager.num++;
    gOnvifEventManager.max_sub_num++;
    pSubscribe = (OnvifEventSubscriber_S*) malloc(sizeof(OnvifEventSubscriber_S));
    pSubscribe->id = gOnvifEventManager.max_sub_num;
    pSubscribe->tick = timeout; 
    pSubscribe->next = gOnvifEventManager.head;
    gOnvifEventManager.head = pSubscribe;
    ret = pSubscribe->id;
    pthread_mutex_unlock(&gOnvifEventManager.mutex);
    log_inf("pSubscribe id: %d tick: %d", pSubscribe->id, pSubscribe->tick);
end:
    return ret;
}


int parse_datetime_type(char *InitialTerminationTime) 
{
    log_inf("enter");
    char *buf = InitialTerminationTime;
    char date[128] = {0};
    char time[128] = {0};
    char buffer[128] = {0};
    char *p1, *p2;
    int e = 0;
    int Time;
    struct tm t;

    p2 = NULL;
    p2 = strchr(buf, 'T');

    if (p2 != NULL) {
        if (sscanf(buf, "%[^T]", date) == 0) {
            return 0;
        }
        // printf("date=%s\r\n", date);
        if (sscanf(p2, "%*[T]%s", time) == 0) {
            return 0;
        }
        // printf("time=%s\r\n", time);
    }

    if (date != NULL) {
        if (sscanf(date, "%[0-9]", buffer) != 0) {
            if (buffer != NULL) {
                t.tm_year = ((e = atoi(buffer)) - 1900);
            } else {
                return 0;
            }

            if (e < 1902 || e > 2037) {
                return 0;
            }

            // printf("Year =%d\r\n", e);
        } else {
            return 0;
        }

        p1 = NULL;
        p1 = strchr(date, '-');
        if (sscanf(p1, "%*[-]%[0-9]", buffer) != 0) {
            if (buffer != NULL) {
                t.tm_mon = (e = atoi(buffer));
            } else {
                return 0;
            }

            if (e < 0 || e > 11) {
                return 0;
            }
            // printf("Month =%d\r\n", e);
        } else {
            return 0;
        }

        p1 = strchr(p1 + 1, '-');
        if (sscanf(p1, "%*[-]%[0-9]", buffer) != 0) {
            if (buffer != NULL) {
                t.tm_mday = (e = atoi(buffer));
            } else {
                return 0;
            }

            if (e < 1 || e > 31) {
                return 0;
            }
            // printf("Day =%d\r\n", e);
        } else {
            return 0;
        }
    } else {
        return 0;
    }

    if (time != NULL) {
        if (sscanf(time, "%[0-9]", buffer) != 0) {
            if (buffer != NULL) {
                t.tm_hour = (e = atoi(buffer));
            } else {
                return 0;
            }

            if (e < 0 || e > 23) {
                return 0;
            }
            // printf("Hour =%d\r\n", e);
        } else {
            return 0;
        }

        p1 = NULL;
        p1 = strchr(time, ':');

        if (sscanf(p1, "%*[:]%[0-9]", buffer) != 0) {
            if (buffer != NULL) {
                t.tm_min = (e = atoi(buffer));
            } else {
                return 0;
            }

            if (e < 0 || e > 59) {
                return 0;
            }
            // printf("mintues =%d\r\n", e);
        } else {
            return 0;
        }

        p1 = strchr(p1 + 1, ':');
        if (sscanf(p1, "%*[:]%[0-9]", buffer) != 0) {
            if (buffer != NULL) {
                t.tm_sec = (e = atoi(buffer));
            } else {
                return 0;
            }
            if (e < 0 || e > 59) {
                return 0;
            }
            // printf("Second =%d\r\n", e);
        } else {
            return 0;
        }
    } else {
        return 0;
    }

    if ((Time = mktime(&t)) == -1) {
        return 0;
    } else {
        printf("All seconds = %d\r\n", Time);
        return Time;
    }
}

int parse_duration_type(char *InitialTerminationTime) 
{
    // P*Y*M*DT*H*M*S // char* buf = "P8DT2H1S";
    log_inf("enter");
    char *buf = InitialTerminationTime;
    char time[128] = {0};
    char buffer[128] = {0};
    char *p1, *p2, *p3, *p4;
    int e = 0;
    long Time = 0;
    p3 = NULL;
    p4 = NULL;
    p3 = strchr(buf, 'P');
    p4 = strchr(buf, 'T');

    if (p3 != NULL && p4 != NULL) {
        if (sscanf(p4, "%s", time) == 0) {
            return 0;
        }

        printf("time=%s\r\n", time);
    } else {
        return 0;
    }

    p1 = NULL;
    p2 = NULL;
    memset(buffer, 0, 128);
    p1 = strchr(time, 'T');

    if (p1 != NULL) {
        p2 = strchr(time, 'S');

        if (p2 != NULL) {
            if (sscanf(p1, "%*[T]%[0-9]", buffer) != 0) {
                if (buffer != NULL) {
                    Time = (e = atoi(buffer));
                } else {
                    return 0;
                }
            }
        }
    } else {
        return 0;
    }
    printf("All seconds = %d\r\n", Time);
    return Time;
}




