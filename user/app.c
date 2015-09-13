/******************************************************************************************
   App.c (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#include "app.h"
#include "timer.h"
//#include "hardware.h"
//#include <iolpc2129.h>
#include "stm32f10x_gpio.h"
#include <stdio.h>
char led_enable=1;
char valve1, valve2, heater, cycle, state, ledapp;
unsigned int liquid, temp;

timer_typedef* apptimer,*ledapptimer,*buttontimer;


void ledappOn(void) {

  GPIO_WriteBit(GPIOF,GPIO_Pin_9,0);
}

void ledappOff(void) {

  GPIO_WriteBit(GPIOF,GPIO_Pin_9,1);
}


void app_init(void) {

  valve1=OFF;
  valve2=OFF;
  heater=OFF;
  cycle=NO;
  state=STARTED;
  liquid=0;
  temp=15;
  ledapp=0;

  timer_start(ledapptimer, 500);
//  timer_start(buttontimer, 200);
}


void app_process(void) {
  #if 0
  if (BUTTON1 && state == STOP)
    state = STARTED;
  if (BUTTON2 && timer_expired(buttontimer)) {
    timer_start(buttontimer, 200);
    if (cycle)
      cycle = 0;
    else
      cycle = 1;
  }
  #endif
  switch(state) {
    case STARTED:
      valve1 = ON;
      state = FILL;
      timer_start(apptimer, CLOCK_SECOND);
      break;
    case FILL:
      if (valve1 == ON && timer_expired(apptimer)) {
        liquid++;
        timer_start(apptimer, CLOCK_SECOND);
      }
      if (liquid == 30) {
        valve1 = OFF;
        heater = ON;
        state = HEAT;
        timer_start(apptimer, CLOCK_SECOND);
      }
      if (timer_expired(ledapptimer)) {
        if (ledapp) {
	  ledapp=0;
	  ledappOff();
	}else {
	  ledapp=1;
	  ledappOn();
  	}
	timer_start(ledapptimer, CLOCK_SECOND/2);
      }
      break;
    case HEAT:
      if (heater == ON  && timer_expired(apptimer)) {
        temp++;
        timer_start(apptimer, CLOCK_SECOND);
      }
      if (temp == 45) {
        heater = OFF;
	valve2 = ON;
	state = EMPTY;
        timer_start(apptimer, CLOCK_SECOND);
      }
      if (timer_expired(ledapptimer)) {
        if (ledapp) {
	  ledapp=0;
	  ledappOff();
	  timer_start(ledapptimer, CLOCK_SECOND);
	}else {
	  ledapp=1;
	  ledappOn();
	  timer_start(ledapptimer, CLOCK_SECOND/4);
        }
      }
      break;
    case EMPTY:
      if (valve2 == ON && timer_expired(apptimer)) {
        liquid--;
	temp--;
	timer_start(apptimer, CLOCK_SECOND);
      }
      if (liquid == 0) {
	valve2 = OFF;
	state = STOP;
	ledapp=0;
	ledappOff();
	if (cycle)
	  state = STARTED;
	break;
      }
      if (timer_expired(ledapptimer)) {
        if (ledapp) {
	  ledapp=0;
	  ledappOff();
	}else {
	  ledapp=1;
	  ledappOn();
  	}
      timer_start(ledapptimer, CLOCK_SECOND/20);				
      }
      break;
  }
}

