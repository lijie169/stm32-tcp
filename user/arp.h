/******************************************************************************************
   arp.h (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#ifndef __arp_H
#define __arp_H
#include "ethernet.h"
#define ARP_CACHE_SIZE	10

#define ARP_ENTRY_FREE		0
#define ARP_ENTRY_RESOLVING	1
#define ARP_ENTRY_OK		2

#define ARP_ENTRY_FOUND		 0
#define ARP_ENTRY_NOT_FOUND	-1

#define ARP_ENTRY_TOUT		100		// (10 sec)
#define ARP_REQUEST_TOUT	20		// (2 sec)
#define ARP_DEF_RETRIES		2
#define ARP_ENTRY_TIMED_OUT	0

#define ARP_Request 0x0001
#define ARP_Reply   0x0002

#define ARPr  ((struct arp_hdr *)&rx_buf[(sizeof(struct ethernet_hdr))])
#define ARPt  ((struct arp_hdr *)&tx_buf[(sizeof(struct ethernet_hdr))])

struct arp_hdr {
  unsigned short hwtype;
  unsigned short protype;
  char           hwaddrlen;
  char           proaddrlen;
  unsigned short opcode;
  char   	 senhwaddr[6];
  char           senproaddr[4];
  char           tarhwaddr[6];
  char           tarproaddr[4];
};



void arp_init(void);
void arp_poll(void);
int arp_resolve(char * IPdest, char * hwaddr);
void arp_request(char *proaddr);
void arp_show_table(void);
void arp_process(void);

#endif






