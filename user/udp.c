/******************************************************************************************
   udp.c (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#include "stack.h"
#include "ip.h"
#include "icmp.h"
#include "udp.h"
#ifdef ETHERNET
  #include "ethernet.h"
#endif
#ifdef PPP_LINK
  #include "PPP.h"
#endif
#include <stdio.h>
#include <string.h>


#define UDPr  ((struct udp_hdr *)&rx_buf[sizeof(struct ip_hdr) + DATALINK_HDR_SIZE])	
#define UDPt  ((struct udp_hdr *)&tx_buf[sizeof(struct ip_hdr) + DATALINK_HDR_SIZE])	

#pragma pack(2)
struct udp_hdr {
  unsigned short srcport;		// Source Port
  unsigned short destport;		// Destination Port
  unsigned short totlen;	        // total length ( header + data)
  unsigned short udpchksum;		// UDP Checksum
};

#pragma pack(4)
struct ucb {
  char           state;
  unsigned short local_port;
  char           options;
  void	         (*event_handler)(int,char,char*,unsigned short,unsigned short,unsigned short);
};
#pragma pack()

#define UDP_SOCKET_FREE	0
#define UDP_SOCKET_USED 1

struct ucb udp_socket[UDP_MAX_SOCKETS];	
static unsigned short last_port = 1024;


/***************************************/
/*   UDP Private Functions             */
/***************************************/
#if debug_udp
void udp_display(void) {
  int i;

  printf("UDP Message:\n");
  printf("Source Port:       %d\n", HTONS(UDPr->srcport)); 			
  printf("Destination Port:  %d\n", HTONS(UDPr->destport));
  printf("Total Length:      %d bytes\n", HTONS(UDPr->totlen));
  printf("Checksum:          %04x\n", HTONS(UDPr->udpchksum));
  printf("Data:\n");
  for (i=0; i<(HTONS(UDPr->totlen)-8); i++) {
    printf("%c", rx_buf[UDP_DATA_START+i]);			
  }
  printf("\n-------------------------------------------\n");
}
#endif //debug_udp


int udp_validate_socket(int socket) {

  if ((socket < 1) || (socket > UDP_MAX_SOCKETS)) {
    #if debug_udp
	
	printf("UDP: Invalid Socket Error!\r\n");
	
	#endif
    return UDP_INVALID_SOCKET;
  }
  return 0;
}

/***************************************/
/*   UDP Public Functions              */
/***************************************/
void udp_init(void) {

  int iSocket;
  struct ucb* pSocket;

  for(iSocket=0; iSocket<UDP_MAX_SOCKETS; iSocket++) {
    pSocket = &udp_socket[iSocket];
    pSocket->state = UDP_SOCKET_FREE;
    pSocket->local_port = 0;
    pSocket->options = 0;
    pSocket->event_handler = 0;
  }
  return;
}


void udp_process(unsigned short len) {

  struct pseudo_hdr p_hdr;
  unsigned short chksum1;
  int chksum2;		
  int iSocket, j;
  struct ucb* pSocket;

#if debug_udp
  udp_display();
#endif //debug_udp

  len = len;		// avoid compiler warning!
  if (UDPr->udpchksum != 0) {
    chksum1 = HTONS(UDPr->udpchksum);
    memcpy(&p_hdr.srcipaddr[0], &IPr->srcipaddr[0], 4);
    memcpy(&p_hdr.destipaddr[0], &IPr->destipaddr[0], 4);
    p_hdr.zero = 0;
    p_hdr.protocol = UDP_PROTOCOL;	
    p_hdr.len = UDPr->totlen;
    chksum2 = chksum16(&p_hdr.srcipaddr[0], sizeof(p_hdr), 0, 0);
    UDPr->udpchksum = 0;
    chksum2 = chksum16(&UDPr->srcport,HTONS(UDPr->totlen), chksum2, 1);
    if (chksum2 == 0)
      chksum2 = 0xFFFF;
    if (chksum2 != chksum1) {
      #if debug_udp
	
	  printf("UDP: Bad Checksum %04x (it should be %04x)\n",chksum1, chksum2);

	  #endif
      return;											// returns if chksum failed!
    }
    UDPr->udpchksum = HTONS(chksum1);			// restore checksum
  }
  // look up the socket for the recipient port
  // and call the application event_handler
  for(iSocket=0; iSocket<UDP_MAX_SOCKETS; iSocket++) {
    pSocket = &udp_socket[iSocket];
    if (pSocket->local_port == HTONS(UDPr->destport) &&
                      pSocket->state != UDP_SOCKET_FREE) {

      pSocket->event_handler(iSocket+1,UDP_EVENT_DATA,IPr->srcipaddr,HTONS(UDPr->srcport),
	                        UDP_DATA_START,HTONS(UDPr->totlen)-sizeof(struct udp_hdr));
      return;
    }
  }
  // If we are here, the Port is unreachable...Send an ICMP informing that !
  #if debug_udp
	
  	printf("Destination Port %d not found!\n", HTONS(UDPr->destport));
	
  #endif
  j = ICMP_DATA_START;
  // Copy original IP header + first 8 bytes of IP Data
  memcpy(&tx_buf[j+4], &rx_buf[DATALINK_HDR_SIZE], sizeof(struct ip_hdr)+8);			
  ICMPt->type = ICMP_Destination_Unreachable;
  ICMPt->code = ICMP_Port_Unreachable;			 				
  tx_buf[j]   = 0;				  // Unused (reserved)
  tx_buf[j+1] = 0;				  // Unused (reserved)
  tx_buf[j+2] = 0;			   	  // Unused (reserved)
  tx_buf[j+3] = 0;                                // Unused (reserved)          										
  icmp_send(IPr->srcipaddr, sizeof(struct icmp_hdr)+4+sizeof(struct ip_hdr)+8);
}


