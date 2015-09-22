/******************************************************************************************
   tcp.c (v1.0)
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
#include "timer.h"
#include "tcp.h"
#ifdef ETHERNET
  #include "ethernet.h"
#endif
#include "icmp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define TCPr  ((struct tcp_hdr *)&rx_buf[sizeof(struct ip_hdr) + DATALINK_HDR_SIZE])	
#define TCPt  ((struct tcp_hdr *)&tx_buf[sizeof(struct ip_hdr) + DATALINK_HDR_SIZE])	

#define TCP_DEF_rTOUT	    30	  // Re-Transmission default TimeOut (3 sec)
#define TCP_DEF_TIME_WAIT   5	  // Time-Out for the TCP_SOCKET_TIME_WAIT state (0,5 sec)
#define TCP_DEF_RETRIES	    2	  // default retries = 2 (original segment and 2 re-transmissions
#define TCP_DEF_cTOUT	    1200  // Connection default TimeOut (120 sec)

#pragma pack(2)
struct tcp_hdr {
  unsigned short srcport;	  // Source Port
  unsigned short destport;	  // Destination Port
  unsigned int	 seq;		  // Sequence Number
  unsigned int 	 ack;		  // Acknowledgement Number
  char		 data_offset;	  // Data Offset
  char		 flags;		  // Flags (Control bits)
  unsigned short window;	  // Window Size
  unsigned short tcpchksum;	  // TCP Checksum
  unsigned short urgent_ptr;	  // Urgent Pointer
};
#pragma pack()


struct tcb {
  char           state;
  unsigned short local_port;
  char		 remote_ip[4];
  unsigned short remote_port;
  unsigned short remote_mss;
  char 		 flags;
  unsigned int	 snduna;
  unsigned int 	 sndnxt;
  unsigned short sndwnd;
  unsigned int	 rcvnxt;
  unsigned short rcvwnd;
  unsigned short rTimer;		
  unsigned short rTimeout;
  char		 retries;
  unsigned short cTimer;
  void	         (*event_handler)(int,char,char*,unsigned short,unsigned short,unsigned short);
  unsigned short more_data;
  unsigned short position;
  int	         (*fill_buffer) (int,unsigned short *,unsigned short *);
};		

#define TCP_SOCKET_FREE	  	  0
#define TCP_SOCKET_CLOSED 	  1
#define TCP_SOCKET_LISTEN	  2
#define TCP_SOCKET_SYN_SENT	  3
#define TCP_SOCKET_SYN_RECEIVED	  4
#define TCP_SOCKET_ESTABLISHED	  5
#define TCP_SOCKET_FIN_WAIT_1	  6
#define TCP_SOCKET_FIN_WAIT_2	  7
#define TCP_SOCKET_CLOSING	  8
#define TCP_SOCKET_TIME_WAIT	  9
#define TCP_SOCKET_CLOSE_WAIT	  10
#define TCP_SOCKET_LAST_ACK	  11

#define FIN		0x01
#define SYN		0x02
#define	RST		0x04
#define PSH		0x08
#define ACK		0x10
#define URG		0x20

#define	TCP_MSS		TCP_DATA_MAX
static unsigned short len;
unsigned short max_len;
struct tcb tcp_socket[TCP_MAX_SOCKETS];	
static unsigned short last_port = 1024;
static unsigned short opt_len = 0;
static unsigned short data_len = 0;
static char tcp_options = 0;
static char data_to_ack = 0;
timer_typedef tcptimer ;

extern char test = 0;	// test Re-Transmissions

#define TEST_INIT_WORD "hello,world"
#define TEST_DATA_LEN (1<<9)
static char test_data[TEST_DATA_LEN] = TEST_INIT_WORD; //my_tcp_event_handler use it
static short test_data_len = 11 ;
/***************************************/
/*   TCP Private Functions             */
/***************************************/

static uint8_t debug_tcp = 0 ;

void tcp_display(void) {
  int i;

  printf("TCP Message:\n");
  printf("Source Port:       %d\n", HTONS(TCPr->srcport)); 			
  printf("Destination Port:  %d\n", HTONS(TCPr->destport));
  printf("Sequence Number:   %u\n", SWAP(TCPr->seq));
  printf("Acknowledg.Number: %u\n", TCPr->ack);
  printf("Data Offset:       %d bytes (Options Length: %d bytes / Data Length: %d bytes)\n",
        							((TCPr->data_offset)>>4)*4, opt_len, data_len);
  printf("Flags:  ");
  if (TCPr->flags & FIN)
    printf("FIN ");
  if (TCPr->flags & SYN)
    printf("SYN ");
  if (TCPr->flags & RST)
    printf("RESET ");
  if (TCPr->flags & PSH)
    printf("PUSH ");
  if (TCPr->flags & ACK)
    printf("ACK ");
  if (TCPr->flags & URG)
    printf("URG ");					
  printf("\nWindow:            %d\n", HTONS(TCPr->window));
  printf("Checksum:          %04x\n", HTONS(TCPr->tcpchksum));
  printf("Urgent Pointer:    %d\n", HTONS(TCPr->urgent_ptr));
  printf("Data:\n");
  for (i=0; i<data_len; i++) {
    printf("%c", rx_buf[TCP_DATA_START+i]);			
  }
  printf("\n-------------------------------------------\n");
}
 //debug_tcp


int tcp_validate_socket(int socket) {

  if ((socket < 1) || (socket > TCP_MAX_SOCKETS)) {
    

	printf("TCP: Invalid Socket Error!\r\n");
	
    return TCP_INVALID_SOCKET;
  }
  return 0;
}


