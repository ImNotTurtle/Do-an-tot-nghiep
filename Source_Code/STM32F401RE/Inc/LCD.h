/*
 * LCD.h
 *
 *  Created on: Mar 14, 2024
 *      Author: Administrator
 */

#ifndef LCD_H_
#define LCD_H_
#include "Ucglib.h"
#include "DeviceManager.h"


void LCD_Init(void);
void LCD_DisplayState(CONNECTION_STATE_e state);
void LCD_DisplayLevel(uint8_t level);
void LCD_ClearScreen(void);

#endif /* LCD_H_ */
