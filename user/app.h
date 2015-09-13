/******************************************************************************************
   App.h (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#ifndef __APP_H
#define __APP_H

extern char valve1, valve2, heater, cycle, state;
extern unsigned int liquid, temp;

// state values
#define STOP	0
#define STARTED	1
#define FILL	2
#define HEAT	3
#define EMPTY	4

// valve1, valve2, and heater values
#define OFF		0
#define	ON		1

// cycle values
#define NO		0
#define YES		1

void app_init(void);
void app_process(void);

#endif