short get_max_len(int socket) {
  int iSocket = socket-1;

  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return TCP_SOCKET_ERROR;
  max_len = tcp_socket[iSocket].remote_mss;	
  if (max_len > tcp_socket[iSocket].sndwnd)
    max_len = tcp_socket[iSocket].sndwnd;		
  if (max_len > TCP_DATA_MAX)
    max_len = TCP_DATA_MAX;					
  return max_len;
}


void tcp_send(int socket, unsigned short len) {

  struct pseudo_hdr p_hdr;
  int chksum2;	
  int iSocket = socket-1;

  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return;
  if (tcp_socket[iSocket].state == TCP_SOCKET_FREE) {
    

	printf("TCP Send: the socket is free: %d\n", socket);

    
    return;
  }
  switch (tcp_socket[iSocket].state) {
    case TCP_SOCKET_SYN_RECEIVED:	
      case TCP_SOCKET_SYN_SENT:
        tcp_socket[iSocket].flags = SYN;
	//send a MSS=1460 TCP Option
	tx_buf[TCP_DATA_START]  = 0x02;
	tx_buf[TCP_DATA_START+1]= 0x04;
	tx_buf[TCP_DATA_START+2]= TCP_MSS>>8;
	tx_buf[TCP_DATA_START+3]= TCP_MSS&0xFF;
	tcp_options = 4;	
	break;
    case TCP_SOCKET_FIN_WAIT_1:
      tcp_socket[iSocket].flags = FIN;
      break;
  }
  if (len>0)
    tcp_socket[iSocket].flags |= PSH;
  if (tcp_socket[iSocket].state != TCP_SOCKET_SYN_SENT)
    tcp_socket[iSocket].flags |= ACK;

  TCPt->srcport = HTONS(tcp_socket[iSocket].local_port);
  TCPt->destport = HTONS(tcp_socket[iSocket].remote_port);	
  TCPt->seq = SWAP(tcp_socket[iSocket].sndnxt);			
  TCPt->ack = SWAP(tcp_socket[iSocket].rcvnxt);
  TCPt->data_offset = ((sizeof(struct tcp_hdr) + tcp_options)/4)<<4;
  TCPt->flags = tcp_socket[iSocket].flags;								
  TCPt->window = tcp_socket[iSocket].rcvwnd;		
  TCPt->tcpchksum = 0;
  TCPt->urgent_ptr = 0;

  // Calculate checksum
  memcpy(&p_hdr.srcipaddr[0], &MyIP[0], 4);
  memcpy(&p_hdr.destipaddr[0], tcp_socket[iSocket].remote_ip, 4);
  p_hdr.zero = 0;
  p_hdr.protocol = TCP_PROTOCOL;	
  p_hdr.len = HTONS(len+tcp_options+20);		
  chksum2 = chksum16(&p_hdr.srcipaddr[0], sizeof(p_hdr), 0, 0);
  chksum2 = chksum16(&TCPt->srcport,len+tcp_options+20, chksum2, 1);	
  TCPt->tcpchksum = HTONS(chksum2);
  ip_send(TCP_PROTOCOL, tcp_socket[iSocket].remote_ip, len+tcp_options+sizeof(struct tcp_hdr));
  if (tcp_socket[iSocket].flags != ACK)
    tcp_socket[iSocket].rTimer = tcp_socket[iSocket].rTimeout;	// Init timer
  if (len>0) {
    tcp_socket[iSocket].sndnxt += len;
  }else {
    if ((TCPt->flags & SYN) || (TCPt->flags & FIN))
      tcp_socket[iSocket].sndnxt++;
  }
  data_to_ack = 0;
  tcp_socket[iSocket].flags = 0;			
  tcp_options = 0;
}


void tcp_resend(int socket) {
  unsigned short len=0;
  int iSocket = socket-1;

  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return;
  if (tcp_socket[iSocket].state == TCP_SOCKET_ESTABLISHED) {
    len = tcp_socket[iSocket].sndnxt - tcp_socket[iSocket].snduna;
    tcp_socket[iSocket].position -= len;
    tcp_socket[iSocket].more_data = tcp_socket[iSocket].fill_buffer(socket, &tcp_socket[iSocket].position, &len);
    len = tcp_socket[iSocket].sndnxt - tcp_socket[iSocket].snduna;
    tcp_socket[iSocket].sndnxt = tcp_socket[iSocket].snduna;
    
	
	printf("%d Resending...\r\n",socket);
	tcp_socket_show(socket);
	
    tcp_send(socket, len);
    return;
  }
  if (tcp_socket[iSocket].state == TCP_SOCKET_CLOSING ||
          tcp_socket[iSocket].state == TCP_SOCKET_LAST_ACK)
    tcp_socket[iSocket].flags = FIN;
  tcp_socket[iSocket].sndnxt = tcp_socket[iSocket].snduna;
  tcp_send(socket, 0);
}


unsigned int get_ISN(void) {
  return ((rand()<<16)+rand());
}


unsigned short get_remote_mss(void) {
  int i;

  for (i=0;i<opt_len;) {
    if (rx_buf[TCP_DATA_START+i]<2) {
      i++;
    }else {
      if (rx_buf[TCP_DATA_START+i]==2) {
        return ((rx_buf[TCP_DATA_START+i+2]<<8)+rx_buf[TCP_DATA_START+i+3]);
      }
      i += (rx_buf[TCP_DATA_START+i+1]);
    }
  }
  // if mss option is not found, return default 536
  return 536;
}


