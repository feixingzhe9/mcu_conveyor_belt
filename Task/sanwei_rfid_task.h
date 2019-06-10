#ifndef _TASK_SANWEI_RFID_TASK_H_
#define _TASK_SANWEI_RFID_TASK_H_

#include "stm32f10x.h"
#include "sys.h"
#include "ucos_ii.h"
#include "sanwei_rfid.h"
#include <string.h>

#define SANWEI_RFID_TASK_STK_SIZE       64

extern OS_STK sanwei_rfid_task_stk[SANWEI_RFID_TASK_STK_SIZE];

//extern OS_MEM *fp_rcv_mem_handle;
extern OS_MEM *sw_rfid_uart_rcv_mem_handle;
//extern fp_rcv_buf_t fp_rcv_mem[SW_RFID_UART_RCV_BUF_NUM][1];
extern sw_rfid_uart_rcv_buf_t sw_rfid_uart_rcv_mem[SW_RFID_UART_RCV_BUF_NUM][1];



void sanwei_rfid_task(void *pdata);
int sw_rfid_uart_rcv_buf_head_init(void);
int put_sw_rfid_uart_rcv_buf(uint8_t *buf, uint16_t len);

#endif

