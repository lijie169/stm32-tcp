#include "iap.h"
#include "timer.h"
#include "stm32f10x.h"
#include "led.h"
extern void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);	
u32 appaddr = FLASH_APP1_ADDR ;
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//IAP ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/24
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////	
extern uint8_t poll ;
extern uint8_t is_download_ok ;
typedef int (*iapfun)(void);

iapfun jump2app; 
//u16 iapbuf[1024];   
//appxaddr:Ӧ�ó������ʼ��ַ
//appbuf:Ӧ�ó���CODE.
//appsize:Ӧ�ó����С(�ֽ�).
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u16 t;
	u16 i=0;
	u16 temp;
	u32 fwaddr=appxaddr;//��ǰд��ĵ�ַ
	u8 *dfu=appbuf;
	u16 *iapbuf = (u16*)appbuf ;
	for(t=0;t<appsize;t+=2)
	{						    
		temp=(u16)dfu[1]<<8;
		temp+=(u16)dfu[0];	  
		dfu+=2;//ƫ��2���ֽ�
		iapbuf[i++]=temp;	    
		if(i==1024)
		{
			i=0;
			STMFLASH_Write(fwaddr,iapbuf,1024);	
			fwaddr+=2048;//ƫ��2048  16=2*8.����Ҫ����2.
		}
	}
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//������һЩ�����ֽ�д��ȥ.  
}




//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
	{ 
		printf("iap run....\n");
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
		__set_MSP(*(vu32*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x10000);
		jump2app();									//��ת��APP.
	}
	else
	{
		printf("app loss\n");
	}
}		 



void reset_app_addr(void)
{
	appaddr = FLASH_APP1_ADDR ;
}
	

#define IAP_CHARS 8 
#define DOWNLOAD_TIMES (CLOCK_SECOND*60)
#define REQ_TIME (CLOCK_SECOND*5)
#define REQ_CNT (10)

void lanuch_gui(void)
{
	u8 times = 9 ;
	u8 dats_input[IAP_CHARS] = {0};
	u8 count = 0 ;
	timer_typedef tim ;

	/*wait for usr cmd*/
	timer_set(&tim, REQ_TIME);
	printf("do you want to upgrade firmware? (press key usr1 or input anything for ok , other for no) %d: \n",times);
	do
	{
		if(times&&timer_expired(&tim))
		{
			times-- ;
			printf("do you want to upgrade firmware? (press key usr1 or input anything , other for no) %d: \n",times);
			timer_set(&tim, REQ_TIME);
		}
		if(Key_Down()||input(dats_input,IAP_CHARS,0))
		{
			count = 1;
			break ;
		}

	stack_process();
	Led_Process();
	}while(times);

	/*tftp ����*/
	if(count)
	{
		Led_Config(SECOND_2_HZ);
		count = 0 ;
		times = REQ_CNT ;
		printf("upgrade ...\n tftp request..\n");
		timer_set(&tim, REQ_TIME);
		tftp_poll();
		do
		{
			if(times&&timer_expired(&tim))
			{
				timer_set(&tim,REQ_TIME);
				tftp_poll();
				times-- ;
			}

			if(tftp_isdownloadstart())
			{
				break ;
			}
			
			stack_process();
			Led_Process();
		}while(times);
		
	}

	/*upgrade proc*/
	if(tftp_isdownloadstart())
	{
		printf("download .... ing\n");
		timer_set(&tim,DOWNLOAD_TIMES);
		Led_Config(SECOND_1_HZ);
		do
		{
			if(tftp_isdownloadend())
			{
				printf("exit download proc\n");
				break ;
			}
			stack_process();
			Led_Process();
		}while(!timer_expired(&tim));
	}
}

void bootstart(void)
{
	lanuch_gui();
	iap_load_app(FLASH_APP1_ADDR);
}



