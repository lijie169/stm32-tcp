#include "stm32f10x.h"
#define ARP_TABLE 1
#define ARP_REQ   2


#define ARG_MAX_SIZE   (25)
#define ARG_NOR_END    (1)
#define ARG_SPACE_END  (0)
#define ARG_EOF_END    (-1)

#define ARG_OK          (0)
#define ARG_ERROR_LONG (-2)
#define ARG_ERROR_END  (-3)
#define ARG_MAX_NUMS   (6)

void telnet_dbg_data(void);
void help_usart(void);
void shell_arp(void);
void shell_eth(void);

//static uint8_t usart_tx_ptr = 0 ;
// uint8_t usart_tx_buf[0xFF]= {0};
extern uint8_t usart_rx_buf[];
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
	{"?",help_usart,"is help",1},
	{"arp",shell_arp,"arp debug",1},
	{"eth",shell_eth,"eth debug",3},
	{"file",telnet_dbg_data,"eth debug",0},
};





int8_t str_parse(uint8_t* dst_str,uint8_t* src,uint8_t * pos)
{

	uint8_t len = 0 ;
	int8_t ret ;
	int8_t * src_str = src ;
	int8_t tmp ;
	while(*src_str != '\0' && *src_str ==' ')
	{
			src_str++;		 
	}

	while(1)
	{
		tmp = *src_str ;
		if(tmp =='\0')
		{
			ret = ARG_EOF_END;
			break ;
		}
		else if(tmp == ' ')
		{
			ret = ARG_SPACE_END ;
			break ;
		}
		else if(tmp == '\r' && *(src_str+1) == '\n')
		{
			src_str += 2 ;
			ret = ARG_NOR_END ;
			break;
		}
		else
		{
			*dst_str = tmp ;
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

#if 0
int8_t cmd_parse(uint8_t* cmd_str,uint8_t* num)
{
	int8_t ret ;
	uint8_t pos = 0 ;
	uint8_t curpos = 0 ;
	uint8_t idx;
	uint8_t tmp ;
	uint8_t len = *num;

	if(len < 2 || cmd_str[len - 2] !='\r' || cmd_str[len - 1] !='\n' )
	{
		printf("input must end with \\r\\n\n");
		printf("len is %d,strlen is %d,%s\n",*num,len,cmd_str);
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
#endif

void shell(void)
{
	int8_t ret ;
	int32_t curpos = 0 ;
	int8_t curlen = 0 ;
	int32_t len = 0 ;
	uint8_t idx;
	uint8_t arg_count ;
	uint8_t count ;
	int8_t* cmd_str = usart_rx_buf ;

	
	len = input(cmd_str,0xFF,0);
	if(!len)
	{
		return  ;
	}

	while(curpos <= len)
	{
	
		for(arg_count = 0 ; arg_count < ARG_MAX_NUMS ; arg_count++)
		{
			memset(cmd_pointer[arg_count],0,sizeof(cmd_pointer[arg_count]));
			ret = str_parse(cmd_pointer[arg_count], cmd_str+curpos, &curlen);

			curpos += curlen ;	
			if(ret == ARG_NOR_END)
				break ;
			else if(ret == ARG_EOF_END)
			{
//				printf("over\n");
				return  ;
			}
		}

		count = sizeof(shell_cmd)/sizeof(cmd_typedef);
		for(idx = 0 ; idx < count ; idx++)
		{
		//	printf("cmp str :%s,%s,%d,%d\n",shell_cmd[idx].fun_str,cmd_pointer[0],strlen(cmd_pointer[0]),strlen(shell_cmd[idx].fun_str));
			
			if(!strcmp(cmd_pointer[0],shell_cmd[idx].fun_str)&& (arg_count) >= shell_cmd[idx].fun_count)
			{
				shell_cmd[idx].function();
//				printf("found\n");
				break  ;
			}
		}

//		printf("cannot found\n");
	}
	//vertify args
//	printf("end\n");
	return  ;

}

#if 0
void shell_help(void)
{
	uint8_t count = 0 ;
	uint8_t idx ;
	int8_t ret ;
	count = input(usart_rx_buf,0xFF);
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


void shell_process(uint8_t *shell_str,uint8_t count)
{
	uint8_t idx ;
	int8_t ret ;
//	count = usart_get_buff(usart_rx_buf,USART_BUFF_SIZE);
	if(!count || (0 == shell_str))
	{
		return ;
	}

#if 1
	ret = cmd_parse(shell_str,&count);
	if(ret != ARG_OK)
	{
		printf("parse arg error %d\n",ret);
//		printf("%x %s %x %x\n",usart_rx_buf[0],usart_rx_buf,usart_rx_buf[strlen(usart_rx_buf)-2],usart_rx_buf[strlen(usart_rx_buf)-1]);
		return ;
	}

#endif
	
	if(usart_dbg == 1)
	{
		printf("recv %s\n",shell_str);
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

	printf("\r\n");
}	

#endif
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
	else if(ETH_WRITE == mode)
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



