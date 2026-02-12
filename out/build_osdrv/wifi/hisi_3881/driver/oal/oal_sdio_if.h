/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: oal_sdio.c 的头文件
 * Author: Hisilicon
 * Create: 2018-08-04
 */
#ifndef __OAL_SDIO_IF_H__
#define __OAL_SDIO_IF_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifndef HAVE_PCLINT_CHECK
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/host.h>
#include <linux/pm_runtime.h>
#include <linux/random.h>
#endif
#include "oal_time.h"

#elif (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#if ((!defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(3,3,3)) || \
    (defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(5,0,1)))
#include <host.h>
#include <../core/sdio_func.h>
#include <los_mux.h>
#else
#include <mmc/host.h>
#include <mmc/sdio_func.h>
#include <mmc/sdio.h>
#include <mmc/card.h>
#include <gpio.h>
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/delay.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define sdio_get_max_block_count(func)  ((func)->card->host->max_blk_count)
#define sdio_get_max_req_size(func)     ((func)->card->host->max_req_size)
#define sdio_get_max_blk_size(func)     ((func)->card->host->max_blk_size)
#define sdio_get_max_seg_size(func)     ((func)->card->host->max_seg_size)
#define sdio_get_max_segs(func)         ((func)->card->host->max_segs)
#define sdio_en_timeout(func)           ((func)->enable_timeout)
#define sdio_func_num(func)             ((func)->num)

/**
 *  sdio_memcpy_fromio - read a chunk of memory from a SDIO function
 *  @func: SDIO function to access
 *  @dst: buffer to store the data
 *  @addr: address to begin reading from
 *  @count: number of bytes to read
 *
 *  Reads from the address space of a given SDIO function. Return
 *  value indicates if the transfer succeeded or not.
 */
static inline hi_s32 oal_sdio_memcpy_fromio(struct sdio_func *func, hi_void *dst, hi_u32 addr, hi_s32 count)
{
    hi_s32 ret;
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    oal_time_t_stru time_start;
    time_start = oal_ktime_get();
#endif
    ret = sdio_memcpy_fromio(func, dst, addr, count);
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    if (oal_unlikely(ret)) {
        /* If sdio transfer failed, dump the sdio info */
        hi_u64  trans_us;
        oal_time_t_stru time_stop = oal_ktime_get();
        trans_us = (hi_u64)oal_ktime_to_us(oal_ktime_sub(time_stop, time_start));
        printk(KERN_WARNING"[E]sdio_memcpy_fromio fail=%d, time cost:%llu us,[dst:%p,addr:%u,count:%d]\n",
               ret, trans_us, dst, addr, count);
    }
#endif
    return ret;
}

/**
 *  oal_sdio_readsb - read from a FIFO on a SDIO function
 *  @func: SDIO function to access
 *  @dst: buffer to store the data
 *  @addr: address of (single byte) FIFO
 *  @count: number of bytes to read
 *
 *  Reads from the specified FIFO of a given SDIO function. Return
 *  value indicates if the transfer succeeded or not.
 */
static inline  hi_s32 oal_sdio_readsb(struct sdio_func *func, hi_void *dst, hi_u32 addr, hi_s32 count)
{
    hi_s32 ret;
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    oal_time_t_stru time_start;
    time_start = oal_ktime_get();
#endif
    ret = sdio_readsb(func, dst, addr, count);
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    if (oal_unlikely(ret)) {
        /* If sdio transfer failed, dump the sdio info */
        hi_u64  trans_us;
        oal_time_t_stru time_stop = oal_ktime_get();
        trans_us = (hi_u64)oal_ktime_to_us(oal_ktime_sub(time_stop, time_start));
        printk(KERN_WARNING"[E]sdio_readsb fail=%d, time cost:%llu us,[dst:%p,addr:%u,count:%d]\n",
               ret, trans_us, dst, addr, count);
    }
#endif
    return ret;
}

/**
 *  oal_sdio_writesb - write to a FIFO of a SDIO function
 *  @func: SDIO function to access
 *  @addr: address of (single byte) FIFO
 *  @src: buffer that contains the data to write
 *  @count: number of bytes to write
 *
 *  Writes to the specified FIFO of a given SDIO function. Return
 *  value indicates if the transfer succeeded or not.
 */
