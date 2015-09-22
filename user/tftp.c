#include <stdio.h>
#include "stm32f10x.h"
#include "stack.h"
#include "udp.h"
#include "timer.h"
#define TFTP_RRQ 1
#define TFTP_DATA 3
#define TFTP_ACK 4
#define TFTP_ACK_LEN 4
#define TFTP_ASCII_MODE  0    //netascii
#define TFTP_BIN_MODE    1    //octet
#define TFTP_PORT  69
#define TFTP_ONE_PKG 512

#define FLASH_ONE_PAGE 2048
#define PKG_NUM   ((FLASH_ONE_PAGE + TFTP_ONE_PKG -1)/TFTP_ONE_PKG)

static uint16_t tftp_pkg_cur ;
extern char rx_buf[];
#define TFTP_OK		       0
#define TFTP_ERR_RPT	   5 
char IP_DST[] ={192,168,1,100} ;

#define TFTP_DBG 0
uint16_t tftp_socket_handler = 0 ;
timer_typedef tftp_timer ;
//uint8_t poll = 0 ;
uint8_t is_download_ok = 0 ;

uint16_t tftp_data_port ;
extern u32 appaddr ;
uint8_t appbuf[FLASH_ONE_PAGE];
uint8_t * tftp_getfilebuff(void)
{
	return appbuf ;
}

int8_t * tftp_getfilename(void)
{
	return "app.bin" ;	
}

int8_t* tftp_sendbuf(void)
{
	return &tx_buf[UDP_DATA_START];
}

int16_t tftp_getcurpkg(void)
{
	return tftp_pkg_cur ;
}

int16_t tftp_setcurpkg(int16_t pkgnum)
{
	tftp_pkg_cur = pkgnum ;
	return 0 ;
}

uint8_t tftp_isdownloadstart(void)
{
	return 0 != tftp_getcurpkg();
}

uint8_t tftp_isdownloadend(void)
{
	return 0!= is_download_ok ;
}
uint8_t tftp_sendPkgAck(int socket,char *IPdest,uint16_t num)
{
	uint8_t * tftp_tx = tftp_sendbuf();
	//opt code
	*tftp_tx = 0 ;
	tftp_tx++ ;
	*tftp_tx = (uint8_t)(TFTP_ACK&0xff) ;
	tftp_tx++ ;

	//pkg num
	*tftp_tx  =  (uint8_t)((num>>8)&0xff)    ;
	tftp_tx++      ;
	*tftp_tx  =  (uint8_t)(num&0xff)      ;
	tftp_tx++                            ;

	udp_send(socket, IPdest, tftp_data_port, TFTP_ACK_LEN);
	
}

uint8_t tftp_readreq(int socket,char *IPdest,uint8_t* file,uint8_t mode)
{
	int8_t *modetype[] = {"netascii","octet"};
	int8_t* sendbuf   = tftp_sendbuf();
	uint8_t length = 0 ;
	//optcode
	*sendbuf = 0 ;
	sendbuf ++ ;
	*sendbuf = TFTP_RRQ ;
	sendbuf++ ;
		//add len
	length += 2 ;
	
	//filename
	memcpy(sendbuf,file,strlen(file));
	sendbuf += strlen(file);

	length += strlen(file);
	// cp null char
	*sendbuf = 0 ;
	sendbuf++;

	length += 1 ;
	// cp filemode
	memcpy(sendbuf,modetype[mode],strlen(modetype[mode]));
	sendbuf += strlen(modetype[mode]);

	length += strlen(modetype[mode]);
	// cp null char
	*sendbuf = 0 ;

	length += 1 ;

	udp_send(socket, IPdest, TFTP_PORT, length);
	printf("tftp req send...\n");
}

uint8_t tftp_recvdata(int socket,char *IPdest,int8_t *data,uint16_t len)
{
	uint16_t nextno;
	int8_t ret = TFTP_OK ;
	uint16_t before ;
	static uint16_t buff_pos = 0 ;
	//static uint16_t currno = 0;
	nextno = ((uint16_t)data[0]<<8) + (uint8_t)data[1] ;
 do{
		if(nextno != tftp_getcurpkg() + 1)
		{
			ret =  TFTP_ERR_RPT ;
			printf("repeat one pkg %d\n",nextno);
			break;
		}

		data += 2 ;
		len -= 2 ;
		
		memcpy(tftp_getfilebuff()+buff_pos,data, len) ;
		printf("rcv one pkg %d,len = %d\n",nextno,len);
		tftp_setcurpkg(nextno) ;
		buff_pos += len ;
		if(0 == nextno % PKG_NUM)
		{
			iap_write_appbin( appaddr,  appbuf, buff_pos);
			printf("write flash addr %08x,len is %d\n",appaddr,buff_pos);
			appaddr += buff_pos;
			buff_pos = 0 ;
			
		}
		else if(TFTP_ONE_PKG != len)
		{
			//memcpy(tftp_getfilebuff()+buff_pos,0, 4) ;
			iap_write_appbin( appaddr,  appbuf, buff_pos);
			printf("write flash addr %08x,len is %d\n",appaddr,buff_pos);
			printf("receive file over\n");
		}

		if(TFTP_ONE_PKG != len)
		{
			reset_app_addr();
			is_download_ok = 1 ;
			printf("download ok\n");
		}

  }while(0);

	tftp_sendPkgAck(socket,IPdest,tftp_getcurpkg());

 	return TFTP_OK ;
}


void tftp_err_output(int8_t* data,uint16_t len)
{
	uint16_t errcode = (((uint8_t)data[0])<<8 )+(uint8_t)data[1];
	uint16_t loop ;

	
	printf("error code : %d,len = %d\n",errcode,len);
	for(loop = 2 ; loop < len ; loop++)
	{
		printf("%02x ",(uint8_t)data[loop]);
		if(loop - 2 + 1 % 16 == 0)
			printf("\n");
	}
	printf("here");
}

void tftp_service(int socket ,char event,char* senderIp,unsigned short senderPort,unsigned short start,unsigned short len)
{
	int8_t* buf = &rx_buf[UDP_DATA_START];
	uint16_t optcode = ((uint16_t)buf[0]<<8)+(uint8_t)buf[1] ;
	uint16_t loop ;
	
	if(TFTP_DBG == 1)
	{
		for(loop = 0 ; loop < len ; loop++)
		{
			printf("%02x ",(uint8_t)buf[loop]);
			if(0 == loop + 1 % 16)
				printf("\n");
		}

		printf("\n");
	}

	if(0 == tftp_data_port)
	{
		tftp_data_port = senderPort ;
	}
	//poll = 1 ;
	if(TFTP_DATA == optcode)
	{

		tftp_recvdata(socket,senderIp,&buf[2], len - 2);

	}
	else if(TFTP_ERR_RPT == optcode)
	{
		tftp_err_output(&buf[2],len-2);
	}
	else 
	{
		printf("unexpect opt code %d\n",optcode);
	}
}

void my_tftp_init(void)
{
	timer_set(&tftp_timer,CLOCK_SECOND*5);
	tftp_socket_handler = udp_open_socket(0, 0, tftp_service);
	printf("tftp init %d \n",tftp_socket_handler);
}

void tftp_poll(void)
{
	tftp_readreq(tftp_socket_handler,IP_DST,tftp_getfilename(),TFTP_BIN_MODE);
}