int check_free_port(unsigned short port) {
  int i;

  for(i=0; i<TCP_MAX_SOCKETS; i++) {
    if (tcp_socket[i].local_port == port)
      return 0;
  }
  return 1;
}


void tcp_send_reset(int socket, char *destipaddr,unsigned short destport) {
  struct pseudo_hdr p_hdr;
  int chksum2;	
  int iSocket = socket-1;
  unsigned short len=0;

  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return;
  tcp_socket[iSocket].flags = RST + ACK;
  TCPt->srcport = HTONS(tcp_socket[iSocket].local_port);
  TCPt->destport = HTONS(destport);	
  TCPt->seq = SWAP(tcp_socket[iSocket].sndnxt);			
  TCPt->ack = SWAP(tcp_socket[iSocket].rcvnxt);
  if (TCPt->ack == 0)
    TCPt->ack = SWAP(SWAP(TCPr->seq) + 1);
  TCPt->data_offset = ((sizeof(struct tcp_hdr) + tcp_options)/4)<<4;
  TCPt->flags = tcp_socket[iSocket].flags;								
  TCPt->window = tcp_socket[iSocket].rcvwnd;		
  TCPt->tcpchksum = 0;
  TCPt->urgent_ptr = 0;

  // Calculate checksum
  memcpy(&p_hdr.srcipaddr[0], &MyIP[0], 4);
  memcpy(&p_hdr.destipaddr[0], destipaddr, 4);
  p_hdr.zero = 0;
  p_hdr.protocol = TCP_PROTOCOL;	
  p_hdr.len = HTONS(len+tcp_options+20);		
  chksum2 = chksum16(&p_hdr.srcipaddr[0], sizeof(p_hdr), 0, 0);
  chksum2 = chksum16(&TCPt->srcport,len+tcp_options+20, chksum2, 1);	
  TCPt->tcpchksum = HTONS(chksum2);

  ip_send(TCP_PROTOCOL, destipaddr, len+tcp_options+sizeof(struct tcp_hdr));
  tcp_socket[iSocket].flags = 0;			
  tcp_options = 0;
}


