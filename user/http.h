/******************************************************************************************
   http.h (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#ifndef __Http_H
#define __Http_H

#define HTML	0
#define TEXT	1
#define GIF	2
#define JPEG	3
#define APP	4

#define NUMBER_OF_FILES	23		


extern int file_index[];



void filesystem_add(int i, const char *filename, char type,
                    const char *file, int filesize);

int parse_variable(char str[], char var[], unsigned short len, char value[]);
int HTTP_process(unsigned short data_index, unsigned short data_len);
int create_content(int socket, unsigned short *position, unsigned short *len);
void http_server_init(unsigned short port, int (*POST_CallBack)(unsigned short data_index,
                                                unsigned short data_len),
                                           int (*TagExpand_CallBack) (char tagID,
                                                unsigned short pos,unsigned short len) );

#endif

