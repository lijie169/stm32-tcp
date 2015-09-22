#include <stdio.h>
#include "spi.h"
#include "stm32f10X_spi.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "timer.h"
#include "file_adapter.h"

FILE __stdout;  
#define USART_TRY_TIMES 10000
#define USART_BUFF_SIZE 0xFF 
extern   uint16_t current_clock ;
uint8_t usart_tx_buf[USART_BUFF_SIZE]= {0};
uint8_t usart_rx_buf[USART_BUFF_SIZE]= {0};
static uint8_t usart_tx_size = 0 ;
void Usart2_Init( uint32_t BaudRate)
{
  
    USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART Rx as input floating */
    /* ???STM32?USART?RX??,????????? */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
  
    /* USARTx ????:
          - BaudRate = 115200 baud  
          - Word Length = 8 Bits
          - One Stop Bit
          - No parity
          - Hardware flow control disabled (RTS and CTS signals)
          - Receive and transmit enabled
    */
    USART_InitStructure.USART_BaudRate = BaudRate;              //??????,??115200 ???4.5Mbits/s
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //?????(8??9?)
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //???????-??1?2????
    USART_InitStructure.USART_Parity = USART_Parity_No;         //?????  
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //??????
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //????,???????
  
    USART_Init(USART2, &USART_InitStructure);  // ??STM32?USART???????
    USART_Cmd(USART2, ENABLE);
    
}



int32_t usart_get_buff(int8_t* data,int32_t maxlen)
{
	int32_t count = 0 ;
	uint16_t cc = 0 ;
	uint8_t ch;
	if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET)
		{
			ch =  (uint8_t)(USART2->DR & 0xFF);
			data[count++] = ch ;

			if(ch == '\n')
				break ;
			if(count >= maxlen )
				break ;
			while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET)
			{
				cc++ ;
				if(cc > USART_TRY_TIMES)
				{
					break ;
				}
			}

			
		//	printf("wait %u times\n",cc);
			cc = 0 ;
		
		}
		
	}
	
	//data[count] = '\0';
	return count ;
}   
//??_sys_exit()??????????    
_sys_exit(int x) 
{ 
	x = x; 
} 



/* ???fputc?? ????MicroLIB??????fputc???? */  
int usart_write(char*buff, int size)
{
	int loop = 0 ;

	while(loop < size)
	{
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
		{    	
		}
		
		
		USART_SendData(USART2,  buff[loop]);
		loop ++ ;
	}


    return loop;
}


void install_uart_dev(void)
{
	unregister(0);
	register_open(usart_get_buff, usart_write);
}


int fputc(int ch, FILE *f)
{
	uint8_t usize  ;
	usart_tx_buf[usart_tx_size++] = ch ;
	
	if(usart_tx_size == USART_BUFF_SIZE - 1 || ch == '\n')
	{
		output(usart_tx_buf,usart_tx_size,0);

		usart_tx_size = 0 ;
	}
    return ch;
}
/*
??????putchar
?????? int putchar(int ch),??stdio.h??????
 #define putchar(c) putc(c, stdout)
*/

int ferror(FILE *f) {  
    return -1 ;//return EOF;  
} 
					  
//��������ӿ�SPI�ĳ�ʼ����SPI���ó���ģʽ							  


void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
//    EXTI_InitTypeDef EXTI_InitStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the BUTTON Clock */
    /* ʹ��KEY������ӦGPIO��Clockʱ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);

    /* Configure Button pin as input floating */
    /* ��ʼ��KEY������GPIO�ܽţ�����Ϊ������������ */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}

uint32_t Key_Down(void)
{
    /* ���ض�ȡ��������GPIO�ܽŵ�״̬�����������Ƿ��б����� */
    return GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3) == 0;
}

void SPIx_Init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Enable SPI1 and GPIOA clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE);
	
	/* Configure SPI1 pins: NSS, SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//SPI1 NSS 
	#if 1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_SetBits(GPIOC, GPIO_Pin_4); 
	#endif
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);   

	/* SPI1 configuration */ 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //SPI1����Ϊ����ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	                   //����SPI1Ϊ��ģʽ
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  //SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;	 		               //����ʱ���ڲ�����ʱ��ʱ��Ϊ�ߵ�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;		               //�ڶ���ʱ���ؿ�ʼ��������
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			               //NSS�ź��������ʹ��SSIλ������
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; //���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ8
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				   //���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;						   //CRCֵ����Ķ���ʽ
	SPI_Init(SPI1, &SPI_InitStructure);
	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE); 											  //ʹ��SPI1����
}   

//SPIx ��дһ���ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPIx_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 
	while((SPI1->SR&1<<1)==0)//�ȴ���������	
	{
		retry++;
		if(retry>200)return 0;
	}			  
	SPI1->DR=TxData;	 	  //����һ��byte 
	retry=0;
	while((SPI1->SR&1<<0)==0) //�ȴ�������һ��byte  
	{
		retry++;
		if(retry>200)return 0;
	}	  						    
	return SPI1->DR;          //�����յ�������				    
}























