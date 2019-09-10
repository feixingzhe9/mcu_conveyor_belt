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
            /*
            TODO: post state
            */
        }

        delay_ms(20);
    }
}

#define DOOR_MOVE_TIME_MAX      (5 * OS_TICKS_PER_SEC)

void reset_lower_door_position(void)
{
    uint32_t start_tick = 0;
    if((get_upper_door_state() & UPPER_UP_LIMIT_TRIGGED) == 0)
    {
        lower_door_ctrl(1);
        start_tick = get_tick();
    }

    while((get_upper_door_state() & UPPER_UP_LIMIT_TRIGGED) == 0)
    {
        delay_ms(20);
        if(get_tick() - start_tick >= OS_TICKS_PER_SEC)
        {
            
        }
    }
}

void door_ctrl_task(void *pdata)
{
    delay_ms(1000);
    //校准位置
    reset_lower_door_position();
    for(;;)
    {
    }
}
