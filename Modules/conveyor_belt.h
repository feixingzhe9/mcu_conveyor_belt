#ifndef __CONVEYOR_BELT_H__
#define __CONVEYOR_BELT_H__

#include "sys.h"


#define CONVEYOR_BELT_STATUS_STOP               0x00
#define CONVEYOR_BELT_STATUS_LOAD               0x01
#define CONVEYOR_BELT_STATUS_UNLOAD             0x02

#define CONVEYOR_LOAD_FINISHED_OK               0x01
#define CONVEYOR_UNLOAD_FINISHED_OK             0x02

#define CONVEYOR_BELT_LOAD_TIME_OUT             0xF0
#define CONVEYOR_BELT_UNLOAD_TIME_OUT           0xF1
#define CONVEYOR_BELT_STATUS_ERROR              0xFF

#define CONVEYOR_BELT_EXEC_OK                   0x00
#define CONVEYOR_BELT_IS_OCCUPIED               0x81
#define CONVEYOR_BELT_IS_ALREADY_EMPTY          0x82


#define LOCK_STATUS_LOCK                        0x01
#define LOCK_STATUS_UNLOCK                      0x00
typedef struct
{
    uint8_t work_mode;
    uint8_t err_status;
    uint8_t pho_switch_status;
    //uint8_t motor_speed;
    //uint8_t motor_dir;
}conveyor_belt_t;

extern conveyor_belt_t conveyor_belt;

void conveyor_belt_init(void);
uint8_t set_conveyor_belt_load(void);
uint8_t set_conveyor_belt_unload(void);
uint8_t set_conveyor_belt_stop(void);
uint8_t lock_ctrl(uint8_t state);
#endif