static inline  hi_s32 oal_sdio_writesb(struct sdio_func *func, hi_u32 addr, hi_void *src, int count)
{
    hi_s32 ret;
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    oal_time_t_stru time_start;
    time_start = oal_ktime_get();
#endif
    ret = sdio_writesb(func, addr, src, count);
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    if (oal_unlikely(ret)) {
        /* If sdio transfer failed, dump the sdio info */
        hi_u64  trans_us;
        oal_time_t_stru time_stop = oal_ktime_get();
        trans_us = (hi_u64)oal_ktime_to_us(oal_ktime_sub(time_stop, time_start));
        printk(KERN_WARNING"[E]oal_sdio_writesb fail=%d, time cost:%llu us,[src:%p,addr:%u,count:%d]\n",
               ret, trans_us, src, addr, count);
    }
#endif
    return ret;
}

/**
 *  oal_sdio_readb - read a single byte from a SDIO function
 *  @func: SDIO function to access
 *  @addr: address to read
 *  @err_ret: optional status value from transfer
 *
 *  Reads a single byte from the address space of a given SDIO
 *  function. If there is a problem reading the address, 0xff
 *  is returned and @err_ret will contain the error code.
 */
static inline hi_u8 oal_sdio_readb(struct sdio_func *func, hi_u32 addr, hi_s32 *err_ret)
{
    return sdio_readb(func, addr, err_ret);
}

/**
 *  oal_sdio_writeb - write a single byte to a SDIO function
 *  @func: SDIO function to access
 *  @b: byte to write
 *  @addr: address to write to
 *  @err_ret: optional status value from transfer
 *
 *  Writes a single byte to the address space of a given SDIO
 *  function. @err_ret will contain the status of the actual
 *  transfer.
 */
static inline void oal_sdio_writeb(struct sdio_func *func, hi_u8 b, hi_u32 addr, hi_s32 *err_ret)
{
    sdio_writeb(func, b, addr, err_ret);
}

/**
 *  oal_sdio_readl - read a 32 bit integer from a SDIO function
 *  @func: SDIO function to access
 *  @addr: address to read
 *  @err_ret: optional status value from transfer
 *
 *  Reads a 32 bit integer from the address space of a given SDIO
 *  function. If there is a problem reading the address,
 *  0xffffffff is returned and @err_ret will contain the error
 *  code.
 */
static inline hi_u32 oal_sdio_readl(struct sdio_func *func, hi_u32 addr, hi_s32 *err_ret)
{
    return sdio_readl(func, addr, err_ret);
}

/**
 *  oal_sdio_writel - write a 32 bit integer to a SDIO function
 *  @func: SDIO function to access
 *  @b: integer to write
 *  @addr: address to write to
 *  @err_ret: optional status value from transfer
 *
 *  Writes a 32 bit integer to the address space of a given SDIO
 *  function. @err_ret will contain the status of the actual
 *  transfer.
 */
static inline hi_void oal_sdio_writel(struct sdio_func *func, hi_u32 b, hi_u32 addr, hi_s32 *err_ret)
{
    sdio_writel(func, b, addr, err_ret);
}

#elif (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)

#if ((!defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(3,3,3)) || \
    (defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(5,0,1)))
extern hi_u32 g_sdio_card_mutex;
extern hi_s32 g_sdio_card_lock_cnt;
#define sdio_mutex_init(mutex)              LOS_MuxCreate(mutex)
#define sdio_mutex_lock(mutex, timeout)     LOS_MuxPend(mutex, timeout)
#define sdio_mutex_unlock(mutex)            LOS_MuxPost(mutex)
#define sdio_mutex_delete(mutex)            LOS_MuxDelete(mutex)
#define sdio_acquire_card()    \
    do { \
    int ret = 0; \
    ret = sdio_mutex_lock(g_sdio_card_mutex, LOS_WAIT_FOREVER); \
    if (ret != 0) { \
        printk("acquire card fail! ret = %d\n", ret); \
        } \
    g_sdio_card_lock_cnt += 1; \
    } while (0)
#define sdio_release_card()    \
    do { \
    int ret = 0; \
    g_sdio_card_lock_cnt--; \
    ret = sdio_mutex_unlock(g_sdio_card_mutex); \
    if (ret != 0) { \
        printk("release card fail!,ret = %d\n", ret); \
    } \
    } while (0)
#endif

#ifdef CONFIG_MMC
#if ((!defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(3,3,3)) || \
    (defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(5,0,1)))
