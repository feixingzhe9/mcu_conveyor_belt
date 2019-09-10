/*
 *  Author: Kaka Xie
 *  brief: conveyor belt operation
 */

#include "conveyor_belt.h"
#include "photoelectric_switch.h"
#include "platform.h"


conveyor_belt_t conveyor_belt[DECK_MAX] = {0};
uint8_t conveyor_index = DECK_UPPER;
void conveyor_belt_init(void)
{

}

uint8_t set_conveyor_belt_load(uint8_t need_lock)
{
    uint8_t state = pho_switch_state;
    if(state == 0)
    {
        conveyor_belt[conveyor_index].work_mode = CONVEYOR_BELT_STATUS_LOAD;
        conveyor_belt[conveyor_index].need_lock = need_lock;
        return CONVEYOR_BELT_EXEC_OK;
    }
    else
    {
        conveyor_belt[conveyor_index].work_mode = CONVEYOR_BELT_STATUS_STOP;
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
        conveyor_belt[conveyor_index].work_mode = CONVEYOR_BELT_STATUS_UNLOAD;
        return CONVEYOR_BELT_EXEC_OK;
    }
    else
    {
        conveyor_belt[conveyor_index].work_mode = CONVEYOR_BELT_STATUS_STOP;
        return CONVEYOR_BELT_IS_ALREADY_EMPTY;
    }
//    conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_UNLOAD;
//    return CONVEYOR_BELT_EXEC_OK;
}

uint8_t set_conveyor_belt_stop(void)
{
    conveyor_belt[conveyor_index].work_mode = CONVEYOR_BELT_STATUS_STOP;
    return CONVEYOR_BELT_EXEC_OK;
}



uint8_t lock_ctrl(uint8_t state)
{
    return 1;

}


