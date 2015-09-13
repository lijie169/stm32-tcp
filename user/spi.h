#ifndef __SPI_H
#define __SPI_H
//#include "sys.h"

typedef unsigned char u8 ;
// SPI总线速度设置 
#define SPI_SPEED_2   0
#define SPI_SPEED_8   1
#define SPI_SPEED_16  2
#define SPI_SPEED_256 3
typedef struct __FILE 
{ 
	int handle; 
}FILE; 
						  	    													  
void SPIx_Init(void);			 //初始化SPI口
u8 SPIx_ReadWriteByte(u8 TxData);//SPI总线读写一个字节
void shell_help(void);	

#endif

