/******************************************************************************************
   ip.c (v1.0)
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

#include "ethernet.h"

#include <stdio.h>
#include <string.h>	

unsigned short ipid;			


/***************************************/
/* IP Private Functions                */
/***************************************/
#if debug_ip
void ip_display(void) {
  vprintf("IP Message:\n");
  vprintf("IP Version:       %d\n", (IPr->verhl>>4));
  vprintf("Hdr Len:          %d bytes\n", (IPr->verhl & 0x0F) * 4);
  vprintf("TOS:              0x%02x\n", IPr->tos);
  vprintf("Tot Len:          %d bytes\n", HTONS(IPr->len));
  vprintf("Id:               %d\n", HTONS(IPr->id));
  vprintf("Fragment:         0x%04x\n", HTONS(IPr->fragment));
  vprintf("TTL:              %d\n", IPr->ttl);
  vprintf("Protocol:         %d\n", IPr->protocol);
  vprintf("Hdr Chksum:       0x%04x\n", HTONS(IPr->hdrchksum));
  vprintf("Source Address:   %d.%d.%d.%d\n", IPr->srcipaddr[0], IPr->srcipaddr[1], IPr->srcipaddr[2], IPr->srcipaddr[3]);
  vprintf("Dest.  Address:   %d.%d.%d.%d\n", IPr->destipaddr[0], IPr->destipaddr[1], IPr->destipaddr[2], IPr->destipaddr[3]);														
  vprintf("\n--------------------------------\n");
}
#endif //debug_ip



/***************************************/
/*   IP Public Functions               */
/***************************************/
int chksum16(void *buf1, short len, int chksum, char complement) {
  unsigned short * buf = buf1;
  int chksum16;

  while(len > 0) {	
    if (len == 1)
      chksum16 = ((*buf)&0x00FF);
    else
      chksum16 = (*buf);
    chksum = chksum + HTONS(chksum16);
    *buf++;
    len -=2;
  }
  if (complement)
    return (~(chksum + ((chksum & 0xFFFF0000) >> 16))&0xFFFF);
  return chksum;
}


void ip_process(void) {
  unsigned short len;
  unsigned short chksum1;
  unsigned short chksum2;
  int i;

#if debug_ip
  ip_display();
#endif //debug_ip

  if ((IPr->verhl & 0xF0) != 0x40) {
    #if debug_ip
	 vprintf("IP: packet version not supported!\r\n");
	#endif
    return;
  }
  if ((IPr->verhl & 0x0F) != 0x05) {
    #if debug_ip
	 vprintf("IP: header options not supported!\r\n");
	#endif
    return;
  }
  if ((HTONS(IPr->fragment) & 0x1FFF) != 0x0000) {
    #if debug_ip
	 vprintf("IP: fragmented packets not supported!\r\n");
	#endif
    return;
  }
  chksum1 = HTONS(IPr->hdrchksum);
  IPr->hdrchksum = 0;
  chksum2 = chksum16(&IPr->verhl, (IPr->verhl & 0x0F) * 4, 0, 1);
  if (chksum2 != chksum1) {
    #if debug_ip
	 vprintf("IP: Bad Checksum %04x (it should be %04x)\n",chksum1, chksum2);
	#endif
    return;				  // returns if chksum failed!
  }
  IPr->hdrchksum = HTONS(chksum1);	  // restore checksum
  if (memcmp(IPr->destipaddr, MyIP, 4))	  // is this IP datagram for us?
    return;				  // if not, return

  len = HTONS(IPr->len) - ((IPr->verhl & 0x0F) * 4); 		// Length of IP Data
  switch (IPr->protocol) {
    case ICMP_PROTOCOL:
      icmp_process(len);
      break;
    case UDP_PROTOCOL:
      udp_process(len);	 //Not Yet Implemented...
      break;
    //case TCP_PROTOCOL:
      //tcp_process(len);	 Not Yet Implemented...
      //break;
    default:				  // send Protocol Unreachable ICMP message
      i = ICMP_DATA_START;
      // Copy original IP header + first 8 bytes of IP Data
      memcpy(&tx_buf[i+4], &rx_buf[DATALINK_HDR_SIZE], sizeof(struct ip_hdr)+8);			
      ICMPt->type = ICMP_Destination_Unreachable;
      ICMPt->code = ICMP_Protocol_Unreachable;			 				
      tx_buf[i]   = 0;			  // Unused (reserved)
      tx_buf[i+1] = 0;			  // Unused (reserved)
      tx_buf[i+2] = 0;			  // Unused (reserved)
      tx_buf[i+3] = 0;                    // Unused (reserved)          										
      icmp_send(IPr->srcipaddr, sizeof(struct icmp_hdr)+4+sizeof(struct ip_hdr)+8);
      break;
  }
}


void ip_send(char protocol, char *IPdest, unsigned short len) {

  IPt->verhl=0x45;
  IPt->tos=0x00;
  IPt->len=HTONS(len+sizeof(struct ip_hdr));		
  ipid++;
  IPt->id = HTONS(ipid);
  IPt->fragment=0;
  IPt->ttl=128;
  IPt->protocol=protocol;     		
  IPt->hdrchksum=0;
  memcpy(IPt->destipaddr, IPdest, 4);
  memcpy(IPt->srcipaddr, MyIP, 4);		
  IPt->hdrchksum = HTONS(chksum16(&IPt->verhl, (IPt->verhl & 0x0F) * 4, 0, 1));
  datalink_send(IP_PROTOCOL, len+sizeof(struct ip_hdr));					
}

