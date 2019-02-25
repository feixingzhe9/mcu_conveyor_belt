/*
 *  Author: Kaka Xie
 *  brief: conveyor belt task
 */

#include "conveyor_belt_task.h"
#include "delay.h"
#include "platform.h"
#include "photoelectric_switch.h"

OS_STK conveyor_belt_task_stk[CONVEYOR_BELT_TASK_STK_SIZE] = {0};
OS_STK pho_switch_status_task_stk[PHO_SWITCH_STATUS_TASK_STK_SIZE] = {0};

extern void upload_conveyor_belt_status(uint8_t status);
uint8_t pho_switch_state = 0;


void pho_switch_status_task(void *pdata)
{
    delay_ms(5000);
    while(1)
    {
        pho_switch_state = get_pho_switch_state();
        delay_ms(50);
    }
}

void conveyor_belt_task(void *pdata)
{
    delay_ms(5000);
    while(1)
    {
        set_conveyor_belt_forward();
        delay_ms(10000);
        set_conveyor_belt_stop();
        delay_ms(3000);
        set_conveyor_belt_reverse();
        delay_ms(10000);
        set_conveyor_belt_stop();
        delay_ms(3000);

    }
}
