/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ioctl implementatioin.
 * Author: Hisilicon
 * Create: 2020-09-28
 */
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/netlink.h>
#include <net/sock.h>
#include "wal_netlink.h"
#include "securec.h"
#include "oal_net.h"
#include "oal_netbuf.h"
#include "oam_ext_if.h"
#include "wal_cfg80211_apt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  2 枚举、结构体定义
*****************************************************************************/
typedef struct {
    struct sock *netlink_sk;
    hi_u32 user_pid;
}netlink_user_s;

/* kernel network namespace */
static hi_void wal_recieve_user_msg(struct sk_buff *skb);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
static struct netlink_kernel_cfg cfg = {
    .input  = wal_recieve_user_msg,
};
#endif
#ifdef _PRE_WLAN_FEATURE_PROMIS
hi_s32 wifi_promis_set(oal_net_device_stru *netdev, hi_u8 filter_value);
#endif
/*****************************************************************************
  3 宏定义、全局变量
*****************************************************************************/
#define NETLINK_CHANNEL_MODEID          28
#undef NLMSG_ALIGNTO
#define NLMSG_ALIGNTO                   1
#define NETLINK_IF_NAME                 16

typedef struct {
    hi_char ifname[NETLINK_IF_NAME + 1];
    hi_u8 enable;
}promis_mode;

static netlink_user_s g_netlink_user;
/*****************************************************************************
  4 函数实现
*****************************************************************************/
hi_s32 wal_send_user_msg(hi_u8 *pbuf, hi_u32 len)
{
    struct sk_buff *nl_skb = HI_NULL;
    struct nlmsghdr *nlh = HI_NULL;

    if (g_netlink_user.netlink_sk == HI_NULL) {
        return HI_FAIL;
    }

    if ((pbuf == HI_NULL) || (len == 0) || (len > MAX_USER_LONG_DATA_LEN)) {
        oam_error_log1(0, OAM_SF_ANY, "send_usrmsg is fail:len:%d\n", len);
        return HI_FAIL;
    }

    /* 创建sk_buff 空间 */
    nl_skb = nlmsg_new(len, GFP_ATOMIC);
    if (nl_skb == HI_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "nlmsg_new is fail.\n");
        return HI_FAIL;
    }

    /* 设置netlink消息头部 */
    nlh = nlmsg_put(nl_skb, 0, 0, NETLINK_CHANNEL_MODEID, len, 0);
    if (nlh == HI_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "nlmsg_put is fail.\n");
        nlmsg_free(nl_skb);
        return HI_FAIL;
    }

    /* 拷贝数据发送 */
    if (memcpy_s(nlmsg_data(nlh), nlmsg_len(nlh), pbuf, len) != EOK) {
        nlmsg_free(nl_skb);
        return HI_FAIL;
    }

    hi_s32 ret = netlink_unicast(g_netlink_user.netlink_sk, nl_skb, g_netlink_user.user_pid, MSG_DONTWAIT);
    if (ret == -1) {
        oam_error_log0(0, OAM_SF_ANY, "netlink_unicast is fail.\n");
        nlmsg_free(nl_skb);
        return HI_FAIL;
    }

    /* nlmsg_free(nl_skb); don`t free nl_skb */
    return HI_SUCCESS;
}

#ifdef _PRE_WLAN_FEATURE_PROMIS
static hi_u8 promis_enable = HI_FALSE;

hi_s32 wal_upload_frame_cb(hi_void* recv_buf, hi_s32 frame_len, hi_s8 rssi)
{
    return wal_send_user_msg(recv_buf, frame_len);
}

