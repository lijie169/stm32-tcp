#include "stm32f10x.h"
#include "led.h"
#include "timer.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

static uint8_t shine_frq = SECOND_1_HZ ;
void Led_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	GPIO_SetBits(GPIOF,GPIO_Pin_6);
}

void Led_Shine(uint8_t state)
{
	
	GPIO_WriteBit(GPIOF,GPIO_Pin_6,state);
	
}

void Led_Process(void)
{

	static uint8_t state = 0 ;
	static uint16_t count = 0 ;

	if((uint16_t)(clock_time()- count) >=  shine_frq)
	{
		Led_Shine(state);
		count = clock_time() ;
		state = !state ;
	}
}

void Led_Config(uint16_t newfre)
{
	shine_frq = newfre ;
}



