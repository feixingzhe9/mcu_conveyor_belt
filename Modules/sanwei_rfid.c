/*
 *  Author: Kaka Xie
 *  brief: sanwei rfid
 */

#include "sanwei_rfid.h"
#include "usart.h"
#include "delay.h"
#include <string.h>

//static uint8_t pre_send_data_buf[PRE_SEND_DATA_LEN] = {0x02,0x04,0x05,0x02,0x03,0x10,0x03};
static uint8_t pre_send_data_buf[PRE_SEND_DATA_LEN] = {0};
static uint8_t send_data_buf[SEND_DATA_LEN] = {0};


#define PROTOCOL_HEAD           0x02
#define PROTOCOL_TAIL           0x03
#define INSERT_ESCAPE           0x10


OS_MEM *sw_rfid_uart_rcv_mem_handle;

OS_MEM *sw_rfid_ack_mem_handle;

void sanwei_rfid_init(void)
{
    uart2_dma_init(19200);
}

sw_rfid_uart_rcv_buf_t *rcv_buf_head = NULL;

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



static bool is_escape(uint8_t c)
{
    if((c == PROTOCOL_HEAD) || (c == PROTOCOL_TAIL) || (c == INSERT_ESCAPE))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static uint8_t cal_check_value(uint8_t *data, uint8_t len)
{
    uint8_t sum = 0;
    for(uint8_t i = 0; i < len; i++)
    {
        sum += data[i];
    }
    return sum;
}

static uint8_t pre_proc_data(uint8_t *in, uint8_t in_len, uint8_t *out)
{
    uint8_t out_len = 0;
    uint8_t cnt = 1;
    uint8_t i = 1;
    if((in[0] == PROTOCOL_HEAD) && (in[in_len - 1] == PROTOCOL_TAIL))
    {
        out[0] = in[0];
        out_len = in_len;
        for(i = 1; i < in_len - 1; i++, cnt++)
        {
            if(is_escape(in[i]))
            {
                out[cnt++] = INSERT_ESCAPE;
                out[cnt] = in[i];
            }
            else
            {
                out[cnt] = in[i];
            }
        }
        out[cnt] = in[i];
        out_len = cnt + 1;
        return out_len;
    }
    else    //输入参数有问题
    {
        return 0;
    }

}

void test_sanwei_rfid_send_data(void)
{
    uint8_t len = pre_proc_data(pre_send_data_buf, 7, send_data_buf);
    uart2_send(send_data_buf, len);
}



/******************** uart send protocol *****************/
uint8_t set_rfid_work_mode(uint8_t mode)    //02 00 00 04 3a 41 7f 03           ack: 02 00 00 10 03 3A 00 3D 03 
{
    uint8_t check_value = 0;
    uint8_t len = 0;
    pre_send_data_buf[0] = PROTOCOL_HEAD;
    pre_send_data_buf[1] = 0;       //模块地址
    pre_send_data_buf[2] = 0;       //模块地址
    pre_send_data_buf[3] = 4;       //length:从长度字到校验字的字节数
    pre_send_data_buf[4] = SW_RFID_PROTOCOL_CMD_SET_MODE;
    /* data */
    pre_send_data_buf[5] = 0x41;    //'A'

    check_value = cal_check_value(&pre_send_data_buf[1], 5);    //从模块地址到数据域结束
    pre_send_data_buf[6] = check_value;         //校验字
    pre_send_data_buf[7] = PROTOCOL_TAIL;

    len = pre_proc_data(pre_send_data_buf, 8, send_data_buf);
    if(len > 0)
    {
        uart2_send(send_data_buf, len);
    }
    return 0;
}


uint8_t search_card(void)      //02 00 00 04 46 52 9c 03       ack:02 00 00 05 46 00 04 00 4F 03 
{
    uint8_t check_value = 0;
    uint8_t len = 0;
    pre_send_data_buf[0] = PROTOCOL_HEAD;
    pre_send_data_buf[1] = 0;       //模块地址
    pre_send_data_buf[2] = 0;       //模块地址
    pre_send_data_buf[3] = 4;       //length:从长度字到校验字的字节数
    pre_send_data_buf[4] = 0x46;
    /* data */
//    pre_send_data_buf[5] = 0x52;
    pre_send_data_buf[5] = 0x26;

    check_value = cal_check_value(&pre_send_data_buf[1], 5);    //从模块地址到数据域结束
    pre_send_data_buf[6] = check_value;         //校验字
    pre_send_data_buf[7] = PROTOCOL_TAIL;

    len = pre_proc_data(pre_send_data_buf, 8, send_data_buf);
    if(len > 0)
    {
        uart2_send(send_data_buf, len);
    }
    return 0;
}

uint8_t prevent_conflict(void)      //02 00 00 04 47 04 4f 03    ack:02 00 00 07 47 00 12 3C 56 56 48 03 
{
    uint8_t check_value = 0;
    uint8_t len = 0;
    pre_send_data_buf[0] = PROTOCOL_HEAD;
    pre_send_data_buf[1] = 0;       //模块地址
    pre_send_data_buf[2] = 0;       //模块地址
    pre_send_data_buf[3] = 4;       //length:从长度字到校验字的字节数
    pre_send_data_buf[4] = 0x47;
    /* data */
    pre_send_data_buf[5] = 0x04;

    check_value = cal_check_value(&pre_send_data_buf[1], 5);    //从模块地址到数据域结束
    pre_send_data_buf[6] = check_value;         //校验字
    pre_send_data_buf[7] = PROTOCOL_TAIL;

    len = pre_proc_data(pre_send_data_buf, 8, send_data_buf);
    if(len > 0)
    {
        uart2_send(send_data_buf, len);
    }
    return 0;
}




//02 00 00 07 48 c0 4e e6 a4 e7 03
uint8_t select_card(uint32_t id)        //02 00 00 07 48 12 3c 56 56 46 03          ack:02 00 00 04 48 00 08 54 03 
{
    uint8_t check_value = 0;
    uint8_t len = 0;
    pre_send_data_buf[0] = PROTOCOL_HEAD;
    pre_send_data_buf[1] = 0;       //模块地址
    pre_send_data_buf[2] = 0;       //模块地址
    pre_send_data_buf[3] = 4;       //length:从长度字到校验字的字节数
    pre_send_data_buf[4] = 0x48;
    /* data */
    pre_send_data_buf[5] = id >> 24;
    pre_send_data_buf[6] = (id >> 16) & 0xff;
    pre_send_data_buf[7] = (id >> 8) & 0xff;
    pre_send_data_buf[8] = id & 0xff;

    check_value = cal_check_value(&pre_send_data_buf[1], 8);    //从模块地址到数据域结束
    pre_send_data_buf[9] = check_value;         //校验字
    pre_send_data_buf[10] = PROTOCOL_TAIL;

    len = pre_proc_data(pre_send_data_buf, 11, send_data_buf);
    if(len > 0)
    {
        uart2_send(send_data_buf, len);
    }
    return 0;
}

uint8_t verify_secret_key(uint8_t absolut_block_num)//固定key为6个0xff        //02 00 00 0b 4A 60 34 ff ff ff ff ff ff e3 03          ack:02 00 00 10 03 4A 00 4D 03 
{
    uint8_t check_value = 0;
    uint8_t len = 0;
    pre_send_data_buf[0] = PROTOCOL_HEAD;
    pre_send_data_buf[1] = 0;       //模块地址
    pre_send_data_buf[2] = 0;       //模块地址
    pre_send_data_buf[3] = 11;       //length:从长度字到校验字的字节数
    pre_send_data_buf[4] = 0x4A;
    /* data */
    pre_send_data_buf[5] = 0x60;    //验证A密钥
    pre_send_data_buf[6] = absolut_block_num;    //块 52
    pre_send_data_buf[7] = 0xff;
    pre_send_data_buf[8] = 0xff;
    pre_send_data_buf[9] = 0xff;
    pre_send_data_buf[10] = 0xff;
    pre_send_data_buf[11] = 0xff;
    pre_send_data_buf[12] = 0xff;

    check_value = cal_check_value(&pre_send_data_buf[1], 12);    //从模块地址到数据域结束
    pre_send_data_buf[13] = check_value;         //校验字
    pre_send_data_buf[14] = PROTOCOL_TAIL;

    len = pre_proc_data(pre_send_data_buf, 15, send_data_buf);
    if(len > 0)
    {
        uart2_send(send_data_buf, len);
    }
    return 0;
}



//02 00 00 04 4b 38 87 03
uint8_t read_rfid(uint8_t absolute_block_num)   // e.g: 02 00 00 04 4b 34 83 03           ack: 02 00 00 13 4B 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 5E 03 
{
    uint8_t check_value = 0;
    uint8_t len = 0;
    pre_send_data_buf[0] = PROTOCOL_HEAD;
    pre_send_data_buf[1] = 0;       //模块地址
    pre_send_data_buf[2] = 0;       //模块地址
    pre_send_data_buf[3] = 4;       //length:从长度字到校验字的字节数
    pre_send_data_buf[4] = 0x4b;    //cmd: 读块
    /* data */
    pre_send_data_buf[5] = absolute_block_num;  //绝对块号

    check_value = cal_check_value(&pre_send_data_buf[1], 5);    //从模块地址到数据域结束
    pre_send_data_buf[6] = check_value;         //校验字
    pre_send_data_buf[7] = PROTOCOL_TAIL;

    len = pre_proc_data(pre_send_data_buf, 8, send_data_buf);
    if(len > 0)
    {
        uart2_send(send_data_buf, len);
    }
    return 0;
}


uint8_t write_rfid(uint8_t absolute_block_num, uint8_t *data)   //写入对应块的16个字节
{
    uint8_t check_value = 0;
    uint8_t len = 0;
    pre_send_data_buf[0] = PROTOCOL_HEAD;
    pre_send_data_buf[1] = 0;       //模块地址
    pre_send_data_buf[2] = 0;       //模块地址
    pre_send_data_buf[3] = 20;       //length:从长度字到校验字的字节数
    pre_send_data_buf[4] = SW_RFID_PROTOCOL_CMD_WRITE_BLOCK;    //cmd: 读块
    /* data */
    pre_send_data_buf[5] = absolute_block_num;  //绝对块号

    for(uint8_t i = 0; i < 16; i++)
    {
        pre_send_data_buf[6 + i] = data[i];
    }
    check_value = cal_check_value(&pre_send_data_buf[1], 21);    //从模块地址到数据域结束
    pre_send_data_buf[22] = check_value;         //校验字
    pre_send_data_buf[23] = PROTOCOL_TAIL;

    len = pre_proc_data(pre_send_data_buf, 24, send_data_buf);
    if(len > 0)
    {
        uart2_send(send_data_buf, len);
    }
    return 0;
}

#define SW_RFID_PROTOCOL_CMD_SELECT_CARD                0x48
#define SW_RFID_PROTOCOL_CMD_VERIFY_SECRET_KEY          0x4A
#define SW_RFID_PROTOCOL_CMD_READ_BLOCK                 0x4B
#define SW_RFID_PROTOCOL_CMD_WRITE_BLOCK                0x4C

/******************** uart receive protocol *****************/

static bool is_protocol_cmd(uint8_t cmd)
{
    if((cmd == SW_RFID_PROTOCOL_CMD_SET_MODE) || (cmd == SW_RFID_PROTOCOL_CMD_SEARCH_CARD) || (cmd == SW_RFID_PROTOCOL_CMD_PREVENT_CONFLICT) || (cmd == SW_RFID_PROTOCOL_CMD_SELECT_CARD) || \
        (cmd == SW_RFID_PROTOCOL_CMD_VERIFY_SECRET_KEY) || (cmd == SW_RFID_PROTOCOL_CMD_READ_BLOCK) || (cmd == SW_RFID_PROTOCOL_CMD_WRITE_BLOCK))
    {
        return TRUE;
    }
    return FALSE;
}

static bool is_frame_valid(uint8_t *data, uint8_t len)
{
    if(data == NULL)                                            return FALSE;
    if(len < 8)                                                 return FALSE;
    if(data[0] != PROTOCOL_HEAD)                                return FALSE;
    if(data[len - 1] != PROTOCOL_TAIL)                          return FALSE;
    if(cal_check_value(&data[1], len - 3) != data[len - 2])     return FALSE;

    return TRUE;
}

static uint8_t pre_porc_ack_data(uint8_t *in_data, uint8_t in_len, uint8_t *out_data)
{
    uint8_t cnt = 1;
    out_data[0] = in_data[0];
    for(uint8_t i = 1; i < in_len - 1; i++)
    {
        if(in_data[i] != INSERT_ESCAPE)
        {
            out_data[cnt++] = in_data[i];
        }
        else
        {
            i++;
            out_data[cnt++] = in_data[i];
        }
    }
    cnt++;
    out_data[cnt - 1] = in_data[in_len - 1];
    return cnt;
}

extern OS_EVENT *sw_rfid_ack_queue_handle;

uint8_t sanwei_rfid_rcv_proccess(uint8_t *data_tmp, uint8_t len_tmp)
{
    uint16_t module_addr = 0;
    uint8_t cmd = 0;
    uint8_t result = 0xff;
    sw_rfid_ack_t *ack = NULL;
    uint8_t data[32] = {0};
    uint8_t len = 0;
    len = pre_porc_ack_data(data_tmp, len_tmp, data);
    if(is_frame_valid(data, len))
    {
        module_addr = data[1] | (data[2] << 8);
        cmd = data[4];
        result = data[5];
        if(is_protocol_cmd(cmd))
        {
            uint8_t err = 0;
            ack = (sw_rfid_ack_t *)OSMemGet(sw_rfid_ack_mem_handle, &err);
            if(ack)
            {
                ack->cmd = cmd;
                ack->result = result;
                ack->module_addr = module_addr;
                ack->data_len = len - 8;
                if(ack->data_len > 0)
                {
                    memcpy(ack->data, &data[6], ack->data_len);
                }
                OSQPost(sw_rfid_ack_queue_handle, (void *)ack);
            }
        }
    }
    return FALSE;
}



#define SW_RFID_ACK_TIME_OUT    (5 * OS_TICKS_PER_SEC / 10)
int get_sw_rfid_id(uint16_t *card_id, uint16_t *station_id)    //不做出错重发，出错就返回，重发留给工控
{
    sw_rfid_ack_t *ack = NULL;
    uint8_t err = 0;
    uint32_t card_uuid = 0;
    
    /*******寻卡*******/
    search_card();
    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
    if(ack)
    {
        if((ack->cmd == SW_RFID_PROTOCOL_CMD_SEARCH_CARD) && (ack->result == 0))
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
        }
        else
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
            return -1;
        }
    }
    else
    {
        return -1;
    }

    //delay_ms(30);
    /*******防冲突*******/
    prevent_conflict();
    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
    if(ack)
    {
        if((ack->cmd == SW_RFID_PROTOCOL_CMD_PREVENT_CONFLICT) && (ack->result == 0))
        {
            if(ack->data_len == 4)
            {
                card_uuid = (ack->data[0] << 24) | (ack->data[1] << 16) | (ack->data[2] << 8) | ack->data[3];
            }
            OSMemPut(sw_rfid_ack_mem_handle, ack);
        }
        else
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
            return -1;
        }
    }
    else
    {
        return -1;
    }

    //delay_ms(30);
    /*******根据卡的唯一id来选卡*******/
    select_card(card_uuid);
    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
    if(ack)
    {
        if((ack->cmd == SW_RFID_PROTOCOL_CMD_SELECT_CARD) && (ack->result == 0))
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
        }
        else
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
            return -1;
        }
    }
    else
    {
        return -1;
    }

    //delay_ms(30);
    /*******验证密钥 0xff 0xff 0xff 0xff 0xff 0xff*******/
    verify_secret_key(52);
    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
    if(ack)
    {
        if((ack->cmd == SW_RFID_PROTOCOL_CMD_VERIFY_SECRET_KEY) && (ack->result == 0))
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
        }
        else
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
            return -1;
        }
    }

    //delay_ms(30);
    /*******读块，id 数据存在 块52的前两个字节*******/
    read_rfid(52);
    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
    if(ack)
    {
        if((ack->cmd == SW_RFID_PROTOCOL_CMD_READ_BLOCK) && (ack->result == 0))
        {
            if(ack->data_len == 16)
            {
//                memcpy((uint8_t *)id, ack->data, 2);
                *card_id = (ack->data[0] << 8) | ack->data[1];
            }
            OSMemPut(sw_rfid_ack_mem_handle, ack);
        }
        else
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
            return -1;
        }
    }
    return 0;
}


