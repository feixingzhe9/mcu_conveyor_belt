#ifndef _TASK_SANWEI_RFID_TASK_H_
#define _TASK_SANWEI_RFID_TASK_H_

#include "stm32f10x.h"
#include "sys.h"
#include "ucos_ii.h"
#include "sanwei_rfid.h"


#define SANWEI_RFID_MAIN_TASK_STK_SIZE                  64
#define SANWEI_RFID_RCV_TASK_STK_SIZE                   64


enum
{
    STATE_NONE = 0,
    STATE_SEARCH_CARD = 1,
    STATE_PREVENT_CONFLICT,
    STATE_SELECT_CARD,
    STATE_VERIFY_SECRET_KEY,
    STATE_READ_CARD,
    STATE_WRITE_CARD
};

extern OS_STK sanwei_rfid_main_task_stk[SANWEI_RFID_MAIN_TASK_STK_SIZE];
extern OS_STK sanwei_rfid_rcv_task_stk[SANWEI_RFID_RCV_TASK_STK_SIZE];

extern OS_MEM *sw_rfid_uart_rcv_mem_handle;
extern sw_rfid_uart_rcv_buf_t sw_rfid_uart_rcv_mem[SW_RFID_UART_RCV_BUF_NUM][1];

extern OS_MEM *sw_rfid_ack_mem_handle;
extern sw_rfid_ack_t sw_rfid_ack_mem[SW_RFID_ACK_BUF_NUM][1];

extern OS_EVENT *sw_rfid_ack_queue_handle;
extern void *sw_rfid_ack_queue_p[SW_RFID_ACK_QUEUE_NUM];

void sanwei_rfid_rcv_task(void *pdata);
void sanwei_rfid_main_task(void *pdata);
int sw_rfid_uart_rcv_buf_head_init(void);
int put_sw_rfid_uart_rcv_buf(uint8_t *buf, uint16_t len);

#endif

