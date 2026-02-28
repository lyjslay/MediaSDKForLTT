#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "cvi_osal.h"
#include "cvi_mq.h"
//#define CVI_LOG_LEVEL CVI_LOG_VERBOSE
#include "cvi_log.h"

/* Used to retry syscalls that can return EINTR. */
#define TEMP_FAILURE_MQ_RETRY(exp) ({         \
    typeof (exp) _rc;                      \
    do {                                   \
        _rc = (exp);                       \
    } while (_rc == -1 && errno == EINTR); \
    _rc; })

struct CVI_MQ_ENDPOINT_S {
    CVI_MQ_ENDPOINT_CONFIG_S       config;

    cvi_osal_task_handle_t         recv_task;
    volatile bool                  exit;

    int                            socket_fd;
};
#define CVI_MQ_SOCKET_PATH         "/tmp/CVI_MQ"
#define CVI_MQ_SOCKET_CLIENT_PATH         "/tmp/CVI_MQCLIENT"
static int mq_create_socket(CVI_MQ_ENDPOINT_HANDLE_t ep)
{
    struct sockaddr_un un;
    int addr_len = sizeof(un);

    int socket_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        CVI_LOGE("[MQ] create socket fail, ret = %d, errno = %d\n", socket_fd, errno);
        return CVI_MQ_ERR_FAILURE;
    }
    //int recvBuff = CVI_MQ_QUEUE_SIZE * sizeof(struct CVI_MQ_MSG_s);
    //setsockopt(socket_fd , SOL_SOCKET, SO_RCVBUF, &recvBuff, sizeof(int));

    memset(&un, 0, sizeof(un));
    un.sun_family = AF_LOCAL;
    snprintf(un.sun_path, sizeof(un.sun_path), CVI_MQ_SOCKET_PATH"%08x", ep->config.id);
    unlink(un.sun_path);

    if (bind(socket_fd, (struct sockaddr *)&un, addr_len) < 0) {
        CVI_LOGE("[MQ] bind socket fail, errno = %d, addr = %s", errno, un.sun_path);
        close(socket_fd);
        return CVI_MQ_ERR_FAILURE;
    }
    ep->socket_fd = socket_fd;
    CVI_LOGD("[MQ] Socket %d bind to %s\n", ep->socket_fd, un.sun_path);
    return CVI_MQ_SUCCESS;
}

static int mq_destroy_socket(CVI_MQ_ENDPOINT_HANDLE_t ep)
{
    close(ep->socket_fd);
    return CVI_MQ_SUCCESS;
}

static void mq_recv_worker(void *arg)
{
    CVI_MQ_ENDPOINT_HANDLE_t ep = arg;

    // for debug purpose
    prctl(PR_SET_NAME, (unsigned long int)"MQ_Recv_Task", 0, 0, 0);

    CVI_LOGD("[MQ] Recv Task: start id=0x%x\n", ep->config.id);

    while (ep->exit != true) {
        #define MQ_LOOP_TIMEOUT_MS      200
        struct timeval timeout;
        fd_set fds;
        int ret;
        char *dptr;
        int16_t client_id;
        CVI_MQ_MSG_S msg = {0};
        size_t buf_len = sizeof(CVI_MQ_MSG_S);
        ssize_t recv_len = 0;
        struct sockaddr_un remoteClient;
        socklen_t addr_len = sizeof(remoteClient);
        timeout.tv_sec  = MQ_LOOP_TIMEOUT_MS / 1000;
        timeout.tv_usec = MQ_LOOP_TIMEOUT_MS * 1000;

        FD_ZERO(&fds);
        FD_SET(ep->socket_fd, &fds);

        ret = select(ep->socket_fd + 1, &fds, NULL, NULL, &timeout);
        if (ret > 0 && FD_ISSET(ep->socket_fd, &fds)) {
            recv_len = recvfrom(ep->socket_fd, (void *)&msg, buf_len, MSG_WAITALL, (struct sockaddr *)&remoteClient, &addr_len);
            if(recv_len > 0){
                if(msg.needack) {
                    dptr = &remoteClient.sun_path[17];
                    client_id = atoi(dptr);
                    msg.client_id = client_id;
                }
                ret = ep->config.recv_cb(ep, &msg, ep->config.recv_cb_arg);
                if (ret != CVI_MQ_SUCCESS) {
                    CVI_LOGF("[MQ] recv cb return err %d\n", ret);
                    // break;
                }
            }
        } else if (ret < 0) {
            CVI_LOGE("[MQ[ Select fail!\n");
        }
        cvi_osal_task_sleep(20 * 1000);
    }

    CVI_LOGD("[MQ] Recv Task: exit id=0x%x\n", ep->config.id);
}

