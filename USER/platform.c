/*
 *  Author: Kaka Xie
 *  brief: platform
 */

#include "platform.h"
#include "delay.h"
#include "stdio.h"

#include "led.h"
#include "can.h"
#include "timer.h"
#include "sys.h"
#include "id.h"

sys_status_t sys_status_ram = {0};
sys_status_t *sys_status = &sys_status_ram;

const platform_gpio_t platform_gpio_pins[] =
{

    [PLATFORM_GPIO_SYS_LED]                     = { GPIOC,  GPIO_Pin_9 },

    [PLATFORM_GPIO_5V_EN]                       = { GPIOE,  GPIO_Pin_7 },
    [PLATFORM_GPIO_12V_EN]                      = { GPIOE,  GPIO_Pin_8 },
    [PLATFORM_GPIO_24V_EN]                      = { GPIOE,  GPIO_Pin_9 },

    [PLATFORM_GPIO_PWR_NV]                      = { GPIOF, GPIO_Pin_13 },

    [PLATFORM_GPIO_PWR_RESERVE]                 = { GPIOG,  GPIO_Pin_1 },

    [PLATFORM_GPIO_VSYS_24V_NV_EN]              = { GPIOG,  GPIO_Pin_6 },

    [PLATFORM_GPIO_PHO_SWITCH_1]                = {GPIOC, GPIO_Pin_10},
    [PLATFORM_GPIO_PHO_SWITCH_2]                = {GPIOB, GPIO_Pin_10},
    [PLATFORM_GPIO_PHO_SWITCH_3]                = {GPIOC, GPIO_Pin_11},
    [PLATFORM_GPIO_MOTOR_REVERSE_FORWARD]       = {GPIOF, GPIO_Pin_13},
    [PLATFORM_GPIO_MOTOR_START_STOP]            = {GPIOG, GPIO_Pin_1},
};



uint32_t get_tick(void)
{
    return OSTimeGet();
}


void mcu_restart(void)
{
    __set_FAULTMASK(1);//close all interrupt
    NVIC_SystemReset();//reset all
}


static void input_gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

}

static void output_gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /*GPIO_E*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /*GPIO_G*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
}

static void charge_gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    EXTI_InitTypeDef exit_init_structure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);   // πƒ‹ ±÷”


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource7);

    exit_init_structure.EXTI_Line = EXTI_Line6 | EXTI_Line7;
    exit_init_structure.EXTI_Mode = EXTI_Mode_Interrupt;
    exit_init_structure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    exit_init_structure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exit_init_structure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


static void init_reset_gpio(void)
{
    GPIO_ResetBits(GPIOE, GPIO_Pin_9);
    GPIO_ResetBits(GPIOG, GPIO_Pin_5);
}

static void init_set_gpio(void)
{
    GPIO_SetBits(GPIOD, GPIO_Pin_10);
}

static void pho_switch_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    /*GPIO_C*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;

    GPIO_Init(GPIOC, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;

    GPIO_Init(GPIOB, &GPIO_InitStructure);

}

static void motor_ctrl_gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /*GPIO_F*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    /*GPIO_G*/
    GPIO_ResetBits(GPIOG, GPIO_Pin_1);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOG, GPIO_Pin_1);
}

static void platform_gpio_init(void)
{
    output_gpio_init();
    init_set_gpio();
    init_reset_gpio();

    input_gpio_init();
    //charge_gpio_init();
    pho_switch_init();
    motor_ctrl_gpio_init();
}


uint8_t get_switch_state(void)
{
    return GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11);
}


uint8_t set_conveyor_start(void)
{
    GPIO_SetBits(GPIOG, GPIO_Pin_1);
    return 0;
}

uint8_t set_conveyor_stop(void)
{
    GPIO_ResetBits(GPIOG, GPIO_Pin_1);
    return 0;
}

uint8_t forward_conveyor_belt(void)
{
    set_conveyor_start();
    GPIO_ResetBits(GPIOF, GPIO_Pin_13);
    return 0;
}

uint8_t reverse_conveyor_belt(void)
{
    set_conveyor_start();
    GPIO_SetBits(GPIOF, GPIO_Pin_13);
    return 0;
}

uint8_t stop_conveyor_belt(void)
{
    set_conveyor_stop();
    return 1;
}

uint8_t get_pho_switch_1_state(void)
{
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_10) == 1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

uint8_t get_pho_switch_2_state(void)
{
    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

uint8_t get_pho_switch_3_state(void)
{
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11) == 1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

uint32_t test_hardware_version = 0;
void hardware_init(void)
{
    platform_gpio_init();
    led_init();
    init_can1();
}

void user_param_init(void)
{

}

