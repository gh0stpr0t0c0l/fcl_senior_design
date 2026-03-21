/**
 *
 * ESP-Drone Firmware
 *
 * Copyright 2019-2020  Espressif Systems (Shanghai)
 * Copyright (C) 2012 BitCraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * zranger.h: Z-Ranger deck driver
 */

#ifndef _YRANGER2_H_
#define _YRANGER2_H_

#include "stabilizer_types.h"
//#include "deck_core.h"

void yRanger2Init(void);

bool yRanger2Test(void);
void yRanger2Task(void* arg);

#endif /* _YRANGER2_H_ */