int CVI_MQ_CreateEndpoint(CVI_MQ_ENDPOINT_CONFIG_S *config, CVI_MQ_ENDPOINT_HANDLE_t *ep)
{
    CVI_MQ_ENDPOINT_HANDLE_t tep;
    cvi_osal_task_attr_t ta;
    int rc;

    // CVI_LOGD("[MQ] create ep id=0x%x\n", config->id);

    // Allocate route table together with client object
    tep = calloc(1, sizeof(CVI_MQ_ENDPOINT_t));
    if (tep == NULL) {
        CVI_LOGE("[MQ]: create ep alloc fail, errno = %d\n", errno);
        return CVI_MQ_ERR_NOMEM;
    }
    memcpy(&tep->config, config, sizeof(CVI_MQ_ENDPOINT_CONFIG_S));

    // create socket
    rc = mq_create_socket(tep);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("[MQ]: create ep create socket fail, rc = %d\n", rc);
        rc = CVI_MQ_ERR_FAILURE;
        goto err_socket;
    }

    // Create recv task
    if (config->recv_cb) {
        tep->exit = false;
        ta.name = "MQ Task";
        ta.entry = mq_recv_worker;
        ta.param = tep;
        ta.priority = CVI_OSAL_PRI_NORMAL;
        ta.detached = false;
        rc = cvi_osal_task_create(&ta, &tep->recv_task);
        if (rc != CVI_OSAL_SUCCESS) {
            CVI_LOGE("[MQ]: create ep create task fail, rc = %d\n", rc);
            rc = CVI_MQ_ERR_FAILURE;
            goto err_task;
        }
    } else {
        CVI_LOGD("[MQ] ep id=0x%x for TX only\n", config->id);
    }

    *ep= tep;
    return rc;

err_task:
    mq_destroy_socket(tep);
err_socket:
    free(tep);

    return rc;
}

int CVI_MQ_DestroyEndpoint(
	CVI_MQ_ENDPOINT_HANDLE_t       ep)
{
    ep->exit = true;
    cvi_osal_task_join(ep->recv_task);
    cvi_osal_task_destroy(&ep->recv_task);
    mq_destroy_socket(ep);
    free(ep);
    return CVI_MQ_SUCCESS;
}

int CVI_MQ_SendRAW(CVI_MQ_MSG_S *msg)
{
    struct sockaddr_un un;
    int addr_len, ret, rc = CVI_MQ_SUCCESS;
    int socket_fd;
    memset(&un, 0, sizeof(un));
    un.sun_family = AF_LOCAL;
    snprintf(un.sun_path, sizeof(un.sun_path), CVI_MQ_SOCKET_PATH"%08x", msg->target_id);
    if(access(un.sun_path, F_OK) != 0){
        CVI_LOGE("[MQ] no such file %s, errno = %d\n", un.sun_path, errno);
        return CVI_MQ_ERR_FAILURE;
    }

    addr_len = sizeof(un);

    /* Create local socket (no connection mode) */
    socket_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        CVI_LOGE("[MQ] create socket fail, ret = %d, errno = %d\n", socket_fd, errno);
        return CVI_MQ_ERR_FAILURE;
    }

    ret = sendto(socket_fd, (void*)msg, msg->len, MSG_DONTWAIT, (struct sockaddr *)&un, addr_len);
    if (ret < 0) {
        CVI_LOGE("[MQ] Send message fail, ret = %d, errno = %d, addr = %s", ret, errno, un.sun_path);
        if (errno == EAGAIN) {
            rc = CVI_MQ_ERR_AGAIN;
        } else {
            rc = CVI_MQ_ERR_FAILURE;
        }
    }
    cvi_osal_task_sleep(10 * 1000);
    close(socket_fd);

    return rc;
}

int CVI_MQ_Send(
    CVI_MQ_ID_t                    target_id,
    int32_t                        arg1,
    int32_t                        arg2,
    int16_t                        seq_no,
    char                          *payload,
    int16_t                        payload_len)
{
    CVI_MQ_MSG_S msg;

    if (payload_len > CVI_MQ_MSG_PAYLOAD_LEN) {
        return CVI_MQ_ERR_FAILURE;
    }
    msg.target_id = target_id;
    msg.arg1 = arg1;
    msg.arg2 = arg2;
    msg.seq_no = seq_no;
    msg.len = CVI_MQ_MSG_HEADER_LEN + payload_len;
    msg.needack = 0;
    uint64_t boot_time;
    cvi_osal_get_boot_time(&boot_time);
    msg.crete_time = boot_time;
    memcpy(msg.payload, payload, payload_len);

    return CVI_MQ_SendRAW(&msg);
}

int CVI_MQ_SendAck(CVI_MQ_ENDPOINT_HANDLE_t ep,
    MSG_ACK_t   *ack_msg, int16_t client_id) {
    struct sockaddr_un un;
    memset(&un, 0, sizeof(un));
    un.sun_family = AF_LOCAL;
    sprintf(un.sun_path, CVI_MQ_SOCKET_CLIENT_PATH"%d", client_id);

    int addr_len = sizeof(struct sockaddr_un);
    int ret = sendto(ep->socket_fd, (void*)ack_msg, sizeof(MSG_ACK_t), 0, (struct sockaddr*) &un, addr_len);
    if (ret < 0) {
        CVI_LOGE("[MQ] Send Ack fail, ret = %d, errno = %d",
                ret, errno);
        if (errno == EAGAIN) {
            ret = CVI_MQ_ERR_AGAIN;
        } else {
            ret = CVI_MQ_ERR_FAILURE;
        }
    } else if (ret == sizeof(MSG_ACK_t)) {
        ret = 0;
    }
    return ret;
}


