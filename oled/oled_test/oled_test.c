#include <stdio.h>
#include <unistd.h>
#include "oled.h"
#include "bitmap.h"
#include "font.h"


#define MOVE_POINT( pos, v ) 	\
	pos += v;					\
	if( pos <= 0 ) {			\
		pos = -pos;				\
		v = -v;					\
	} else if ( pos >= 127 ) {	\
		pos = 254 - pos;		\
		v = -v;					\
	}							

typedef struct {
	int x1;
	int y1;
	int x2;
	int y2;
} line_pos_t;

#define HYST_SIZE	(24)

line_pos_t ptBuf[HYST_SIZE] = {{0}};

color_t cTbl[HYST_SIZE];
	
void lineArt(void)
{
	int x1, y1, x2, y2;
	int vx1, vy1, vx2, vy2;
	int idx,i,j;
	uint16_t level;

	x1 = 1; y1 = 1;
	x2 = 10; y2 = 10;
	vx1 = 1, vy1 = 2;
	vx2 = 3, vy2 = -1;
	
	for(i=0;i<HYST_SIZE-1;i++){
		level = 256*(HYST_SIZE-i)/HYST_SIZE;
		if(level>0xff)level = 0xff;
		cTbl[i] = level << 8;
	}
	cTbl[i] = 0;
	
	sleep(1);
	
	OLED_SetBgColor(C_BLACK);

	OLED_DrawBitmapFile("bmp/topics_pic_01.bin", 0, 0, 128, 128);
	sleep(2);

	OLED_DrawBitmapFile("bmp/co2644092.bin", 0, 0, 128, 128);
	sleep(2);

	OLED_DrawBitmapFile("bmp/raspberry-pi.bin", 0, 0, 128, 128);
	sleep(2);

	OLED_DrawBitmapFile("bmp/android.bin", 0, 0, 128, 128);
	sleep(2);

	OLED_DrawBitmapFile("bmp/python.bin", 0, 0, 128, 128);
	sleep(2);

	OLED_DrawBitmapFile("bmp/linux.bin", 0, 0, 128, 128);
	sleep(2);

	OLED_DrawBitmapFile("bmp/cc.bin", 0, 0, 128, 128);
	sleep(2);

	OLED_DrawBitmapFile("bmp/bs.bin", 0, 0, 128, 128);
	sleep(2);

	OLED_ClearScreen(C_BLACK);

	j = 0;
	idx = 0;
	for(;;) {

		ptBuf[idx].x1 = x1;
		ptBuf[idx].y1 = y1;
		ptBuf[idx].x2 = x2;
		ptBuf[idx].y2 = y2;
		idx++;
		if(idx>=HYST_SIZE) idx = 0;

		j = idx;
		for ( i = 0; i < HYST_SIZE; i ++ ) {
			OLED_DrawLine(ptBuf[j].x1,ptBuf[j].y1,ptBuf[j].x2,ptBuf[j].y2,cTbl[HYST_SIZE-1-i]);
			j++;if(j>=HYST_SIZE)j=0;
		}
		usleep(50000);


		MOVE_POINT( x1, vx1 );
		MOVE_POINT( y1, vy1 );
		MOVE_POINT( x2, vx2 );
		MOVE_POINT( y2, vy2 );
	}
}



int main(int argc, char *argv[])
{
//	int i;
	int col, raw;
	OLED_Init();
	OLED_StartControl();
	
#if 0
	for ( i = 0; i < 10; i++ ) {
		for ( raw = 0; raw < 16; raw++ ) {
			OLED_TextOutBold( "Matsumura Shige", 0, raw );
		}
		usleep(100*1000);
		for ( raw = 0; raw < 16; raw++ ) {
			OLED_TextOutBold( "               ", 0, raw );
		}
		usleep(100*1000);
	}
#endif

	OLED_ClearScreen(C_DGRAY);
	col = 0; raw = 8;
	OLED_SetFgBgColor(C_WHITE, C_DGRAY);
	OLED_TextOutBold("MatsuDoroid", col++, raw++);
	OLED_SetFgColor(C_YELLOW);
	OLED_TextOutBold("MatsuDoroid", col++, raw++);
	OLED_SetFgColor(C_CYAN);
	OLED_TextOutBold("MatsuDoroid", col++, raw++);
	OLED_SetFgColor(C_MAGENDA);
	OLED_TextOutBold("MatsuDoroid", col++, raw++);
	OLED_SetFgColor(C_RED);
	OLED_TextOutBold("MatsuDoroid", col++, raw++);
	OLED_SetFgColor(C_BLUE);
	OLED_TextOutBold("MatsuDoroid", col++, raw++);
	OLED_SetFgColor(C_GREEN);
	OLED_TextOutBold("MatsuDoroid", col++, raw++);
	
	OLED_FillRect(0,0,63,63,C_BLUE);
	OLED_FillRect(64,0,127,63,C_RED);
	OLED_DrawRect(0,0,63,63,C_YELLOW);
	OLED_DrawRect(64,0,127,63,C_CYAN);
	
	OLED_DrawBitmap(droid48x48, 8, 8, 48, 48);
	OLED_DrawBitmap(droid48x48, 72, 8, 48, 48);
	
	OLED_SetFgBgColor(C_CYAN, C_BLUE);
	OLED_DrawKanji(K_DEN, 4, 108);
	OLED_DrawKanji(K_RYU, 22, 108);
	
	OLED_FillCircle(63,63,50,C_RED);
	OLED_DrawCircle(63,63,60,C_GREEN);
	OLED_DrawCircle(63,63,50,C_GREEN);
	OLED_DrawCircle(63,63,40,C_GREEN);
	OLED_DrawCircle(63,63,30,C_GREEN);
	OLED_DrawCircle(63,63,20,C_GREEN);
	OLED_DrawCircle(63,63,10,C_GREEN);
	
//	OLED_DrawLine(0,0,127,127,C_BLUE);
	lineArt();
	
//	OLED_SetSleepModeOn();
//	OLED_PeripheralDeInit();
	
	return 0;
}
