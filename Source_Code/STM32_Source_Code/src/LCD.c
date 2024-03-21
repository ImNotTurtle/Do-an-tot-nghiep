/*
 * LCD.c
 *
 *  Created on: Mar 14, 2024
 *      Author: Administrator
 */
#include "LCD.h"


static ucg_t s_ucg;
static char s_levelSrc[40] = {0};
static char s_connectionSrc[40] = {0};

void LCD_Init(void){
	Ucglib4WireSWSPI_begin(&s_ucg, UCG_FONT_MODE_SOLID);
	ucg_ClearScreen(&s_ucg);
	ucg_SetFont(&s_ucg, ucg_font_ncenR08_hr);
	ucg_SetColor(&s_ucg, 0, 255, 255, 255);
	ucg_SetColor(&s_ucg, 1, 0, 0, 0);
	ucg_SetRotate270(&s_ucg);
}
void LCD_DisplayState(CONNECTION_STATE_e state){
	if(state == ONLINE){
		sprintf(s_connectionSrc, "Device state: Online");
		ucg_DrawString(&s_ucg, 5, 24, 0, s_connectionSrc);
	}
	else if(state == OFFLINE){
		sprintf(s_connectionSrc, "Device connection state: Offline");
		ucg_DrawString(&s_ucg, 5, 24, 0, s_connectionSrc);
	}
}
void LCD_DisplayLevel(uint8_t level){
	//copy str to global source
	sprintf(s_levelSrc, "Curtain Level: %d%%    ", level);
	ucg_DrawString(&s_ucg, 5, 36, 0, s_levelSrc);
}

