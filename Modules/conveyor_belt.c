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
    uint8_t state = pho_switch_state;
    if(state == 0)
    {
        conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_LOAD;
        return CONVEYOR_BELT_EXEC_OK;
    }
    else
    {
        conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_STOP;
        return CONVEYOR_BELT_IS_OCCUPIED;
    }
//    conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_LOAD;
//    return CONVEYOR_BELT_EXEC_OK;
}


uint8_t set_conveyor_belt_unload(void)
{
    uint8_t state = pho_switch_state;
    if(state > 0)
    {
        conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_UNLOAD;
        return CONVEYOR_BELT_EXEC_OK;
    }
    else
    {
        conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_STOP;
        return CONVEYOR_BELT_IS_ALREADY_EMPTY;
    }
//    conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_UNLOAD;
//    return CONVEYOR_BELT_EXEC_OK;
}

uint8_t set_conveyor_belt_stop(void)
{
    conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_STOP;
    return CONVEYOR_BELT_EXEC_OK;
}

