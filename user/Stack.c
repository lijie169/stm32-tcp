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

#include "Stack.h"
#ifdef ETHERNET
  #include "ethernet.h"
  #include "arp.h"
#endif
#include "udp.h"
#include "tcp.h"

void stack_init(void) {

  #ifdef ETHERNET
    ethernet_init();
    arp_init();		
    arp_request(Gateway);
  #endif
  	udp_init();
  	my_udp_init();
	tcp_init();
	my_tcp_service_init();
}


void stack_process(void) {
  #ifdef ETHERNET
    ethernet_poll();
    arp_poll();
  #endif
    tcp_poll();
}
