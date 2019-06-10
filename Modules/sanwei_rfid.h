#ifndef __MODULE__SANWEI_RFID_H_
#define __MODULE__SANWEI_RFID_H_

#include "stm32f10x.h"
#include "sys.h"

#define PRE_SEND_DATA_LEN   32
#define SEND_DATA_LEN       32

#define SW_RFID_UART_RCV_SIZE    32

#define SW_RFID_UART_RCV_BUF_NUM  10
//typedef struct fp_rcv_buf_t
//{
//    uint8_t rcv_buf[SW_RFID_UART_RCV_SIZE];
//    uint16_t rcv_len;
//    struct fp_rcv_buf_t *next;
//}fp_rcv_buf_t;

typedef struct sw_rfid_uart_rcv_buf_t
{
    uint8_t rcv_buf[SW_RFID_UART_RCV_SIZE];
    uint16_t rcv_len;
    struct sw_rfid_uart_rcv_buf_t *next;
}sw_rfid_uart_rcv_buf_t;


void sanwei_rfid_init(void);
uint8_t read_rfid(uint8_t absolute_block_num);
//test funciton
void test_sanwei_rfid_send_data(void);

#endif