void tcp_process_segment(int socket) {
  struct tcb* pSocket;
  int iSocket = socket-1;

  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return;
  pSocket = &tcp_socket[iSocket];		// Get the pointer to the structure
  switch(pSocket->state) {
    case TCP_SOCKET_SYN_RECEIVED:
      if (TCPr->flags & RST) {
        if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->event_handler(iSocket+1,TCP_EVENT_RESET,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, 0);
	  tcp_socket_close(iSocket+1);
	  return;
	}
      }
      if (TCPr->flags == ACK) {	
        if (SWAP(TCPr->ack) == pSocket->sndnxt) {		
          pSocket->snduna = pSocket->sndnxt;
	  pSocket->sndwnd = HTONS(TCPr->window);
	  pSocket->rTimer = 0;
	  pSocket->rTimeout = TCP_DEF_rTOUT;
	  pSocket->retries = TCP_DEF_RETRIES;
	  pSocket->cTimer = TCP_DEF_cTOUT;
	  pSocket->state = TCP_SOCKET_ESTABLISHED;
          pSocket->event_handler(iSocket+1,TCP_EVENT_ESTABLISHED,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, 0);
        }
      }
      break;

    case TCP_SOCKET_SYN_SENT:
      if (TCPr->flags & RST) {
        if (SWAP(TCPr->ack) == pSocket->sndnxt) {
	  pSocket->event_handler(iSocket+1,TCP_EVENT_RESET,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, 0);
	  tcp_socket_close(iSocket+1);
	  return;
	}
      }
      if ((TCPr->flags & SYN) && (TCPr->flags & ACK)) {
        if (SWAP(TCPr->ack) == pSocket->sndnxt) {
	  pSocket->remote_mss = get_remote_mss();
	  pSocket->rcvnxt = SWAP(TCPr->seq) + 1;
	  pSocket->snduna = pSocket->sndnxt;
	  pSocket->sndwnd = HTONS(TCPr->window);
	  pSocket->rTimer = 0;
	  pSocket->rTimeout = TCP_DEF_rTOUT;
	  pSocket->retries = TCP_DEF_RETRIES;
	  pSocket->cTimer = TCP_DEF_cTOUT;
	  pSocket->state = TCP_SOCKET_ESTABLISHED;
	  tcp_send(iSocket+1, 0);
	  pSocket->event_handler(iSocket+1,TCP_EVENT_ESTABLISHED,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, 0);
	}
      }
      break;

    case TCP_SOCKET_ESTABLISHED:
      if (TCPr->flags & RST) {
	if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->event_handler(iSocket+1,TCP_EVENT_RESET,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, 0);
  	  tcp_socket_close(iSocket+1);
  	  return;
  	}
      }
      if ((TCPr->flags & SYN) && (TCPr->flags & ACK)) {
        if (SWAP(TCPr->seq) == pSocket->rcvnxt-1) {
	  tcp_send(iSocket+1, 0);	  			
	  return;
	}
      }
      if ((TCPr->flags & ACK) && !test) {			// test Re-Transmissions
        if (SWAP(TCPr->ack) == pSocket->sndnxt) {	
	  pSocket->sndwnd = HTONS(TCPr->window);
	  if (pSocket->snduna != pSocket->sndnxt) {	// we have unacknowledged data...
	    pSocket->snduna = pSocket->sndnxt;
	    pSocket->rTimer=0;
	    pSocket->rTimeout = TCP_DEF_rTOUT;
	    pSocket->retries = TCP_DEF_RETRIES;
	    if (pSocket->more_data == 0) {						
	      pSocket->event_handler(iSocket+1,TCP_EVENT_SEND_COMPLETED,pSocket->remote_ip,
                                     pSocket->remote_port,TCP_DATA_START, 0);
            }else {
              len = get_max_len(iSocket+1);
              pSocket->more_data = pSocket->fill_buffer(iSocket+1, &pSocket->position, &len);
  	      tcp_send(iSocket+1, max_len - len);
  	    }
	  }
        }
      }
      if (data_len > 0) {
	data_to_ack = 1;
	if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->rcvnxt += data_len;
	  pSocket->event_handler(iSocket+1,TCP_EVENT_DATA,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, data_len);
	}
	if (data_to_ack) {
	  tcp_send(iSocket+1, 0);
	}
      }
      pSocket->cTimer = TCP_DEF_cTOUT;	
      if (TCPr->flags & FIN) {
	if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  if (pSocket->snduna == pSocket->sndnxt) {
	    tcp_socket[iSocket].cTimer = 0;
            pSocket->rcvnxt++;
	    pSocket->state = TCP_SOCKET_CLOSE_WAIT;
	    tcp_send(iSocket+1, 0);
	    pSocket->event_handler(iSocket+1,TCP_EVENT_CLOSE_REQ,pSocket->remote_ip,
                                   pSocket->remote_port,TCP_DATA_START, 0);
          }
        }
      }
      break;

    case TCP_SOCKET_CLOSE_WAIT:
      if (TCPr->flags & RST) {
	if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->event_handler(iSocket+1,TCP_EVENT_RESET,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, 0);
  	  tcp_socket_close(iSocket+1);
  	  return;
  	}
      }
      if (TCPr->flags & FIN) {
	if (SWAP(TCPr->seq) == (pSocket->rcvnxt - 1)) {	
	  tcp_send(iSocket+1, 0);
	  return;
	}
      }
      if (TCPr->flags & ACK) {
	if (SWAP(TCPr->ack) == pSocket->sndnxt) {	
	  pSocket->sndwnd = HTONS(TCPr->window);
	  if (pSocket->snduna != pSocket->sndnxt) {	// we have unacknowledged data...
	    pSocket->snduna = pSocket->sndnxt;
	    pSocket->rTimer=0;
	    pSocket->rTimeout = TCP_DEF_rTOUT;
	    pSocket->retries = TCP_DEF_RETRIES;
	    if (pSocket->more_data == 0) {						
	      pSocket->event_handler(iSocket+1,TCP_EVENT_SEND_COMPLETED,pSocket->remote_ip,
                                     pSocket->remote_port,TCP_DATA_START, 0);
            }else {
              len = get_max_len(iSocket+1);
              pSocket->more_data = pSocket->fill_buffer(iSocket+1, &pSocket->position, &len);
  	      tcp_send(iSocket+1, max_len - len);
  	    }
	  }
        }
      }
      break;

    case TCP_SOCKET_LAST_ACK:
      if (TCPr->flags & RST) {
	if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->event_handler(iSocket+1,TCP_EVENT_RESET,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, 0);
  	  tcp_socket_close(iSocket+1);
  	  return;
  	}
      }
      if (TCPr->flags == ACK) {
        if (SWAP(TCPr->seq) == pSocket->rcvnxt) {			
	  pSocket->event_handler(iSocket+1,TCP_EVENT_CONN_CLOSED,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, 0);					  	
        }
      }
      break;

    case TCP_SOCKET_FIN_WAIT_1:
      if (TCPr->flags & RST) {
	if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->event_handler(iSocket+1,TCP_EVENT_RESET,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, 0);
  	  tcp_socket_close(iSocket+1);
  	  return;
  	}
      }
      if (TCPr->flags == ACK) {
        if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->retries = 0;
	  pSocket->rTimer = TCP_DEF_rTOUT * 2;
	  pSocket->state = TCP_SOCKET_FIN_WAIT_2;
	  return;
	}
      }
      if (TCPr->flags & FIN) {
	if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->rcvnxt++;
	  pSocket->state = TCP_SOCKET_CLOSING;
	  tcp_send(iSocket+1, 0);
	}
      }
      break;

    case TCP_SOCKET_FIN_WAIT_2:
      if (TCPr->flags & RST) {
	if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->event_handler(iSocket+1,TCP_EVENT_RESET,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, 0);
  	  tcp_socket_close(iSocket+1);
  	  return;
  	}
      }
      if (TCPr->flags & FIN) {
	if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->rcvnxt++;
	  tcp_send(iSocket+1, 0);
	  pSocket->retries = 0;
	  pSocket->rTimer = TCP_DEF_TIME_WAIT;
	  pSocket->state = TCP_SOCKET_TIME_WAIT;
	  return;
        }
      }
      if (data_len > 0) {
	data_to_ack = 1;
	if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->rcvnxt += data_len;
	  pSocket->event_handler(iSocket+1,TCP_EVENT_DATA,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, data_len);
	}
	if (data_to_ack) {
	  tcp_send(iSocket+1, 0);
	}
      }
      break;

    case TCP_SOCKET_CLOSING:		
      if (TCPr->flags & RST) {
	if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->event_handler(iSocket+1,TCP_EVENT_RESET,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, 0);
  	  tcp_socket_close(iSocket+1);
  	  return;
  	}
      }
      if (TCPr->flags & FIN) {
	if (SWAP(TCPr->seq) == (pSocket->rcvnxt - 1)) {
	  tcp_send(iSocket+1, 0);
	  return;
	}
      }
      if (TCPr->flags == ACK) {
	if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->retries = 0;
	  pSocket->rTimer = TCP_DEF_TIME_WAIT;
	  pSocket->state = TCP_SOCKET_TIME_WAIT;
	}
      }
      break;

    case TCP_SOCKET_TIME_WAIT:
      if (TCPr->flags & RST) {
	if (SWAP(TCPr->seq) == pSocket->rcvnxt) {
	  pSocket->event_handler(iSocket+1,TCP_EVENT_RESET,pSocket->remote_ip,
                                 pSocket->remote_port,TCP_DATA_START, 0);
  	  tcp_socket_close(iSocket+1);
  	  return;
  	}
      }
      if (TCPr->flags == FIN) {
	if (SWAP(TCPr->seq) == (pSocket->rcvnxt - 1)) {
	  tcp_send(iSocket+1, 0);
        }
      }
      break;

    default:
      

	printf("Unknown State %d\n", pSocket->state);
	
      break;
  }
}


