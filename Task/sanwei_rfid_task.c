/*
 *  Author: Kaka Xie
 *  brief: sanwei rfid task
 */
#include "sanwei_rfid_task.h"
#include "delay.h"

OS_STK sanwei_rfid_task_stk[SANWEI_RFID_TASK_STK_SIZE];



OS_MEM *sw_rfid_uart_rcv_mem_handle;

sw_rfid_uart_rcv_buf_t sw_rfid_uart_rcv_mem[SW_RFID_UART_RCV_BUF_NUM][1] = {{0}};


static sw_rfid_uart_rcv_buf_t *rcv_buf_head = NULL;

int sw_rfid_uart_rcv_buf_head_init(void)
{
    uint8_t err = 0;
    rcv_buf_head = (sw_rfid_uart_rcv_buf_t *)OSMemGet(sw_rfid_uart_rcv_mem_handle, &err);
    if(rcv_buf_head != NULL)
    {
        rcv_buf_head->next = NULL;
        rcv_buf_head->rcv_len = 0;
        memset(rcv_buf_head->rcv_buf, 0, SW_RFID_UART_RCV_SIZE);
    }
    else
    {
        /*
        todo: err process
        */
        return -1;
    }
    return 0;
}

static uint16_t get_used_buf_size(void)
{
    uint16_t cnt = 0;
    sw_rfid_uart_rcv_buf_t *ptr = NULL;
    ptr = rcv_buf_head;
    while(ptr->next != NULL)
    {
        ptr = ptr->next;
        cnt++;
    }
    return cnt;
}

int put_sw_rfid_uart_rcv_buf(uint8_t *buf, uint16_t len)
{
    sw_rfid_uart_rcv_buf_t *p = NULL;
    sw_rfid_uart_rcv_buf_t *node = NULL;
    uint8_t err = 0;
    if(get_used_buf_size() < SW_RFID_UART_RCV_BUF_NUM)
    {
        node = (sw_rfid_uart_rcv_buf_t *)OSMemGet(sw_rfid_uart_rcv_mem_handle, &err);
        if(node == NULL)
        {
            /*
            todo: err process
            */
            return -1;
        }
        node->next = NULL;
        node->rcv_len = len;
        memcpy(node->rcv_buf, buf, len);
    }
    else
    {
        /*
        todo: err process
        */
        return -1;
    }
    p = rcv_buf_head;
    while(p->next != NULL)  //链表头不保存数据
    {
        p = p->next;
    }
    p->next = node;
    return 0;
}

int free_one_rcv_buf(sw_rfid_uart_rcv_buf_t *buf)
{
    sw_rfid_uart_rcv_buf_t *p = NULL;
    sw_rfid_uart_rcv_buf_t *pre_p = NULL;
    p = rcv_buf_head;
    pre_p = rcv_buf_head;
    do
    {
        if(p != buf)
        {
            pre_p = p;
            p = p->next;
        }
        else
        {
            break;
        }
    }
    while(p != NULL);

    if(p == NULL)
    {
        return -1;  //can not find such buf
    }

    OSMemPut(sw_rfid_uart_rcv_mem_handle, p);
    if(p->next == NULL)
    {
        pre_p->next = NULL;
    }
    else
    {
        pre_p->next = p->next;
    }
    return 0;
}

sw_rfid_uart_rcv_buf_t *get_latest_buf(void)
{
    sw_rfid_uart_rcv_buf_t *p = NULL;
    sw_rfid_uart_rcv_buf_t *node = NULL;
    p = rcv_buf_head;
    if(p->next != NULL)
    {
        node = p->next;
    }
    return node;
}


void sanwei_rfid_task(void *pdata)
{
    delay_ms(3000);
    read_rfid(52);
    for(;;)
    {
        delay_ms(5000);
    }
}
