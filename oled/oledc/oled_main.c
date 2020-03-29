
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "oled_sub.h"
//test

static struct {
	char *cmd;
	int (*func)(int, char**);
} funcTbl[] = {
	{"init",	proc_cmd_init},
	{"on",		proc_cmd_on},
	{"off",		proc_cmd_off},
	{"bmp",		proc_cmd_bmp},
	{"cls",		proc_cmd_cls},
	{"reset",	proc_cmd_reset},
	{"txt",		proc_cmd_txt},
	{"bright",	proc_cmd_bright},
	{"kanji",	proc_cmd_kanji},
	{"vcc",		proc_cmd_vcc},
};

static void print_usage(const char *prog)
{
/*
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n"
	     "  -v --verbose  Verbose (show tx buffer)\n"
	     "  -p            Send data (e.g. \"1234\\xde\\xad\")\n"
	     "  -N --no-cs    no chip select\n"
	     "  -R --ready    slave pulls low to pause\n"
	     "  -2 --dual     dual transfer\n"
	     "  -4 --quad     quad transfer\n");
*/
	printf("Usage: %s command [argument1 ...]\n", prog);
	puts("Commands:\n"
	     "  init - \n"
	     "  on - \n"
	     "  off - \n"
	     "  bmp filename - \n"
	     "  cls [color] - \n"
	     "  reset - \n"
	     "  txt col raw text - \n"
	     "  bright 0..15 - \n"
	     "  kanji code x y - \n"
	     "  vcc [on|off] - \n"
	     );
	
	exit(1);
}


int main(int argc, char *argv[])
{
	int ret = 0;
	int i;
	
	if( argc < 2 ) {
		print_usage( argv[0] );
	};

	for( i = 0; i < sizeof(funcTbl)/sizeof(funcTbl[0]); i++ ) {
		char *cmd = funcTbl[i].cmd;
		int (*func)(int, char**) = funcTbl[i].func;
		if ( cmd != NULL && func != NULL && strcmp( argv[1], cmd ) == 0 ) {
			if ( func( argc, argv ) == -1 ) {
				print_usage( argv[0] );
			}
			break;
		}
	}
	
	if ( i == sizeof(funcTbl)/sizeof(funcTbl[0]) ) {
		print_usage( argv[0] );
	}
	
	return ret;
}
