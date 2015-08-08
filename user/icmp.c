/******************************************************************************************
   icmp.c (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#include "stack.h"
#include "icmp.h"
#include "ip.h"
#ifdef ETHERNET
  #include "ethernet.h"
#endif
#ifdef PPP_LINK
  #include "PPP.h"
#endif	
#include <stdio.h>
#include <string.h>


/***************************************/
/* ICMP Private Functions              */
/***************************************/
#if debug_icmp
void icmp_display(unsigned short data_len) {
  unsigned short i = ICMP_DATA_START;

  vprintf("ICMP Header:\n");
  vprintf("Type:       %d\n", ICMPr->type);
  vprintf("Code:       %d\n", ICMPr->code);
  vprintf("Checksum:   %04x\n", HTONS(ICMPr->icmpchksum));
  vprintf("Id:         %d\n", (rx_buf[i+1]<<8)+rx_buf[i]);
  vprintf("Sequence:   %d\n", (rx_buf[i+3]<<8)+rx_buf[i+2]);
  vprintf("Data:\n");
  for (i=0; i<(data_len-4); i++) {
    vprintf("%c", rx_buf[ICMP_DATA_START+4+i]);			
  }
  vprintf("\n--------------------------------\n");
}
#endif //debug_icmp


/***************************************/
/*   ICMP Public Functions             */
/***************************************/
void icmp_process(unsigned short len) {
  unsigned short chksum1;
  unsigned short chksum2;
  short i;

#if debug_icmp
  icmp_display(len-4);				// ICMP Echo Data Length (ICMP message length - 4)
#endif //debug_icmp

  chksum1 = HTONS(ICMPr->icmpchksum);
  ICMPr->icmpchksum = 0;
  chksum2 = chksum16(&ICMPr->type, len, 0, 1);
  if (chksum2 != chksum1) {
	 #if debug_icmp 
	 vprintf("ICMP: Bad Checksum %04x (it should be %04x)\n",chksum1, chksum2);
	 #endif
	return;
  }
  ICMPr->icmpchksum = HTONS(chksum1);		  // restore checksum
  i = ICMP_DATA_START;
  switch(ICMPr->type) {					
    case ICMP_Echo:				  // Echo Request (8)
	  #if debug_icmp
      vprintf("ICMP: Ping Request received from %d.%d.%d.%d \r\n", IPr->srcipaddr[0],IPr->srcipaddr[1],
	                                                          IPr->srcipaddr[2],IPr->srcipaddr[3]);  												
	  #endif
      ICMPt->type = ICMP_Echo_Reply;	          // Echo Reply  (0)
      ICMPt->code = 0;			                	
      tx_buf[i] = rx_buf[i];	    		  // id
      tx_buf[i+1] = rx_buf[i+1];
      tx_buf[i+2] = rx_buf[i+2];		  // seq
      tx_buf[i+3] = rx_buf[i+3];                           										
      memcpy(&tx_buf[i+4], &rx_buf[i+4], len-8);  // copy ping data  	
      icmp_send(IPr->srcipaddr, len);	
      break;
    case ICMP_Echo_Reply:
		#if debug_icmp
      vprintf("ICMP: Ping Reply received from %d.%d.%d.%d \r\n", IPr->srcipaddr[0],IPr->srcipaddr[1],
	                                                        IPr->srcipaddr[2],IPr->srcipaddr[3]);
		#endif
      break;
    case ICMP_Destination_Unreachable:
      switch(ICMPr->code) {
	case ICMP_Protocol_Unreachable:
	  #if debug_icmp 
	  vprintf("ICMP Dest.Unreachable received: Protocol %d Unreachable\n",rx_buf[i+13]);
	  #endif
	  break;
	case ICMP_Port_Unreachable:
	  #if debug_icmp 
	vprintf("ICMP Dest.Unreachable received: Port %d Unreachable\n",(rx_buf[i+26]<<8)+rx_buf[i+27]);
	#endif
	  break;
	default:
	  #if debug_icmp 
	vprintf("ICMP Dest.Unreachable received: Unknown Code %d\n",ICMPr->code);
	#endif
          break;
      }
      break;
    default:
      #if debug_icmp 
	vprintf("ICMP received: unknown type %d\n",ICMPr->type);
	#endif
      break;
  }
}


void icmp_ping(char *IPdest) {
  short i;
  static char id;

  i = IP_DATA_START;
  ICMPt->type=8;			//Echo Request
  ICMPt->code=0;
  tx_buf[i+4] =	0x03;			// identifier (any number...)
  tx_buf[i+5] =	0x00;
  tx_buf[i+6] = ++id;	                // sequence number
  tx_buf[i+7] = 0x00;			
  i = ICMP_DATA_START+4;
  strcpy(&tx_buf[i],"Embedded Internet PING");
  icmp_send(IPdest, 30);  	
}


void icmp_send(char *IPdest, unsigned short len) {

  ICMPt->icmpchksum = 0;										  // reset the ICMP Checksum field
  ICMPt->icmpchksum = HTONS(chksum16(&ICMPt->type, len, 0, 1));   // and recalculate it
  ip_send(ICMP_PROTOCOL, IPdest, len);				  // use the IP layer to send data
}

