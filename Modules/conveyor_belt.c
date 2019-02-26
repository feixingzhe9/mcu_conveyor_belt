/*
 *  Author: Kaka Xie
 *  brief: conveyor belt operation
 */

#include "conveyor_belt.h"
#include "photoelectric_switch.h"


conveyor_belt_t conveyor_belt = {0};

void conveyor_belt_init(void)
{

}

uint8_t set_conveyor_belt_load(void)
{
//    uint8_t state = pho_switch_state;
//    if(state == 0)
//    {
//        conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_LOAD;
//        return 1;
//    }
//    else
//    {
//        return 0;
//    }
    conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_LOAD;
    return 1;
}


uint8_t set_conveyor_belt_unload(void)
{
    conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_UNLOAD;
    return 1;
}

uint8_t set_conveyor_belt_stop(void)
{
    conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_STOP;
    return 1;
}

