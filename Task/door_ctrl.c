/*
 *  Author: Kaka Xie
 *  brief: conveyor belt task
 */

#include "conveyor_belt_task.h"
#include "delay.h"
#include "platform.h"

#include <string.h>
#include "door_ctrl.h"

#define UPPER_STATE_DEBOUNCE_CNT    3
#define LOWER_STATE_DEBOUNCE_CNT    3
uint8_t upper_door_limit_state = 0;
uint8_t lower_door_limit_state = 0;

OS_STK door_ctrl_task_stk[DOOR_CTRL_TASK_STK_SIZE] = {0};
OS_STK door_state_detection_task_stk[DOOR_STATE_DETECTION_TASK_STK_SIZE] = {0};


uint8_t get_upper_door_state(void)
{
    return (get_upper_door_up_limit_state() ? UPPER_UP_LIMIT_TRIGGED : 0) | (get_upper_door_down_limit_state() ? UPPER_DOWN_LIMIT_TRIGGED : 0);
}

uint8_t get_lower_door_state(void)
{
    return (get_lower_door_up_limit_state() ? LOWER_UP_LIMIT_TRIGGED : 0) | (get_lower_door_down_limit_state() ? LOWER_DOWN_LIMIT_TRIGGED : 0);
}


void door_state_detect_task(void *pdata)
{
    uint8_t upper_state = 0;
    uint8_t lower_state = 0;
    uint8_t pre_upper_state = 0;
    uint8_t pre_lower_state = 0;

    uint8_t upper_state_cnt = 0;
    uint8_t lower_state_cnt = 0;
    delay_ms(1000);
    for(;;)
    {
        upper_state = get_upper_door_state();
        if(pre_upper_state == upper_state)
        {
            upper_state_cnt++;
        }
        else
        {
            upper_state_cnt = 0;
        }
        pre_upper_state = upper_state;
        if(upper_state_cnt >= UPPER_STATE_DEBOUNCE_CNT)
        {
            upper_state_cnt = 0;
            OS_ENTER_CRITICAL();
            upper_door_limit_state = upper_state;
            OS_EXIT_CRITICAL();
            /*
            TODO: post state
            */
        }


        lower_state = get_lower_door_state();
        if(pre_lower_state == lower_state)
        {
            lower_state_cnt++;
        }
        else
        {
            lower_state_cnt = 0;
        }
        pre_lower_state = lower_state;
        if(lower_state_cnt >= UPPER_STATE_DEBOUNCE_CNT)
        {
            lower_state_cnt = 0;
            OS_ENTER_CRITICAL();
            lower_door_limit_state = lower_state;
            OS_EXIT_CRITICAL();
            /*
            TODO: post state
            */
        }

        delay_ms(20);
    }
}

#define DOOR_MOVE_TIME_MAX      (5 * OS_TICKS_PER_SEC)

int reset_lower_door_position(void)
{
    uint32_t start_tick = 0;
    uint8_t lower_state = 0;
    uint8_t machine_state = 0;
    OS_ENTER_CRITICAL();
    lower_state = lower_door_limit_state;
    OS_EXIT_CRITICAL();
    if((get_upper_door_state() & UPPER_UP_LIMIT_TRIGGED) == 0)
    {
        lower_door_ctrl(1);
        start_tick = get_tick();
    }

    while((lower_state & UPPER_UP_LIMIT_TRIGGED) == 0)
    {
        delay_ms(20);
        if(get_tick() - start_tick >= DOOR_MOVE_TIME_MAX)
        {
            /*
            timeout
            */
            return -1;
        }
        OS_ENTER_CRITICAL();
        lower_state = lower_door_limit_state;
        OS_EXIT_CRITICAL();
        switch(machine_state)
        {
            case 0:     //wait for reaching down limit
                if(lower_state & UPPER_DOWN_LIMIT_TRIGGED)
                {
                    lower_door_ctrl(0);
                    delay_ms(500);
                    machine_state = 1;
                    start_tick = get_tick();
                }
                break;
            case 1:     //reached down limit, restart to move the door
                lower_door_ctrl(1);
                machine_state = 2;
                break;
            case 2:
                if(lower_state & UPPER_UP_LIMIT_TRIGGED)
                {
                    return 0;
                }
                break;
        }
    }
    return 0;
}

void door_ctrl_task(void *pdata)
{
    delay_ms(3000);     //wait for door state deteciton ready
    //校准位置
    reset_lower_door_position();
    for(;;)
    {
        delay_ms(20);
    }
}
