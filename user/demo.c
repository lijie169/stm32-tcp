#include "spi.h"
#include "stack.h"
#include "stm32f10x.h"
#include "iap.h"
u8 mac[] = {0x00,0xd5,0xcf,0x88,0x37,0x1e};
#if 0 
char packets[] ={
0xff, 0xff , 0xff , 0xff , 0xff , 0xff , 0x00,0xd5,0xcf,0x88,0x37,0x1e , 0x08 , 0x06 , 0x00 , 0x01,
0x08, 0x00 , 0x06 , 0x04 , 0x00 , 0x01 , 0x00,0xd5,0xcf,0x88,0x37,0x1e , 0xc0 , 0xa8 , 0x01 , 0x63 ,
0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xc0 , 0xa8 , 0x02 , 0x64 };
static u8 len = sizeof(packets);
#endif
extern u8 is_download_ok ;
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
void print(uint16_t* data,uint16_t len)
{
	uint16_t loop ;
	for(loop = 0 ; loop < len ; loop++)
	{
		printf("%04x ",data[loop]);
		if((loop + 1) % 8 == 0)
			printf("\n");
	}
}

void test_hardhandler(char t1,int *p)
{
	*p = 1 ;
}
int main(void)
{
//	uint32_t pre = 0 ;
	uint8_t* p = 0;
	timer_config();
	SPIx_Init();
	Led_Init();

	Key_Init();
	Usart2_Init(9600);
//	enc28j60_init(mac);
	install_uart_dev();
	stack_init();
//	test_hardhandler(2,(int*)1);
	my_tftp_init();
	telnet_service_init();
//	web_server_init();
//	*p = 1 ;
	#if 0
	while(1)
	{
		stack_process();
		Led_Process();
		shell();
		
	}
	#endif
	bootstart();

 return 0 ;
}

void HardFault_Handler_C(unsigned int* hardfault_args)
{

	#if 0
		wait();
	#else
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
	#endif


    while (1);
}
