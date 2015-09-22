#ifndef __FILE_ADAPTER__H
#define __FILE_ADAPTER__H
#include<stdio.h>
typedef int (*fp_rdwr)(char* buff,int size) ;//
void dev_init(void);//

int unregister(int fd);//	  
int register_open(fp_rdwr rd,  fp_rdwr wr);
int output(char buff[],int size,int fd);//
int input(char buff[],int size,int fd);//

#endif