int write_sw_rfid_info(uint16_t dst_id, uint16_t src_id, uint16_t service_id, uint16_t time)
{
    uint16_t card_id = 0;
    uint16_t station_id = 0;
    uint8_t data[16] = {0};
    sw_rfid_ack_t *ack = NULL;
    uint8_t err = 0;

    if(get_sw_rfid_id(&card_id, &station_id) < 0)
    {
        return -1;
    }

    data[0] = card_id >> 8;
    data[1] = card_id & 0xff;

    data[2] = station_id >> 8;
    data[3] = station_id & 0xff;

    data[4] = dst_id >> 8;
    data[5] = dst_id & 0xff;

    data[6] = src_id >> 8;
    data[7] = src_id & 0xff;

    data[8] = service_id >> 8;
    data[9] = service_id & 0xff;

    data[10] = time >> 8;
    data[11] = time & 0xff;

    /*******验证密钥 0xff 0xff 0xff 0xff 0xff 0xff*******/
    verify_secret_key(56);
    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
    if(ack)
    {
        if((ack->cmd == SW_RFID_PROTOCOL_CMD_VERIFY_SECRET_KEY) && (ack->result == 0))
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
        }
        else
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
            return -1;
        }
    }
    //delay_ms(30);

    write_rfid(56, data);

    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
    if(ack)
    {
        if((ack->cmd == SW_RFID_PROTOCOL_CMD_WRITE_BLOCK) && (ack->result == 0))
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
        }
        else
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
            return -1;
        }
    }
    return 0;
}