/***************************************/
/*   TCP Public Functions              */
/***************************************/
void tcp_init(void) {

  int i;
  struct tcb* pSocket;
  char a[4] = {0,0,0,0};

  for(i=0; i<TCP_MAX_SOCKETS; i++) {
    pSocket = &tcp_socket[i];
    pSocket->state = TCP_SOCKET_FREE;
    pSocket->local_port = 0;
    memcpy(&pSocket->remote_ip[0], &a[0],4);
    pSocket->remote_port = 0;
    pSocket->remote_mss = 0;
    pSocket->flags = 0;
    pSocket->snduna = 0;
    pSocket->sndnxt = 0;
    pSocket->sndwnd = 0;
    pSocket->rcvnxt = 0;
    pSocket->rcvwnd = HTONS(TCP_MSS);
    pSocket->rTimer = 0;
    pSocket->rTimeout = 0;
    pSocket->retries = 0;
    pSocket->cTimer = 0;
    pSocket->event_handler = 0;
    pSocket->more_data = 0;
    pSocket->position = 0;
    pSocket->fill_buffer = 0;
  }
  timer_start(&tcptimer, CLOCK_SECOND/10);
  return;
}


void tcp_poll(void) {
  int i;

  if (timer_expired(&tcptimer)) {		// 100 msec has elapsed
    for(i=0; i<TCP_MAX_SOCKETS; i++) {
      if (tcp_socket[i].rTimer > 1)
	tcp_socket[i].rTimer--;
      if (tcp_socket[i].cTimer > 1 && tcp_socket[i].state == TCP_SOCKET_ESTABLISHED)
	tcp_socket[i].cTimer--;
    }
    timer_start(&tcptimer, CLOCK_SECOND/10);
  }else
    return;
  for(i=0; i<TCP_MAX_SOCKETS; i++) {
    if (tcp_socket[i].rTimer == 1) {       //re-transmission socket timer has expired
      if (tcp_socket[i].retries == 0) {
	if (tcp_socket[i].state == TCP_SOCKET_TIME_WAIT) {
	  tcp_socket[i].event_handler(i+1,TCP_EVENT_CONN_CLOSED,tcp_socket[i].remote_ip,
                                      tcp_socket[i].remote_port,TCP_DATA_START, 0);
	}else {
	  tcp_socket[i].event_handler(i+1,TCP_EVENT_rTOUT,tcp_socket[i].remote_ip,
                                      tcp_socket[i].remote_port,TCP_DATA_START, 0);
	}
        tcp_socket_close(i+1);
      }else {
	tcp_socket[i].rTimeout = tcp_socket[i].rTimeout * 2;    // BackOff Algorithm
	tcp_socket[i].retries--;
	tcp_resend(i+1);
      }
    }
    if (tcp_socket[i].cTimer == 1 && tcp_socket[i].state == TCP_SOCKET_ESTABLISHED) {
      tcp_socket[i].event_handler(i+1,TCP_EVENT_cTOUT,tcp_socket[i].remote_ip,
                                  tcp_socket[i].remote_port,TCP_DATA_START, 0);

      tcp_send_reset(i+1,tcp_socket[i].remote_ip,tcp_socket[i].remote_port);
      tcp_socket_close(i+1);
    }
  }
}


