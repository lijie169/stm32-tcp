#include "spi.h"
#include "stack.h"
#include "stm32f10x.h"
u8 mac[] = {0x00,0xd5,0xcf,0x88,0x37,0x1e};
char packets[] ={
0xff, 0xff , 0xff , 0xff , 0xff , 0xff , 0x00,0xd5,0xcf,0x88,0x37,0x1e , 0x08 , 0x06 , 0x00 , 0x01,
0x08, 0x00 , 0x06 , 0x04 , 0x00 , 0x01 , 0x00,0xd5,0xcf,0x88,0x37,0x1e , 0xc0 , 0xa8 , 0x01 , 0x63 ,
0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xc0 , 0xa8 , 0x02 , 0x64 };
static u8 len = sizeof(packets);
extern  uint16_t current_clock ;

//extern void my_udp_init(void);

void delay(uint32_t nCount)		 //????for??nCount?,????????
{
  for(; nCount != 0; nCount--);
}

#if 0
void LwIP_Config (void) 
{
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;
    
    // 调用LWIP初始化函数
    lwip_init();    
    
    IP4_ADDR(&ipaddr, 192, 168, 1, 16);             // 设置网络接口的ip地址
    IP4_ADDR(&netmask, 255, 255, 255, 0);           // 子网掩码
    IP4_ADDR(&gw, 192, 168, 1, 1);                  // 网关
    
    // 初始化enc28j60与LWIP的接口，参数为网络接口结构体、ip地址、
    // 子网掩码、网关、网卡信息指针、初始化函数、输入函数
    netif_add(&enc28j60, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);
    
    // 把enc28j60设置为默认网卡
    netif_set_default(&enc28j60);
    
    netif_set_up(&enc28j60);
}

#endif


int main(void)
{
//	uint32_t pre = 0 ;
	uint8_t* p = 0;
	SPIx_Init();
	Led_Init();
	Usart2_Init(9600);
//	enc28j60_init(mac);
	timer_config();
	stack_init();
	web_server_init();
	
	printf("init success\n");
//	*p =1 ;
    while (1) 
	{
//		printf("init success\r\n");
		stack_process();
//		app_process();
		Led_Process();
		shell_help();
 	}

 return 0 ;
}

void HardFault_Handler_C(unsigned int* hardfault_args)
{
    printf("R0    = 0x%.8X\n",hardfault_args[0]);         
    printf("R1    = 0x%.8X\n",hardfault_args[1]);         
    printf("R2    = 0x%.8X\n",hardfault_args[2]);         
    printf("R3    = 0x%.8X\n",hardfault_args[3]);         
    printf("R12   = 0x%.8X\n",hardfault_args[4]);         
    printf("LR    = 0x%.8X\n",hardfault_args[5]);         
    printf("PC    = 0x%.8X\n",hardfault_args[6]);         
    printf("PSR   = 0x%.8X\n",hardfault_args[7]);         
    printf("BFAR  = 0x%.8X\n",*(unsigned int*)0xE000ED38);
    printf("CFSR  = 0x%.8X\n",*(unsigned int*)0xE000ED28);
    printf("HFSR  = 0x%.8X\n",*(unsigned int*)0xE000ED2C);
    printf("DFSR  = 0x%.8X\n",*(unsigned int*)0xE000ED30);
    printf("AFSR  = 0x%.8X\n",*(unsigned int*)0xE000ED3C);
    printf("SHCSR = 0x%.8X\n",SCB->SHCSR);                
    while (1);
}