static hi_s32 wal_set_promis_status(hi_char *data, hi_u32 len)
{
    promis_mode *promis;
    hi_u8 filter_value;
    oal_net_device_stru *netdev = HI_NULL;
    hi_u8 enable;

    promis = (promis_mode *)data;

    if (len != sizeof(promis_mode)) {
        oam_error_log2(0, OAM_SF_ANY, "oal_netlink_init %d:%d is fail\n", len, sizeof(promis_mode));
        return HI_FAIL;
    }

    netdev = oal_get_netdev_by_name(promis->ifname);
    if (netdev == HI_NULL) {
        oam_error_log0(0, 0, "wal_recieve_user_msg device not fonud.");
        return HI_FAIL;
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_dev_put(netdev);
#endif

    enable = (promis->enable >> 4) & 0x1; /* 偏移4bit，取最后一位 */
    if (enable == HI_FALSE) {
        filter_value = 0;
        hisi_wlan_register_upload_frame_cb(HI_NULL);
        promis_enable = HI_FALSE;
    } else {
        filter_value = promis->enable & 0x0F;
        if (promis_enable != HI_TRUE) {
            hisi_wlan_register_upload_frame_cb(wal_upload_frame_cb);
            promis_enable = HI_TRUE;
        }
    }

    if (wifi_promis_set(netdev, filter_value) != HI_SUCCESS) {
        oam_error_log0(0, 0, "wifi_promis_set is fail.");
        return HI_FAIL;
    }

    return HI_SUCCESS;
}
#endif

static hi_void wal_recieve_user_msg(struct sk_buff *skb)
{
    hi_u32 payload_len;
    hi_char *umsg = HI_NULL;
    struct nlmsghdr *nlh = HI_NULL;

    if (skb == HI_NULL) {
        return;
    }

    if (skb->len <= nlmsg_total_size(0)) {
        oam_warning_log2(0, OAM_SF_ANY, "oal_netlink_deinit is success\n", skb->len, nlmsg_total_size(0));
        return;
    }

    nlh = nlmsg_hdr(skb);
    umsg = NLMSG_DATA(nlh);
    payload_len = nlh->nlmsg_len - NLMSG_HDRLEN; /* header len is nlmsg_total_size */
    g_netlink_user.user_pid = nlh->nlmsg_pid;

#ifdef _PRE_WLAN_FEATURE_PROMIS
    if (wal_set_promis_status(umsg, payload_len) != HI_SUCCESS) {
        oam_error_log0(0, 0, "wifi_promis_set is fail.");
    }
#endif
}

hi_s32 wal_netlink_init(hi_void)
{
    if (g_netlink_user.netlink_sk != HI_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "oal_netlink_init is fail\n");
        return HI_FAIL;
    }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22))
    g_netlink_user.netlink_sk = netlink_kernel_create(NETLINK_CHANNEL_MODEID, 0,
        wal_recieve_user_msg, THIS_MODULE);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    g_netlink_user.netlink_sk = netlink_kernel_create(NETLINK_CHANNEL_MODEID, 0,
        wal_recieve_user_msg, HI_NULL, THIS_MODULE);
#elif(LINUX_VERSION_CODE  < KERNEL_VERSION(3, 6, 0))
    g_netlink_user.netlink_sk = netlink_kernel_create(&init_net, NETLINK_CHANNEL_MODEID,
        0, wal_recieve_user_msg, HI_NULL, THIS_MODULE);
#else
#if(LINUX_VERSION_CODE  < KERNEL_VERSION(3, 7, 0))
    g_netlink_user.netlink_sk = netlink_kernel_create(&init_net, NETLINK_CHANNEL_MODEID, THIS_MODULE, &cfg);
#else
    g_netlink_user.netlink_sk = netlink_kernel_create(&init_net, NETLINK_CHANNEL_MODEID, &cfg);
#endif
#endif
    if (g_netlink_user.netlink_sk == HI_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "netlink_kernel_create is fail\n");
        return HI_FAIL;
    }

    oam_warning_log0(0, OAM_SF_ANY, "wal_netlink_init is success\n");
    return HI_SUCCESS;
}

hi_s32 wal_netlink_deinit(hi_void)
{
    if (g_netlink_user.netlink_sk == HI_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "oal_netlink_deinit is fail\n");
        return HI_FAIL;
    }

    netlink_kernel_release(g_netlink_user.netlink_sk); /* release. */
    g_netlink_user.netlink_sk = HI_NULL;
    g_netlink_user.user_pid = 0;
    oam_warning_log0(0, OAM_SF_ANY, "wal_netlink_deinit is success\n");
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