void tcp_process(unsigned short len) {

  struct pseudo_hdr p_hdr;
  unsigned short chksum1;
  int chksum2;		
  int iSocket, j;
  struct tcb* pSocket;

  opt_len = (((TCPr->data_offset)>>4)*4) - sizeof(struct tcp_hdr);
  data_len = HTONS(IPr->len) - sizeof(struct ip_hdr)  - sizeof(struct tcp_hdr) - opt_len;


  if(debug_tcp)
  tcp_display();
 //debug_tcp

  chksum1 = HTONS(TCPr->tcpchksum);
  memcpy(&p_hdr.srcipaddr[0], &IPr->srcipaddr[0], 4);
  memcpy(&p_hdr.destipaddr[0], &IPr->destipaddr[0], 4);
  p_hdr.zero = 0;
  p_hdr.protocol = TCP_PROTOCOL;	
  p_hdr.len = HTONS(len);

  chksum2 = chksum16(&p_hdr.srcipaddr[0], sizeof(p_hdr), 0, 0);
  TCPr->tcpchksum = 0;
  chksum2 = chksum16(&TCPr->srcport,len, chksum2, 1);
  if (chksum2 != chksum1) {


	printf("TCP: Bad Checksum %04x (it should be %04x)\n",chksum1, chksum2);
    return;											// returns if chksum failed!
  }
  TCPr->tcpchksum = HTONS(chksum1);			        // restore checksum

  // look up the socket for the recipient port
  // and process the incomming segment
  for(iSocket=0; iSocket<TCP_MAX_SOCKETS; iSocket++) {
    pSocket = &tcp_socket[iSocket];
    if (pSocket->local_port == HTONS(TCPr->destport) &&
	            pSocket->state != TCP_SOCKET_FREE) {	// avoid a segment with port=0

      if (TCPr->flags == SYN) {			// if Connection Request (only SYN)
	for(j=0;j<TCP_MAX_SOCKETS;j++) {        // check if this is a retransmission of a prior SYN
          pSocket = &tcp_socket[j];
	  if (pSocket->state == TCP_SOCKET_SYN_RECEIVED &&
	      pSocket->remote_port == HTONS(TCPr->srcport) &&
	      pSocket->rcvnxt == (SWAP(TCPr->seq) + 1) &&
	      pSocket->remote_ip[0] == IPr->srcipaddr[0] &&
	      pSocket->remote_ip[1] == IPr->srcipaddr[1] &&
	      pSocket->remote_ip[2] == IPr->srcipaddr[2] &&
	      pSocket->remote_ip[3] == IPr->srcipaddr[3]) {
	  		
	      // it is a re-transmission of a previous SYN
	      // we don't have to create another socket
	      // the timeout of our SYN+ACK will re-send the segment
	    return;
	  }		  		
	}
	pSocket = &tcp_socket[iSocket];	 // if continue, get the reference again
	if (pSocket->state == TCP_SOCKET_LISTEN) {
	  pSocket->event_handler(iSocket+1,TCP_EVENT_CONN_REQ,IPr->srcipaddr,
                                 HTONS(TCPr->srcport),TCP_DATA_START, 0);

  	  return;
        }
      }else {												   // any other segment (SYN+ACK,DATA,FIN,ACK)
	if (pSocket->remote_port == HTONS(TCPr->srcport) &&
	    pSocket->remote_ip[0] == IPr->srcipaddr[0] &&
	    pSocket->remote_ip[1] == IPr->srcipaddr[1] &&
	    pSocket->remote_ip[2] == IPr->srcipaddr[2] &&
	    pSocket->remote_ip[3] == IPr->srcipaddr[3]) {

          tcp_process_segment(iSocket + 1);
	  return;
	}
      }
    }
  }
  // If we are here, the Port is unreachable...Send an ICMP informing that !
  

	printf("Destination Port %d not found!\n", HTONS(TCPr->destport));
	
  j = ICMP_DATA_START;
  // Copy original IP header + first 8 bytes of IP Data
  memcpy(&tx_buf[j+4], &rx_buf[DATALINK_HDR_SIZE], sizeof(struct ip_hdr)+8);			
  ICMPt->type = ICMP_Destination_Unreachable;
  ICMPt->code = ICMP_Port_Unreachable;			 				
  tx_buf[j]   = 0;		  // Unused (reserved)
  tx_buf[j+1] = 0;		  // Unused (reserved)
  tx_buf[j+2] = 0;		  // Unused (reserved)
  tx_buf[j+3] = 0;                // Unused (reserved)          										
  icmp_send(IPr->srcipaddr, sizeof(struct icmp_hdr)+4+sizeof(struct ip_hdr)+8);
}


int tcp_socket_open(unsigned short port,
                    void (*event_handler)(int,char,char*,unsigned short,unsigned short,unsigned short)) {
  int i;
  struct tcb* pSocket;

  if (port == 0) {
    while(check_free_port(last_port++) != 1);
    if (last_port >= 49999)
      last_port = 1024;
    port = last_port++;
  }else {
    if (check_free_port(port) != 1)
      return TCP_PORT_ALREADY_USED;
  }
  for(i=0; i<TCP_MAX_SOCKETS; i++) {
    pSocket = &tcp_socket[i];
    if(pSocket->state != TCP_SOCKET_FREE)
      continue;
    pSocket->state = TCP_SOCKET_CLOSED;
    pSocket->local_port = port;
    pSocket->rTimeout = TCP_DEF_rTOUT;
    pSocket->retries = TCP_DEF_RETRIES;
    pSocket->event_handler = event_handler;
    return (i+1);
  }
  return TCP_INVALID_SOCKET;
}


void tcp_socket_close(int socket) {
  struct tcb* pSocket;
  int iSocket = socket-1;
  char a[4] = {0,0,0,0};

  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return;
  pSocket = &tcp_socket[iSocket];
  pSocket->state = TCP_SOCKET_FREE;
  pSocket->local_port = 0;
  memcpy(pSocket->remote_ip, &a[0], 4);
  pSocket->remote_port = 0;
  pSocket->remote_mss = 0;
  pSocket->flags = 0;
  pSocket->snduna = 0;
  pSocket->sndnxt = 0;
  pSocket->sndwnd = 0;
  pSocket->rcvnxt = 0;
  pSocket->rTimer = 0;
  pSocket->rTimeout = 0;
  pSocket->retries = 0;
  pSocket->cTimer = 0;
  pSocket->event_handler = 0;
  pSocket->more_data = 0;
  pSocket->position = 0;
  pSocket->fill_buffer = 0;
}



