#include "spi.h"
#include "stm32f10X_spi.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "timer.h"
FILE __stdout;  
#define USART_TRY_TIMES 10000

extern   uint16_t current_clock ;
uint8_t usart_tx_buf[0xFF]= {0};
uint8_t usart_rx_buf[0xFF]= {0};
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

uint8_t usart_recv_data(void)
{
	uint8_t count = 0 ;
	uint16_t cc = 0 ;
	uint8_t ch;
	if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET)
		{
			ch =  (uint8_t)(USART2->DR & 0xFF);
			usart_rx_buf[count++] = ch ;

			if(ch == '\n')
				break ;
			if(count >= 0xFE)
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
	
	usart_rx_buf[count] = '\0';
	return count ;
}
   
//??_sys_exit()??????????    
_sys_exit(int x) 
{ 
	x = x; 
} 
/* ???fputc?? ????MicroLIB??????fputc???? */  
int fputc(int ch, FILE *f)
{
	uint8_t usize  ;
	usart_tx_buf[usart_tx_size++] = ch ;
	
	if(usart_tx_size == 0xFE || ch == '\n')
	{
		usize = 0 ;
		while(usize < usart_tx_size)
		{
	    	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
	    	{    	
	    	}
			
			
			USART_SendData(USART2, (uint8_t) usart_tx_buf[usize]);
			usize ++ ;
		}

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
					  
//串行外设接口SPI的初始化，SPI配置成主模式							  
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
	//count++;
	//uint16_t tmp = (uint16_t)(count + ) ; 
	if((uint16_t)(current_clock - count) >=  CLOCK_SECOND/2)
	{
		Led_Shine(state);
		count = current_clock ;
		state = !state ;
//		printf("hello,world\r\n");
//		printf("time is %u\r\n",current_clock);
	}
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
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //SPI1设置为两线全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	                   //设置SPI1为主模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  //SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;	 		               //串行时钟在不操作时，时钟为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;		               //第二个时钟沿开始采样数据
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			               //NSS信号由软件（使用SSI位）管理
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; //定义波特率预分频的值:波特率预分频值为8
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				   //数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;						   //CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);
	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE); 											  //使能SPI1外设
}   

//SPIx 读写一个字节
//返回值:读取到的字节
u8 SPIx_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 
	while((SPI1->SR&1<<1)==0)//等待发送区空	
	{
		retry++;
		if(retry>200)return 0;
	}			  
	SPI1->DR=TxData;	 	  //发送一个byte 
	retry=0;
	while((SPI1->SR&1<<0)==0) //等待接收完一个byte  
	{
		retry++;
		if(retry>200)return 0;
	}	  						    
	return SPI1->DR;          //返回收到的数据				    
}























