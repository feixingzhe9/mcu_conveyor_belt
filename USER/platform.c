/*
 *  Author: Kaka Xie
 *  brief: platform
 */

#include "platform.h"
#include "led.h"
#include "can.h"
#include "sys.h"
#include "delay.h"
#include "sanwei_rfid.h"

sys_status_t sys_status_ram = {0};
sys_status_t *sys_status = &sys_status_ram;

const platform_gpio_t platform_gpio_pins[] =
{
    [PLATFORM_GPIO_SYS_LED]                     = { GPIOB,  GPIO_Pin_3},

    [PLATFORM_GPIO_MOTOR_DIR]                   = {GPIOA, GPIO_Pin_6},
    [PLATFORM_GPIO_MOTOR_EN]                    = {GPIOA, GPIO_Pin_5},
    [PLATFORM_GPIO_MOTOR_PWR_EN]                = {GPIOA, GPIO_Pin_4},
    //[PLATFORM_GPIO_LOCK_CTRL]                   = {GPIOB, GPIO_Pin_7},

#if defined DOUBLE_DECK
    [PLATFORM_GPIO_PHO_SWITCH_LOWER_INSIDE]     = {GPIOB, GPIO_Pin_5},
    [PLATFORM_GPIO_PHO_SWITCH_LOWER_MID]        = {GPIOB, GPIO_Pin_6},
    [PLATFORM_GPIO_PHO_SWITCH_LOWER_OUTSIDE]    = {GPIOB, GPIO_Pin_7},
    [PLATFORM_GPIO_PHO_SWITCH_UPPER_INSIDE]     = {GPIOB, GPIO_Pin_8},
    [PLATFORM_GPIO_PHO_SWITCH_UPPER_MID]        = {GPIOB, GPIO_Pin_9},
    [PLATFORM_GPIO_PHO_SWITCH_UPPER_OUTSIDE]    = {GPIOB, GPIO_Pin_11},


    [PLATFORM_GPIO_UPPER_MOTOR_LOAD]            = {GPIOA, GPIO_Pin_5},
    [PLATFORM_GPIO_UPPER_MOTOR_UNLOAD]          = {GPIOA, GPIO_Pin_6},

    [PLATFORM_GPIO_LOWER_MOTOR_LOAD]            = {GPIOA, GPIO_Pin_7},
    [PLATFORM_GPIO_LOWER_MOTOR_UNLOAD]          = {GPIOB, GPIO_Pin_0},


    [PLATFORM_GPIO_UPPER_DOOR_OPEN_CTRL]             = {GPIOB, GPIO_Pin_1},
    [PLATFORM_GPIO_UPPER_DOOR_EN_CTRL]             = {GPIOB, GPIO_Pin_10},

    [PLATFORM_GPIO_LOWER_DOOR_OPEN_CTRL]             = {GPIOA, GPIO_Pin_9},
    [PLATFORM_GPIO_LOWER_DOOR_EN_CTRL]             = {GPIOA, GPIO_Pin_10},

    [PLATFORM_GPIO_DOOR_DETECT_UPPER_DOWN_LIMIT]= {GPIOA, GPIO_Pin_8},
    [PLATFORM_GPIO_DOOR_DETECT_UPPER_UP_LIMIT]  = {GPIOB, GPIO_Pin_4},

    [PLATFORM_GPIO_DOOR_DETECT_LOWER_DOWN_LIMIT]= {GPIOB, GPIO_Pin_14},
    [PLATFORM_GPIO_DOOR_DETECT_LOWER_UP_LIMIT]  = {GPIOB, GPIO_Pin_15},
#endif
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


static void pho_switch_init(void)   //new, done
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    /*GPIO_B*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void motor_ctrl_gpio_init(void)  //motor en, new, done
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /*GPIO_A*/
    GPIO_ResetBits(GPIOA, GPIO_Pin_5 | GPIO_Pin_7);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOA, GPIO_Pin_5 | GPIO_Pin_7);
}


static void motor_power_ctrl_gpio_init(void)        //same
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /*GPIO_A*/
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
}


static void motor_dir_gpio_init(void)       //new, done
{
     GPIO_InitTypeDef  GPIO_InitStructure;

    /*GPIO_A*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOA, GPIO_Pin_6);

    /*GPIO_B*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOB, GPIO_Pin_0);
}


static void upper_door_ctrl_gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /*GPIO_B*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB, GPIO_Pin_1 | GPIO_Pin_10);

}

static void upper_door_detect_gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    /*GPIO_B*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    GPIO_Init(GPIOB, &GPIO_InitStructure);


    /*GPIO_A*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void lower_door_ctrl_gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /*GPIO_A*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA/* | RCC_APB2Periph_AFIO*/, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOA, GPIO_Pin_10 | GPIO_Pin_9);
}