void tcp_sockets_show(void) {
  int i;
  char state[10];
  struct tcb* pSocket;

  printf("\r\nSocket  State    Loc_Port     Rem_IP        Rem_Port  cTimer\r\n");
  printf("------------------------------------------------------------\r\n");
  for(i=0; i<TCP_MAX_SOCKETS; i++) {
    pSocket = &tcp_socket[i];
    switch(pSocket->state) {
      case 0: sprintf(state, "%s", "Free      "); break;
      case 1: sprintf(state, "%s", "Closed    "); break;
      case 2: sprintf(state, "%s", "Listen    "); break;
      case 3: sprintf(state, "%s", "Syn_Sent  "); break;
      case 4: sprintf(state, "%s", "Syn_Recvd "); break;
      case 5: sprintf(state, "%s", "Establish."); break;
      case 6: sprintf(state, "%s", "Fin_Wait_1"); break;
      case 7: sprintf(state, "%s", "Fin_Wait_2"); break;
      case 8: sprintf(state, "%s", "Closing   "); break;
      case 9: sprintf(state, "%s", "Time_Wait "); break;
      case 10: sprintf(state, "%s", "Close_Wait"); break;
      case 11: sprintf(state, "%s", "Last_Ack  "); break;
	  default: sprintf(state, "%s", "Unknown!  "); break;
    }
    if (pSocket->state < 2)
      printf(" %2d     %s\r\n", i+1, state);													
    else if (pSocket->state == 2)
      printf(" %2d     %s   %4d\r\n", i+1, state, pSocket->local_port);													
    else
      printf(" %2d     %s   %4d    %d.%d.%d.%d\t%4d   %d\r\n", i+1, state, pSocket->local_port,
                                             pSocket->remote_ip[0], pSocket->remote_ip[1],
	        			     pSocket->remote_ip[2], pSocket->remote_ip[3],
    		          		     pSocket->remote_port, pSocket->cTimer);
  }
  printf("------------------------------------------------------------\r\n");
}

void tcp_socket_show(unsigned char index) {
//  int i;
  char state[10];
  struct tcb* pSocket;

  printf("\r\nSocket  State    Loc_Port     Rem_IP        Rem_Port  cTimer\r\n");
  printf("------------------------------------------------------------\r\n");
  if(index <TCP_MAX_SOCKETS){
    pSocket = &tcp_socket[index];
    switch(pSocket->state) {
      case 0: sprintf(state, "%s", "Free      "); break;
      case 1: sprintf(state, "%s", "Closed    "); break;
      case 2: sprintf(state, "%s", "Listen    "); break;
      case 3: sprintf(state, "%s", "Syn_Sent  "); break;
      case 4: sprintf(state, "%s", "Syn_Recvd "); break;
      case 5: sprintf(state, "%s", "Establish."); break;
      case 6: sprintf(state, "%s", "Fin_Wait_1"); break;
      case 7: sprintf(state, "%s", "Fin_Wait_2"); break;
      case 8: sprintf(state, "%s", "Closing   "); break;
      case 9: sprintf(state, "%s", "Time_Wait "); break;
      case 10: sprintf(state, "%s", "Close_Wait"); break;
      case 11: sprintf(state, "%s", "Last_Ack  "); break;
	  default: sprintf(state, "%s", "Unknown!  "); break;
    }
    if (pSocket->state < 2)
      printf(" %2d     %s\r\n", index+1, state);													
    else if (pSocket->state == 2)
      printf(" %2d     %s   %4d\r\n", index+1, state, pSocket->local_port);													
    else
      printf(" %2d     %s   %4d    %d.%d.%d.%d\t%4d   %d\r\n", index+1, state, pSocket->local_port,
                                             pSocket->remote_ip[0], pSocket->remote_ip[1],
	        			     pSocket->remote_ip[2], pSocket->remote_ip[3],
    		          		     pSocket->remote_port, pSocket->cTimer);
  }
  printf("------------------------------------------------------------\r\n");
}

void tcp_conn_open(int socket, char *destipaddr,unsigned short destport) {
  struct tcb* pSocket;
  int iSocket = socket-1;

  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return;
  pSocket = &tcp_socket[iSocket];
  memcpy(pSocket->remote_ip, destipaddr, 4);
  pSocket->remote_port = destport;
  pSocket->snduna = get_ISN();		
  pSocket->sndnxt = pSocket->snduna;
  pSocket->rcvnxt = 0;
  pSocket->state = TCP_SOCKET_SYN_SENT;
  tcp_send(socket, 0);
}


int tcp_conn_accept(int socket) {
  int i;
  struct tcb* pSocket;
  int iSocket = socket-1;

  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return TCP_SOCKET_ERROR;
  for(i=0;i<TCP_MAX_SOCKETS;i++) {
    pSocket = &tcp_socket[i];
    if (pSocket->state != TCP_SOCKET_FREE)
      continue;
    pSocket->local_port = tcp_socket[iSocket].local_port;
    memcpy(pSocket->remote_ip, IPr->srcipaddr, 4);
    pSocket->remote_port = HTONS(TCPr->srcport);
    pSocket->remote_mss = get_remote_mss();
    pSocket->event_handler = tcp_socket[iSocket].event_handler;
    pSocket->snduna = get_ISN();	
    pSocket->sndnxt = pSocket->snduna;
    pSocket->sndwnd = HTONS(TCPr->window);
    pSocket->rcvnxt = SWAP(TCPr->seq) + 1;
    pSocket->rTimeout = TCP_DEF_rTOUT;
    pSocket->retries = TCP_DEF_RETRIES;
    pSocket->cTimer = 0;
    pSocket->state = TCP_SOCKET_SYN_RECEIVED;
    tcp_send(i+1, 0); 	
    return (i+1);
  }
  return TCP_SOCKET_ERROR;		// there isn't a free socket to assign to this new connection
}


void tcp_conn_reject(int socket) {

  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return;
  tcp_send_reset(socket,IPr->srcipaddr,HTONS(TCPr->srcport));
}


