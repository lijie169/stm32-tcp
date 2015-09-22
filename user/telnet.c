#include<stdio.h>
#include "telnet.h"
#include "file_adapter.h"
#include "tcp.h"
#include "stack.h"


#define BUFFER_SIZE (0x1000)


extern char rx_buf[] ;
extern char tx_buf[] ;
char rcv_buffer[BUFFER_SIZE] = { 0 };
short buffer_pos = 0 ;
int telnet_socket_id = -1 ;
extern int usart_write(char*buff, int size) ;

void telnet_save_buffer(char* buff,int size)
{
	if(buffer_pos+ size > BUFFER_SIZE)
		return ;
	memcpy(rcv_buffer,buff,size);
	buffer_pos += size ;
}

void telnet_dbg_data(void)
{
	printf("data len is :%d\n",buffer_pos);
	printf("data is :%s\n",rcv_buffer);

}

void telnet_event_handler(int socketId,char eventId,char* remoteIp,unsigned short port,unsigned short datastart,unsigned short datalen)
{
	short actLen = datalen ;
	static unsigned char accept_req = 0 ;
	static unsigned char telent_fd = 0 ;
	/*
	TCP_EVENT_CONN_REQ		  
	TCP_EVENT_ESTABLISHED	
	TCP_EVENT_DATA	    	
	TCP_EVENT_SEND_COMPLETED
	TCP_EVENT_CONN_CLOSED	
	TCP_EVENT_CLOSE_REQ		
	TCP_EVENT_RESET			
	TCP_EVENT_rTOUT 		
	TCP_EVENT_cTOUT 		
	*/
	//printf("event Id %d\n",eventId);
	switch(eventId)
	{
		case TCP_EVENT_CONN_REQ :
		{
			if(-1 != telnet_socket_id)
			{
				tcp_conn_reject(socketId);
				return ;
			}

			socketId = tcp_conn_accept(socketId);
			break ;
		}
		case TCP_EVENT_ESTABLISHED:
		{

			telnet_socket_id = socketId ;
			install_telnet_dev();
			break ;
		}
		case TCP_EVENT_CLOSE_REQ :
		case TCP_EVENT_RESET :
		case TCP_EVENT_rTOUT :
		case TCP_EVENT_cTOUT :
		{
			tcp_conn_close(socketId);
			install_uart_dev();
			telnet_socket_id = -1 ;
			break ;
		}
		case TCP_EVENT_CONN_CLOSED :
		{
			tcp_socket_close(socketId);
			break ;
		}
		
		default:
			if(TCP_EVENT_DATA != eventId )
				return ;
			telnet_save_buffer(&rx_buf[datastart], datalen);
	
		}

}
void telnet_service_init(void)
{
	int socketId = tcp_socket_open(9023,telnet_event_handler );
	tcp_listen(socketId);
}

#define TELNET_PROMPT "ksh>"
#define PROMPT_SIZE (strlen(TELNET_PROMPT))

int telnet_output(char* buff,int size)
{
	int pos = TCP_DATA_START ;
	int loop = 0 ;

	memcpy(&tx_buf[pos],TELNET_PROMPT,PROMPT_SIZE);
	pos += PROMPT_SIZE;

	if(size > TCP_DATA_MAX - PROMPT_SIZE)
		size = TCP_DATA_MAX - PROMPT_SIZE ;

	memcpy(&tx_buf[pos],buff,size);
	tcp_send(telnet_socket_id, size+PROMPT_SIZE);	

	return size ;	
}

int telnet_input(char* buff,int maxsize)
{
	int size = buffer_pos ;
	if(size > maxsize)
		size = maxsize ;

	memcpy(buff,rcv_buffer,size);
	buffer_pos -= size ;

	return size ;
		
}
void install_telnet_dev(void)
{
	unregister(0);
	register_open(telnet_input,telnet_output);
}