static void lower_door_detect_gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    /*GPIO_B*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void upper_door_ctrl(uint8_t en)
{
    if(en)
    {
        GPIO_SetBits(platform_gpio_pins[PLATFORM_GPIO_UPPER_DOOR_OPEN_CTRL].GPIOx, platform_gpio_pins[PLATFORM_GPIO_UPPER_DOOR_OPEN_CTRL].GPIO_Pin);
        GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_UPPER_DOOR_EN_CTRL].GPIOx, platform_gpio_pins[PLATFORM_GPIO_UPPER_DOOR_EN_CTRL].GPIO_Pin);
    }
    else
    {
        GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_UPPER_DOOR_EN_CTRL].GPIOx, platform_gpio_pins[PLATFORM_GPIO_UPPER_DOOR_EN_CTRL].GPIO_Pin);
        GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_UPPER_DOOR_OPEN_CTRL].GPIOx, platform_gpio_pins[PLATFORM_GPIO_UPPER_DOOR_OPEN_CTRL].GPIO_Pin);
    }
}

void stop_upper_door(void)
{
    GPIO_SetBits(platform_gpio_pins[PLATFORM_GPIO_UPPER_DOOR_EN_CTRL].GPIOx, platform_gpio_pins[PLATFORM_GPIO_UPPER_DOOR_EN_CTRL].GPIO_Pin);
    GPIO_SetBits(platform_gpio_pins[PLATFORM_GPIO_UPPER_DOOR_OPEN_CTRL].GPIOx, platform_gpio_pins[PLATFORM_GPIO_UPPER_DOOR_OPEN_CTRL].GPIO_Pin);
}


void lower_door_ctrl(uint8_t en)
{
    if(en)
    {
        GPIO_SetBits(platform_gpio_pins[PLATFORM_GPIO_LOWER_DOOR_OPEN_CTRL].GPIOx, platform_gpio_pins[PLATFORM_GPIO_LOWER_DOOR_OPEN_CTRL].GPIO_Pin);
        GPIO_SetBits(platform_gpio_pins[PLATFORM_GPIO_LOWER_DOOR_EN_CTRL].GPIOx, platform_gpio_pins[PLATFORM_GPIO_LOWER_DOOR_EN_CTRL].GPIO_Pin);
    }
    else
    {
        GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_LOWER_DOOR_OPEN_CTRL].GPIOx, platform_gpio_pins[PLATFORM_GPIO_LOWER_DOOR_OPEN_CTRL].GPIO_Pin);
        GPIO_SetBits(platform_gpio_pins[PLATFORM_GPIO_LOWER_DOOR_EN_CTRL].GPIOx, platform_gpio_pins[PLATFORM_GPIO_LOWER_DOOR_EN_CTRL].GPIO_Pin);
    }
}


void stop_lower_door(void)
{
    GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_LOWER_DOOR_OPEN_CTRL].GPIOx, platform_gpio_pins[PLATFORM_GPIO_LOWER_DOOR_OPEN_CTRL].GPIO_Pin);
    GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_LOWER_DOOR_EN_CTRL].GPIOx, platform_gpio_pins[PLATFORM_GPIO_LOWER_DOOR_EN_CTRL].GPIO_Pin);
}


uint8_t get_upper_door_up_limit_state(void)
{
    return GPIO_ReadInputDataBit(platform_gpio_pins[PLATFORM_GPIO_DOOR_DETECT_UPPER_UP_LIMIT].GPIOx, platform_gpio_pins[PLATFORM_GPIO_DOOR_DETECT_UPPER_UP_LIMIT].GPIO_Pin) == 1 ? 0 : 1; 
}

uint8_t get_upper_door_down_limit_state(void)
{
    return GPIO_ReadInputDataBit(platform_gpio_pins[PLATFORM_GPIO_DOOR_DETECT_UPPER_DOWN_LIMIT].GPIOx, platform_gpio_pins[PLATFORM_GPIO_DOOR_DETECT_UPPER_DOWN_LIMIT].GPIO_Pin) == 1 ? 0 : 1; 
}

uint8_t get_lower_door_up_limit_state(void)
{
    return GPIO_ReadInputDataBit(platform_gpio_pins[PLATFORM_GPIO_DOOR_DETECT_LOWER_UP_LIMIT].GPIOx, platform_gpio_pins[PLATFORM_GPIO_DOOR_DETECT_LOWER_UP_LIMIT].GPIO_Pin) == 1 ? 0 : 1; 
}

uint8_t get_lower_door_down_limit_state(void)
{
    return GPIO_ReadInputDataBit(platform_gpio_pins[PLATFORM_GPIO_DOOR_DETECT_LOWER_DOWN_LIMIT].GPIOx, platform_gpio_pins[PLATFORM_GPIO_DOOR_DETECT_LOWER_DOWN_LIMIT].GPIO_Pin) == 1 ? 0 : 1; 
}



void motor_pwr_on(void)
{
    GPIO_SetBits(platform_gpio_pins[PLATFORM_GPIO_MOTOR_PWR_EN].GPIOx, platform_gpio_pins[PLATFORM_GPIO_MOTOR_PWR_EN].GPIO_Pin);
}

void motor_pwr_off(void)
{
    GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_MOTOR_PWR_EN].GPIOx, platform_gpio_pins[PLATFORM_GPIO_MOTOR_PWR_EN].GPIO_Pin);
}



static void platform_gpio_init(void)
{
    motor_dir_gpio_init();
    pho_switch_init();
    motor_power_ctrl_gpio_init();
    motor_ctrl_gpio_init();
    upper_door_ctrl_gpio_init();
    upper_door_detect_gpio_init();
    lower_door_ctrl_gpio_init();
    lower_door_detect_gpio_init();
}


uint8_t set_conveyor_start(void)
{
    GPIO_SetBits(platform_gpio_pins[PLATFORM_GPIO_MOTOR_EN].GPIOx, platform_gpio_pins[PLATFORM_GPIO_MOTOR_EN].GPIO_Pin);
    return 0;
}


uint8_t set_conveyor_stop(void)
{
    GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_MOTOR_EN].GPIOx, platform_gpio_pins[PLATFORM_GPIO_MOTOR_EN].GPIO_Pin);
    GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_UPPER_MOTOR_LOAD].GPIOx, platform_gpio_pins[PLATFORM_GPIO_UPPER_MOTOR_LOAD].GPIO_Pin);
    GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_UPPER_MOTOR_UNLOAD].GPIOx, platform_gpio_pins[PLATFORM_GPIO_UPPER_MOTOR_UNLOAD].GPIO_Pin);
    GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_LOWER_MOTOR_LOAD].GPIOx, platform_gpio_pins[PLATFORM_GPIO_LOWER_MOTOR_LOAD].GPIO_Pin);
    GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_LOWER_MOTOR_UNLOAD].GPIOx, platform_gpio_pins[PLATFORM_GPIO_LOWER_MOTOR_UNLOAD].GPIO_Pin);
    return 0;
}


void set_upper_dir_load(void)
{
    GPIO_SetBits(platform_gpio_pins[PLATFORM_GPIO_UPPER_MOTOR_LOAD].GPIOx, platform_gpio_pins[PLATFORM_GPIO_UPPER_MOTOR_LOAD].GPIO_Pin);
    GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_UPPER_MOTOR_UNLOAD].GPIOx, platform_gpio_pins[PLATFORM_GPIO_UPPER_MOTOR_UNLOAD].GPIO_Pin);
}

void set_lower_dir_load(void)
{
    GPIO_SetBits(platform_gpio_pins[PLATFORM_GPIO_LOWER_MOTOR_LOAD].GPIOx, platform_gpio_pins[PLATFORM_GPIO_LOWER_MOTOR_LOAD].GPIO_Pin);
    GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_LOWER_MOTOR_UNLOAD].GPIOx, platform_gpio_pins[PLATFORM_GPIO_LOWER_MOTOR_UNLOAD].GPIO_Pin);
}

uint8_t upper_conveyor_belt_load(void)
{
    motor_pwr_on();
    delay_ms(100);
    set_upper_dir_load();
    return 0;
}

uint8_t lower_conveyor_belt_load(void)
{
    motor_pwr_on();
    delay_ms(100);
    set_lower_dir_load();
    return 0;
}

uint8_t forward_conveyor_belt(void)
{
    motor_pwr_on();
    delay_ms(100);
    set_conveyor_start();
    GPIO_SetBits(platform_gpio_pins[PLATFORM_GPIO_MOTOR_DIR].GPIOx, platform_gpio_pins[PLATFORM_GPIO_MOTOR_DIR].GPIO_Pin);
    return 0;
}




void set_upper_dir_unload(void)
{
    GPIO_SetBits(platform_gpio_pins[PLATFORM_GPIO_UPPER_MOTOR_UNLOAD].GPIOx, platform_gpio_pins[PLATFORM_GPIO_UPPER_MOTOR_UNLOAD].GPIO_Pin);
    GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_UPPER_MOTOR_LOAD].GPIOx, platform_gpio_pins[PLATFORM_GPIO_UPPER_MOTOR_LOAD].GPIO_Pin);
}

void set_lower_dir_unload(void)
{
    GPIO_SetBits(platform_gpio_pins[PLATFORM_GPIO_LOWER_MOTOR_UNLOAD].GPIOx, platform_gpio_pins[PLATFORM_GPIO_LOWER_MOTOR_UNLOAD].GPIO_Pin);
    GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_LOWER_MOTOR_LOAD].GPIOx, platform_gpio_pins[PLATFORM_GPIO_LOWER_MOTOR_LOAD].GPIO_Pin);
}

uint8_t upper_conveyor_belt_unload(void)
{
    motor_pwr_on();
    delay_ms(100);
    set_upper_dir_unload();
    return 0;
}

uint8_t lower_conveyor_belt_unload(void)
{
    motor_pwr_on();
    delay_ms(100);
    set_lower_dir_unload();
    return 0;
}



uint8_t reverse_conveyor_belt(void)
{
    motor_pwr_on();
    delay_ms(100);
    set_conveyor_start();
    GPIO_ResetBits(platform_gpio_pins[PLATFORM_GPIO_MOTOR_DIR].GPIOx, platform_gpio_pins[PLATFORM_GPIO_MOTOR_DIR].GPIO_Pin);
    return 0;
}

uint8_t stop_conveyor_belt(void)
{
    motor_pwr_off();
    set_conveyor_stop();
    return 1;
}


#if defined DOUBLE_DECK
uint8_t get_pho_switch_upper_outsise_state(void)
{
    return GPIO_ReadInputDataBit(platform_gpio_pins[PLATFORM_GPIO_PHO_SWITCH_UPPER_OUTSIDE].GPIOx, platform_gpio_pins[PLATFORM_GPIO_PHO_SWITCH_UPPER_OUTSIDE].GPIO_Pin) == 1 ? 0 : 1;
}

uint8_t get_pho_switch_upper_mid_state(void)
{
    return GPIO_ReadInputDataBit(platform_gpio_pins[PLATFORM_GPIO_PHO_SWITCH_UPPER_MID].GPIOx, platform_gpio_pins[PLATFORM_GPIO_PHO_SWITCH_UPPER_MID].GPIO_Pin) == 1 ? 0 : 1;
}

uint8_t get_pho_switch_upper_insise_state(void)
{
    return GPIO_ReadInputDataBit(platform_gpio_pins[PLATFORM_GPIO_PHO_SWITCH_UPPER_INSIDE].GPIOx, platform_gpio_pins[PLATFORM_GPIO_PHO_SWITCH_UPPER_INSIDE].GPIO_Pin) == 1 ? 0 : 1;
}



uint8_t get_pho_switch_lower_outsise_state(void)
{
    return GPIO_ReadInputDataBit(platform_gpio_pins[PLATFORM_GPIO_PHO_SWITCH_LOWER_OUTSIDE].GPIOx, platform_gpio_pins[PLATFORM_GPIO_PHO_SWITCH_LOWER_OUTSIDE].GPIO_Pin) == 1 ? 0 : 1;
}

uint8_t get_pho_switch_lower_mid_state(void)
{
    return GPIO_ReadInputDataBit(platform_gpio_pins[PLATFORM_GPIO_PHO_SWITCH_LOWER_MID].GPIOx, platform_gpio_pins[PLATFORM_GPIO_PHO_SWITCH_LOWER_MID].GPIO_Pin) == 1 ? 0 : 1;
}

uint8_t get_pho_switch_lower_insise_state(void)
{
    return GPIO_ReadInputDataBit(platform_gpio_pins[PLATFORM_GPIO_PHO_SWITCH_LOWER_INSIDE].GPIOx, platform_gpio_pins[PLATFORM_GPIO_PHO_SWITCH_LOWER_INSIDE].GPIO_Pin) == 1 ? 0 : 1;
}
#endif


uint32_t test_hardware_version = 0;
void hardware_init(void)
{
    platform_gpio_init();
    led_init();
    init_can1();
    sanwei_rfid_init();
}

void user_param_init(void)
{

}
