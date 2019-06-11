/*
 *  Author: Kaka Xie
 *  brief: sanwei rfid task
 */
#include "sanwei_rfid_task.h"
#include "delay.h"
#include <string.h>

OS_STK sanwei_rfid_rcv_task_stk[SANWEI_RFID_RCV_TASK_STK_SIZE] = {0};

OS_STK sanwei_rfid_main_task_stk[SANWEI_RFID_MAIN_TASK_STK_SIZE] = {0};

sw_rfid_uart_rcv_buf_t sw_rfid_uart_rcv_mem[SW_RFID_UART_RCV_BUF_NUM][1] = {{0}};



void sanwei_rfid_rcv_task(void *pdata)
{
    sw_rfid_uart_rcv_buf_t *rcv_node = NULL;
    sw_rfid_uart_rcv_buf_t node;
    delay_ms(1000);
    for(;;)
    {
        delay_ms(50);
        OS_ENTER_CRITICAL();
        rcv_node = get_latest_buf();
        if(rcv_node)
        {
            memcpy(&node, rcv_node, sizeof(sw_rfid_uart_rcv_buf_t));
            free_one_rcv_buf(rcv_node);
            sanwei_rfid_rcv_proccess(node.rcv_buf, node.rcv_len);
        }
        OS_EXIT_CRITICAL();
    }
}

void sanwei_rfid_main_task(void *pdata)
{
    uint8_t state = 0;
    delay_ms(4000);
    //read_rfid(52);
    search_card();
    delay_ms(1000);
    search_card();
    for(;;)
    {
        switch(state)
        {
            case 0:
                delay_ms(50);
                break;

            case STATE_SEARCH_CARD:             //寻卡
                /*
                */
                break;
            case STATE_PREVENT_CONFLICT:       //防冲突
                /*
                */
                break;
            default: break;
        }
        delay_ms(1000);
        search_card();
    }
}
