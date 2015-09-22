/******************************************************************************************
   ip.h (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#ifndef __ip_H
#define __ip_H

#ifdef ETHERNET
  #define datalink_send ethernet_send
#endif
#ifdef PPP_LINK
  #define datalink_send ppp_send
#endif
#define IPr  ((struct ip_hdr *)&rx_buf[DATALINK_HDR_SIZE])
#define IPt  ((struct ip_hdr *)&tx_buf[DATALINK_HDR_SIZE])
#define IP_DATA_START  (DATALINK_HDR_SIZE + sizeof(struct ip_hdr))
#define IP_DATA_MAX    (PACKET_BUF_SIZE - DATALINK_HDR_SIZE - sizeof(struct ip_hdr))

#define SWAP(n)	    ((n&0xFF)<<24) + ((n&0xFF00)<<8) + ((n&0xFF0000)>>8) + ((n&0xFF000000)>>24)

struct ip_hdr {
  char           verhl;		      // IP version and Header Length
  char           tos;		      // type of service
  unsigned short len;		      // total length
  unsigned short id;		      // identification
  unsigned short fragment;	      // Flags and Fragment Offset
  char		 ttl;		      // time to live
  char		 protocol;	      // protocol
  unsigned short hdrchksum;           // IP header Checksum
  char		 srcipaddr[4];        // source IP address
  char		 destipaddr[4];	      // destination IP address
};


struct pseudo_hdr {		      // Pseudo Header for UDP and TCP Chksum Calculations
  char           srcipaddr[4];
  char           destipaddr[4];
  char           zero;
  char           protocol;
  unsigned short len;
};



void ip_process(void);
void ip_send(char, char *, unsigned short);
unsigned int chksum16(void *, short, int, char);

#endif

