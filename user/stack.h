/******************************************************************************************
   Stack.h (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#ifndef __stack_H
#define __stack_H
#define  ETHERNET


#define PACKET_BUF_SIZE	1514
#define DATALINK_HDR_SIZE     14

extern char rx_buf[];
extern char tx_buf[];
void stack_init(void);
void stack_process(void);


#endif

