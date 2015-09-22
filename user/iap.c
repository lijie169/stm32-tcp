#include "iap.h"
#include "timer.h"
#include "stm32f10x.h"
#include "led.h"
extern void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);	
u32 appaddr = FLASH_APP1_ADDR ;
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//IAP 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/24
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////	
extern uint8_t poll ;
extern uint8_t is_download_ok ;
typedef int (*iapfun)(void);

iapfun jump2app; 
//u16 iapbuf[1024];   
//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u16 t;
	u16 i=0;
	u16 temp;
	u32 fwaddr=appxaddr;//当前写入的地址
	u8 *dfu=appbuf;
	u16 *iapbuf = (u16*)appbuf ;
	for(t=0;t<appsize;t+=2)
	{						    
		temp=(u16)dfu[1]<<8;
		temp+=(u16)dfu[0];	  
		dfu+=2;//偏移2个字节
		iapbuf[i++]=temp;	    
		if(i==1024)
		{
			i=0;
			STMFLASH_Write(fwaddr,iapbuf,1024);	
			fwaddr+=2048;//偏移2048  16=2*8.所以要乘以2.
		}
	}
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//将最后的一些内容字节写进去.  
}




//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//检查栈顶地址是否合法.
	{ 
		printf("iap run....\n");
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//用户代码区第二个字为程序开始地址(复位地址)		
		__set_MSP(*(vu32*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x10000);
		jump2app();									//跳转到APP.
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

	/*tftp 请求*/
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



