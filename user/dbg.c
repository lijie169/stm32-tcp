#include "stm32f10x.h"
#define ARP_TABLE 1
#define ARP_REQ   2


#define ARG_MAX_SIZE (25)
#define ARG_SPACE_END  (0)
#define ARG_EOF_END    (-1)

#define ARG_OK          (0)
#define ARG_ERROR_LONG (-2)
#define ARG_ERROR_END  (-3)
#define ARG_MAX_NUMS   (6)


void help_usart(void);
void shell_arp(void);
void shell_eth(void);

//static uint8_t usart_tx_ptr = 0 ;
// uint8_t usart_tx_buf[0xFF]= {0};
extern uint8_t usart_rx_buf[0xFF];
uint8_t usart_dbg = 0 ;

uint8_t cmd_pointer[ARG_MAX_NUMS + 1][ARG_MAX_SIZE] = { 0 };
typedef void (*fun_t)(void) ;

typedef struct
{
	uint8_t fun_str[ARG_MAX_SIZE];
	fun_t function;
	uint8_t* fun_help ; 
	uint8_t fun_count ;
}cmd_typedef;


cmd_typedef shell_cmd[] =
{
	{"?",help_usart,"is help",0},
	{"arp",shell_arp,"arp debug",1},
	{"eth",shell_eth,"eth debug",1}
};





int8_t str_parse(uint8_t* dst_str,uint8_t* src,uint8_t * pos)
{

	uint8_t len = 0 ;
	int8_t ret ;
	int8_t * src_str = src ;
	while(*src_str != '\0' && *src_str ==' ')
	{
			src_str++;		 
	}

	while(1)
	{
		if(*src_str =='\0')
		{
			ret = ARG_EOF_END;
			break ;
		}
		else if(*src_str == ' ')
		{
			ret = ARG_SPACE_END ;
			break ;
		}
		else
		{
			*dst_str = *src_str;
			dst_str ++ ;
			src_str++;
			len++ ;

			if(len >=  ARG_MAX_SIZE)
			{
				ret = ARG_ERROR_LONG ;
				break ;
			}
		}
			
	}

	*pos = src_str - src ;
	return ret ;
	
}

int8_t cmd_parse(uint8_t* cmd_str,uint8_t* num)
{
	int8_t ret ;
	uint8_t pos = 0 ;
	uint8_t curpos = 0 ;
	uint8_t idx;
	uint8_t tmp ;
	uint8_t len = strlen(cmd_str);

	if(len < 2 || cmd_str[len - 2] !='\r' || cmd_str[len - 1] !='\n' )
	{
		printf("input must end with \\r\\n\n");
		return ARG_ERROR_END ;
	}

	cmd_str[len-1] = '\0' ;
	cmd_str[len-2] = '\0' ;		
	
	for(idx = 0 ; idx < ARG_MAX_NUMS ; idx++)
	{
		memset(cmd_pointer[idx],0,sizeof(cmd_pointer[idx]));
		ret = str_parse(cmd_pointer[idx], cmd_str+pos, &curpos);

		pos += curpos ;	
		if(ret != ARG_SPACE_END)
			break ;		
	}

	//vertify args
	if(ret == ARG_SPACE_END)
	{
		ret = ARG_ERROR_END ;
		while( *(cmd_str+pos) == ' ') pos++ ;

		if(*(cmd_str+pos) == '\0')
			  ret = ARG_OK ;
	}
	else if(ret == ARG_EOF_END)
	{
		#if 0
		pos = strlen(cmd_pointer[idx]);

		if(cmd_pointer[idx][pos-1] == '\n' && cmd_pointer[idx][pos-2] == '\r')
		{
			ret = ARG_OK  ;
	
		}
		else
			ret = ARG_ERROR_END ;
		#endif

		ret = ARG_OK  ;
	}

	*num = idx+1 ;
	return ret ;

}


void shell_help(void)
{
	uint8_t count = 0 ;
	uint8_t idx ;
	int8_t ret ;
	count = usart_recv_data();
	if(!count)
	{
		return ;
	}

#if 1
	ret = cmd_parse(usart_rx_buf,&count);
	if(ret != ARG_OK)
	{
		printf("parse arg error %d\n",ret);
//		printf("%x %s %x %x\n",usart_rx_buf[0],usart_rx_buf,usart_rx_buf[strlen(usart_rx_buf)-2],usart_rx_buf[strlen(usart_rx_buf)-1]);
		return ;
	}

#endif
	
	if(usart_dbg == 1)
	{
		printf("recv %s\n",usart_rx_buf);
		for(ret = 0 ; ret < count ; ret ++)
		{
			printf("%d : %s\n",ret+1,cmd_pointer[ret]);
		}
	}

	count = sizeof(shell_cmd)/sizeof(cmd_typedef);
	for(idx = 0 ; idx < count ; idx++)
	{
	//	printf("cmp str :%s,%s,%d,%d\n",shell_cmd[idx].fun_str,cmd_pointer[0],strlen(cmd_pointer[0]),strlen(shell_cmd[idx].fun_str));
		
		if(!strcmp(cmd_pointer[0],shell_cmd[idx].fun_str))
		{
			shell_cmd[idx].function();
			return  ;
		}
	}

	printf("unfind\n");
}	

void help_usart(void)
{
//	printf("\n");
	usart_dbg = atoi(cmd_pointer[1]);
	printf("dbg = %d\n",usart_dbg);
}


#if 1
void shell_arp(void)
{
	uint8_t tarip[] = {192,168,1,100};
	uint8_t mode = atoi(cmd_pointer[1]);
	
	if(ARP_TABLE == mode)
	{
		arp_show_table();
	}
	else if(ARP_REQ == mode)
	{
		arp_request(tarip);
		printf("a arp packet to 192.168.1.100 send\n");
	}
	else 
	{
		tcp_sockets_show();
	}
}

#endif
#if 1
#define ETH_READ 	1
#define ETH_WRITE 	2
#define PHY_READ    3
#define PHY_WRITE 	4
void shell_eth(void)
{
	uint8_t addr  = 0 ;
	uint8_t tarip[] = {192,168,1,100};
	uint8_t mode = atoi(cmd_pointer[1]);
	uint16_t value = 0xffff;
	addr = atoi(cmd_pointer[2]);
	
	if(ETH_READ == mode)
	{
		value = enc28j60_read(addr);
		printf("read eth address = 0x%x(%u),value = 0x%x(%u)\n",addr,addr,value,value);
	}
	else if(ETH_WRITE)
	{
		value = atoi(cmd_pointer[3]);
		enc28j60_write(addr,value);
		//value = enc28j60_readphy(addr);
		printf("wtite mi/eth address = 0x%x(%u),value = 0x%x(%u)\n",addr,addr,value,value);
	}
	else if(PHY_WRITE == mode)
	{
		//arp_request(tarip);
		value = atoi(cmd_pointer[3]);
		enc28j60_writephy(addr,value);
		//value = enc28j60_readphy(addr);
		printf("wtite phy address = 0x%x(%u),value = 0x%x(%u)\n",addr,addr,value,value);
		//printf("unimplement\n");
	}
	else if(PHY_READ == mode)
	{
		value = enc28j60_readphy(addr);
		printf("read phy address = 0x%x(%u),value = 0x%x(%u)\n",addr,addr,value,value);
	}
	else 
	{
		printf("unimplement\n");
	}
}

#endif



