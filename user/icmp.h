/******************************************************************************************
   icmp.h (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#ifndef __icmp_H
#define __icmp_H

#define ICMPr  ((struct icmp_hdr *)&rx_buf[sizeof(struct ip_hdr) + DATALINK_HDR_SIZE])	
#define ICMPt  ((struct icmp_hdr *)&tx_buf[sizeof(struct ip_hdr) + DATALINK_HDR_SIZE])	
#define ICMP_DATA_START  (DATALINK_HDR_SIZE + sizeof(struct ip_hdr) + sizeof(struct icmp_hdr))
#define ICMP_DATA_MAX    (PACKET_BUF_SIZE - DATALINK_HDR_SIZE - sizeof(struct ip_hdr) - sizeof(struct icmp_hdr))

#define ICMP_Echo       		  8
#define ICMP_Echo_Reply 		  0
#define ICMP_Destination_Unreachable	  3	// Type = 3
#define ICMP_Protocol_Unreachable	  2	// Code = 2
#define ICMP_Port_Unreachable		  3	// Code = 3


struct icmp_hdr {
  char		  type;			// type of icmp message	
  char 		  code;			// "subtype" of icmp message
  unsigned short  icmpchksum;	        // icmp message checksum
};

void icmp_process(unsigned short);
void icmp_send(char *IPdest, unsigned short len);
void icmp_ping(char *IPdest);

#endif

