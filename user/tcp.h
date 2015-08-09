/******************************************************************************************
   tcp.h (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#ifndef __tcp_H
#define __tcp_H

#define TCP_MAX_SOCKETS		     	10	
#define TCP_INVALID_SOCKET      	-1
#define TCP_PORT_ALREADY_USED   	-2
#define TCP_SOCKET_ERROR		-3

#define TCP_EVENT_CONN_REQ		    0
#define TCP_EVENT_ESTABLISHED		1
#define TCP_EVENT_DATA	    		2
#define TCP_EVENT_SEND_COMPLETED	3
#define TCP_EVENT_CONN_CLOSED		4
#define TCP_EVENT_CLOSE_REQ			5
#define TCP_EVENT_RESET				6   // a RST was received
#define TCP_EVENT_rTOUT 			7   // re-transm. time out - connection closed - socket closed
#define TCP_EVENT_cTOUT 			8   // connection time out - connection closed - socket closed

void tcp_init(void);
void tcp_poll(void);
void tcp_process(unsigned short len);
int  tcp_socket_open(unsigned short port, void (*event_handler)(int,char,char*,unsigned short,unsigned short,unsigned short));
void tcp_socket_close(int socket);
void tcp_sockets_show(void);
void tcp_conn_open(int socket, char *destipaddr,unsigned short destport);
int tcp_conn_accept(int socket);
void tcp_conn_reject(int socket);
void tcp_conn_close(int socket);
short tcp_get_port(int socket);
int tcp_send_data(int socket, int (*fill_buffer)(int socket, unsigned short * position, unsigned short * len));
int tcp_listen(int socket);

#endif

