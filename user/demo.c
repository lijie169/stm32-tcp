#include "spi.h"
#include "stack.h"
#include "stm32f10x.h"
u8 mac[] = {0x00,0xd5,0xcf,0x88,0x37,0x1e};
char packets[] ={
0xff, 0xff , 0xff , 0xff , 0xff , 0xff , 0x00,0xd5,0xcf,0x88,0x37,0x1e , 0x08 , 0x06 , 0x00 , 0x01,
0x08, 0x00 , 0x06 , 0x04 , 0x00 , 0x01 , 0x00,0xd5,0xcf,0x88,0x37,0x1e , 0xc0 , 0xa8 , 0x01 , 0x63 ,
0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xc0 , 0xa8 , 0x02 , 0x64 };
u8 len = sizeof(packets);
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
    
    // ����LWIP��ʼ������
    lwip_init();    
    
    IP4_ADDR(&ipaddr, 192, 168, 1, 16);             // ��������ӿڵ�ip��ַ
    IP4_ADDR(&netmask, 255, 255, 255, 0);           // ��������
    IP4_ADDR(&gw, 192, 168, 1, 1);                  // ����
    
    // ��ʼ��enc28j60��LWIP�Ľӿڣ�����Ϊ����ӿڽṹ�塢ip��ַ��
    // �������롢���ء�������Ϣָ�롢��ʼ�����������뺯��
    netif_add(&enc28j60, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);
    
    // ��enc28j60����ΪĬ������
    netif_set_default(&enc28j60);
    
    netif_set_up(&enc28j60);
}

#endif


int main(void)
{
//	uint32_t pre = 0 ;
	SPIx_Init();
	Led_Init();
//	enc28j60_init(mac);
	timer_config();
	stack_init();
    while (1) 
	{
		stack_process();
 	}

 return 0 ;
}
