#ifndef __MODULE__SANWEI_RFID_H_
#define __MODULE__SANWEI_RFID_H_

#include "stm32f10x.h"
#include "sys.h"
#include "ucos_ii.h"

#define PRE_SEND_DATA_LEN   32
#define SEND_DATA_LEN       32

#define SW_RFID_UART_RCV_SIZE       32

#define SW_RFID_UART_RCV_BUF_NUM    5


#define SW_RFID_PROTOCOL_CMD_SEARCH_CARD                0x46
#define SW_RFID_PROTOCOL_CMD_PREVENT_CONFLICT           0x47
#define SW_RFID_PROTOCOL_CMD_SELECT_CARD                0x48
#define SW_RFID_PROTOCOL_CMD_VERIFY_SECRET_KEY          0x4A
#define SW_RFID_PROTOCOL_CMD_READ_BLOCK                 0x4B
#define SW_RFID_PROTOCOL_CMD_WRITE_BLOCK                0x4C

typedef struct sw_rfid_uart_rcv_buf_t
{
    uint8_t rcv_buf[SW_RFID_UART_RCV_SIZE];
    uint16_t rcv_len;
    struct sw_rfid_uart_rcv_buf_t *next;
}sw_rfid_uart_rcv_buf_t;


typedef struct
{
    uint16_t module_addr;
    uint8_t cmd;
    uint8_t result;
    uint8_t data[32];
    uint8_t data_len;
}protocol_ack_t;

void sanwei_rfid_init(void);
uint8_t search_card(void);
uint8_t read_rfid(uint8_t absolute_block_num);

uint8_t sanwei_rfid_rcv_proccess(uint8_t *data, uint8_t len);


int free_one_rcv_buf(sw_rfid_uart_rcv_buf_t *buf);
sw_rfid_uart_rcv_buf_t *get_latest_buf(void);

//test funciton
void test_sanwei_rfid_send_data(void);

#endif