#define sdio_get_max_block_count(func)     511
#define sdio_get_max_req_size(func)        (511 * 512)
#define sdio_get_max_blk_size(func)        512
#else
#define sdio_get_max_block_count(func)     (func->card->host->max_blk_num)
#define sdio_get_max_req_size(func)        (func->card->host->max_request_size)
#define sdio_get_max_blk_size(func)        (func->card->host->max_blk_size)
#endif
#define sdio_en_timeout(func)        (func->en_timeout_ms)

#define sdio_func_num(func)             (func->func_num)

#define SDIO_ANY_ID (~0)

#define SDIO_DEVICE(vend, dev) \
    .class = SDIO_ANY_ID, \
             .vendor = (vend), .device = (dev)

struct sdio_device_id {
    unsigned char    class;                  /* Standard interface or SDIO_ANY_ID */
    unsigned short int   vendor;                 /* Vendor or SDIO_ANY_ID */
    unsigned short int   device;                 /* Device ID or SDIO_ANY_ID */
};

#if ((!defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(3,3,3)) || \
    (defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(5,0,1)))
#define sdio_get_drvdata(func)              (g_sdio_priv_data)
#define sdio_set_drvdata(func, priv)        (g_sdio_priv_data = (void *)priv)
#else
#define sdio_get_drvdata(func)                (func->data)
#define sdio_set_drvdata(func, priv)        (func->data = (void *)priv)
#endif

/*
 * SDIO function device driver
 */
struct sdio_driver {
    char *name;
    const struct sdio_device_id *id_table;

    int (*probe)(struct sdio_func *, const struct sdio_device_id *);
    void (*remove)(struct sdio_func *);
};

#define sdio_enable_func(func)                      sdio_en_func(func)
#define sdio_disable_func(func)                     sdio_dis_func(func)
#define sdio_set_block_size(func, blksz)            sdio_set_cur_blk_size(func, blksz)
#define sdio_readb(func, addr, err)                 sdio_read_byte(func, addr, err)
#define sdio_writeb(func, byte, addr, err)          sdio_write_byte(func, byte, addr, err)
#define sdio_memcpy_fromio(func, dst, addr, size)   sdio_read_incr_block(func, dst, addr, size)

static inline hi_s32 sdio_register_driver(struct sdio_driver *driver)
{
    hi_unref_param(driver);
    return HI_SUCCESS;
}

extern struct sdio_func *g_sdio_func;
static inline hi_void sdio_unregister_driver(struct sdio_driver *driver)
{
    hi_unref_param(driver);
    return;
}

static inline hi_void sdio_claim_host(const struct sdio_func *func)
{
#if ((!defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(3,3,3)) || \
    (defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(5,0,1)))
    if ((func == HI_NULL) || (func->sc == HI_NULL)) {
        return;
    }
    sdio_acquire_card();
#else
    if ((func == HI_NULL) || (func->card == HI_NULL)) {
        return;
    }
    mmc_acquire_card(func->card);
#endif
    return;
}

static inline hi_void sdio_release_host(const struct sdio_func *func)
{
#if ((!defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(3,3,3)) || \
    (defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(5,0,1)))
    if ((func == HI_NULL) || (func->sc == HI_NULL)) {
        return;
    }
    sdio_release_card();
#else
    if ((func == HI_NULL) || (func->card == HI_NULL)) {
        return;
    }
    mmc_release_card(func->card);
#endif
    return;
}

/**
 *  oal_sdio_writesb - write to a FIFO of a SDIO function
 *  @func: SDIO function to access
 *  @addr: address of (single byte) FIFO
 *  @src: buffer that contains the data to write
 *  @count: number of bytes to write
 *
 *  Writes to the specified FIFO of a given SDIO function. Return
 *  value indicates if the transfer succeeded or not.
 */
static inline  hi_s32 oal_sdio_writesb(struct sdio_func *func, hi_u32 addr, hi_void *src, int count)
{
    hi_s32 ret;
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    oal_time_t_stru time_start;
    time_start = oal_ktime_get();
#endif

    ret = sdio_write_fifo_block(func, addr, src, count);
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    if (oal_unlikely(ret)) {
        /* If sdio transfer failed, dump the sdio info */
        hi_u64  trans_us;
        oal_time_t_stru time_stop = oal_ktime_get();
        trans_us = (hi_u64)oal_ktime_to_us(oal_ktime_sub(time_stop, time_start));
        printk(KERN_WARNING"[E]oal_sdio_writesb fail=%d, time cost:%llu us,[src:%p,addr:%u,count:%d]\n",
               ret, trans_us, src, addr, count);
    }
#endif
    return ret;
}

