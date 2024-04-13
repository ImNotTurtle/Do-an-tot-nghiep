/*
 * LCD.c
 *
 *  Created on: Mar 14, 2024
 *      Author: Administrator
 */
#include "LCD.h"
#include "stm32f4xx_exit.h"


static ucg_t s_ucg;
static char s_levelSrc[40] = {0};
static char s_connectionSrc[40] = {0};
static char s_drawString[40] = {0};
static char s_curtainString[40] = {0};
static char s_emptyString[40] = {0};


void LCD_Init(void){
	Ucglib4WireSWSPI_begin(&s_ucg, UCG_FONT_MODE_SOLID);
	ucg_ClearScreen(&s_ucg);
	ucg_SetFont(&s_ucg, ucg_font_ncenR08_hr);
	ucg_SetColor(&s_ucg, 0, 255, 255, 255);
	ucg_SetColor(&s_ucg, 1, 0, 0, 0);
	ucg_SetRotate270(&s_ucg);

	sprintf(s_drawString, "------------------------------");//30 ki tu
	sprintf(s_curtainString, "---");
	sprintf(s_emptyString, "   ");
}
void LCD_DisplayState(CONNECTION_STATE_e state){
	if(state == ONLINE){
		sprintf(s_connectionSrc, "Device state: Online        ");
		ucg_DrawString(&s_ucg, 5, 12, 0, s_connectionSrc);
		//draw the top liner
		ucg_DrawString(&s_ucg, 5, 36, 0, s_drawString);
	}
	else if(state == OFFLINE){
		sprintf(s_connectionSrc, "Device state: Offline       ");
		ucg_DrawString(&s_ucg, 5, 12, 0, s_connectionSrc);
	}
}
void LCD_DisplayLevel(uint8_t level){
	//copy str to global source
	sprintf(s_levelSrc, "Curtain Level: %d%%    ", level);
	ucg_DrawString(&s_ucg, 5, 24, 0, s_levelSrc);
	//draw the curtain
	uint8_t levelCount = 8;
	for(uint8_t i = 0; i < level / 25; i++){
		uint8_t leftX = 16 * i + 5;
		uint8_t rightX = 16 * (levelCount - i - 1) + 5;
		//draw left
		ucg_DrawString(&s_ucg, leftX, 46, 0, s_curtainString);
		ucg_DrawString(&s_ucg, leftX, 54, 0, s_curtainString);
		ucg_DrawString(&s_ucg, leftX, 62, 0, s_curtainString);
		ucg_DrawString(&s_ucg, leftX, 70, 0, s_curtainString);
		ucg_DrawString(&s_ucg, leftX, 78, 0, s_curtainString);
		ucg_DrawString(&s_ucg, leftX, 86, 0, s_curtainString);
		ucg_DrawString(&s_ucg, leftX, 94, 0, s_curtainString);
		ucg_DrawString(&s_ucg, leftX, 102, 0, s_curtainString);

		//draw right
		ucg_DrawString(&s_ucg, rightX, 46, 0, s_curtainString);
		ucg_DrawString(&s_ucg, rightX, 54, 0, s_curtainString);
		ucg_DrawString(&s_ucg, rightX, 62, 0, s_curtainString);
		ucg_DrawString(&s_ucg, rightX, 70, 0, s_curtainString);
		ucg_DrawString(&s_ucg, rightX, 78, 0, s_curtainString);
		ucg_DrawString(&s_ucg, rightX, 86, 0, s_curtainString);
		ucg_DrawString(&s_ucg, rightX, 94, 0, s_curtainString);
		ucg_DrawString(&s_ucg, rightX, 102, 0, s_curtainString);
	}
}
void LCD_ClearScreen(void){
	ucg_ClearScreen(&s_ucg);
}

