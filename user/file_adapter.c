#include<stdio.h>
#include "file_adapter.h"


typedef struct 
{
	char uc_state ;
	fp_rdwr write_fun ;
	fp_rdwr read_fun ;
	
}tdev;
#define FILE_MAX_NUMS 2

tdev file_dev[FILE_MAX_NUMS] = {NULL};

#define FD_INVALID  0 
#define FD_VALID    1 

int input(char buff[],int size,int fd)
{
	int count ;
	if(FD_VALID != file_dev[fd].uc_state)
		return -1 ;
	if(NULL != file_dev[fd].read_fun)
		count = file_dev[fd].read_fun(buff,size - 1);

	buff[count] = 0 ;

	return count ;
	
}

int output(char buff[],int size,int fd)
{
	if(FD_VALID != file_dev[fd].uc_state)
		return -1 ;
	if(NULL != file_dev[fd].write_fun)
		return file_dev[fd].write_fun(buff,size);

	return -2 ;
}

int unregister(int fd)
{
	file_dev[fd].uc_state = FD_INVALID ;
	return 0 ;
}

int register_open(fp_rdwr rd,  fp_rdwr wr)
{
	unsigned char loop ;
	for(loop = 0 ; loop < FILE_MAX_NUMS ; loop++)
	{
		if(FD_INVALID == file_dev[loop].uc_state)
		{
			file_dev[loop].uc_state = FD_VALID ;
			file_dev[loop].write_fun = wr ;
			file_dev[loop].read_fun = rd ;

			return loop ;
		}
	}

	return -1 ;
}

void dev_init(void)
{
	unsigned char loop ;
	
	for(loop = 0 ; loop < FILE_MAX_NUMS ; loop++)
	{
		file_dev[loop].uc_state = FD_INVALID ;
	}
}

