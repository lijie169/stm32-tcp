/*
********************************************************************************
*                                   
* 模块名称 : ENC28J60驱动模块    
* 文件名称 : enc28j60.c
* 版    本 : V0.1
* 说    明 : 
* 修改记录 :
*   版本号  日期        作者      说明
*   V0.1    2013-07-12  徐凯      优化驱动
*   V0.2    2013-07-14  徐凯      增加适应LwIP的驱动函数
*
********************************************************************************
*/
#include  "enc28j60.h"
#include  "spi.h"

/* 存储区编号  ENC28J60 具有Bank0到Bank3 4个存储区 需要通过ECON1寄存器选择 */
static unsigned char enc28j60_bank;
/* 下一个数据包指针，详见数据手册P43 图7-3 */
static unsigned int next_pkt;
// 需要根据目标板修改引脚
#define 	ENC28J60_CSL()		GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define 	ENC28J60_CSH()		GPIO_SetBits(GPIOA,GPIO_Pin_4)
#define     BSP_SPI1SendByte(data)   SPIx_ReadWriteByte(data) 
/*
********************************************************************************
* 函 数 名: enc28j60_readop
* 功能说明: 读寄存器命令
* 参    数: unsigned char op        操作码
*           unsigned char address   寄存器地址
* 返 回 值:                         返回数据
* 使用说明: 该函数支持的操作码只读控制寄存器 读缓冲器
* 调用方法: enc28j60_readop
********************************************************************************
*/
unsigned char enc28j60_readop(unsigned char op, unsigned char address)
{
	unsigned char dat = 0;
	
	/* CS拉低 使能ENC28J60 */
	ENC28J60_CSL();
	/* 操作码和地址 */
	dat = op | (address & ADDR_MASK);
    /* 通过SPI写数据*/
	BSP_SPI1SendByte(dat);
    /* 通过SPI读出数据 */
	dat = BSP_SPI1SendByte(0xFF);
	
    /* 如果是MAC和MII寄存器，第一个读取的字节无效，该信息包含在地址的最高位*/
	if(address & 0x80)
	{
		/* 再次通过SPI读取数据 */
        dat = BSP_SPI1SendByte(0xFF);
	}
	
    /* CS拉高 禁止ENC28J60 */
	ENC28J60_CSH();
    
    /* 返回数据 */
	return dat;
}

/*
********************************************************************************
* 函 数 名: enc28j60_writeop
* 功能说明: 写寄存器命令
* 参    数: unsigned char op        操作码
*           unsigned char address   寄存器地址
*           unsigned char data      写入数据
* 返 回 值:                         无
* 使用说明: 该函数支持的操作码有: 写控制寄存器 位域清零 位域置1
* 调用方法: enc28j60_writeop
********************************************************************************
*/
void enc28j60_writeop(unsigned char op, unsigned char address, unsigned char data)
{
	unsigned char dat = 0;
    /* 使能ENC28J60 */							  	  
	ENC28J60_CSL();	     
    /* 通过SPI发送 操作码和寄存器地址 */                 		
	dat = op | (address & ADDR_MASK);
    /* 通过SPI1发送数据 */
	BSP_SPI1SendByte(dat);
    /* 准备寄存器数值 */				  
	dat = data;
    /* 通过SPI发送数据 */
	BSP_SPI1SendByte(dat);
    /* 禁止ENC28J60 */				 
	ENC28J60_CSH();	
}

/*
********************************************************************************
* 函 数 名: enc28j60_readbuf
* 功能说明: 读缓冲区
* 参    数: unsigned int len        读取长度
*           unsigned char* data     读取指针
* 返 回 值:                         无
* 使用说明: 适用于uIP和LwIP
*           LwIP中，在low_level_input中调用
*           替代read data into( q->payload, q->len );
* 调用方法: enc28j60_readbuf( q->payload, q->len );
********************************************************************************
*/
void enc28j60_readbuf(char* pdata,int len)
{
    /* 使能ENC28J60 */
    ENC28J60_CSL();
	/* 通过SPI发送读取缓冲区命令*/
	BSP_SPI1SendByte(ENC28J60_READ_BUF_MEM);
    
    /* 循环读取 */
	while(len)
	{
        len--;
        /* 读取数据 */
        *pdata = (unsigned char)BSP_SPI1SendByte(0);
        /* 地址指针累加 */
        pdata++;
	}
    
    /* 禁止ENC28J60 */
	ENC28J60_CSH();
}

