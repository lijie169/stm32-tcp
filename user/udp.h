/******************************************************************************************
   udp.h (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#ifndef __udp_H
#define __udp_H


#define UDP_MAX_SOCKETS		    3	
#define UDP_INVALID_SOCKET          -1
#define UDP_PORT_ALREADY_USED       -2
#define UDP_SOCKET_ERROR	    -3

#define UDP_CHKSUM_NONE		    0
#define UDP_CHKSUM_SEND		    1

#define UDP_EVENT_DATA	            1

void udp_init(void);
void udp_process(unsigned short len);
int  udp_open_socket(unsigned short, char, void (*event_handler)(int,char,char*,unsigned short,unsigned short,unsigned short));
void udp_close_socket(int socket);
void udp_send(int, char *, unsigned short, unsigned short);
int udp_get_port(int socket);
void my_udp_init(void);
#endif

