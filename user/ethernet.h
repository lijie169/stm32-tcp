
#ifndef __Ethernet_H
#define __Ethernet_H

#define ARP_PROTOCOL	    0x0806
#define IP_PROTOCOL	    0x0800
#define ICMP_PROTOCOL	    1
#define UDP_PROTOCOL	    17
#define TCP_PROTOCOL	    6

extern char MyMAC[];
extern char MyIP[];
extern char Gateway[];
extern char SubNetMask[];

#define HTONS(n) ((((unsigned short)((n) & 0xff)) << 8) | (((n) & 0xff00) >> 8))
#define FRAMEr   ((struct ethernet_hdr *)&rx_buf[0])
#define FRAMEt   ((struct ethernet_hdr *)&tx_buf[0])

#pragma pack(1)
struct ethernet_hdr {
  char destination[6];
  char source[6];
  unsigned short protocol;
};
#pragma pack()

void ethernet_init(void);
void ethernet_poll(void);
void frame_process(void);
int frame_get(void);
void ethernet_send(unsigned short protocol, unsigned short len);
#endif


