#ifndef __DRIVER_H
#define __DRIVER_H
#include "stm32f10x.h"
#include "stdbool.h"

void Driver_init(bool flag,u8 sub);
void Driver_start(u8 speed);
void Driver_keep(void);
void Driver_change(u8 _spd);
void Motor_stop(void);

#endif
