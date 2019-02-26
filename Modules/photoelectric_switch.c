/*
 *  Author: Kaka Xie
 *  brief: photoelectric switch operation
 */

#include "photoelectric_switch.h"
#include "platform.h"

uint8_t pho_switch_state = 0;

void pho_switch_init(void)
{

}

uint8_t get_pho_switch_state(void)
{
    return get_pho_switch_1_state() | get_pho_switch_2_state() << 1 | get_pho_switch_3_state() << 2;
}
