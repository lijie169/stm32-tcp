#include "ethernet.h"
#include "arp.h"
#include "enc28j60.h"
#include "stack.h"

#define MyMACAddress			{0x00, 0x08, 0x54, 0x18, 0x5D, 0x50}
#define GatewayAddress	 	{192, 168, 1, 1}
#define SubNetMaskAddress		{255, 255, 255, 0}
#define MyIPAddress			{192, 168, 1, 99}
#define pcIP				    {192, 168, 1, 100}


char rx_buf[PACKET_BUF_SIZE];
char tx_buf[PACKET_BUF_SIZE];
char MyMAC[6] = MyMACAddress;
char MyIP[4]  = MyIPAddress;	        // device IPv4 address	
char Gateway[4] = GatewayAddress;
char SubNetMask[4] = SubNetMaskAddress;

void ethernet_send(unsigned short protocol, unsigned short len) {
  char IPdest[4] = pcIP;
			
  if (protocol == ARP_PROTOCOL) {
    if (HTONS(ARPt->opcode) == 0x0001) {
      memset(FRAMEt->destination,0xFF,6);
    }else {
      memcpy(FRAMEt->destination,ARPt->tarhwaddr,6);
    }
  }else {	
    if (arp_resolve(IPdest, FRAMEt->destination) == ARP_ENTRY_NOT_FOUND) {
      return;
    }
  }	
  memcpy(FRAMEt->source, MyMAC, 6);
  FRAMEt->protocol = HTONS(protocol);
  enc28j60_packet_send((char*)FRAMEt,len + (sizeof(struct ethernet_hdr)));
//  frame_send(len + (sizeof(struct ethernet_hdr)));	
}

int frame_get(void)
{
	if (enc28j60_packet_getcount() == 0)
    		return 0 ;
	return enc28j60_packet_receive(rx_buf,PACKET_BUF_SIZE);
}
	
void frame_process(void) {
	if(frame_get() == 0)
		return ;
	
  switch (HTONS(FRAMEr->protocol)) {
    case ARP_PROTOCOL:
      arp_process();
      break;
    case IP_PROTOCOL:
        ip_process();
      break;
  }
}


void ethernet_poll(void) {
    frame_process();
}

void ethernet_init(void)
{
	enc28j60_init(MyMAC);
}

int vprintf(char* fmt,...)
{

	return 0 ;
}
