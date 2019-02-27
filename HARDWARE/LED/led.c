/*
 *  Author: Kaka Xie
 *  brief: led init
 */

#include "led.h"

void led_init(void)
{

    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);   //ʹ�ܶ˿�ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;           // �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //IO���ٶ�Ϊ50MHz
    GPIO_Init(GPIOC, &GPIO_InitStructure);              //�����趨������ʼ��GPIO
    GPIO_ResetBits(GPIOC,GPIO_Pin_9);                   // �����

}