void tcp_conn_close(int socket) {
  int iSocket = socket-1;

  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return;
  tcp_socket[iSocket].cTimer = 0;
  if (tcp_socket[iSocket].state == TCP_SOCKET_ESTABLISHED)
    tcp_socket[iSocket].state = TCP_SOCKET_FIN_WAIT_1;
  else {
    tcp_socket[iSocket].state = TCP_SOCKET_LAST_ACK;
    tcp_socket[iSocket].flags = FIN;
  }
  tcp_send(iSocket+1, 0);
}


short tcp_get_port(int socket) {
  int iSocket = socket-1;

  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return TCP_SOCKET_ERROR;
  return tcp_socket[iSocket].local_port;
}


int tcp_send_data(int socket, int (*fill_buffer)(int socket,
                                   unsigned short *position,unsigned short *len)) {
  struct tcb* pSocket;
  int iSocket = socket-1;

  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return TCP_INVALID_SOCKET;
  pSocket = &tcp_socket[iSocket];
  if (pSocket->more_data > 0)		// Is there data to send from previous request?
    return TCP_SOCKET_ERROR;
  // Setup the socket
  pSocket->more_data = 0;
  pSocket->position = 0;		// start filling the buffer from the beginning
  pSocket->fill_buffer = fill_buffer;   // save the App´s function to call for more data to fill...
  // Now call the function
  len = get_max_len(socket);
  pSocket->more_data = pSocket->fill_buffer(socket, &pSocket->position, &len);
  printf("send more data = %d\n",pSocket->more_data);
  tcp_send(socket, max_len - len);
  return 0;
}

int tcp_send_data_once(int socket, int (*fill_buffer)(int socket,
                                   unsigned short *position,unsigned short *len)) {
  struct tcb* pSocket;
  timer_typedef tim ;
  int iSocket = socket-1;
  int delay ;
  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return TCP_INVALID_SOCKET;
  pSocket = &tcp_socket[iSocket];
  if (pSocket->more_data > 0)		// Is there data to send from previous request?
    return TCP_SOCKET_ERROR;
  // Setup the socket
  pSocket->more_data = 0;
  pSocket->position = 0;		// start filling the buffer from the beginning
  pSocket->fill_buffer = fill_buffer;   // save the App´s function to call for more data to fill...
  // Now call the function
  do{
  	  timer_set(&tim, CLOCK_SECOND/50);
	  len = get_max_len(socket);
	  pSocket->more_data = pSocket->fill_buffer(socket, &pSocket->position, &len);
	  printf("send more data = %d\n",pSocket->more_data);
	  tcp_send(socket, max_len - len);
	  while(!timer_expired(&tim));
 	}while(pSocket->more_data);
 
  return 0;
}

int tcp_has_more_data(int socket)
{
	if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    	return 0 ;
	return tcp_socket[socket].more_data ;
}
int tcp_listen(int socket) {
  int iSocket = socket-1;

  if (tcp_validate_socket(socket) == TCP_INVALID_SOCKET)
    return TCP_SOCKET_ERROR;
  if (tcp_socket[iSocket].state != TCP_SOCKET_CLOSED &&
      tcp_socket[iSocket].state != TCP_SOCKET_LISTEN) {

    return -1;		// Error
  }
  tcp_socket[iSocket].state = TCP_SOCKET_LISTEN;
  return 0;
}


int	 my_fill_buffer (int socketId,unsigned short * ppostion,unsigned short * pmaxlen)
{
	if(test_data_len > *pmaxlen)
	{
		test_data_len = *pmaxlen ;
	}

	*pmaxlen -= test_data_len ;
	if(test_data_len)
		memcpy(&tx_buf[TCP_DATA_START],test_data,test_data_len);

	return 0 ;
}
void my_tcp_event_handler(int socketId,char eventId,char* remoteIp,unsigned short port,unsigned short datastart,unsigned short datalen)
{
	short actLen = datalen ;
	/*
	TCP_EVENT_CONN_REQ		  
	TCP_EVENT_ESTABLISHED	
	TCP_EVENT_DATA	    	
	TCP_EVENT_SEND_COMPLETED
	TCP_EVENT_CONN_CLOSED	
	TCP_EVENT_CLOSE_REQ		
	TCP_EVENT_RESET			
	TCP_EVENT_rTOUT 		
	TCP_EVENT_cTOUT 		
	*/
	if(TCP_EVENT_CONN_REQ == eventId)
	{
		socketId = tcp_conn_accept(socketId);
		//tcp_send_data(socketId,my_fill_buffer);
	}
	else if(TCP_EVENT_ESTABLISHED ==  eventId)
	{
		tcp_send_data(socketId, my_fill_buffer);
	}
	else if(TCP_EVENT_CLOSE_REQ == eventId)
	{
		tcp_conn_close(socketId);
	}
	else if(TCP_EVENT_CONN_CLOSED == eventId)
	{
		tcp_socket_close(socketId);
	}

	if(TCP_EVENT_DATA != eventId )
		return ;
	
	if(actLen > TEST_DATA_LEN)
	{
		actLen = TEST_DATA_LEN ;
	}
	memcpy(test_data,&rx_buf[datastart],actLen);
	test_data_len = actLen ;
	tcp_socket[socketId - 1].more_data = 0 ;
	
	tcp_send_data(socketId, my_fill_buffer);


}
void my_tcp_service_init(void)
{
	int socketId = tcp_socket_open(1066,my_tcp_event_handler );
	tcp_listen(socketId);
}


