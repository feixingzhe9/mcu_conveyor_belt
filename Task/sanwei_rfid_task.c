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

sw_rfid_ack_t sw_rfid_ack_mem[SW_RFID_ACK_BUF_NUM][1] = {{0}};

OS_EVENT *sw_rfid_ack_queue_handle = NULL;
void *sw_rfid_ack_queue_p[SW_RFID_ACK_QUEUE_NUM] = {NULL};

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

#define SW_RFID_ACK_TIME_OUT    (5 * OS_TICKS_PER_SEC / 10)
void sanwei_rfid_main_task(void *pdata)
{
    uint8_t state = STATE_NONE;
    sw_rfid_ack_t *ack = NULL;
    uint8_t err = 0;
    uint32_t card_id = 0;
    uint16_t tag_type = 0;
    uint8_t card_space = 0;
    uint8_t card_context[16] = {0};
    uint8_t write_test_data[16] = {0xaa, 0xbb, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint8_t ack_ok_flag = FALSE;
    delay_ms(5000);


//    uint16_t src_id = 0xaa16;
//    uint16_t dst_id = 0xcc25;
//    uint16_t time = 0xbbbb;
//    uint16_t service_id = src_id;
//    if(write_sw_rfid_info(dst_id, src_id, service_id, time) < 0)
//    {
//        delay_ms(5000);
//    }
    for(;;)
    {
        /**************测试任务代码****************/
        switch(state)
        {
            case STATE_SET_MODE:     //寻卡
            {
                uint8_t retry_cnt = 3;
                while(retry_cnt--)
                {
                    set_rfid_work_mode(0);
                    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
                    if(ack)
                    {
                        if(ack->cmd == SW_RFID_PROTOCOL_CMD_SET_MODE)
                        {
                            if(ack->result == 0)
                            {
                                state = STATE_SEARCH_CARD;
                                OSMemPut(sw_rfid_ack_mem_handle, ack);
                                break;
                            }
                        }
                    }
                }
                state = STATE_SEARCH_CARD;
                break;
            }

            case STATE_SEARCH_CARD:     //寻卡
            {
                uint8_t retry_cnt = 3;
                while(retry_cnt--)
                {
                    search_card();
                    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
                    if(ack)
                    {
                        if(ack->cmd == SW_RFID_PROTOCOL_CMD_SEARCH_CARD)
                        {
                            if(ack->result == 0)
                            {
                                if(ack->data_len == 2)
                                {
                                    tag_type = ack->data[0] << 8 | ack->data[1];
                                }
                                state = STATE_PREVENT_CONFLICT;
                                OSMemPut(sw_rfid_ack_mem_handle, ack);
                                break;
                            }
                        }
                    }
                }
                state = STATE_PREVENT_CONFLICT;
                break;
            }

            case STATE_PREVENT_CONFLICT:       //防冲突
            {
                uint8_t retry_cnt = 3;
                while(retry_cnt--)
                {
                    prevent_conflict();
                    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
                    if(ack)
                    {
                        if(ack->cmd == SW_RFID_PROTOCOL_CMD_PREVENT_CONFLICT)
                        {
                            if(ack->result == 0)
                            {
                                if(ack->data_len == 4)
                                {
                                    card_id = (ack->data[0] << 24) | (ack->data[1] << 16) | (ack->data[2] << 8) | ack->data[3];
                                }
                                state = STATE_SELECT_CARD;
                                OSMemPut(sw_rfid_ack_mem_handle, ack);
                                break;
                            }
                        }
                    }
                }
                state = STATE_SELECT_CARD;
                break;
            }

            case STATE_SELECT_CARD:       //选卡
            {
                uint8_t retry_cnt = 3;
                while(retry_cnt--)
                {
                    select_card(card_id);
                    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
                    if(ack)
                    {
                        if(ack->cmd == SW_RFID_PROTOCOL_CMD_SELECT_CARD)
                        {
                            if(ack->result == 0)
                            {
                                if(ack->data_len == 1)
                                {
                                    card_space = ack->data[0];
                                }
                                state = STATE_VERIFY_SECRET_KEY;
                                OSMemPut(sw_rfid_ack_mem_handle, ack);
                                break;
                            }
                        }
                        OSMemPut(sw_rfid_ack_mem_handle, ack);
                    }
                }
                state = STATE_VERIFY_SECRET_KEY;
                break;
            }

            case STATE_VERIFY_SECRET_KEY:       //验证密钥
            {
                uint8_t retry_cnt = 3;
                while(retry_cnt--)
                {
                    verify_secret_key(52);
                    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
                    if(ack)
                    {
                        if(ack->cmd == SW_RFID_PROTOCOL_CMD_VERIFY_SECRET_KEY)
                        {
                            if(ack->result == 0)
                            {
                                state = STATE_READ_CARD;
                                OSMemPut(sw_rfid_ack_mem_handle, ack);
                                break;
                            }
                        }
                        else
                        {
                            //retry_cnt++;
                        }
                        OSMemPut(sw_rfid_ack_mem_handle, ack);
                    }
                }
                state = STATE_READ_CARD;
                break;
            }

            case STATE_READ_CARD:       //读卡
            {
                uint8_t retry_cnt = 3;
                while(retry_cnt--)
                {
                    read_rfid(52);
                    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
                    if(ack)
                    {
                        if(ack->cmd == SW_RFID_PROTOCOL_CMD_READ_BLOCK)
                        {
                            if(ack->result == 0)
                            {
                                if(ack->data_len == 16)
                                {
                                    memcpy(card_context, ack->data, 16);
                                }
                                state = STATE_WRITE_CARD;
                                OSMemPut(sw_rfid_ack_mem_handle, ack);
                                break;
                            }
                        }
                        OSMemPut(sw_rfid_ack_mem_handle, ack);
                    }
                }
                break;
            }

            case STATE_WRITE_CARD:       //写卡
            {
                uint8_t retry_cnt = 3;
                while(retry_cnt--)
                {
                    write_rfid(52, write_test_data);
                    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
                    if(ack)
                    {
                        if(ack->cmd == SW_RFID_PROTOCOL_CMD_WRITE_BLOCK)
                        {
                            if(ack->result == 0)
                            {
                                state = STATE_NONE;
                                OSMemPut(sw_rfid_ack_mem_handle, ack);
                                break;
                            }
                        }
                        OSMemPut(sw_rfid_ack_mem_handle, ack);
                    }
                }
                state = STATE_NONE;
                break;
            }
            default: break;
        }
        delay_ms(30);
    }
}
