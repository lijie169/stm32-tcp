#ifndef __SPI_H
#define __SPI_H
//#include "sys.h"

typedef unsigned char u8 ;
// SPI�����ٶ����� 
#define SPI_SPEED_2   0
#define SPI_SPEED_8   1
#define SPI_SPEED_16  2
#define SPI_SPEED_256 3
typedef struct __FILE 
{ 
	int handle; 
}FILE; 
						  	    													  
void SPIx_Init(void);			 //��ʼ��SPI��
u8 SPIx_ReadWriteByte(u8 TxData);//SPI���߶�дһ���ֽ�
void shell_help(void);	

#endif