void udp_send(int socket, char *IPdest, unsigned short destport, unsigned short len) {

  struct pseudo_hdr p_hdr;
  int chksum2;	
  int iSocket = socket-1;

  if (udp_validate_socket(socket) == UDP_INVALID_SOCKET)
    return;
  if (udp_socket[iSocket].state == UDP_SOCKET_FREE) {
    #if debug_udp
	
	printf("UDP Send: the socket is free: %d\n", iSocket+1);
	
	#endif
    return;
  }
  UDPt->srcport = HTONS(udp_socket[iSocket].local_port);
  UDPt->destport = HTONS(destport);
  UDPt->totlen = HTONS(len + 8);
  if (udp_socket[iSocket].options == UDP_CHKSUM_SEND) {
    memcpy(&p_hdr.srcipaddr[0], &MyIP[0], 4);
    memcpy(&p_hdr.destipaddr[0], IPdest, 4);
    p_hdr.zero = 0;
    p_hdr.protocol = UDP_PROTOCOL;	
    p_hdr.len = UDPt->totlen;
    chksum2 = chksum16(&p_hdr.srcipaddr[0], sizeof(p_hdr), 0, 0);
    UDPt->udpchksum = 0;
    chksum2 = chksum16(&UDPt->srcport,HTONS(UDPt->totlen), chksum2, 1);
    if (chksum2 == 0)
      chksum2 = 0xFFFF;
    UDPt->udpchksum = HTONS(chksum2);
  }else {
    UDPt->udpchksum = 0;
  }
  ip_send(UDP_PROTOCOL, IPdest, len+8);
}


static int check_free_port(unsigned short port) {
  int iSocket;

  for(iSocket=0; iSocket<UDP_MAX_SOCKETS; iSocket++) {
    if (udp_socket[iSocket].local_port == port)
      return 0;
  }
  return 1;
}


int udp_get_port(int socket) {
  int iSocket = socket-1;

  if (udp_validate_socket(socket) == UDP_INVALID_SOCKET)
    return UDP_SOCKET_ERROR;
  return udp_socket[iSocket].local_port;
}


int udp_open_socket(unsigned short port, char options,
                    void (*event_handler)(int,char,char*,unsigned short,unsigned short,unsigned short)) {
  int i;
  struct ucb* pSocket;

  if (port == 0) {
    while(check_free_port(last_port++) != 1);
    if (last_port >= 49999)
      last_port = 1024;
    port = last_port++;
  }else {
    if (check_free_port(port) != 1)
      return UDP_PORT_ALREADY_USED;
  }
  for(i=0; i<UDP_MAX_SOCKETS; i++) {
    pSocket = &udp_socket[i];
    if(pSocket->state != UDP_SOCKET_FREE)
      continue;
    pSocket->state = UDP_SOCKET_USED;
    pSocket->local_port = port;	
    pSocket->options = options;
    pSocket->event_handler = event_handler;
    return (i+1);
  }
  return UDP_INVALID_SOCKET;
}


void udp_close_socket(int socket) {
  int iSocket = socket-1;

  if (udp_validate_socket(socket) == UDP_INVALID_SOCKET)
    return;
  udp_socket[iSocket].state = UDP_SOCKET_FREE;
  udp_socket[iSocket].local_port = 0;
  udp_socket[iSocket].options = 0;
  udp_socket[iSocket].event_handler = 0;
}

void my_udp_service(int socket ,char event,char* senderIp,unsigned short senderPort,unsigned short start,unsigned short len)
{
	memcpy(UDPt+1,UDPr+1,len);

	udp_send(socket, senderIp, senderPort, len);
}

void my_udp_init(void)
{
	udp_open_socket(1066, 0, my_udp_service);
}

