/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "includes.h"
#include "usart.h"


void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}


void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

//void PendSV_Handler(void)
//{
//}

 //void SysTick_Handler(void)
//{
//}


void DMA1_Channel4_IRQHandler(void)
{
    OSIntEnter();
    if (DMA_GetITStatus(DMA1_IT_TC4) != RESET)
    {
        DMA_Cmd(DMA1_Channel4, DISABLE);
        DMA_ClearFlag(DMA1_FLAG_TC4);
        DMA_ClearITPendingBit(DMA1_IT_TC4); // 清除中断标志位
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);    //等待一包数据发送完成
    }
    OSIntExit();
}

void DMA1_Channel5_IRQHandler(void)
{
    OSIntEnter();
    if (DMA_GetITStatus(DMA1_IT_TC5) != RESET)
    {
        DMA_Cmd(DMA1_Channel5, DISABLE);
        DMA_ClearFlag(DMA1_FLAG_TC5);
        DMA_ClearITPendingBit(DMA1_IT_TC5); // 清除中断标志位
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);    //等待一包数据发送完成
    }
    OSIntExit();
}

void USART1_IRQHandler(void)
{
    volatile unsigned char temper=0;

    OSIntEnter();
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        temper = USART1->SR;
        temper = USART1->DR;    //清USART_IT_IDLE
    }
    OSIntExit();
}

void UART4_IRQHandler(void)
{
    volatile unsigned char temper=0;

    OSIntEnter();

    if (USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
    {
        temper = UART4->SR;
        temper = UART4->DR;    //清USART_IT_IDLE
    }
    OSIntExit();
}

void DMA1_Channel7_IRQHandler(void)
{
    OSIntEnter();
    if (DMA_GetITStatus(DMA1_IT_TC7) != RESET)
    {
        DMA_Cmd(DMA1_Channel7, DISABLE);
        DMA_ClearFlag(DMA1_FLAG_TC7);
        DMA_ClearITPendingBit(DMA1_IT_TC7); // 清除中断标志位
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);    //等待一包数据发送完成
    }
    OSIntExit();
}

void DMA1_Channel6_IRQHandler(void)
{
    OSIntEnter();
    if (DMA_GetITStatus(DMA1_IT_TC6) != RESET)
    {
        DMA_Cmd(DMA1_Channel6, DISABLE);
        DMA_ClearFlag(DMA1_FLAG_TC6);
        DMA_ClearITPendingBit(DMA1_IT_TC6); // 清除中断标志位
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);    //等待一包数据发送完成
    }
    OSIntExit();
}



/*    USART2 IDLE interrupt    */
void USART2_IRQHandler(void)
{
    volatile unsigned char temper=0;

    OSIntEnter();
    if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        temper = USART2->SR;
        temper = USART2->DR;    //清USART_IT_IDLE

        DMA_Cmd(DMA1_Channel6, DISABLE);

        DMA_Cmd(DMA1_Channel6,ENABLE);
    }
    OSIntExit();
}


void EXTI9_5_IRQHandler(void)
{

    OSIntEnter();
    if(EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line6);
    }
    if(EXTI_GetITStatus(EXTI_Line7) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line7);
    }
    OSIntExit();
}


#include "can_protocol_task.h"
extern CanRxMsg RxMessage;
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    OSIntEnter();
    can_pkg_t *can_buf;
    uint8_t err = 0;
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    can_buf = (can_pkg_t *)OSMemGet(can_rcv_buf_mem_handle, &err);

    if((can_buf != 0) && (err == OS_ERR_NONE))
    {
        can_buf->id.canx_id = RxMessage.ExtId;
        can_buf->len = RxMessage.DLC;
        memcpy(can_buf->data.can_data, RxMessage.Data, can_buf->len);
        OSQPost(can_rcv_buf_queue_handle, (void *)can_buf);
    }
    OSIntExit();
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
