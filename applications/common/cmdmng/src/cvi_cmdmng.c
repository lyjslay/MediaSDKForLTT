#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cvi_osal.h"
#include "cvi_mq.h"

int32_t CVI_CMDMNG_SendMqCmd(int32_t client_id, int32_t chn_id,
    int32_t cmd_id, int32_t arg_val) {
  static int32_t i = 0;

  // TODO: implement ACK

  // test CVI_MQ_SendRAW
  CVI_MQ_MSG_S msg;
  msg.target_id = CVI_MQ_ID(client_id, chn_id);
  msg.arg1 = cmd_id;
  msg.arg2 = arg_val;
  msg.needack = 0;
  msg.seq_no = i++;
  msg.len = (int32_t)CVI_MQ_MSG_HEADER_LEN;
  uint64_t boot_time;
  cvi_osal_get_boot_time(&boot_time);
  msg.crete_time = boot_time;
  return CVI_MQ_SendRAW(&msg);
}

int32_t CVI_CMDMNG_SendMqCmd_Str(int32_t client_id, int32_t chn_id,
    int32_t cmd_id, int32_t arg_val, const char *str) {
  static int32_t i = 0;

  // TODO: implement ACK

  // test CVI_MQ_SendRAW
  CVI_MQ_MSG_S msg;
  msg.target_id = CVI_MQ_ID(client_id, chn_id);
  msg.arg1 = cmd_id;
  msg.arg2 = arg_val;
  strncpy(msg.payload, str, CVI_MQ_MSG_PAYLOAD_LEN);
  msg.seq_no = i++;
  msg.needack = 0;
  msg.len = (int32_t)CVI_MQ_MSG_HEADER_LEN
            + (int32_t)strnlen(msg.payload, CVI_MQ_MSG_PAYLOAD_LEN) + 1;
  uint64_t boot_time;
  cvi_osal_get_boot_time(&boot_time);
  msg.crete_time = boot_time;
  return CVI_MQ_SendRAW(&msg);
}