/*
********************************************************************************
* 函 数 名: enc28j60_writeBuffer
* 功能说明: 写缓冲区
* 参    数: unsigned int len        读取长度
*           unsigned char* data     读取指针
* 返 回 值: 无
* 使用说明: 可适用于LwIP，在low_level_output中调用
*           替代send data from(q->payload, q->len);
* 调用方法: LwIP中调用方法
*           enc28j60_writebuf( q->payload, q->len );
********************************************************************************
*/
void enc28j60_writebuf(char* pdata,int len )
{
    /* 使能ENC28J60 */
    ENC28J60_CSL();
	/* 通过SPI发送写取缓冲区命令 */
	BSP_SPI1SendByte(ENC28J60_WRITE_BUF_MEM);
	
    /* 循环发送 */
	while(len)
	{
		len--;
        /* 发送数据 */
		BSP_SPI1SendByte(*pdata);
        /* 地址指针累加 */
		pdata++;
	}
    
    /* 禁止ENC28J60 */
	ENC28J60_CSH();
}

/*
********************************************************************************
* 函 数 名: enc28j60_setbank
* 功能说明: 设定寄存器存储区域
* 参    数: unsigned char address   寄存器地址
* 返 回 值:                         无
* 使用说明: 
* 调用方法: 
********************************************************************************
*/ 
void enc28j60_setbank(unsigned char address)
{
	/* 计算本次寄存器地址在存取区域的位置 */
	if((address & BANK_MASK) != enc28j60_bank)
	{
        /* 清除ECON1的BSEL1 BSEL0 详见数据手册15页 */
        enc28j60_writeop(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
        /* 请注意寄存器地址的宏定义，bit6 bit5代码寄存器存储区域位置 */
        enc28j60_writeop(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
        /* 重新确定当前寄存器存储区域 */
        enc28j60_bank = (address & BANK_MASK);
	}
}

/*
********************************************************************************
* 函 数 名: enc28j60_read
* 功能说明: 读取寄存器值
* 参    数: unsigned char address   寄存器地址
* 返 回 值:                         寄存器值
* 使用说明: 
* 调用方法: 
********************************************************************************
*/
unsigned char enc28j60_read(unsigned char address)
{
	/* 设定寄存器地址区域 */
	enc28j60_setbank(address);
	/* 读取寄存器值 发送读寄存器命令和地址 */
	return enc28j60_readop(ENC28J60_READ_CTRL_REG, address);
}

/*
********************************************************************************
* 函 数 名: enc28j60_write
* 功能说明: 写寄存器
* 参    数: unsigned char address   寄存器地址
*           unsigned char data      寄存器数值
* 返 回 值:                         无
* 使用说明: 
* 调用方法: 
********************************************************************************
*/
void enc28j60_write(unsigned char address, unsigned char data)
{
	/* 设定寄存器地址区域 */
	enc28j60_setbank(address);
	/* 写寄存器值 发送写寄存器命令和地址 */
	enc28j60_writeop(ENC28J60_WRITE_CTRL_REG, address, data);
}

/*
********************************************************************************
* 函 数 名: enc28j60_writephy
* 功能说明: 写物理寄存器
* 参    数: unsigned char address   物理寄存器地址
*           unsigned int data       物理寄存器数值 物理寄存器均为16位宽
* 返 回 值:                         无
* 使用说明: PHY寄存器不能通过SPI命令直接访问，而是通过一组特殊的寄存器来访问
*           详见数据手册19页
* 调用方法: 
********************************************************************************
*/
void enc28j60_writephy(unsigned char address, unsigned int data)
{
	/* 向MIREGADR写入地址 详见数据手册19页*/
	enc28j60_write(MIREGADR, address);
	/* 写入低8位数据 */
	enc28j60_write (MIWRL, data);
    /* 写入高8位数据 */
	enc28j60_write(MIWRH, data>>8);
	/* 等待PHY寄存器写入完成 */
	while( enc28j60_read(MISTAT) & MISTAT_BUSY );
}

/*
********************************************************************************
* 函 数 名: enc28j60_init
* 功能说明: 初始化enc28j60
* 参    数: unsigned char* mac_addr  mac地址参数指针
* 返 回 值:                         无
* 使用说明: 
* 调用方法: 
********************************************************************************
*/
void enc28j60_init(unsigned char* mac_addr)
{

	int time ;
	#if 0
    GPIO_InitTypeDef GPIO_InitStructure;  
    /* 打开GPIOA时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
    
    /* enc28j60 CS @GPIOA.4 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    #endif
    /* 禁止ENC28J60 */
    ENC28J60_CSH();
	/* ENC28J60软件复位 该函数可以改进 */
	enc28j60_writeop(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET); 
    /* 查询ESTAT.CLKRDY位 */
	for (time = 50000; time > 0; time--);
	while(!((enc28j60_read(ESTAT)) & ESTAT_CLKRDY));
    
	/* 设置接收缓冲区起始地址 该变量用于每次读取缓冲区时保留下一个包的首地址 */
	next_pkt = RXSTART_INIT;
    
    /* 设置接收缓冲区 起始指针 */
	enc28j60_write(ERXSTL, RXSTART_INIT & 0xFF);
	enc28j60_write(ERXSTH, RXSTART_INIT >> 8);
    
    /* 设置接收缓冲区 读指针 */ 
	enc28j60_write(ERXRDPTL, RXSTART_INIT&0xFF);
	enc28j60_write(ERXRDPTH, RXSTART_INIT>>8);
    
    /* 设置接收缓冲区 结束指针 */
	enc28j60_write(ERXNDL, RXSTOP_INIT&0xFF);
	enc28j60_write(ERXNDH, RXSTOP_INIT>>8);
    
	/* 设置发送缓冲区 起始指针 */
	enc28j60_write(ETXSTL, TXSTART_INIT&0xFF);
	enc28j60_write(ETXSTH, TXSTART_INIT>>8);
	/* 设置发送缓冲区 结束指针 */
	enc28j60_write(ETXNDL, TXSTOP_INIT&0xFF);
	enc28j60_write(ETXNDH, TXSTOP_INIT>>8);
    
    /* 使能单播过滤 使能CRC校验 使能 格式匹配自动过滤*/
	enc28j60_write(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
	enc28j60_write(EPMM0,   0x3f);
	enc28j60_write(EPMM1,   0x30);
	enc28j60_write(EPMCSL,  0xf9);
	enc28j60_write(EPMCSH,  0xf7);
    
    /* 使能MAC接收 允许MAC发送暂停控制帧 当接收到暂停控制帧时停止发送*/
    /* 数据手册34页 */
	enc28j60_write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
    
    /* 退出复位状态 */
	enc28j60_write(MACON2, 0x00);
    
    /* 用0填充所有短帧至60字节长 并追加一个CRC 发送CRC使能 帧长度校验使能 MAC全双工使能*/
	/* 提示 由于ENC28J60不支持802.3的自动协商机制，所以对端的网络卡需要强制设置为全双工 */
	enc28j60_writeop( ENC28J60_BIT_FIELD_SET, MACON3, 
                     MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX );
    
    /* 填入默认值 */
	enc28j60_write(MAIPGL, 0x12);
    /* 填入默认值 */
	enc28j60_write(MAIPGH, 0x0C);
    /* 填入默认值 */
	enc28j60_write(MABBIPG, 0x15);
    
    /* 最大帧长度 */
	enc28j60_write(MAMXFLL, MAX_FRAMELEN & 0xFF);	
	enc28j60_write(MAMXFLH, MAX_FRAMELEN >> 8);
    
    /* 写入MAC地址 */
	enc28j60_write(MAADR5, mac_addr[0]);	
	enc28j60_write(MAADR4, mac_addr[1]);
	enc28j60_write(MAADR3, mac_addr[2]);
	enc28j60_write(MAADR2, mac_addr[3]);
	enc28j60_write(MAADR1, mac_addr[4]);
	enc28j60_write(MAADR0, mac_addr[5]);
    
	/* 配置PHY为全双工  LEDB为拉电流 */
	enc28j60_writephy(PHCON1, PHCON1_PDPXMD);
    
    /* LED状态 */
    enc28j60_writephy(PHLCON,0x0476);	
    
    /* 半双工回环禁止 */
	enc28j60_writephy(PHCON2, PHCON2_HDLDIS);
    
    /* 返回BANK0 */	
	enc28j60_setbank(ECON1);
    
    /* 使能中断 全局中断 接收中断 接收错误中断 */
	enc28j60_writeop(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE|EIE_RXERIE);
    
    /* 接收使能位 */
	enc28j60_writeop(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
    
}

/*
********************************************************************************
* 函 数 名: enc28j60_packet_send
* 功能说明: 通过enc28j60发送数据包
* 参    数: int len       数据包长度
*           char* packet  数据包指针
* 返 回 值: 无
* 使用说明: 仅适用于uIP
* 调用方法: 
********************************************************************************
*/
void enc28j60_packet_send(char* packet,int len )
{
	/* 查询发送逻辑复位位 */
	while((enc28j60_read(ECON1) & ECON1_TXRTS)!= 0);
    
    /* 设置发送缓冲区起始地址 */    
	enc28j60_write(EWRPTL, TXSTART_INIT & 0xFF);
	enc28j60_write(EWRPTH, TXSTART_INIT >> 8);
    
	/* 设置发送缓冲区结束地址 该值对应发送数据包长度 */   
	enc28j60_write(ETXNDL, (TXSTART_INIT + len) & 0xFF);
	enc28j60_write(ETXNDH, (TXSTART_INIT + len) >>8);
    
	/* 发送控制字节 控制字节为0x00,表示使用macon3设置 */
    enc28j60_writeop(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
    
	/* 通过ENC28J60发送数据包 */
	enc28j60_writebuf( packet,len );
    
	/* 启动发送 */
	enc28j60_writeop(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
    
    /* 复位发送逻辑的问题。参见 Rev. B4 Silicon Errata point 12. */
	if( (enc28j60_read(EIR) & EIR_TXERIF) )
	{
		enc28j60_setbank(ECON1);
        enc28j60_writeop(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
    }
}

/*
********************************************************************************
* 函 数 名: enc28j60_init_send
* 功能说明: 数据包发送函数【1】 修改发送缓冲区，并发送控制字节
*           控制字节为0，采用macon3参数，自动填充0，使能CRC校验
* 参    数: int len       数据包长度
* 返 回 值: 无
* 使用说明: 适用于LwIP，在low_level_out中使用，替代initiate transfer();
* 调用方法: enc28j60_init_send(p->tot_len); // 表示pbuf数据包整体长度
********************************************************************************
*/
void enc28j60_init_send(int len)
{
	/* 查询发送逻辑复位位 */
	while((enc28j60_read(ECON1) & ECON1_TXRTS)!= 0);
    
    /* 设置发送缓冲区起始地址 */    
	enc28j60_write(EWRPTL, TXSTART_INIT & 0xFF);
	enc28j60_write(EWRPTH, TXSTART_INIT >> 8);
    
	/* 设置发送缓冲区结束地址 该值对应发送数据包长度 */   
	enc28j60_write(ETXNDL, (TXSTART_INIT + len) & 0xFF);
	enc28j60_write(ETXNDH, (TXSTART_INIT + len) >>8);
    
	/* 发送控制字节 控制字节为0x00,表示使用macon3设置 */
    enc28j60_writeop(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
}

/*
********************************************************************************
* 函 数 名: enc28j60_start_send
* 功能说明: 数据包发送函数【3】 启动发送
* 参    数: 无
* 返 回 值: 无
* 使用说明: 适用于LwIP，在low_level_out中使用
*           替代signal that packet should be sent();
* 调用方法: enc28j60_start_send
********************************************************************************
*/
void enc28j60_start_send( void )
{
	/* 启动发送 */
	enc28j60_writeop(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
    
    /* 复位发送逻辑的问题。参见 Rev. B4 Silicon Errata point 12. */
	if( (enc28j60_read(EIR) & EIR_TXERIF) )
	{
		enc28j60_setbank(ECON1);
        enc28j60_writeop(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
    }
}

/*
********************************************************************************
* 函 数 名: enc28j60_packet_receive
* 功能说明: 通过enc28j60接收数据包
* 参    数: unsigned int maxlen       接收数据包最大长度值
*           unsigned char* packet     数据包指针
* 返 回 值:                           接收数据包长度
* 使用说明: 仅适用于uIP
* 调用方法: 
********************************************************************************
*/
int enc28j60_packet_receive(char* packet,int maxlen )
{
	int rxstat;
	int len;
    
	/* 是否收到以太网数据包 */
	if( enc28j60_read(EPKTCNT) == 0 )
	{
		return 0;
    }
    
	/* 设置接收缓冲器读指针 */
	enc28j60_write( ERDPTL, (next_pkt) );
	enc28j60_write( ERDPTH, (next_pkt)>>8 );
    
    /* 接收数据包结构示例 数据手册43页 */
    
	/* 读下一个包的指针 */
	next_pkt  = enc28j60_readop( ENC28J60_READ_BUF_MEM, 0 );
	next_pkt |= enc28j60_readop( ENC28J60_READ_BUF_MEM, 0 )<<8;
    
	/* 读包的长度 */
	len  = enc28j60_readop( ENC28J60_READ_BUF_MEM, 0 );
	len |= enc28j60_readop( ENC28J60_READ_BUF_MEM, 0 )<<8;
    
    /* 删除CRC计数 */
    len -= 4; 	
    
	/* 读取接收状态 */
	rxstat  = enc28j60_readop(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= enc28j60_readop(ENC28J60_READ_BUF_MEM, 0) << 8;
    
	/* 限制检索的长度	*/  
    if (len > maxlen - 1)
	{
        len = maxlen - 1;
    }
    /* 检查CRC和符号错误 */
    /* ERXFCON.CRCEN是默认设置。通常我们不需要检查 */
    if ((rxstat & 0x80)==0)
	{
        len = 0;
	}
	else
	{
        /* 从接收缓冲器中复制数据包 */
        enc28j60_readbuf( packet , len );
    }
    
    /* 移动接收缓冲区 读指针*/
	enc28j60_write(ERXRDPTL, (next_pkt));
	enc28j60_write(ERXRDPTH, (next_pkt)>>8);
    
	/* 数据包递减 */
	enc28j60_writeop(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
    
    /* 返回长度 */
	return len;
}

/*
********************************************************************************
* 函 数 名: enc28j60_packet_getcount
* 功能说明: 获得enc28j60数据包个数
* 参    数: 无
* 返 回 值: 数据包个数
* 使用说明: 适用于uIP或者LwIP，可定时循环调用该数据
*           若LwIP系统，可调用ethernetif_input();
* 调用方法: 
********************************************************************************
*/
int enc28j60_packet_getcount( void )
{
    return (int)enc28j60_read(EPKTCNT);
}

/*
********************************************************************************
* 函 数 名: enc28j60_packet_getlen
* 功能说明: 获得enc28j60数据包长度
* 参    数: 无
* 返 回 值: 数据包长度
* 使用说明: 适用于LwIP，low_level_input中调用，替代len = ;
*           获得数据包长度，接着分配pbuf
* 调用方法: len = enc28j60_packet_getlen();
********************************************************************************
*/
int enc28j60_packet_getlen(void)
{
	/* 数据包总长度 */
    int len = 0;
    int rxstat;
    
    /* 设置接收缓冲器读指针 */
	enc28j60_write( ERDPTL, (next_pkt) );
	enc28j60_write( ERDPTH, (next_pkt)>>8 );
    
    /* 接收数据包结构示例 数据手册43页 */
	/* 读下一个包的指针 */
	next_pkt  = enc28j60_readop( ENC28J60_READ_BUF_MEM, 0 );
	next_pkt |= enc28j60_readop( ENC28J60_READ_BUF_MEM, 0 )<<8;
    
	/* 读包的长度 */
	len  = enc28j60_readop( ENC28J60_READ_BUF_MEM, 0 );
	len |= enc28j60_readop( ENC28J60_READ_BUF_MEM, 0 )<<8;
    
    /* 删除CRC计数 */
    len -= 4; 	
    
	/* 读取接收状态 */
	rxstat  = enc28j60_readop(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= enc28j60_readop(ENC28J60_READ_BUF_MEM, 0) << 8;
    
    /* 注意取消了CRC校验检查部分 */
    /* 返回接收数据包长度 */
    return len;
}


/*
********************************************************************************
* 函 数 名: enc28j60_finish_recevie
* 功能说明: 结束读取接收缓冲区过程，移动接收缓冲区指针
* 参    数: 无
* 返 回 值: 无
* 使用说明: 适用于LwIP，low_level_input中调用
*           acknowledge that packet has been read(); 或
*           drop packet();
* 调用方法: enc28j60_finish_receive()
********************************************************************************
*/
void enc28j60_finish_receive( void )
{
    /* 移动接收缓冲区 读指针*/
	enc28j60_write(ERXRDPTL, (next_pkt));
	enc28j60_write(ERXRDPTH, (next_pkt)>>8);
    
	/* 数据包递减 */
	enc28j60_writeop(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
}
