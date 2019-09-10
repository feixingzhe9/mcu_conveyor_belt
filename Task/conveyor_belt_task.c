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


typedef uint8_t (*forward_conveyor_belt_fn)(void);
typedef uint8_t (*reverse_conveyor_belt_fn)(void);
typedef uint8_t (*stop_conveyor_belt_fn)(void);
typedef uint8_t (*get_pho_switch_state_fn)(void);


forward_conveyor_belt_fn forward_conveyor_belt_f;
reverse_conveyor_belt_fn reverse_conveyor_belt_f;
stop_conveyor_belt_fn stop_conveyor_belt_f;
get_pho_switch_state_fn get_pho_switch_state_f;



void set_work_conveyor(uint8_t deck)
{
    switch(deck)
    {
        case DECK_LOWER:
            forward_conveyor_belt_f = lower_conveyor_belt_load;
            reverse_conveyor_belt_f = lower_conveyor_belt_unload;
            get_pho_switch_state_f = get_pho_switch_lower_state;
            stop_conveyor_belt_f = stop_conveyor_belt;
            conveyor_index = DECK_LOWER;
            break;

        case DECK_UPPER:
            forward_conveyor_belt_f = upper_conveyor_belt_load;
            reverse_conveyor_belt_f = upper_conveyor_belt_unload;
            get_pho_switch_state_f = get_pho_switch_upper_state;
            stop_conveyor_belt_f = stop_conveyor_belt;
            conveyor_index = DECK_UPPER;
            break;

        default: return;
    }
    stop_conveyor_belt_f();
}

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
    delay_ms(500);
    while(1)
    {
        state_buf[cnt] = get_pho_switch_state_f();
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
static uint8_t work_mode[DECK_MAX] = {0};
static uint8_t pre_work_mode[DECK_MAX] = {0};
static uint32_t tick_cnt[DECK_MAX] = {0};
static uint32_t stop_cnt[DECK_MAX] = {0};
static uint32_t unload_stop_cnt[DECK_MAX] = {0};
static uint32_t load_cnt[DECK_MAX] = {0};
static uint32_t unload_cnt[DECK_MAX] = {0};
static uint8_t load_state[DECK_MAX] = {0};
static uint8_t unload_state[DECK_MAX] = {0};
static uint8_t switch_state[DECK_MAX] = {0};
void conveyor_belt_task(void *pdata)
{

    set_work_conveyor(DECK_LOWER);
//    set_conveyor_belt_load(0);
    delay_ms(3000);
    set_conveyor_belt_load(0);
    while(1)
    {
        OS_ENTER_CRITICAL();
        work_mode[conveyor_index] = conveyor_belt[conveyor_index].work_mode;
        switch_state[conveyor_index] = pho_switch_state;
        OS_EXIT_CRITICAL();
        if(work_mode[conveyor_index] != pre_work_mode[conveyor_index])
        {
            pre_work_mode[conveyor_index] = work_mode[conveyor_index];
            stop_conveyor_belt_f();
            stop_cnt[conveyor_index] = STOP_TICK_CNT;
            load_cnt[conveyor_index] = 0;
            unload_cnt[conveyor_index] = 0;
        }
        if(stop_cnt[conveyor_index] > 0)
        {
            stop_cnt[conveyor_index]--;
        }
        else
        {
            if(work_mode[conveyor_index] == CONVEYOR_BELT_STATUS_LOAD)
            {
                load_cnt[conveyor_index]++;
                switch(load_state[conveyor_index])
                {
                    case 0: //forward
                        lock_ctrl(LOCK_STATUS_UNLOCK);
                        if(switch_state[conveyor_index] & PHO_SWITCH_3_TRIGGERED)
                        {
                            stop_conveyor_belt_f();   //stop immediately !
                            if(conveyor_belt[conveyor_index].need_lock == 0)
                            {
                                load_state[conveyor_index] = 4;
                            }
                            else
                            {
                                load_state[conveyor_index] = 3;
                            }
                        }
                        else
                        {
                            load_state[conveyor_index] = 1;
                        }
                        break;

                    case 1:
                        forward_conveyor_belt_f();
                        load_state[conveyor_index] = 2;
                        break;

                    case 2:
                        if(switch_state[conveyor_index] & PHO_SWITCH_3_TRIGGERED)
                        {
                            stop_conveyor_belt_f();   //stop immediately !
                            if(conveyor_belt[conveyor_index].need_lock == 0)
                            {
                                load_state[conveyor_index] = 4;
                            }
                            else
                            {
                                load_state[conveyor_index] = 3;
                            }
                        }
                        break;

                    case 3:     //delay and lock
                        if(conveyor_belt[conveyor_index].need_lock == 1)
                        {
                            //delay_ms(800);
                            lock_ctrl(LOCK_STATUS_LOCK);
                        }
                        load_state[conveyor_index] = 4;
                        break;

                    case 4:     //load finished
                        OS_ENTER_CRITICAL();
                        conveyor_belt[conveyor_index].work_mode = CONVEYOR_BELT_STATUS_STOP;
                        OS_EXIT_CRITICAL();
                        work_mode[conveyor_index] = CONVEYOR_BELT_STATUS_STOP;
                        upload_conveyor_belt_status(CONVEYOR_LOAD_FINISHED_OK);
                        load_state[conveyor_index] = 0;
                        break;

                    default :
                        break;
                }
                if(load_cnt[conveyor_index] >= LOAD_TIME_OUT_CNT)
                {
                    OS_ENTER_CRITICAL();
                    conveyor_belt[conveyor_index].work_mode = CONVEYOR_BELT_STATUS_STOP;
                    OS_EXIT_CRITICAL();
                    work_mode[conveyor_index] = CONVEYOR_BELT_STATUS_STOP;
                    upload_conveyor_belt_status(CONVEYOR_BELT_LOAD_TIME_OUT);
                    load_state[conveyor_index] = 0;
                }
            }
            else if(work_mode[conveyor_index] == CONVEYOR_BELT_STATUS_UNLOAD)
            {
                unload_cnt[conveyor_index]++;
                switch(unload_state[conveyor_index])
                {
                    case 0:     //unlock first
                        lock_ctrl(LOCK_STATUS_UNLOCK);
                        delay_ms(800);
                        unload_state[conveyor_index] = 1;
                        break;

                    case 1:     //
                        //reverse_conveyor_belt();
                        if(switch_state[conveyor_index] == 0)
                        {
                            unload_state[conveyor_index] = 3;
                            unload_stop_cnt[conveyor_index] = UNLOAD_STOP_CNT;
                        }
                        else
                        {
                            unload_state[conveyor_index] = 2;
                        }
                        break;

                    case 2:
                        reverse_conveyor_belt_f();
                        unload_state[conveyor_index] = 3;
                        break;

                    case 3:
                        if(switch_state[conveyor_index] == 0)
                        {
                            unload_state[conveyor_index] = 4;
                            unload_stop_cnt[conveyor_index] = UNLOAD_STOP_CNT;
                        }
                        break;

                    case 4:
                        if(switch_state[conveyor_index] > 0)
                        {
                            //unload_state = 1;
                        }
                        if(unload_stop_cnt[conveyor_index] > 0)
                        {
                            unload_stop_cnt[conveyor_index]--;
                        }
                        else
                        {
                            OS_ENTER_CRITICAL();
                            conveyor_belt[conveyor_index].work_mode = CONVEYOR_BELT_STATUS_STOP;
                            OS_EXIT_CRITICAL();
                            work_mode[conveyor_index] = CONVEYOR_BELT_STATUS_STOP;
                            unload_state[conveyor_index] = 0;
                            upload_conveyor_belt_status(CONVEYOR_UNLOAD_FINISHED_OK);
                        }
                        break;

                    default :
                        break;
                }
                if(unload_cnt[conveyor_index] >= UNLOAD_TIME_OUT_CNT)
                {
                    OS_ENTER_CRITICAL();
                    conveyor_belt[conveyor_index].work_mode = CONVEYOR_BELT_STATUS_STOP;
                    OS_EXIT_CRITICAL();
                    work_mode[conveyor_index] = CONVEYOR_BELT_STATUS_STOP;
                    upload_conveyor_belt_status(CONVEYOR_BELT_UNLOAD_TIME_OUT);
                    unload_state[conveyor_index] = 0;
                }
            }
        }

        delay_ms(TICK_DELAY_MS);
        tick_cnt[conveyor_index]++;

    }
}