/**
 *  oal_sdio_readsb - read from a FIFO on a SDIO function
 *  @func: SDIO function to access
 *  @dst: buffer to store the data
 *  @addr: address of (single byte) FIFO
 *  @count: number of bytes to read
 *
 *  Reads from the specified FIFO of a given SDIO function. Return
 *  value indicates if the transfer succeeded or not.
 */
static inline  hi_s32 oal_sdio_readsb(struct sdio_func *func, hi_void *dst, hi_u32 addr, hi_s32 count)
{
    hi_s32 ret;
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    oal_time_t_stru time_start;
    time_start = oal_ktime_get();
#endif

    ret = sdio_read_fifo_block(func, dst, addr, count);
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    if (oal_unlikely(ret)) {
        /* If sdio transfer failed, dump the sdio info */
        hi_u64  trans_us;
        oal_time_t_stru time_stop = oal_ktime_get();
        trans_us = (hi_u64)oal_ktime_to_us(oal_ktime_sub(time_stop, time_start));
        printk(KERN_WARNING"[E]sdio_readsb fail=%d, time cost:%llu us,[dst:%p,addr:%u,count:%d]\n",
               ret, trans_us, dst, addr, count);
    }
#endif
    return ret;
}

/**
 *  oal_sdio_readb - read a single byte from a SDIO function
 *  @func: SDIO function to access
 *  @addr: address to read
 *  @err_ret: optional status value from transfer
 *
 *  Reads a single byte from the address space of a given SDIO
 *  function. If there is a problem reading the address, 0xff
 *  is returned and @err_ret will contain the error code.
 */
static inline hi_u8 oal_sdio_readb(struct sdio_func *func, hi_u32 addr, hi_s32 *err_ret)
{
    return sdio_read_byte(func, addr, err_ret);
}

/**
 *  oal_sdio_writeb - write a single byte to a SDIO function
 *  @func: SDIO function to access
 *  @b: byte to write
 *  @addr: address to write to
 *  @err_ret: optional status value from transfer
 *
 *  Writes a single byte to the address space of a given SDIO
 *  function. @err_ret will contain the status of the actual
 *  transfer.
 */
static inline void oal_sdio_writeb(struct sdio_func *func, hi_u8 b, hi_u32 addr, hi_s32 *err_ret)
{
    sdio_write_byte(func, b, addr, err_ret);
}

/**
 *  oal_sdio_readl - read a 32 bit integer from a SDIO function
 *  @func: SDIO function to access
 *  @addr: address to read
 *  @err_ret: optional status value from transfer
 *
 *  Reads a 32 bit integer from the address space of a given SDIO
 *  function. If there is a problem reading the address,
 *  0xffffffff is returned and @err_ret will contain the error
 *  code.
 */
static inline hi_u32 oal_sdio_readl(struct sdio_func *func, hi_u32 addr, hi_s32 *err_ret)
{
    hi_u32 val;
    hi_s32 ret;

    ret = sdio_read_incr_block(func, &val, addr, 4);    /* 4 */
    *err_ret = ret;

    if (ret) {
        return 0xFF;
    } else {
        return val;
    }
}

/**
 *  oal_sdio_writel - write a 32 bit integer to a SDIO function
 *  @func: SDIO function to access
 *  @b: integer to write
 *  @addr: address to write to
 *  @err_ret: optional status value from transfer
 *
 *  Writes a 32 bit integer to the address space of a given SDIO
 *  function. @err_ret will contain the status of the actual
 *  transfer.
 */
static inline hi_void oal_sdio_writel(struct sdio_func *func, hi_u32 b, hi_u32 addr, hi_s32 *err_ret)
{
    hi_s32 ret;
    hi_u32 val = b;
    ret = sdio_write_incr_block(func, addr, &val, 4);   /* 4 */
    if (err_ret) {
        *err_ret = ret;
    }
}

