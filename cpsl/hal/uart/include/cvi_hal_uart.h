/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: uart.h
 * Description:
 */

#ifndef __CVI_HAL_UART_H__
#define __CVI_HAL_UART_H__
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

int32_t CVI_UART_Open(char *node);
int32_t CVI_UART_Close(int32_t fd);
int32_t CVI_UART_Set_Param(int32_t speed, int32_t flow_ctrl, int32_t databits, int32_t stopbits, char parity);
int32_t CVI_UART_Receive(unsigned char *rcv_buf, int32_t data_len,int32_t timeout_ms);
int32_t CVI_UART_Send(char *send_buf, int32_t data_len);
int32_t CVI_UART_Init(char *node);
int32_t CVI_UART_Exit(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_UART_H__ */