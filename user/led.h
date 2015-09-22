#ifndef __LED__H
#define __LED__H
#define SECOND_1_HZ (CLOCK_SECOND>>1)
#define SECOND_2_HZ (CLOCK_SECOND>>2)


void Led_Init(void);
void Led_Process(void);
void Led_Config(uint16_t newfre);
#endif

