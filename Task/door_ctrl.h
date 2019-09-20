#ifndef __TASK_DOOR_CTRL_H__
#define __TASK_DOOR_CTRL_H__

#include "stm32f10x.h"
#include "sys.h"
#include "ucos_ii.h"


#define DOOR_CTRL_TASK_STK_SIZE                         128
#define DOOR_STATE_DETECTION_TASK_STK_SIZE              64

#define UPPER_UP_LIMIT_TRIGGED              0x01
#define UPPER_DOWN_LIMIT_TRIGGED            0x02

#define LOWER_UP_LIMIT_TRIGGED              0x10
#define LOWER_DOWN_LIMIT_TRIGGED            0x20


extern OS_STK door_ctrl_task_stk[DOOR_CTRL_TASK_STK_SIZE];
extern OS_STK door_state_detection_task_stk[DOOR_STATE_DETECTION_TASK_STK_SIZE];

void door_state_detect_task(void *pdata);
void door_ctrl_task(void *pdata);




#endif