/**
 *  sdio_memcpy_fromio - read a chunk of memory from a SDIO function
 *  @func: SDIO function to access
 *  @dst: buffer to store the data
 *  @addr: address to begin reading from
 *  @count: number of bytes to read
 *
 *  Reads from the address space of a given SDIO function. Return
 *  value indicates if the transfer succeeded or not.
 */
static inline hi_s32 oal_sdio_memcpy_fromio(struct sdio_func *func, hi_void *dst, hi_u32 addr, hi_s32 count)
{
    hi_s32 ret;
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    oal_time_t_stru time_start;
    time_start = oal_ktime_get();
#endif
    ret = sdio_memcpy_fromio(func, dst, addr, count);
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    if (oal_unlikely(ret)) {
        /* If sdio transfer failed, dump the sdio info */
        hi_u64  trans_us;
        oal_time_t_stru time_stop = oal_ktime_get();
        trans_us = (hi_u64)oal_ktime_to_us(oal_ktime_sub(time_stop, time_start));
        printk(KERN_WARNING"[E]sdio_memcpy_fromio fail=%d, time cost:%llu us,[dst:%p,addr:%u,count:%d]\n",
               ret, trans_us, dst, addr, count);
    }
#endif
    return ret;
}

#else /* CONFIG_MMC */
#if ((!defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(3,3,3)) || \
    (defined(_PRE_LOSCFG_KERNEL_SMP) && HW_LITEOS_OPEN_VERSION_NUM >= VERSION_NUM(5,0,1)))
#define sdio_get_max_block_count(func)     511
#define sdio_get_max_req_size(func)        (511 * 512)
#define sdio_get_max_blk_size(func)        512
#else
#define sdio_get_max_block_count(func)     (func->card->host->max_blk_count)
#define sdio_get_max_req_size(func)        (func->card->host->max_req_size)
#define sdio_get_max_blk_size(func)        (func->card->host->max_blk_size)
#endif
#define sdio_en_timeout(func)              (func->enable_timeout)
#define sdio_func_num(func)                (func->num)

/**
 *  oal_sdio_writesb - write to a FIFO of a SDIO function
 *  @func: SDIO function to access
 *  @addr: address of (single byte) FIFO
 *  @src: buffer that contains the data to write
 *  @count: number of bytes to write
 *
 *  Writes to the specified FIFO of a given SDIO function. Return
 *  value indicates if the transfer succeeded or not.
 */
static inline  hi_s32 oal_sdio_writesb(struct sdio_func *func, hi_u32 addr, hi_void *src, int count)
{
    hi_s32 ret;
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    oal_time_t_stru time_start;
    time_start = oal_ktime_get();
#endif

    ret = sdio_writesb(func, addr, src, count);
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    if (oal_unlikely(ret)) {
        /* If sdio transfer failed, dump the sdio info */
        hi_u64  trans_us;
        oal_time_t_stru time_stop = oal_ktime_get();
        trans_us = (hi_u64)oal_ktime_to_us(oal_ktime_sub(time_stop, time_start));
        printk(KERN_WARNING"[E]oal_sdio_writesb fail=%d, time cost:%llu us,[src:%p,addr:%u,count:%d]\n",
               ret, trans_us, src, addr, count);
    }
#endif
    return ret;
}

/**
 *  oal_sdio_readsb - read from a FIFO on a SDIO function
 *  @func: SDIO function to access
 *  @dst: buffer to store the data
 *  @addr: address of (single byte) FIFO
 *  @count: number of bytes to read
 *
 *  Reads from the specified FIFO of a given SDIO function. Return
 *  value indicates if the transfer succeeded or not.
 */
static inline  hi_s32 oal_sdio_readsb(struct sdio_func *func, hi_void *dst, hi_u32 addr, hi_s32 count)
{
    hi_s32 ret;
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    oal_time_t_stru time_start;
    time_start = oal_ktime_get();
#endif

    ret = sdio_readsb(func, dst, addr, count);
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    if (oal_unlikely(ret)) {
        /* If sdio transfer failed, dump the sdio info */
        hi_u64  trans_us;
        oal_time_t_stru time_stop = oal_ktime_get();
        trans_us = (hi_u64)oal_ktime_to_us(oal_ktime_sub(time_stop, time_start));
        printk(KERN_WARNING"[E]sdio_readsb fail=%d, time cost:%llu us,[dst:%p,addr:%u,count:%d]\n",
               ret, trans_us, dst, addr, count);
    }
#endif
    return ret;
}

