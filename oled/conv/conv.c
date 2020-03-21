#include <stdio.h>
#include <stdint.h>

int main(void)
{
	FILE *inFp = NULL;
//	FILE *outFp = NULL;
	uint8_t buf[2];
	uint16_t pix;
	int column = 0;
	#define COLUMN_MAX	(16)
	
	inFp = fopen( "android.bin", "rb" );
	if ( inFp == NULL ) {
		perror("Cannot open input file");
		goto error_exit;
	}
	
//	outFp = fopen( "raspberry-py.txt", "w");
//	if ( outFp == NULL ) {
//		perror("Cannot create output file");
//		goto error_exit;
//	}

	printf( "const uint16_t android128x128[128*128] = {\n" );

	while ( fread( buf, 1, 2, inFp ) == 2 ) {
		pix = (uint16_t)( (buf[1] << 8) + buf[0] );
		if ( column == 0 ) {
			printf( "\t\t" );
		}
		printf( "0x%04x,", pix );
		column++;
		if ( column == COLUMN_MAX ) {
			column = 0;
			printf("\n");
		} else {
			printf(" ");
		}
	}
	
	printf("};\n");


error_exit:
	if ( inFp ) fclose(inFp);
//	if ( outFp ) fclose(outFp);
	return 1;	
}
