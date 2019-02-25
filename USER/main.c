/*
 *  Author: Kaka Xie
 *  brief: main entrance
 */

#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "includes.h"
#include "math.h"
#include "common.h"
#include "platform.h"
#include "flash.h"


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

