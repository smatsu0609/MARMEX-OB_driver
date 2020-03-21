/* ========================================
 *
 * Copyright Shigeyuki Matsumura, 2014
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Shigeyuki Matsumura.
 *
 * ========================================
*/

#ifndef __OLED_H__
#define __OLED_H__

#include <stdint.h>

#define OLED_WIDTH		(128)
#define OLED_HEIGHT		(128)
#define OLED_PIXELS		(OLED_WIDTH * OLED_HEIGHT)

// 2: RGB565, 3: RGB666
#define OLED_PIX_DEPTH 2

typedef uint32_t color_t;

// 色の定義
#define C_BLACK		(0x00000000)
#define C_WHITE		(0xffffff00)
#define C_RED		(0xff000000)
#define C_GREEN		(0x00ff0000)
#define C_BLUE		(0x0000ff00)
#define C_YELLOW	(0xffff0000)
#define C_CYAN		(0x00ffff00)
#define C_MAGENDA	(0xff00ff00)
#define C_LGRAY		(0xc0c0c000)
#define C_GRAY		(0x80808000)
#define C_DGRAY		(0x40404000)

// kanji
#define K_SPACE	(0)
#define K_KETSU	(1)
#define K_TOU	(2)
#define K_CHI	(3)
#define K_SOKU	(4)
#define K_TEI	(5)
#define K_SOU	(6)
#define K_SHIN	(7)
#define K_SATSU	(8)
#define K_EI	(9)
#define K_ROKU	(10)
#define K_ONN	(11)
#define K_SETSU	(12)
#define K_0		(13)
#define K_1		(14)
#define K_2		(15)
#define K_3		(16)
#define K_4		(17)
#define K_5		(18)
#define K_6		(19)
#define K_7		(20)
#define K_8		(21)
#define K_9		(22)
#define K_A		(23)
#define K_B		(24)
#define K_C		(25)
#define K_D		(26)
#define K_E		(27)
#define K_F		(28)
#define K_DEN   (29)
#define K_RYU   (30)

void OLED_Init(void);
void OLED_PanelInit(void);
void OLED_ClearScreen(color_t color);
void OLED_DrawChar(uint8_t c, int x, int y);
void OLED_DrawCharB(uint8_t c, int x, int y);
void OLED_DrawString(const char *string, int x, int y);
void OLED_DrawStringBold(const char *string, int x, int y);
void OLED_DrawChars(const uint8_t *chars, int len, int x, int y);
void OLED_DrawCharsBold(const uint8_t *chars, int length, int x, int y);
void OLED_TextOut(const char *string, int col, int raw);
void OLED_TextOutBold(const char *string, int col, int raw);
void OLED_SetBgColor(color_t color);
void OLED_SetFgColor(color_t color);
void OLED_SetFgBgColor(color_t fgColor, color_t bgColor);
void OLED_FillRect(int x1, int y1, int x2, int y2, color_t color);
void OLED_DrawRect(int x1, int y1, int x2, int y2, color_t color);
void OLED_DrawBitmap(const uint16_t *bmp, int x, int y, int width, int height);
void OLED_DrawBitmap4(const uint8_t *bmp, const uint16_t *palette, int x, int y, int width, int height);
void OLED_DrawBitmap16(const uint8_t *bmp, const uint16_t *palette, int x, int y, int width, int height);
void OLED_DrawBitmap256(const uint8_t *bmp, const uint16_t *palette, int x, int y, int width, int height);
void OLED_DrawPix(int x0, int y0, color_t color);
void OLED_DrawKanji(uint8_t c, int x, int y);
void OLED_DrawHorizontalLine(int x1, int x2, int y, color_t color);
void OLED_DrawVerticalLine(int x, int y1, int y2, color_t color);
void OLED_DrawVerticalLine2(int y1, int y2, color_t color);
void OLED_SetColumn(int x);
void OLED_SetSleepModeOn(void);
void OLED_SetSleepModeOff(void);
void OLED_SetMasterCurrentControl(uint8_t current_value);
void OLED_SetContrastCurrent(uint8_t red, uint8_t green, uint8_t blue);
void OLED_SetDefaultContrast(void);
void OLED_SetHorizontalScroll( uint8_t direction, uint8_t startRow, uint8_t numRow, uint8_t mode );
void OLED_StartHorizontalScroll(void);
void OLED_StopHorizontalScroll(void);
void OLED_DrawLine(int x0, int y0, int x1, int y1, color_t color);
void OLED_DrawCircle(int x0, int y0, int r, color_t color);
void OLED_FillCircle(int x0, int y0, int r, color_t color);
int OLED_DrawBitmapFile( const char *filename, int x, int y, int width, int height );

void OLED_StartControl(void);
void OLED_EndControl(void);

#endif

/* [] END OF FILE */
