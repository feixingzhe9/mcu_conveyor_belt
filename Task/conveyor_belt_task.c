/*
 *  Author: Kaka Xie
 *  brief: conveyor belt task
 */

#include "conveyor_belt_task.h"
#include "delay.h"
#include "platform.h"
#include "photoelectric_switch.h"
#include "conveyor_belt.h"
#include "can_protocol_task.h"
#include <string.h>

OS_STK conveyor_belt_task_stk[CONVEYOR_BELT_TASK_STK_SIZE] = {0};
OS_STK pho_switch_status_task_stk[PHO_SWITCH_STATUS_TASK_STK_SIZE] = {0};


OS_STK pho_state_upload_task_stk[PHO_STATE_UPLOAD_TASK_STK_SIZE] = {0};


OS_EVENT * pho_state_mailbox;

extern void upload_conveyor_belt_status(uint8_t status);


void upload_pho_state_upload_task(void *pdata)
{
    uint8_t err = 0;
    uint32_t *pho_state = 0;
    delay_ms(500);
    while(1)
    {
        pho_state = (uint32_t *)OSMboxPend(pho_state_mailbox, 0, &err);
        if((err == OS_ERR_NONE) && (pho_state != (void *)0))
        {
            upload_pho_state(*pho_state);
            delay_ms(50);
        }
    }
}

#define PHO_SWITCH_STATE_BUF_SIZE   7
void pho_switch_status_task(void *pdata)
{
    uint8_t state_buf[PHO_SWITCH_STATE_BUF_SIZE] = {0};
    uint8_t state_cnt_buf[PHO_SWITCH_NUM_MAX] = {0};
    uint8_t state_tmp = 0;
    uint32_t state_ram = 0;
    uint8_t pre_state = 0;
    uint8_t cnt = 0;
    uint8_t i = 0;
    delay_ms(5000);
    while(1)
    {
        state_buf[cnt] = get_pho_switch_state();
        if(cnt < PHO_SWITCH_STATE_BUF_SIZE)
        {
            cnt++;
        }
        else
        {
            cnt = 0;
        }
        memset(state_cnt_buf, 0, sizeof(state_cnt_buf));
        for(i = 0; i < PHO_SWITCH_STATE_BUF_SIZE; i++)
        {
            if(state_buf[i] & PHO_SWITCH_1_TRIGGERED)
            {
                state_cnt_buf[0]++;
            }
            if(state_buf[i] & PHO_SWITCH_2_TRIGGERED)
            {
                state_cnt_buf[1]++;
            }
            if(state_buf[i] & PHO_SWITCH_3_TRIGGERED)
            {
                state_cnt_buf[2]++;
            }
        }
        if(state_cnt_buf[0] > PHO_SWITCH_STATE_BUF_SIZE / 2)
        {
            state_tmp |= PHO_SWITCH_1_TRIGGERED;
        }
        else
        {
            state_tmp &= ~PHO_SWITCH_1_TRIGGERED;
        }
        if(state_cnt_buf[1] > PHO_SWITCH_STATE_BUF_SIZE / 2)
        {
            state_tmp |= PHO_SWITCH_2_TRIGGERED;
        }
        else
        {
            state_tmp &= ~PHO_SWITCH_2_TRIGGERED;
        }
        if(state_cnt_buf[2] > PHO_SWITCH_STATE_BUF_SIZE / 2)
        {
            state_tmp |= PHO_SWITCH_3_TRIGGERED;
        }
        else
        {
            state_tmp &= ~PHO_SWITCH_3_TRIGGERED;
        }
        OS_ENTER_CRITICAL();
        //pho_switch_state = get_pho_switch_state();
        pho_switch_state = state_tmp;
        OS_EXIT_CRITICAL();
        if(pre_state != state_tmp)
        {
            state_ram = state_tmp;
            OSMboxPost(pho_state_mailbox, (void*)&state_ram);
        }
        pre_state = state_tmp;
        delay_ms(20);
    }
}


