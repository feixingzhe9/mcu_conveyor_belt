/*
 *  Author: Kaka Xie
 *  brief: conveyor belt task
 */

#include "conveyor_belt_task.h"
#include "delay.h"
#include "platform.h"
#include "photoelectric_switch.h"
#include "conveyor_belt.h"

OS_STK conveyor_belt_task_stk[CONVEYOR_BELT_TASK_STK_SIZE] = {0};
OS_STK pho_switch_status_task_stk[PHO_SWITCH_STATUS_TASK_STK_SIZE] = {0};

extern void upload_conveyor_belt_status(uint8_t status);


void pho_switch_status_task(void *pdata)
{
    delay_ms(5000);
    while(1)
    {
        OS_ENTER_CRITICAL();
        pho_switch_state = get_pho_switch_state();
        OS_EXIT_CRITICAL();
        delay_ms(50);
    }
}

/*
1.传送带装货流程：
    光电开关从未触发到触发
        1,2,3号初始值为未触发 -> 1 号触发 -> 1和2 同时触发 -> 1,2和3  (或者 2和3) 同时触发 -> 结束

2.传送带卸货流程：
    光电开关从触发到未触发
    1,2,3号 或者 2和3 初始值触发 -> 3 号未触发, 1和2 触发-> 1和2 同时触发 -> 1,2和3  (或者 2和3) 同时触发 -> 结束
*/
#define TICK_DELAY_MS           100
#define STOP_TICK_CNT           (2000 / TICK_DELAY_MS)
#define LOAD_TIME_OUT_CNT       (10000 / TICK_DELAY_MS)
#define UNLOAD_TIME_OUT_CNT     (10000 / TICK_DELAY_MS)
#define UNLOAD_STOP_CNT         (2000 / TICK_DELAY_MS)
void conveyor_belt_task(void *pdata)
{
    uint8_t work_mode = 0;
    uint8_t pre_work_mode = 0;
    uint32_t tick_cnt = 0;
    uint32_t stop_cnt = 0;
    uint32_t unload_stop_cnt = 0;
    uint8_t load_state = 0;
    uint8_t unload_state = 0;
    uint8_t switch_state = 0;
    delay_ms(5000);
    while(1)
    {
//        set_conveyor_belt_forward();
//        delay_ms(10000);
//        set_conveyor_belt_stop();
//        delay_ms(3000);
//        set_conveyor_belt_reverse();
//        delay_ms(10000);
//        set_conveyor_belt_stop();
//        delay_ms(3000);

        OS_ENTER_CRITICAL();
        work_mode = conveyor_belt.work_mode;
        switch_state = pho_switch_state;
        OS_EXIT_CRITICAL();
        if(work_mode != pre_work_mode)
        {
            pre_work_mode = work_mode;
            stop_conveyor_belt();
            stop_cnt = STOP_TICK_CNT;
        }
        if(stop_cnt > 0)
        {
            stop_cnt--;
        }
        else
        {
            if(work_mode == CONVEYOR_BELT_STATUS_LOAD)
            {
                forward_conveyor_belt();
                switch(load_state)
                {
                    case 0:
                        if(switch_state & PHO_SWITCH_3_TRIGGERED)
                        {
                            stop_conveyor_belt();   //stop immediately !
                            OS_ENTER_CRITICAL();
                            conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_STOP;
                            OS_EXIT_CRITICAL();
                            work_mode = CONVEYOR_BELT_STATUS_STOP;
                            load_state = 1;
                        }
                        break;

                    case 1:
                        load_state = 0;
                        break;
                    default :
                        break;
                }
            }
            else if(work_mode == CONVEYOR_BELT_STATUS_UNLOAD)
            {
                reverse_conveyor_belt();
                switch(unload_state)
                {
                    case 0:
                        if(switch_state == 0)
                        {
                            unload_state = 1;
                            unload_stop_cnt = UNLOAD_STOP_CNT;
                        }
                        break;

                    case 1:
                        if(switch_state > 0)
                        {
                            unload_state = 0;
                        }
                        if(unload_stop_cnt > 0)
                        {
                            unload_stop_cnt--;
                        }
                        else
                        {
                            OS_ENTER_CRITICAL();
                            conveyor_belt.work_mode = CONVEYOR_BELT_STATUS_STOP;
                            OS_EXIT_CRITICAL();
                            work_mode = CONVEYOR_BELT_STATUS_STOP;
                            unload_state = 0;
                        }
                        break;

                    default :
                        break;
                }
            }
        }

        delay_ms(TICK_DELAY_MS);
        tick_cnt++;

    }
}
