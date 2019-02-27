/*
 *  Author: Kaka Xie
 *  brief: main entrance
 */

#include "delay.h"
#include "includes.h"
#include "common.h"
#include "platform.h"


uint8_t test_id = 0;
int main(void)
{
    OSInit();
    delay_init();
    NVIC_Configuration();
    __disable_irq();
    hardware_init();
    user_param_init();
    user_init();
    __enable_irq();
    OSStart();
}

