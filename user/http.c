/******************************************************************************************
   http.c (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#include "http.h"
#include "tcp.h"
#include "stack.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


struct {
  char 		 filename[13];
  const char	 *contentpointer;
  unsigned short contentsize;
  const char	 *filepointer;
  unsigned short filesize;
} filesystem[NUMBER_OF_FILES];


#define debug_http 1
int file_index[TCP_MAX_SOCKETS];
int  (*POST_CallBack)(unsigned short data_index, unsigned short data_len);
int  (*TagExpand_HTTP) (char tagID, unsigned short pos, unsigned short len); 	

const char text[] = "Content-Type: text/plain\r\n";
const char html[] = "Content-Type: text/html\r\n";
const char gif[]  = "Content-Type: image/gif\r\nCache-Control: max-age 10000\r\n";
const char jpeg[] = "Content-Type: image/jpeg\r\nCache-Control: max-age 10000\r\n";
const char app[] =  "Content-Type: application/octet-stream\r\n";

const char page404[] = "HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
                       "<HTML><HEAD><TITLE>File Not Found</TITLE></HEAD>"
					   "<BODY><H4>The requested page was not found!</H4></BODY>";

const char page501[] = "HTTP/1.0 501 Not Implemented\r\nContent-Type: text/html\r\n\r\n"
                       "<HTML><HEAD><TITLE>Method Not Implemented</TITLE></HEAD>"
					   "<BODY><H4>The requested method was not implemented!</H4></BODY>";

const char status200[] = "HTTP/1.0 200 OK\r\n";



const char empty_line[] = "\r\n";

//static char debug_http = 0 ;
static char debug_http_file = 0 ;
void filesystem_add(int i, const char *filename, char type,
                            const char *file, int filesize) {

  if (i >= NUMBER_OF_FILES) {
	  printf("FileSystem Error: Increment the NUMBER_OF_FILES parameter\r\n");
    return;
  }
  memcpy(filesystem[i].filename,filename,13);
  switch (type) {
    case HTML:
      filesystem[i].contentpointer = html;	
      filesystem[i].contentsize = sizeof(html)-1;
      break;
    case TEXT:
      filesystem[i].contentpointer = text;	
      filesystem[i].contentsize = sizeof(text)-1;
      break;
    case GIF:
      filesystem[i].contentpointer = gif;	
      filesystem[i].contentsize = sizeof(gif)-1;
      break;
    case JPEG:
      filesystem[i].contentpointer = jpeg;	
      filesystem[i].contentsize = sizeof(jpeg)-1;
      break;
    default:
      filesystem[i].contentpointer = app;	
      filesystem[i].contentsize = sizeof(app)-1;
  }
  filesystem[i].filepointer = file;
  filesystem[i].filesize = filesize;
}


void getFileName(char *p, unsigned short start) {
  int i;

  if (rx_buf[start] == 0x20) {		        // if GET a space (GET /  )
    strcpy(p, "index.htm");			// return index.htm (default page)
    return;
  }
  for(i=0; i<13; i++) {
    if (rx_buf[start+i] == 0x20) {
      *p = '\0';
      break;
    }else {
      *p++ = tolower(rx_buf[start+i]);
    }
  }
}


int parse_variable(char str[], char var[], unsigned short len, char value[]) {
  int i=0;
  int j=0;

  while(i < len) {
    if (var[j]=='\0')
      break;
    if (str[i] == '=') {
      while(str[i]!='&' && i<len) {
	i++;
      }
      j=0;
    }
    if (str[i] == var[j]) {
      j++;
    }else {
      j=0;
    }
    i++;
  }
  if (j==0) {
    return 0;
  }else {
    i++;
    j=0;
    while(str[i] != '&' && i<len) {
      if (str[i] == '+')
        value[j] = ' ';
      else if (str[i]=='%') {
        value[j]  = str[i+1]<='9' ? (str[i+1]-'0')*16 : (tolower(str[i+1])-'a'+10)*16;
	value[j] += str[i+2]<='9' ? (str[i+2]-'0') : (tolower(str[i+2])-'a'+10);
	i +=2;
      }
      else
        value[j] = str[i];
      i++;
      j++;
    }
    value[j] = '\0';
    return 1;
  }
}


int HTTP_process(unsigned short data_index, unsigned short data_len) {
  int i;
  char filename[13];

  if (!memcmp(&rx_buf[data_index],"GET",3)) {
    getFileName(filename, data_index+5);
    for(i=0; i<NUMBER_OF_FILES; i++) {
      if (!strcmp(filesystem[i].filename, filename))
			return i;
    }
	printf("file not found %s\n",filename);
    return -1; 	      // page Not Found (404)
  }else if (!memcmp(&rx_buf[data_index],"POST",4)) {
    if (POST_CallBack(data_index, data_len))
      getFileName(filename, data_index+6);
    else
      strcpy(filename, "index.htm");
    for(i=0; i<NUMBER_OF_FILES; i++) {
      if (!strcmp(filesystem[i].filename, filename))
	return i;
    }
    return -1; 	      // page Not Found (404)
  }else {
    return -2;	      // command Not Implemented (501)
  }
}


int create_content(int soc_handle, unsigned short *position, unsigned short *len) {
  char tagID;
  int resp;
  int j=TCP_DATA_START;
  unsigned short pos;
  printf("create content pos : %d, len : %d\n",*position,*len);
  pos = *position;
  if (file_index[soc_handle-1] == -2) {
    strcpy(&tx_buf[j], page501);		
    (*position) += (sizeof(page501)-1);
    (*len) -= (sizeof(page501)-1);
    return 0;
  }
  if (file_index[soc_handle-1] == -1) {		
    strcpy(&tx_buf[j], page404);
    (*position) += (sizeof(page404)-1);
    (*len) -= (sizeof(page404)-1);
    return 0;
  }

  // fill the status line
  while(pos < sizeof(status200)-1 && *len) {
    tx_buf[j++] = status200[pos];
    pos++;
    (*position)++;
    (*len)--;
  }
  if (*len==0)
    return 1;
  pos -= (sizeof(status200)-1);

  //fill the Content-Type line
  while(pos < filesystem[file_index[soc_handle-1]].contentsize && *len) {
    tx_buf[j++] = *(filesystem[file_index[soc_handle-1]].contentpointer + pos);
    pos++;
    (*position)++;
    (*len)--;
  }
  if (*len==0)
    return 1;
  pos -= (filesystem[file_index[soc_handle-1]].contentsize);

  // fill the empty-line
  while(pos < sizeof(empty_line)-1 && *len) {
    tx_buf[j++] = empty_line[pos];
    pos++;
    (*position)++;
    (*len)--;
  }
  if (*len==0)
    return 1;
  pos -= (sizeof(empty_line)-1);

  //fill the body of the requested page
  while((pos < filesystem[file_index[soc_handle-1]].filesize) && *len) {
    tx_buf[j] = *(filesystem[file_index[soc_handle-1]].filepointer + pos);
	
    if (tx_buf[j] == '&' && (filesystem[file_index[soc_handle-1]].contentpointer == html ||	
        filesystem[file_index[soc_handle-1]].contentpointer == text) &&  TagExpand_HTTP) {	
	
      tagID = ((*(filesystem[file_index[soc_handle-1]].filepointer + pos+1)-0x30)*10) +
	       (*(filesystem[file_index[soc_handle-1]].filepointer + pos+2)-0x30);
	
      resp = TagExpand_HTTP(tagID, j, *len);
      if (resp == -2) {
	  printf("Error: Tag &%02d not found\r\n", tagID);

      }else if (resp == -1) {
	return *len;		 // the available space is not enough to expand the tag...
      }else {
	j += resp;
	pos += 3;
	(*position) += 3;	
	(*len) -= resp;
	continue;
      }
    }
    j++;
    pos++;
    (*position)++;
    (*len)--;
  }

  if (pos < filesystem[file_index[soc_handle-1]].filesize)
    return 1;
  else
    return 0;	
}


static void http_control_process(int soc_handler,char event,char *srcipaddr,unsigned short srcport,
                                     unsigned short data_index, unsigned short data_len) {
  int resp;

  switch(event) {
    case TCP_EVENT_CONN_REQ:
      if (debug_http)
        printf("Connection Request from IP: %d.%d.%d.%d - Port: %d\n",
	       *srcipaddr,*(srcipaddr+1),*(srcipaddr+2),*(srcipaddr+3),srcport);
      resp = tcp_conn_accept(soc_handler);
      if (resp == TCP_SOCKET_ERROR)
	  	printf("Accept Error: no free socket available\n");

      else 
	  {		
	   
        if (debug_http)
	  		printf("Socket %d created\n", resp);
	  
      }
      break;
    case TCP_EVENT_ESTABLISHED:
	
      if (debug_http)
	  	
        printf("Connection Established with IP: %d.%d.%d.%d - Port: %d\n",
	       *srcipaddr,*(srcipaddr+1),*(srcipaddr+2),*(srcipaddr+3),srcport);
		
      break;
    case TCP_EVENT_DATA:
      if (debug_http)
        printf("Event: Data Available from IP: %d.%d.%d.%d - Port: %d\n",
	       *srcipaddr,*(srcipaddr+1),*(srcipaddr+2),*(srcipaddr+3),srcport);
      resp = HTTP_process(data_index, data_len);	
      file_index[soc_handler-1] = resp;		
      if (debug_http_file) {
	if (resp == -2)
	  printf("Serving: 501 Command Not Implemented!\n");
    else if (resp == -1)
	  printf("Serving: 404 Page Not Found!\n");	
	else 
	  printf("Serving file %s\n", filesystem[resp].filename);
	  printf("Serving size %d\r\n", filesystem[resp].filesize);

	  
      }
      	if (tcp_send_data(soc_handler, create_content) == TCP_SOCKET_ERROR)
	 		 	printf("Error sending data: TCP already have data to send!\r\n");

      break;
    case TCP_EVENT_SEND_COMPLETED:
      tcp_conn_close(soc_handler);		
      break;
    case TCP_EVENT_CONN_CLOSED:
      tcp_socket_close(soc_handler);
      if (debug_http)
	  printf("Socket %d closed\n", soc_handler);

      break;
    case TCP_EVENT_RESET:
      if (debug_http)     
	  printf("Error: Socket %d was Reset\r\n", soc_handler);
		tcp_conn_close(soc_handler);
      break;
    case TCP_EVENT_rTOUT:
		tcp_conn_close(soc_handler);
	printf("Error: Socket %d Timed Out\r\n", soc_handler);
      break;
    case TCP_EVENT_cTOUT:
	  printf("Error: Connection in socket %d Timed Out\r\n", soc_handler);

      break;
    case TCP_EVENT_CLOSE_REQ:
		tcp_conn_close(soc_handler);
      break;  	  	    	
    default:
	  printf("Unknown Event: %d\n",event);
      break;
  }
}


void http_server_init(unsigned short port, int (*POST_handler)(unsigned short data_index,
                                                unsigned short data_len),
                                           int (*TagExpand_handler) (char tagID,
	        		                unsigned short pos,unsigned short len) ) {
  int socket;

  POST_CallBack = POST_handler;
  TagExpand_HTTP = TagExpand_handler;
  socket = tcp_socket_open(port, http_control_process);
  if (socket > 0) {
    
	  printf("TCP Socket %d created, port %d\n", socket, tcp_get_port(socket));
	  
  }else {
    if (socket == TCP_INVALID_SOCKET) {
      
	  printf("Error opening a socket: increment TCP_MAX_SOCKETS=%d\n", TCP_MAX_SOCKETS);
	  
    }
    if (socket == TCP_PORT_ALREADY_USED) {
      
	  printf("Error opening a socket: Port %d already used\n", port);
	  
    }
    return;		// Error
  }
  if (tcp_listen(socket) < 0) {
    
	  printf("Error: socket %d can not put in LISTEN state\n", socket);
	  
    return;		// Error	
  }else {
    
	  printf("TCP Socket %d listening on Port %d\n", socket, tcp_get_port(socket));
	  
  }
}


