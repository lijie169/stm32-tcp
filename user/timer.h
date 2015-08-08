/*
********************************************************************************
*                                   
* 模块名称 : 通用定时器模块
* 文件名称 : timer.h
* 版    本 : V0.1
* 说    明 : 使用systick时钟作为一个通用时钟，可以实现毫秒级延时
* 修改记录 :
*   版本号  日期        作者      说明
*   V0.1    2012-12-31  徐凯      创建该文件
*
*
********************************************************************************
*/
#ifndef __TIMER_H
#define __TIMER_H

/* 引用头文件 *****************************************************************/
#include "stm32f10x.h"
/* 外部数据类型 ***************************************************************/
/* 外部常数宏 *****************************************************************/
/* 外部动作宏 *****************************************************************/
/* 外部变量 *******************************************************************/
/* 通用时钟时间 */
#define CLOCK_SECOND  100
/* 定时器 自定义数据结构 */
typedef struct
{
  /* 开始时间 */
  uint16_t start;
  /* 时间间隔 */
  uint16_t interval;
}timer_typedef;

#define timer_start(ptimer,interval) timer_set((ptimer),(interval))


/* 外部函数声明 ***************************************************************/
void timer_config(void);
uint16_t clock_time(void);
void timer_set(timer_typedef* ptimer,uint16_t interval);
void timer_reset(timer_typedef* ptimer);
int8_t timer_expired(timer_typedef* ptimer);
#endif
/***************************************************************END OF FILE****/
