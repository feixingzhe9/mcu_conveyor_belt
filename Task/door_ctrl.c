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
uint8_t door_limit_state = 0;

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
            door_limit_state &= 0xf0;
            door_limit_state += upper_state;
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
            door_limit_state &= 0x0f;
            door_limit_state += lower_state;
            OS_EXIT_CRITICAL();
            /*
            TODO: post state
            */
        }

        delay_ms(20);
    }
}

#define DOOR_MOVE_TIME_MAX      (10 * OS_TICKS_PER_SEC)

int reset_lower_door_position(void)
{
    upper_door_ctrl(0);
    //lower_door_ctrl(0);
    delay_ms(1000 * 10);
    return 0;
}


#define DOOR_MOVE_DIR_UP        1
#define DOOR_MOVE_DIR_DOWN      0


typedef uint8_t (*door_ctrl_fn)(uint8_t);
typedef uint8_t (*stop_door_fn)(void);

int door_operation(door_ctrl_fn ctrl, uint8_t dir, uint8_t stop_condition, stop_door_fn stop_door )
{
    uint32_t start_tick = 0;
    uint8_t state = 0;
    int ret = -1;
    OS_ENTER_CRITICAL();
    state = door_limit_state;
    OS_EXIT_CRITICAL();
    if((state & stop_condition) == 0)
    {
        ctrl(dir);
        start_tick = get_tick();
    }
    else
    {
        ret = 0;
        goto exit;
    }

    while((state & stop_condition) == 0)
    {
        delay_ms(20);
        if(get_tick() - start_tick >= DOOR_MOVE_TIME_MAX)
        {
            /*
            timeout
            */
            ret = -1;
            goto exit;
        }

        OS_ENTER_CRITICAL();
        state = door_limit_state;
        OS_EXIT_CRITICAL();
    }
exit:
    stop_door();
    return ret;
}


int open_lower_door(void)
{
    return door_operation((door_ctrl_fn)lower_door_ctrl, DOOR_MOVE_DIR_UP, LOWER_UP_LIMIT_TRIGGED, (stop_door_fn)stop_lower_door);
}


int close_lower_door(void)
{
    return door_operation((door_ctrl_fn)lower_door_ctrl, DOOR_MOVE_DIR_DOWN, LOWER_DOWN_LIMIT_TRIGGED, (stop_door_fn)stop_lower_door);
}

int open_upper_door(void)
{
    return door_operation((door_ctrl_fn)upper_door_ctrl, DOOR_MOVE_DIR_DOWN, UPPER_DOWN_LIMIT_TRIGGED, (stop_door_fn)stop_upper_door);
}

int close_upper_door(void)
{
    return door_operation((door_ctrl_fn)upper_door_ctrl, DOOR_MOVE_DIR_UP, UPPER_UP_LIMIT_TRIGGED, (stop_door_fn)stop_upper_door);
}



void door_ctrl_task(void *pdata)
{
    delay_ms(5000);     //wait for door state deteciton ready
    close_upper_door();
    close_lower_door();
    for(;;)
    {
        delay_ms(20);
    }
}
