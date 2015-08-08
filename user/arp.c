/******************************************************************************************
   arp.c (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#include "stack.h"
#include "arp.h"
#include "ethernet.h"
#include "timer.h"
#include <string.h>
#include <stdio.h>	


struct arp_entry {
  char 			proaddr[4];
  char			hwaddr[6];
  char			state;
  unsigned short	tout;
  char			retries;
  unsigned short	queries;
};

struct arp_entry arp_cache[ARP_CACHE_SIZE];
timer_typedef arptimer ;

/***************************************/
/* ARP Private Functions               */
/***************************************/

#if debug_arp
void arp_display(void) {
  printf("ARP Message:\n");
  printf("HwType:    %04x\n", HTONS(ARPr->hwtype));
  printf("ProType:   %04x\n", HTONS(ARPr->protype));
  printf("HwAddrLen: %x\n", ARPr->hwaddrlen);
  printf("ProAddrLen:%x\n", ARPr->proaddrlen);
  printf("OpCode:    %04x\n", HTONS(ARPr->opcode));
  printf("Sender:    %02x-%02x-%02x-%02x-%02x-%02x (%d.%d.%d.%d)\n", ARPr->senhwaddr[0], ARPr->senhwaddr[1],
                                                ARPr->senhwaddr[2], ARPr->senhwaddr[3],
				                ARPr->senhwaddr[4], ARPr->senhwaddr[5],
					        ARPr->senproaddr[0], ARPr->senproaddr[1],
					        ARPr->senproaddr[2], ARPr->senproaddr[3]);
														
  printf("Target:    %02x-%02x-%02x-%02x-%02x-%02x (%d.%d.%d.%d)\n", ARPr->tarhwaddr[0], ARPr->tarhwaddr[1],
                                                ARPr->tarhwaddr[2], ARPr->tarhwaddr[3],
				                ARPr->tarhwaddr[4], ARPr->tarhwaddr[5],
					        ARPr->tarproaddr[0], ARPr->tarproaddr[1],
					        ARPr->tarproaddr[2], ARPr->tarproaddr[3]);
  printf("------------------------------------------------------\n");
}

#endif //debug_arp


int arp_allocate(void) {
  int i, j, q;

  for(i=0;i<ARP_CACHE_SIZE;i++) {
    if (arp_cache[i].state == ARP_ENTRY_FREE) {
      return i;
    }
  }
  q = -100 ;
  // there isn't a free entry...we will choose the less utilised entry
  for(i=0;i<ARP_CACHE_SIZE;i++) {
    if (arp_cache[i].queries > q) {
      q = arp_cache[i].queries;
      j = i;
    }
  }
  return j;	
}


void arp_update(void) {
  int i;
  struct arp_entry * entry;

  for(i=0;i<ARP_CACHE_SIZE;i++) {
    entry = &arp_cache[i];
    if (entry->state == ARP_ENTRY_FREE)
      continue;											
    if (memcmp(entry->proaddr,ARPr->senproaddr,4)==0) {	// the ip is resolved
      entry->state = ARP_ENTRY_OK;
      entry->tout = ARP_ENTRY_TOUT;
      entry->retries = ARP_DEF_RETRIES;
      return;
    }
  }
  i = arp_allocate();
  entry = &arp_cache[i];
  memcpy(entry->proaddr, ARPr->senproaddr, 4);
  memcpy(entry->hwaddr, ARPr->senhwaddr, 6);
  entry->state = ARP_ENTRY_OK;
  entry->tout =	ARP_ENTRY_TOUT;
  entry->retries = ARP_DEF_RETRIES;
  entry->queries = 0;
}

char differ_subnet(char *proaddr) {

  return (proaddr[0] & SubNetMask[0])^(MyIP[0] & SubNetMask[0]) |
   		 (proaddr[1] & SubNetMask[1])^(MyIP[1] & SubNetMask[1]) |
		 (proaddr[2] & SubNetMask[2])^(MyIP[2] & SubNetMask[2]) |
		 (proaddr[3] & SubNetMask[3])^(MyIP[3] & SubNetMask[3]);
}




/***************************************/
/*   ARP Public Functions              */
/***************************************/
void arp_init(void) {
  int i;

  for(i=0;i<ARP_CACHE_SIZE;i++) {
    arp_cache[i].state = ARP_ENTRY_FREE;
  }
  timer_start(&arptimer, CLOCK_SECOND/10);
}


