/******************************************************************************************
   tagexpand.c (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#include "tagexpand.h"
#include "stack.h"
#include "app.h"
//#include "Led.h"
#include "web_server.h"
//#include <iolpc2129.h>
#include <stdio.h>
#include <string.h>

const char *dow[] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
const char *month[] = {"January","February","March","April","May","June","July","August","September","October","November","December"};

char HOUR = 20 ;
char  MIN = 49 ;
char SEC = 00 ;

char  DOW  =    3 ;
char MONTH  = 8 ;
short  YEAR  = 2015;
short DOM   = 01 ;


extern char led_enable;
int tag_expand(char tagID, unsigned short pos, unsigned short len) {
  int i;

  switch(tagID) {
    case 10:
      if (len<2)
	return -1;	
      sprintf(&tx_buf[pos],"%02d",HOUR);
      return 2;
    case 11:
      if (len<2)
	return -1;	
      sprintf(&tx_buf[pos],"%02d",MIN);
      return 2;
    case 12:
      if (len<2)
	return -1;	
      sprintf(&tx_buf[pos],"%02d",SEC);
      return 2;
    case 20:
      i = strlen(dow[DOW]);
      if (len<i)
	return -1;	
      memcpy(&tx_buf[pos],dow[DOW],i);
      return i;	
    case 21:
      i = strlen(month[MONTH-1]);
      if (len<i)
        return -1;
      memcpy(&tx_buf[pos],month[MONTH-1],i);
      return i;	
    case 22:
      if (len<2)
	return -1;	  	
      sprintf(&tx_buf[pos],"%02d",DOM);
      return 2;
    case 23:
      if (len<4)
	return -1;	
      sprintf(&tx_buf[pos],"%04d",YEAR);
      return 4;
    case 24:
      if (led_enable) {
        if (len<7)
	  return -1;	
        memcpy(&tx_buf[pos],"CHECKED",7);
	return 7;
      }else
	return 0;
    case 25:
      if (led_enable) {
	if (len<2)
	  return -1;	
        memcpy(&tx_buf[pos],"On",2);
	return 2;
      }else {
	if (len<3)
	  return -1;	
        memcpy(&tx_buf[pos],"Off",3);
	return 3;
      }			
    // applic.htm tags
    case 49:
      if (len<7)
        return -1;	
      switch(state) {
	case 0:
	  sprintf(&tx_buf[pos],"%s","STOPPED");
	  return 7;
	case 1:
	  sprintf(&tx_buf[pos],"%s","STARTED");
	  return 7;
	case 2:
	  sprintf(&tx_buf[pos],"%s","FILLING");
	  return 7;
	case 3:
	  sprintf(&tx_buf[pos],"%s","HEATING");
	  return 7;
	case 4:
	  sprintf(&tx_buf[pos],"%s","EMTYING");
	  return 7;
	default:
	  sprintf(&tx_buf[pos],"%s","UNKNOWN");
	  return 7;
      }
    case 50:
      if (valve1==ON) {
	if (len<41)
	  return -1;
	sprintf(&tx_buf[pos],"%s","<img src=level.gif height=6 width=30><br>");
	return 41;
      }else
	return 0;
    case 51:
      if (valve1==ON) {
	if (len<10)
	  return -1;
	sprintf(&tx_buf[pos],"%s","pumpon.gif");
	return 10;
      }else {
	if (len<11)
	  return -1;
	sprintf(&tx_buf[pos],"%s","pumpoff.gif");
	return 11;
      }
    case 52:
      if (len<3)
        return -1;	
      sprintf(&tx_buf[pos],"%03d",liquid*6);
      return 3;
    case 53:
      if (valve2==ON) {
        if (len<10)
          return -1;
	sprintf(&tx_buf[pos],"%s","pumpon.gif");
	return 10;
      }else {
	if (len<11)
	  return -1;
	sprintf(&tx_buf[pos],"%s","pumpoff.gif");
	return 11;
      }
    case 54:
      if (valve2==ON) {
	if (len<41)
	  return -1;
	sprintf(&tx_buf[pos],"%s","<img src=level.gif height=6 width=30><br>");
	return 41;
      }else
	return 0;
    case 55:
      if (len<3)
	return -1;
      sprintf(&tx_buf[pos],"%03d",temp*4);
	return 3;
    case 56:
      if (valve1==ON) {
	if (len<10)
	  return -1;
        sprintf(&tx_buf[pos],"%s","arrows.gif");
	return 10;
      }else {
	if (len<9)
	  return -1;
	sprintf(&tx_buf[pos],"%s","clear.gif");
	return 9;
      }
    case 57:
      if (heater==ON) {
	if (len<8)
	  return -1;
	sprintf(&tx_buf[pos],"%s","fire.gif");
	return 8;
      }else {
	if (len<9)
	  return -1;
	sprintf(&tx_buf[pos],"%s","clear.gif");
	return 9;
      }
    case 58:
      if (valve2==ON) {
	if (len<10)
	  return -1;
	sprintf(&tx_buf[pos],"%s","arrows.gif");
	return 10;
      }else {
	if (len<9)
	  return -1;
	sprintf(&tx_buf[pos],"%s","clear.gif");
	return 9;
      }
    case 59:
      if (valve1==ON) {
	if (len<2)
	  return -1;
	sprintf(&tx_buf[pos],"%s","ON");
	return 2;
      }else {
	if (len<3)
	  return -1;
	sprintf(&tx_buf[pos],"%s","OFF");
	return 3;
      }
    case 60:
      if (heater==ON) {
	if (len<2)
	  return -1;
	sprintf(&tx_buf[pos],"%s","ON");
	return 2;
      }else {
	if (len<3)
	  return -1;
	sprintf(&tx_buf[pos],"%s","OFF");
	return 3;
      }
    case 61:
      if (valve2==ON) {
	if (len<2)
	  return -1;
	sprintf(&tx_buf[pos],"%s","ON");
	return 2;
      }else {
	if (len<3)
	  return -1;
	sprintf(&tx_buf[pos],"%s","OFF");
	return 3;
      }
    case 62:
      if (len<2)
	return -1;
      sprintf(&tx_buf[pos],"%2d",liquid);
      return 2;
    case 63:
      if (len<2)
	return -1;
      sprintf(&tx_buf[pos],"%2d",temp);
      return 2;
    case 64:
      if (cycle) {
	if (len<7)
	  return -1;
	memcpy(&tx_buf[pos],"CHECKED",7);
	return 7;
      }else
	return 0;
    case 65:
      if (autorefresh) {
	if (len<7)
	  return -1;
	memcpy(&tx_buf[pos],"CHECKED",7);
	return 7;
      }else
	return 0;
    case 66:	  			
      if (autorefresh) {
	if (len<33)
	  return -1;
	sprintf(&tx_buf[pos],"%s","<meta http-equiv=refresh content=");
	return 33;
      }else
	return 0;
    case 67:	  			
      if (autorefresh) {
        if (len<3)
	  return -1;
	sprintf(&tx_buf[pos],"%2d>",refreshtime);
	return 3;
      }else
	return 0;
    case 68:
      if (len<2)
	return -1;
      sprintf(&tx_buf[pos],"%2d",refreshtime);
      return 2;
    default:
      return -2;
  }
}

