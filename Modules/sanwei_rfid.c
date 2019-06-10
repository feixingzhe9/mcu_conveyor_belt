/*
 *  Author: Kaka Xie
 *  brief: sanwei rfid
 */

#include "sanwei_rfid.h"
#include "usart.h"

static uint8_t pre_send_data_buf[PRE_SEND_DATA_LEN] = {0x02,0x04,0x05,0x02,0x03,0x10,0x03};
static uint8_t send_data_buf[SEND_DATA_LEN] = {0};


void sanwei_rfid_init(void)
{
    uart2_dma_init(19200);
}

#define PROTOCOL_HEAD           0x02
#define PROTOCOL_TAIL           0x03
#define INSERT_ESCAPE           0x10

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

uint8_t read_rfid(uint8_t absolute_block_num)
{
    uint8_t check_value = 0;
    uint8_t len = 0;
    pre_send_data_buf[0] = PROTOCOL_HEAD;
    pre_send_data_buf[1] = 0;       //模块地址
    pre_send_data_buf[2] = 4;       //length:从长度字到校验字的字节数
    pre_send_data_buf[3] = 0x4b;    //cmd: 读块
    /* data 域 */
    pre_send_data_buf[4] = absolute_block_num;  //绝对块号

    pre_send_data_buf[5] = check_value;         //校验字
    pre_send_data_buf[6] = PROTOCOL_TAIL;

    len = pre_proc_data(pre_send_data_buf, 7, send_data_buf);
    if(len > 0)
    {
        uart2_send(send_data_buf, len);
    }
    return 0;
}