int read_sw_rfid_info(uint16_t *card_id, uint16_t *station_id, uint16_t *dst_id, uint16_t *src_id, uint16_t *service_id, uint16_t *time)
{
    sw_rfid_ack_t *ack = NULL;
    uint8_t err = 0;

    if(get_sw_rfid_id(card_id, station_id) < 0)
    {
        return -1;
    }


    /*******验证密钥 0xff 0xff 0xff 0xff 0xff 0xff*******/
    verify_secret_key(56);
    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
    if(ack)
    {
        if((ack->cmd == SW_RFID_PROTOCOL_CMD_VERIFY_SECRET_KEY) && (ack->result == 0))
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
        }
        else
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
            return -1;
        }
    }
    //delay_ms(30);

    read_rfid(56);
    ack = (sw_rfid_ack_t *)OSQPend(sw_rfid_ack_queue_handle, SW_RFID_ACK_TIME_OUT, &err);
    if(ack)
    {
        if((ack->cmd == SW_RFID_PROTOCOL_CMD_READ_BLOCK) && (ack->result == 0))
        {
            if(ack->data_len == 16)
            {
                *dst_id = (ack->data[4] << 8) | ack->data[5];
                *src_id = (ack->data[6] << 8) | ack->data[7];
                *service_id = *src_id;
                *time = (ack->data[10] << 8) | ack->data[11];
            }
            OSMemPut(sw_rfid_ack_mem_handle, ack);
        }
        else
        {
            OSMemPut(sw_rfid_ack_mem_handle, ack);
            return -1;
        }
    }
    return 0;
}

