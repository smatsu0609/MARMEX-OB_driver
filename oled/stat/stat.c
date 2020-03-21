#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static uint16_t pixBuf[128*128];
static uint16_t statBuf[65536];
static uint16_t rStat[32];
static uint16_t gStat[64];
static uint16_t bStat[32];

uint16_t getColorNum( uint16_t *pix, uint16_t *stat, uint16_t mask )
{
	int i;
	uint16_t p;
	uint16_t count;
	for ( i = 0; i< 65536; i++ ) {
		stat[i] = 0;
	}
	for ( i = 0; i< 128*128; i++ ) {
		p = pix[i] & mask;
		stat[p]++;
	}
	count = 0;
	for ( i = 0; i < 65536; i++ ) {
		if ( stat[i] ) {
			count++;
		}
	}
	return count;
}

int main( int argc, char** argv )
{
	int i;
	int colorCount;
	FILE *fp;
	uint16_t maxColor;
	uint16_t maxCount;
	const uint16_t maskTbl[] = {
		0xffff,
		0xfffe,
		0xff9f,
		0xf7ff,
		0xff9e,
		0xf79f,
		0xf7fe,
		0xf79e,
		0xf79c,
		0xe71e,
		0xe79c,
	};
	if ( argc < 2 ) {
		return 1;
	}

	fp = fopen( argv[1], "rb" );
	if ( fp == NULL ) {
		perror("cannot open input file \n");
		return 1;
	}

	if ( fread( pixBuf, 2, 128*128, fp ) != 128*128 ) {
		fprintf( stderr, "cannot read data\n" );
		fclose(fp);
		return 1;
	}
	fclose(fp);

	for ( i = 0; i < sizeof(maskTbl)/sizeof(maskTbl[0]); i++ ) {
		uint16_t mask = maskTbl[i];
		fprintf ( stdout, "mask = 0x%04x, %d\n", mask, getColorNum( pixBuf, statBuf, mask));
	}	
#if 0
	for ( i = 0; i < 128*128; i++ ) {
		statBuf[i] = 0;
	}
	for ( i = 0; i < 128*128; i++ ) {
		uint8_t r,g,b;
		uint16_t pix;
//		pix = pixBuf[i] & 0xf79e;
		pix = pixBuf[i] & 0xf71c;
		r = pix >> 11;
		g = (pix & 0x07e0)>>5;
		b = pix & 0x001f;
		statBuf[pix]++;
		rStat[r]++;
		gStat[g]++;
		bStat[b]++;
	}
	colorCount = 0;
	maxCount = 0;
	maxColor = 0;
	for ( i = 0; i < 65536; i++ ) {
		if ( statBuf[i] > maxCount ) {
			maxCount = statBuf[i];
			maxColor = i;
		}
		if ( statBuf[i] ) {
			fprintf( stdout, "%04x, %d\n", i, statBuf[i] );
			colorCount++;
		}
	}
	fprintf ( stdout, "%d colors  max = 0x%04x, %d\n", colorCount, maxColor,maxCount );
	for ( i = 0; i < 32; i++ ) {
		fprintf( stdout, "r[%d] = %d\n", i, rStat[i]);
	}
	for ( i = 0; i < 64; i++ ) {
		fprintf( stdout, "g[%d] = %d\n", i, gStat[i]);
	}
	for ( i = 0; i < 32; i++ ) {
		fprintf( stdout, "b[%d] = %d\n", i, bStat[i]);
	}
#endif
	return 0;
}

