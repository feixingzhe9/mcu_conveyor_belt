#ifndef __PHOTOELECTRIC_SWITCH_H__
#define __PHOTOELECTRIC_SWITCH_H__

#include "stm32f10x.h"
#include "ucos_ii.h"

#define PHO_SWITCH_1_TRIGGERED      0x01
#define PHO_SWITCH_2_TRIGGERED      0x02
#define PHO_SWITCH_3_TRIGGERED      0x04



#define PHO_SWITCH_INSIDE_TRIGGERED         0x04
#define PHO_SWITCH_MID_TRIGGERED            0x02
#define PHO_SWITCH_OUTSIDE_TRIGGERED        0x01


#define PHO_SWITCH_NUM_MAX          3


extern uint8_t pho_switch_state;


//uint8_t get_pho_switch_state(void);

#if defined DOUBLE_DECK
uint8_t get_pho_switch_upper_state(void);
uint8_t get_pho_switch_lower_state(void);
#endif

#endif
