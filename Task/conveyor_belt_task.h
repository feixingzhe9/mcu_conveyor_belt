#ifndef __CONVEYOR_BELT_TASK_H__
#define __CONVEYOR_BELT_TASK_H__
#include "stm32f10x.h"
#include "ucos_ii.h"
#define CONVEYOR_BELT_TASK_STK_SIZE         128
#define PHO_SWITCH_STATUS_TASK_STK_SIZE     64

#define PHO_STATE_UPLOAD_TASK_STK_SIZE      64

extern OS_STK conveyor_belt_task_stk[CONVEYOR_BELT_TASK_STK_SIZE];
extern OS_STK pho_switch_status_task_stk[PHO_SWITCH_STATUS_TASK_STK_SIZE];
extern OS_STK pho_state_upload_task_stk[PHO_STATE_UPLOAD_TASK_STK_SIZE];

extern OS_EVENT * pho_state_mailbox;

void pho_switch_status_task(void *pdata);
void conveyor_belt_task(void *pdata);
void upload_pho_state_upload_task(void *pdata);
#endif