void arp_poll(void) {
  int i;

  if (timer_expired(&arptimer)) {		// 100 msec has elapsed
    for(i=0; i<ARP_CACHE_SIZE; i++) {
      if (arp_cache[i].state != ARP_ENTRY_FREE)
        arp_cache[i].tout--;
    }
    timer_start(&arptimer, CLOCK_SECOND/10);// 100ms
  }else
    return;

  for(i=0; i<ARP_CACHE_SIZE; i++) {
    if (arp_cache[i].state == ARP_ENTRY_FREE)
      continue;
    if (arp_cache[i].tout == ARP_ENTRY_TIMED_OUT) {
      switch(arp_cache[i].state) {
        case ARP_ENTRY_OK:
	  arp_request(arp_cache[i].proaddr);
	  arp_cache[i].tout = ARP_REQUEST_TOUT;
	  arp_cache[i].state = ARP_ENTRY_RESOLVING;
	  break;
        case ARP_ENTRY_RESOLVING:
	  if (arp_cache[i].retries > 0) {
	    arp_cache[i].retries--;
	    arp_request(arp_cache[i].proaddr);
	    arp_cache[i].tout = ARP_REQUEST_TOUT;
          }else
            arp_cache[i].state = ARP_ENTRY_FREE;
	    break;
        default:
		#if debug_arp
	       printf("ARP: Invalid State!\r\n");
		#endif
		;
      }
    }
  }
}


void arp_process(void) {
   int i;
#if debug_arp
  arp_display();
#endif //debug_arp

  if (differ_subnet(ARPr->senproaddr)==0)	  // only update addresses from local network devices
    arp_update();				  // (the rest will use the Gateway's address)
  if (! memcmp(ARPr->tarproaddr, MyIP, 4)) {	  // is this ARP message for us?
    if (ARPr->opcode == HTONS(ARP_Request)) {	  // is this an ARP Request?
      ARPt->hwtype = HTONS(0x0001);
      ARPt->protype = HTONS(0x0800);
      ARPt->hwaddrlen = 6;
      ARPt->proaddrlen = 4;
      ARPt->opcode= HTONS(ARP_Reply);			      // Opcode for ARP Reply
      memcpy(ARPt->tarhwaddr, ARPr->senhwaddr, 6);	      // use the sender MAC address
      memcpy(ARPt->tarproaddr, ARPr->senproaddr, 4);          // use the sender IP address
      memcpy(ARPt->senhwaddr, MyMAC, 6);		      // use our MAC address
      memcpy(ARPt->senproaddr, MyIP, 4);		      // use our IP address
      for(i=0;i<18;i++)
        tx_buf[42+i] = 0x20;
      ethernet_send(ARP_PROTOCOL, sizeof(struct arp_hdr)+18); // Send frame (Padding)
    }
  }
}

#if debug_arp
void arp_show_table(void) {
  int i;

  printf("\nARP Cache content:\r\n");
  printf("#   IP address    MAC address      State Tout Retries Queries\r\n");
  for(i=0;i<ARP_CACHE_SIZE;i++) {
    if (arp_cache[i].state != ARP_ENTRY_FREE) {	
      printf("%d:  %d.%d.%d.%d   %02x-%02x-%02x-%02x-%02x-%02x  %d    %d      %d      %d\r\n", i,
	                             arp_cache[i].proaddr[0], arp_cache[i].proaddr[1],
		                     arp_cache[i].proaddr[2], arp_cache[i].proaddr[3],
		                     arp_cache[i].hwaddr[0], arp_cache[i].hwaddr[1],
			             arp_cache[i].hwaddr[2], arp_cache[i].hwaddr[3],
			             arp_cache[i].hwaddr[4], arp_cache[i].hwaddr[5],
			    	     arp_cache[i].state, arp_cache[i].tout,
				     arp_cache[i].retries, arp_cache[i].queries);
    }
  }
  printf("-------------------------------------------------------------\r\n");
}
#endif

void arp_request(char *proaddr) {
  int i;

  ARPt->hwtype = HTONS(0x0001);
  ARPt->protype = HTONS(0x0800);
  ARPt->hwaddrlen = 6;
  ARPt->proaddrlen = 4;
  ARPt->opcode= HTONS(ARP_Request);
  memcpy(ARPt->tarproaddr, proaddr, 4);	
  memset(ARPt->tarhwaddr, 0x00, 6);
  memcpy(ARPt->senproaddr, MyIP, 4);					
  memcpy(ARPt->senhwaddr, MyMAC, 6);					
  for(i=0;i<18;i++)
    tx_buf[42+i] = 0x20;
  ethernet_send(ARP_PROTOCOL, sizeof(struct arp_hdr)+18);	// Send frame (Padding...)
}





int arp_resolve(char * ipdest, char * hwaddr) {
  int i;
  char proaddr[4];
  struct arp_entry * entry;

  if (differ_subnet(ipdest)) {
#if debug_arp
    printf("Different SubNet: using Gateway IP address\r\n");
#endif
    memcpy(proaddr, Gateway, 4);
  }else {
    memcpy(proaddr, ipdest, 4);
  }
  for(i=0;i<ARP_CACHE_SIZE;i++) {
    entry = &arp_cache[i];
    if (entry->state == ARP_ENTRY_FREE)
      continue;
    if ( memcmp(entry->proaddr, proaddr, 4)==0) {
      memcpy(hwaddr, entry->hwaddr,6);
      entry->queries++;	
      return ARP_ENTRY_FOUND;
    }
  }
  arp_request(proaddr);
#if debug_arp
  printf("ARP: Entry not found! Sending an ARP Request (Broadcast)\r\n");
#endif
  return ARP_ENTRY_NOT_FOUND;
}

