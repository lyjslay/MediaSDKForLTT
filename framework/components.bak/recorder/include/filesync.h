
#ifndef CVI_FILE_SYNC_H
#define CVI_FILE_SYNC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*FILESYNC_EVENT_CALLBACK)(void *, char *, uint32_t, void *, uint32_t);

typedef struct cviFILESYNC_EVENT_CB_S{
    void *cb;
    void *hdl;
    uint32_t event;
    void *argv0;
    uint32_t argv1;
}CVI_FILESYNC_EVENT_CB_S;

void CVI_FILESYNC_Push(char *filename, CVI_FILESYNC_EVENT_CB_S *cb);
int32_t CVI_FILESYNC_Init(void);
int32_t CVI_FILESYNC_Deinit(void);

#ifdef __cplusplus
}
#endif
#endif