int CVI_MQ_Send_RAW_ACK(
    CVI_MQ_MSG_S                  *msg,
    MSG_ACK_t                     *msg_ack,
    int16_t                        client_id,
    int16_t                       recv_timeout)
{
    struct sockaddr_un un,un_self;
    int addr_len, ret, rc = CVI_MQ_SUCCESS;
    int socket_fd;
    static int max_fd;
    /* Create local socket (no connection mode) */
    socket_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        CVI_LOGE("[MQ] create socket fail, ret = %d, errno = %d\n",
                socket_fd, errno);
        return CVI_MQ_ERR_FAILURE;
    }
    if(socket_fd > max_fd) {
        max_fd = socket_fd;
    }
    //int sendBuff = CVI_MQ_QUEUE_SIZE * sizeof(struct CVI_MQ_MSG_s);
    //setsockopt(socket_fd , SOL_SOCKET, SO_SNDBUF, &sendBuff, sizeof(int));
    memset(&un_self, 0, sizeof(un_self));
    un_self.sun_family = AF_LOCAL;
    sprintf(un_self.sun_path, CVI_MQ_SOCKET_CLIENT_PATH"%d", client_id);
	addr_len = sizeof(un_self);
    unlink(un_self.sun_path);
    ret = bind(socket_fd, (struct sockaddr *)&un_self, addr_len);
    if(ret < 0) {
        CVI_LOGE("[MQ] bind fail, ret = %d, errno = %d, addr = %s",
                ret, errno, un.sun_path);
    }
    memset(&un, 0, sizeof(un));
    un.sun_family = AF_LOCAL;
    sprintf(un.sun_path, CVI_MQ_SOCKET_PATH"%08x", msg->target_id);
	addr_len = sizeof(un);


    //CVI_LOGI_MEM(msg, msg->len, "TX MSG");
    CVI_LOGD("[MQ] TX %d bytes, to 0x%x\n", msg->len, msg->target_id);
    ret = sendto(socket_fd, (void*)msg, msg->len, 0, (struct sockaddr *)&un, addr_len);
    if (ret < 0) {
        CVI_LOGE("[MQ] Send message fail, ret = %d, errno = %d, addr = %s",
                ret, errno, un.sun_path);
        if (errno == EAGAIN) {
            rc = CVI_MQ_ERR_AGAIN;
        } else {
            rc = CVI_MQ_ERR_FAILURE;
        }
    }
    fd_set fds;
    struct timeval timeout;
    timeout.tv_sec  = recv_timeout / 1000;
    timeout.tv_usec = (recv_timeout - timeout.tv_sec * 1000 )* 1000;
    int16_t recv_len;
    FD_ZERO(&fds);
    FD_SET(socket_fd, &fds);
    printf("tv_sec = %ld tv_usec = %ld\n",timeout.tv_sec, timeout.tv_usec);
    ret = TEMP_FAILURE_MQ_RETRY(select(max_fd + 1, &fds, NULL, NULL, &timeout));
    if(ret > 0) {
        recv_len = recv(socket_fd, (void *)msg_ack, sizeof(MSG_ACK_t), MSG_WAITALL);
        CVI_LOGD("[MQ] Ack %d bytes\n", recv_len);
    } else if (ret < 0) {
        CVI_LOGE("[MQ[ Select fail! ret = %d errno = %d\n", ret, errno);
        msg_ack->status = ret;
    } else {
        msg_ack->status = -110;
        CVI_LOGE("[MQ[ Ack Timeout !\n");
    }

    close(socket_fd);

    return rc;
}


int CVI_MQ_SendNeedAck(
    CVI_MQ_ID_t                    target_id,
    int32_t                        arg1,
    int32_t                        arg2,
    int16_t                        seq_no,
    char                          *payload,
    int16_t                        payload_len,
    MSG_ACK_t                      *ack_msg,
    int16_t                         client_id,
    int32_t                         timeout_ms)
{
    CVI_MQ_MSG_S msg;

    if (payload_len > CVI_MQ_MSG_PAYLOAD_LEN) {
        return CVI_MQ_ERR_FAILURE;
    }
    msg.target_id = target_id;
    msg.arg1 = arg1;
    msg.arg2 = arg2;
    msg.seq_no = seq_no;
    msg.len = CVI_MQ_MSG_HEADER_LEN + payload_len;
    msg.needack = 1;
    uint64_t boot_time;
    cvi_osal_get_boot_time(&boot_time);
    msg.crete_time = boot_time;
    memcpy(msg.payload, payload, payload_len);

    return CVI_MQ_Send_RAW_ACK(&msg, ack_msg, client_id, timeout_ms);
}
