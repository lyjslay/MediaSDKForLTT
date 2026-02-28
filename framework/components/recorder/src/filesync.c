#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "cvi_log.h"
#include "cvi_osal.h"
#include "filesync.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct FILE_LIST_S{
    char filename[256];
    CVI_FILESYNC_EVENT_CB_S cb;
    struct FILE_LIST_S *next;
}FILE_LIST_T;

typedef struct FILESYNC_INFO_S{
    FILE_LIST_T *file_list;
    cvi_osal_task_handle_t task;
    int32_t run;
}FILESYNC_INFO_S;

static FILESYNC_INFO_S *g_sync_info = NULL;
static pthread_mutex_t sync_mutex = PTHREAD_MUTEX_INITIALIZER;
static sem_t  sync_sem;

void CVI_FILESYNC_Push(char *filename, CVI_FILESYNC_EVENT_CB_S *cb){
    pthread_mutex_lock(&sync_mutex);
    if(g_sync_info && filename && strlen(filename) > 0){
        FILE_LIST_T *file_list = (FILE_LIST_T *)malloc(sizeof(FILE_LIST_T));
        memset(file_list, 0x0, sizeof(FILE_LIST_T));
        strncpy(file_list->filename, filename, sizeof(file_list->filename) - 1);
        if(cb){
            memcpy(&file_list->cb, cb, sizeof(CVI_FILESYNC_EVENT_CB_S));
        }
        if(g_sync_info->file_list == NULL){
            g_sync_info->file_list = file_list;
        }else{
            FILE_LIST_T *head = g_sync_info->file_list;
            while(head->next){
                head = head->next;
            }
            head->next = file_list;
        }
    }
    sem_post(&sync_sem);
    pthread_mutex_unlock(&sync_mutex);
}

static void CVI_FILESYNC_Pop(char *filename, CVI_FILESYNC_EVENT_CB_S *cb){
    pthread_mutex_lock(&sync_mutex);
    if(g_sync_info){
        if(g_sync_info->file_list != NULL){
            FILE_LIST_T *file_list = g_sync_info->file_list;
            g_sync_info->file_list = g_sync_info->file_list->next;
            strcpy(filename, file_list->filename);
            memcpy(cb, &file_list->cb, sizeof(CVI_FILESYNC_EVENT_CB_S));
            free(file_list);
        }
    }
    pthread_mutex_unlock(&sync_mutex);
}

static void fileSync_Task(void *arg){
    FILESYNC_INFO_S *sync = (FILESYNC_INFO_S *)arg;
    char filename[256];
    CVI_FILESYNC_EVENT_CB_S cb;
    while(sync->run){
        memset(filename, 0x0, sizeof(filename));
        memset(&cb, 0x0, sizeof(CVI_FILESYNC_EVENT_CB_S));
        while((0 != sem_wait(&sync_sem)) && (errno == EINTR));
        CVI_FILESYNC_Pop(filename, &cb);

        if(strlen(filename) > 0){
            int32_t fd = open(filename, O_NOATIME | O_NOCTTY | O_RDONLY, 0666);
            if(fd > 0){
                fdatasync(fd);
                close(fd);
                if(cb.cb && cb.hdl){
                    ((FILESYNC_EVENT_CALLBACK)cb.cb)(cb.hdl, filename, cb.event, cb.argv0, cb.argv1);
                }
                CVI_LOGI("%s sync finished", filename);
            }
        }else{
            usleep(50 * 1000);
        }
    }
}

int32_t CVI_FILESYNC_Init(void){
    pthread_mutex_lock(&sync_mutex);
    if(g_sync_info){
        pthread_mutex_unlock(&sync_mutex);
        return 0;
    }

    if(g_sync_info == NULL){
        g_sync_info = (FILESYNC_INFO_S *)malloc(sizeof(FILESYNC_INFO_S));
    }

    if(g_sync_info == NULL){
        pthread_mutex_unlock(&sync_mutex);
        CVI_LOGE("g_sync_info malloc falied");
        return -1;
    }
    pthread_mutex_unlock(&sync_mutex);

    memset(g_sync_info, 0x0, sizeof(FILESYNC_INFO_S));
    sem_init(&sync_sem, 0, 0);

    g_sync_info->run = 1;
    cvi_osal_task_attr_t ta;
    ta.name = "sync_file";
    ta.entry = fileSync_Task;
    ta.param = (void *)g_sync_info;
    ta.priority = CVI_OSAL_PRI_NORMAL;
    ta.detached = false;
    cvi_osal_task_create(&ta, &g_sync_info->task);

    return 0;
}

int32_t CVI_FILESYNC_Deinit(void){
    pthread_mutex_lock(&sync_mutex);
    // if(g_sync_info){
    //     while(g_sync_info->file_list){
    //         cvi_osal_task_sleep(100 * 1000);
    //     }
    //     g_task_flag = 1;
    //     cvi_osal_task_join(g_sync_info->task);
    //     cvi_osal_task_destroy(&g_sync_info->task);
    //     sem_destroy(&sync_sem);
    //     free(g_sync_info);
    //     g_sync_info = NULL;
    // }
    pthread_mutex_unlock(&sync_mutex);
    return 0;
}

#ifdef __cplusplus
}
#endif