#define TICK_DELAY_MS           20
#define STOP_TICK_CNT           (500 / TICK_DELAY_MS)
#define LOAD_TIME_OUT_CNT       (120000 / TICK_DELAY_MS)
#define UNLOAD_TIME_OUT_CNT     (120000 / TICK_DELAY_MS)
#define UNLOAD_STOP_CNT         (2000 / TICK_DELAY_MS)
void conveyor_belt_task(void *pdata)
{
    uint8_t work_mode = 0;
    uint8_t pre_work_mode = 0;
    uint32_t tick_cnt = 0;
    uint32_t stop_cnt = 0;
    uint32_t unload_stop_cnt = 0;
    uint32_t load_cnt = 0;
    uint32_t unload_cnt = 0;
    uint8_t load_state = 0;
    uint8_t unload_state = 0;
    uint8_t switch_state = 0;
    delay_ms(5000);
    while(1)
    {

        OS_ENTER_CRITICAL();
        work_mode = conveyor_belt.work_mode;
        switch_state = pho_switch_state;
        OS_EXIT_CRITICAL();
        if(work_mode != pre_work_mode)
        {
            pre_work_mode = work_mode;
            stop_conveyor_belt();
            stop_cnt = STOP_TICK_CNT;
            load_cnt = 0;
            unload_cnt = 0;
        }
        if(stop_cnt > 0)
        {
            stop_cnt--;
        }
        else
        {
            if(work_mode == CONVEYOR_BELT_STATUS_LOAD)
            {
                load_cnt++;
                switch(load_state)
                {
                    case 0: //forward
                        if(switch_state & PHO_SWITCH_3_TRIGGERED)
                        {
                            stop_conveyor_belt();   //stop immediately !
                            if(conveyor_belt.need_lock == 0)
                            {
                                load_state = 4;
                            }
                            else
                            {
                                load_state = 3;
                            }
                        }
                        else
                        {
                            load_state = 1;
                        }
                        break;

                    case 1:
                        forward_conveyor_belt();
                        load_state = 2;
                        break;

                    case 2:
                        if(switch_state & PHO_SWITCH_3_TRIGGERED)
                        {
                            stop_conveyor_belt();   //stop immediately !
                            if(conveyor_belt.need_lock == 0)
                            {
                                load_state = 4;
                            }
                            else
                            {
                                load_state = 3;
                            }
                        }
                        break;

                    case 3:     //delay and lock
                        if(conveyor_belt.need_lock == 1)
                        {
                            //delay_ms(800);
                            lock_ctrl(LOCK_STATUS_LOCK);
                        }
                        load_state = 4;
                        break;

                    case 4:     //load finished
                        OS_ENTER_CRITICAL();
                        conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_STOP;
                        OS_EXIT_CRITICAL();
                        work_mode = CONVEYOR_BELT_STATUS_STOP;
                        upload_conveyor_belt_status(CONVEYOR_LOAD_FINISHED_OK);
                        load_state = 0;
                        break;

                    default :
                        break;
                }
                if(load_cnt >= LOAD_TIME_OUT_CNT)
                {
                    OS_ENTER_CRITICAL();
                    conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_STOP;
                    OS_EXIT_CRITICAL();
                    work_mode = CONVEYOR_BELT_STATUS_STOP;
                    upload_conveyor_belt_status(CONVEYOR_BELT_LOAD_TIME_OUT);
                    load_state = 0;
                }
            }
            else if(work_mode == CONVEYOR_BELT_STATUS_UNLOAD)
            {
                unload_cnt++;
                switch(unload_state)
                {
                    case 0:     //unlock first
                        lock_ctrl(LOCK_STATUS_UNLOCK);
                        delay_ms(800);
                        unload_state = 1;
                        break;

                    case 1:     //
                        //reverse_conveyor_belt();
                        if(switch_state == 0)
                        {
                            unload_state = 3;
                            unload_stop_cnt = UNLOAD_STOP_CNT;
                        }
                        else
                        {
                            unload_state = 2;
                        }
                        break;

                    case 2:
                        reverse_conveyor_belt();
                        unload_state = 3;
                        break;

                    case 3:
                        if(switch_state == 0)
                        {
                            unload_state = 4;
                            unload_stop_cnt = UNLOAD_STOP_CNT;
                        }
                        break;

                    case 4:
                        if(switch_state > 0)
                        {
                            //unload_state = 1;
                        }
                        if(unload_stop_cnt > 0)
                        {
                            unload_stop_cnt--;
                        }
                        else
                        {
                            OS_ENTER_CRITICAL();
                            conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_STOP;
                            OS_EXIT_CRITICAL();
                            work_mode = CONVEYOR_BELT_STATUS_STOP;
                            unload_state = 0;
                            upload_conveyor_belt_status(CONVEYOR_UNLOAD_FINISHED_OK);
                        }
                        break;

                    default :
                        break;
                }
                if(unload_cnt >= UNLOAD_TIME_OUT_CNT)
                {
                    OS_ENTER_CRITICAL();
                    conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_STOP;
                    OS_EXIT_CRITICAL();
                    work_mode = CONVEYOR_BELT_STATUS_STOP;
                    upload_conveyor_belt_status(CONVEYOR_BELT_UNLOAD_TIME_OUT);
                    unload_state = 0;
                }
            }
        }

        delay_ms(TICK_DELAY_MS);
        tick_cnt++;

    }
}
