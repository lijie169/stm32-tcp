/******************************************************************************************
   web_server.c (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#include "web_server.h"
#include "stack.h"
#include "tagexpand.h"
#include "HTML_pages.h"
#include "graphics.h"
#include "http.h"
#include "app.h"
//#include "Led.h"
//#include <iolpc2129.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
extern char HOUR  ;
extern char  MIN  ;
extern char SEC ;

extern char led_enable ;
extern char  DOW   ;
extern char MONTH  ;
extern short  YEAR;
extern short DOM    ;


char autorefresh=0;
char refreshtime=10;

#define debug_web_processpost	1

void filesystem_init(void) {
  int i=0;

#ifndef LIMIT_32K
  filesystem_add(i++, "arrows.gif", GIF, arrows, sizeof(arrows));
  filesystem_add(i++, "black.gif", GIF, black, sizeof(black));
  filesystem_add(i++, "clear.gif", GIF, clear, sizeof(clear));
  filesystem_add(i++, "fire.gif", GIF, fire, sizeof(fire));
  filesystem_add(i++, "iar.gif", GIF, iar, sizeof(iar));
  filesystem_add(i++, "level.gif", GIF, level, sizeof(level));
  filesystem_add(i++, "logo.gif", GIF, logo, sizeof(logo));
  filesystem_add(i++, "mercury.gif", GIF, mercury, sizeof(mercury));
  filesystem_add(i++, "pearson.gif", GIF, pearson, sizeof(pearson));
  filesystem_add(i++, "philips.jpg", JPEG, philips, sizeof(philips));
  filesystem_add(i++, "psl_logo.gif", GIF, psl_logo, sizeof(psl_logo));
  filesystem_add(i++, "pumpoff.gif", GIF, pumpoff, sizeof(pumpoff));
  filesystem_add(i++, "pumpon.gif", GIF, pumpon, sizeof(pumpon));
  filesystem_add(i++, "scale.gif", GIF, scale, sizeof(scale));
  filesystem_add(i++, "sergio.jpg", JPEG, sergio, sizeof(sergio));
  filesystem_add(i++, "valve.jpg", JPEG, valve, sizeof(valve));
  filesystem_add(i++, "about.htm", HTML, about, sizeof(about)-1);
  filesystem_add(i++, "applic.htm", HTML, applic, sizeof(applic)-1);
  filesystem_add(i++, "index.htm", HTML, index, sizeof(index)-1);
  filesystem_add(i++, "mail.htm", HTML, mail, sizeof(mail)-1);
  filesystem_add(i++, "sent.htm", HTML, sent, sizeof(sent)-1);
  filesystem_add(i++, "setup.htm", HTML, setup, sizeof(setup)-1);
  filesystem_add(i++, "setupok.htm", HTML, setupok, sizeof(setupok)-1);
#else
  filesystem_add(i++, "arrows.gif", GIF, (char *) 0x00010000, 1431);
  filesystem_add(i++, "black.gif", GIF, (char *) 0x00010600, 35);
  filesystem_add(i++, "clear.gif", GIF, (char *) 0x00010630, 45);
  filesystem_add(i++, "fire.gif", GIF, (char *) 0x00010660, 1121);
  filesystem_add(i++, "iar.gif", GIF, (char *) 0x00010AD0, 3199);
  filesystem_add(i++, "level.gif", GIF, (char *) 0x00011760, 77);
  filesystem_add(i++, "logo.gif", GIF, (char *) 0x000117B0, 1883);
  filesystem_add(i++, "mercury.gif", GIF, (char *) 0x00012430, 85);
  filesystem_add(i++, "pearson.gif", GIF, (char *) 0x00012490, 2402);
  filesystem_add(i++, "philips.jpg", JPEG, (char *) 0x00012E00, 2562);
  filesystem_add(i++, "psl_logo.gif", GIF, (char *) 0x00013810, 2778);
  filesystem_add(i++, "pumpoff.gif", GIF, (char *) 0x000142F0, 935);
  filesystem_add(i++, "pumpon.gif", GIF, (char *) 0x000146A0, 376);
  filesystem_add(i++, "scale.gif", GIF, (char *) 0x00014820, 324);
  filesystem_add(i++, "sergio.jpg", JPEG, (char *) 0x00014970, 4923);
  filesystem_add(i++, "valve.jpg", JPEG, (char *) 0x00015CB0, 1214);
  filesystem_add(i++, "about.htm", HTML, (char *) 0x00016170, 2955);
  filesystem_add(i++, "applic.htm", HTML, (char *) 0x00016D00, 5466);
  filesystem_add(i++, "index.htm", HTML, (char *) 0x00018260, 869);
  filesystem_add(i++, "mail.htm", HTML, (char *) 0x000185D0, 1407);
  filesystem_add(i++, "sent.htm", HTML, (char *) 0x00018B50, 900);
  filesystem_add(i++, "setup.htm", HTML, (char *) 0x00018EE0, 2021);
  filesystem_add(i++, "setupok.htm", HTML, (char *) 0x000196D0, 1016);
#endif

}



int process_POST(unsigned short data_index, unsigned short data_len) {
  int i=0;
  int j;

  char value[200];
  char to[50], subject[50], message[200];

  #if debug_web_processpost
    printf("POST: DataIndex %d - Data Len %d\r\n", data_index, data_len);
  #endif

  while(i < data_len) {
    if (rx_buf[data_index+i]=='\r') {
      if (rx_buf[data_index+i+1]=='\n' &&
	  rx_buf[data_index+i+2]=='\r' &&
	  rx_buf[data_index+i+3]=='\n') {

        i += 4;
	break;
      }else {
	i += 4;
      }
    }else {
      i++;
    }
  }

#if debug_web_processpost
  j = i;
  printf("POST data: ");
  while(j < data_len) {								
    printf("%c", rx_buf[data_index+j]);
    j++;
  }
  printf("\r\n");
#endif

  if (parse_variable(&rx_buf[data_index+i], "mail", data_len-i, value)) {
    if (!memcmp(value, "Send", 4)) {
      if (parse_variable(&rx_buf[data_index+i], "to", data_len-i, value)) {
	strcpy(to, value);
      }
      if (parse_variable(&rx_buf[data_index+i], "subject", data_len-i, value)) {
	strcpy(subject, value);
      }
      if (parse_variable(&rx_buf[data_index+i], "message", data_len-i, value)) {
        memcpy(message,value,200);
      }
     // smtp_sendmail(to, subject, message,0,0);
      return 1;
    }
  }
#if 1
  if (parse_variable(&rx_buf[data_index+i], "options", data_len-i, value)) {	
    if (!strcmp(value, "Start")) {
      if (state == STOP) {
        state = STARTED;
	app_process();
      }
    }
    cycle = 0;
    if (parse_variable(&rx_buf[data_index+i], "cycle", data_len-i, value)) {	
      if (!strcmp(value, "Cycle"))
        cycle = 1;			
    }
    autorefresh = 0;
    if (parse_variable(&rx_buf[data_index+i], "autorefresh", data_len-i, value)) {	
      if (!strcmp(value, "Autorefresh"))
	autorefresh = 1;			
    }
    if (parse_variable(&rx_buf[data_index+i], "refreshtime", data_len-i, value)) {	
      if (value[1]==0) {
	value[1] = value[0];
	value[0] = 0x30;
      }
      refreshtime = (value[0]-0x30)*10 + (value[1]-0x30);
    }
    return 1;
  }

  if (parse_variable(&rx_buf[data_index+i], "send", data_len-i, value)) {	
    if (!memcmp(value, "Cancel", 6))	
      return 0;	
  }
  led_enable = 0;
  if (parse_variable(&rx_buf[data_index+i], "led", data_len-i, value)) {
    if (!strcmp(value, "on"))
    {
      led_enable = 1;
	  ledappOn();
	  printf("jack lee led on\n");
    }
	else
		{
			ledappOff();
			printf("jack lee led off\n");
	}
  }
  if (parse_variable(&rx_buf[data_index+i], "dow", data_len-i, value)) {
    for(j=0;j<7;j++) {
      if (strcmp(dow[j],value)==0) {
        DOW = j;
	break;
      }
    }
  }
  if (parse_variable(&rx_buf[data_index+i], "month", data_len-i, value)) {
    for(j=0;j<12;j++) {
      if (strcmp(month[j],value)==0) {
        MONTH = j+1;
	break;
      }
    }
  }
  if (parse_variable(&rx_buf[data_index+i], "dom", data_len-i, value)) {
    DOM = (value[0]-0x30)*10 + (value[1]-0x30);
  }
  if (parse_variable(&rx_buf[data_index+i], "year", data_len-i, value)) {
    YEAR = (value[0]-0x30)*1000 + (value[1]-0x30)*100 + (value[2]-0x30)*10 + (value[3]-0x30);
  }
  if (parse_variable(&rx_buf[data_index+i], "hour", data_len-i, value)) {
    HOUR = (value[0]-0x30)*10 + (value[1]-0x30);
  }
  if (parse_variable(&rx_buf[data_index+i], "min", data_len-i, value)) {
    MIN = (value[0]-0x30)*10 + (value[1]-0x30);
  }
  if (parse_variable(&rx_buf[data_index+i], "sec", data_len-i, value)) {
    SEC = (value[0]-0x30)*10 + (value[1]-0x30);
  }

  #endif
  return 1;
}


void web_server_init(void) {

  filesystem_init();
  http_server_init(80, process_POST, tag_expand);
}