/**
 *  oal_sdio_readb - read a single byte from a SDIO function
 *  @func: SDIO function to access
 *  @addr: address to read
 *  @err_ret: optional status value from transfer
 *
 *  Reads a single byte from the address space of a given SDIO
 *  function. If there is a problem reading the address, 0xff
 *  is returned and @err_ret will contain the error code.
 */
static inline hi_u8 oal_sdio_readb(struct sdio_func *func, hi_u32 addr, hi_s32 *err_ret)
{
    return sdio_readb(func, addr, err_ret);
}

/**
 *  oal_sdio_writeb - write a single byte to a SDIO function
 *  @func: SDIO function to access
 *  @b: byte to write
 *  @addr: address to write to
 *  @err_ret: optional status value from transfer
 *
 *  Writes a single byte to the address space of a given SDIO
 *  function. @err_ret will contain the status of the actual
 *  transfer.
 */
static inline void oal_sdio_writeb(struct sdio_func *func, hi_u8 b, hi_u32 addr, hi_s32 *err_ret)
{
    sdio_writeb(func, b, addr, err_ret);
}

/**
 *  oal_sdio_readl - read a 32 bit integer from a SDIO function
 *  @func: SDIO function to access
 *  @addr: address to read
 *  @err_ret: optional status value from transfer
 *
 *  Reads a 32 bit integer from the address space of a given SDIO
 *  function. If there is a problem reading the address,
 *  0xffffffff is returned and @err_ret will contain the error
 *  code.
 */
static inline hi_u32 oal_sdio_readl(struct sdio_func *func, hi_u32 addr, hi_s32 *err_ret)
{
    return sdio_readl(func, addr, err_ret);
}

/**
 *  oal_sdio_writel - write a 32 bit integer to a SDIO function
 *  @func: SDIO function to access
 *  @b: integer to write
 *  @addr: address to write to
 *  @err_ret: optional status value from transfer
 *
 *  Writes a 32 bit integer to the address space of a given SDIO
 *  function. @err_ret will contain the status of the actual
 *  transfer.
 */
static inline hi_void oal_sdio_writel(struct sdio_func *func, hi_u32 b, hi_u32 addr, hi_s32 *err_ret)
{
    sdio_writel(func, b, addr, err_ret);
}

/**
 *  sdio_memcpy_fromio - read a chunk of memory from a SDIO function
 *  @func: SDIO function to access
 *  @dst: buffer to store the data
 *  @addr: address to begin reading from
 *  @count: number of bytes to read
 *
 *  Reads from the address space of a given SDIO function. Return
 *  value indicates if the transfer succeeded or not.
 */
static inline hi_s32 oal_sdio_memcpy_fromio(struct sdio_func *func, hi_void *dst, hi_u32 addr, hi_s32 count)
{
    hi_s32 ret;
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    oal_time_t_stru time_start;
    time_start = oal_ktime_get();
#endif
    ret = sdio_memcpy_fromio(func, dst, addr, count);
#ifdef CONFIG_HISI_SDIO_TIME_DEBUG
    if (oal_unlikely(ret)) {
        /* If sdio transfer failed, dump the sdio info */
        hi_u64  trans_us;
        oal_time_t_stru time_stop = oal_ktime_get();
        trans_us = (hi_u64)oal_ktime_to_us(oal_ktime_sub(time_stop, time_start));
        printk(KERN_WARNING"[E]sdio_memcpy_fromio fail=%d, time cost:%llu us,[dst:%p,addr:%u,count:%d]\n",
               ret, trans_us, dst, addr, count);
    }
#endif
    return ret;
}

static inline hi_s32 sdio_register_driver(struct sdio_driver *driver)
{
    return HI_SUCCESS;
}

extern struct sdio_func *g_sdio_func;
static inline hi_void sdio_unregister_driver(struct sdio_driver *driver)
{
    driver->remove(g_sdio_func);
    return;
}

static inline hi_void sdio_claim_host(struct sdio_func *func)
{
    return;
}

static inline hi_void sdio_release_host(struct sdio_func *func)
{
    return;
}

static inline int sdio_require_irq(struct sdio_func *func, sdio_irq_handler_t *handler)
{
    return sdio_claim_irq(func, handler);
}
#endif /* end of CONFIG_MMC */

#endif /* end of (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_sdio_if.h */

