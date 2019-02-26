#ifndef __USER_PLATFORM_H__
#define __USER_PLATFORM_H__
#include "stm32f10x.h"
#include "ucos_ii.h"


typedef enum
{
    PLATFORM_GPIO_PWR_NV,
    PLATFORM_GPIO_PWR_RESERVE,

    PLATFORM_GPIO_5V_EN,
    PLATFORM_GPIO_12V_EN,
    PLATFORM_GPIO_24V_EN,

    PLATFORM_GPIO_SYS_LED,

    PLATFORM_GPIO_VSYS_24V_NV_EN,

    PLATFORM_GPIO_PHO_SWITCH_1,
    PLATFORM_GPIO_PHO_SWITCH_2,
    PLATFORM_GPIO_PHO_SWITCH_3,
    PLATFORM_GPIO_MOTOR_START_STOP,
    PLATFORM_GPIO_MOTOR_REVERSE_FORWARD,

    PLATFORM_GPIO_MAX, /* Denotes the total number of GPIO port aliases. Not a valid GPIO alias */
    PLATFORM_GPIO_NONE,
} platform_gpio_e;



//#define POWER_5V_MOTOR             0x00000001
#define POWER_5V_RECHARGE          0x00000002
#define POWER_5V_SENSOR_BOARD      0x00000004
    //POWER_5V_SWITCH           = 0x00000008,
//#define POWER_5V_ROUTER            0x00000010
#define POWER_5V_EN                0x00000020

#define POWER_12V_PAD              0x00000040
//#define POWER_12V_2_1_PA           0x00000080
//#define POWER_12V_EXTEND           0x00000100
#define POWER_12V_X86              0x00000200
#define POWER_12V_NV               0x00000400
#define POWER_12V_EN               0x00000800

#define POWER_24V_EN               0x00001000
//#define POWER_24V_PRINTER          0x00002000
#define POWER_24V_EXTEND           0x00004000
#define POWER_VSYS_24V_NV          0x00008000

//#define POWER_485                  0x00010000
#define POWER_SYS_LED              0x00020000
#define POWER_RECHARGE_LED         0x00040000
#define POWER_SLAM                 0x00080000

#define POWER_LED_MCU              0x00100000
#define POWER_CHARGE_FAN           0x00200000
    //POWER_POLE_MOTOR          = 0x00400000,
    //POWER_5V_KEYPAD           = 0x00800000,
#define LED_MCU_RST                0x00400000

#define POWER_CAMERA_FRONT_LED     0x01000000
#define POWER_CAMERA_BACK_LED      0x02000000
#define POWER_CTRL_OUT             0x04000000
#define POWER_DOOR_CTRL            0x08000000

#define POWER_3V3_CARD_EN_1        0x10000000
#define POWER_3V3_CARD_EN_2        0x20000000
#define POWER_3V3_CARD_EN_3        0x40000000
#define POWER_3V3_CARD_EN_4        0x80000000


#define POWER_ALL                  0xFFFFFFFF


//#define MODULE_POWER_ON     1
//#define MODULE_POWER_OFF    0

typedef enum
{
    MODULE_POWER_OFF = 0,
    MODULE_POWER_ON
}module_power_state_e;




typedef struct
{
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;

}platform_gpio_t;



#define SYS_POWER_STATE_BOOTING_UP      1
#define SYS_POWER_STATE_ON              2
#define SYS_POWER_STATE_SHUTTING_DOWN   3
#define SYS_POWER_STATE_OFF             0


#define                 STATE_RUN_BITS     0x0F
#define                 STATE_POWER_OFF    0x00
#define                 STATE_IS_POWER_ON  0x01
#define                 STATE_POWER_ON     0x02
#define                 STATE_IS_POWER_OFF 0x03
//#define                 STATE_ERROR        0x04

#define                 STATE_IS_CHARGING       0x10
#define                 STATE_IS_LOW_POWER      0x20
#define                 STATE_IS_AUTO_UPLOAD    0x40
#define                 STATE_IS_CHARGER_IN     0x80
#define                 STATE_IS_RECHARGE_IN    0x0100
#define                 SYSTEM_IS_SLEEP         0x00 //set 0x00 to no use


#define                 REMOTE_DEVICE_POWER_SHUTDOWN    1
#define                 REMOTE_DEVICE_POWER_REBOOT      2
typedef struct
{
    uint16_t sys_status;    //œÚ«∞ºÊ»›
    uint8_t power_state;
}sys_status_t;

extern sys_status_t *sys_status;
extern const platform_gpio_t platform_gpio_pins[];

uint32_t get_tick(void);
void mcu_restart(void);

void hardware_init(void);
void user_param_init(void);
void hold_on_power(void);
void release_power(void);
void main_power_module_5v_ctrl(uint8_t on_off);
void main_power_module_12v_ctrl(uint8_t on_off);
void main_power_module_24v_ctrl(uint8_t on_off);

uint8_t get_switch_state(void);
uint8_t get_charge_gpio_value(void);
uint8_t get_recharge_gpio_value(void);

void power_ctrl(uint32_t power_en, uint8_t on_off);
uint8_t forward_conveyor_belt(void);
uint8_t reverse_conveyor_belt(void);
uint8_t stop_conveyor_belt(void);
uint8_t get_pho_switch_1_state(void);
uint8_t get_pho_switch_2_state(void);
uint8_t get_pho_switch_3_state(void);


#endif
