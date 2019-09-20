#ifndef __USER_PLATFORM_H__
#define __USER_PLATFORM_H__
#include "stm32f10x.h"
#include "ucos_ii.h"


#define DOUBLE_DECK

typedef enum
{
    PLATFORM_GPIO_SYS_LED,

    PLATFORM_GPIO_MOTOR_EN,
    PLATFORM_GPIO_MOTOR_PWR_EN,
    PLATFORM_GPIO_MOTOR_DIR,
//    PLATFORM_GPIO_LOCK_CTRL,

#if defined DOUBLE_DECK
    PLATFORM_GPIO_PHO_SWITCH_LOWER_INSIDE,
    PLATFORM_GPIO_PHO_SWITCH_LOWER_MID,
    PLATFORM_GPIO_PHO_SWITCH_LOWER_OUTSIDE,
    PLATFORM_GPIO_PHO_SWITCH_UPPER_INSIDE,
    PLATFORM_GPIO_PHO_SWITCH_UPPER_MID,
    PLATFORM_GPIO_PHO_SWITCH_UPPER_OUTSIDE,

    //PLATFORM_GPIO_MOTOR_PWR_EN,
    PLATFORM_GPIO_UPPER_MOTOR_LOAD,
    PLATFORM_GPIO_UPPER_MOTOR_UNLOAD,
    PLATFORM_GPIO_LOWER_MOTOR_LOAD,
    PLATFORM_GPIO_LOWER_MOTOR_UNLOAD,

    PLATFORM_GPIO_DOOR_DETECT_UPPER_DOWN_LIMIT,
    PLATFORM_GPIO_DOOR_DETECT_UPPER_UP_LIMIT,
    PLATFORM_GPIO_DOOR_DETECT_LOWER_DOWN_LIMIT,
    PLATFORM_GPIO_DOOR_DETECT_LOWER_UP_LIMIT,

    PLATFORM_GPIO_UPPER_DOOR_OPEN_CTRL,
    PLATFORM_GPIO_UPPER_DOOR_EN_CTRL,
    PLATFORM_GPIO_LOWER_DOOR_OPEN_CTRL,
    PLATFORM_GPIO_LOWER_DOOR_EN_CTRL,
#endif

    PLATFORM_GPIO_MAX, /* Denotes the total number of GPIO port aliases. Not a valid GPIO alias */
    PLATFORM_GPIO_NONE,
} platform_gpio_e;


typedef struct
{
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;

}platform_gpio_t;


typedef struct
{
    uint16_t sys_status;
}sys_status_t;

extern sys_status_t *sys_status;
extern const platform_gpio_t platform_gpio_pins[];

uint32_t get_tick(void);
void mcu_restart(void);

void hardware_init(void);
void user_param_init(void);

uint8_t stop_conveyor_belt(void);

void lock_ctrl_unlock(void);
void lock_ctrl_lock(void);


#if defined DOUBLE_DECK
uint8_t get_pho_switch_upper_outsise_state(void);
uint8_t get_pho_switch_upper_mid_state(void);
uint8_t get_pho_switch_upper_insise_state(void);
uint8_t get_pho_switch_lower_outsise_state(void);
uint8_t get_pho_switch_lower_mid_state(void);
uint8_t get_pho_switch_lower_insise_state(void);
#endif


uint8_t upper_conveyor_belt_load(void);
uint8_t upper_conveyor_belt_unload(void);

uint8_t lower_conveyor_belt_load(void);
uint8_t lower_conveyor_belt_unload(void);

uint8_t get_lower_door_down_limit_state(void);
uint8_t get_lower_door_up_limit_state(void);
uint8_t get_upper_door_down_limit_state(void);
uint8_t get_upper_door_up_limit_state(void);

void lower_door_ctrl(uint8_t en);
void upper_door_ctrl(uint8_t en);

void stop_lower_door(void);
void stop_upper_door(void);

#endif
