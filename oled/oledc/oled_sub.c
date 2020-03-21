#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oled.h"
//#include "bitmap.h"
#include "font.h"
#include "oled_sub.h"
#include "oled_port.h"

int proc_cmd_init( int argc, char *argv[] )
{
	OLED_Init();
	return 0;
}

int proc_cmd_on( int argc, char *argv[] )
{
	OLED_StartControl();

	OLED_SetSleepModeOff();

	OLED_EndControl();
	
	return 0;
}

int proc_cmd_off( int argc, char *argv[] )
{
	OLED_StartControl();

	OLED_SetSleepModeOn();

	OLED_EndControl();

	return 0;
}

int proc_cmd_cls( int argc, char *argv[] )
{
	color_t color = C_BLACK;
	
	if ( argc >= 3 ) {
		color = (color_t)strtoul( argv[2], NULL, 0 );
	}
	printf("color = 0x%06x\n", color);

	OLED_StartControl();

	OLED_ClearScreen( color );

	OLED_EndControl();

	return 0;
}

int proc_cmd_bmp( int argc, char *argv[] )
{
	int ret = 0;
	
	if ( argc < 3 ) {
		return -1;
	}
	
	OLED_StartControl();

	if ( OLED_DrawBitmapFile( argv[2], 0, 0, 128, 128 ) != 0 ) {
		ret = -2;
	}

	OLED_EndControl();
	
	return ret;
}

int proc_cmd_reset( int argc, char *argv[] )
{
	if ( argc < 3 ) {
		return -1;
	}
	
	if ( strcmp( "on", argv[2] ) == 0 ) {
		OLED_RESET_ASSERT();
	} else if ( strcmp( "off", argv[2] ) == 0 ) {
		OLED_RESET_RELEASE();
	}

	return 0;
}

int proc_cmd_txt( int argc, char *argv[] )
{
	int col, raw;
	if ( argc < 5 ) {
		return -1;
	}
	
	col = atoi( argv[2] );
	raw = atoi( argv[3] );

	OLED_StartControl();
	
	OLED_TextOutBold( argv[4], col, raw );

	OLED_EndControl();

	return 0;
}

int proc_cmd_bright( int argc, char *argv[] )
{
	int brightness;
	if ( argc < 3 ) {
		return -1;
	}
	
	brightness = atoi( argv[2] );
	if ( brightness < 0 || brightness > 15 ) {
		printf( "Error: specify brightness 0..15\n" );
		return -2;
	}

	OLED_StartControl();
	
	OLED_SetMasterCurrentControl( brightness );

	OLED_EndControl();

	return 0;
}

int proc_cmd_kanji( int argc, char *argv[] )
{
	int code, x, y;
	if ( argc < 5 ) {
		return -1;
	}
	
	code = atoi( argv[2] );
	x = atoi( argv[3] );
	y = atoi( argv[4] );

	OLED_StartControl();
	
	OLED_DrawKanji( code, x, y );

	OLED_EndControl();

	return 0;
}

int proc_cmd_vcc( int argc, char *argv[] )
{
	if ( argc < 3 ) {
		return -1;
	}
	
	if ( strcmp( "on", argv[2] ) == 0 ) {
		OLED_VCC_ON();
	} else if ( strcmp( "off", argv[2] ) == 0 ) {
		OLED_VCC_OFF();
	}

	return 0;
}

