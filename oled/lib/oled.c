/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#define OLED_USE_SEND_BUF
#define OLED_SPI_8BIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oled.h"
#include "font.h"
#include "oled_port.h"


#define SPI_TRANSFER_SIZE_LIMIT		(4096)
#define OLED_DATA_COUNT_THRES		(1024)

// Commands
const uint8_t SetDisplayModeAllOff[] = { 0xA4 };
const uint8_t CommandLock1[] = { 0xFD, 0x12 };
const uint8_t CommandLock2[] = { 0xFD, 0xB1 };
const uint8_t SetSleepModeOn[] = { 0xAE };
const uint8_t SetDisplayClockDivideRatio[] = { 0xB3, 0xF1 };
const uint8_t SetMultiplexRatio[] = { 0xCA, 0x7F };
const uint8_t SetDisplayOffset[] = { 0xA2, 0x00 };
const uint8_t SetDisplayStartLine[] = { 0xA1, 0x00 };
#if (OLED_PIX_DEPTH==3)
const uint8_t SetRemapAndColorDepth[] = {0xA0, 0xB4};
#else
const uint8_t SetRemapAndColorDepth[] = { 0xA0, 0x74 };
#endif
const uint8_t SetGPIO[] = { 0xB5, 0x00 };
const uint8_t FunctionSelection[] = { 0xAB, 0x01 };
const uint8_t SetSegmentLowVoltage[] = { 0xB4, 0xA0, 0xB5, 0x55 };
const uint8_t SetContrastCurrent[] = { 0xC1, 0xC8, 0x80, 0xC8 };
const uint8_t SetMasterCurrentControl[] = { 0xC7, 0x0F };
const uint8_t SetPhaseLength[] = { 0xB1, 0x32 };
const uint8_t EnhanceDrivingSchemeCapability[] = { 0xB2, 0xA4, 0x00, 0x00 };
const uint8_t SetPreChargeVoltage[] = { 0xBB, 0x17 };
const uint8_t SetSecondPreChargePeriod[] = { 0xB6, 0x01 };
const uint8_t SetVCOMHVoltage[] = { 0xBE, 0x05 };
const uint8_t SetDisplayModeReset[] = { 0xA6 };
const uint8_t SetSleepModeOff[] = { 0xAF };
const uint8_t WriteRam[] = { 0x5C };
const uint8_t ResetColumnAddress[] = { 0x15, 0x00, 0x7F };
const uint8_t ResetRowAddress[] = { 0x75, 0x00, 0x7F };
const uint8_t SetHorizontalScroll[] = { 0x96, 0x01, 0x00, 0x80, 0x00, 0x01};
const uint8_t StopHorizontalScroll[] = { 0x9E };
const uint8_t StartHorizontalScroll[] = { 0x9F };

const uint8_t SetGammaLut[] = { 0xB8, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x15,
		0x17, 0x19, 0x1B, 0x1D, 0x1F, 0x21, 0x23, 0x25, 0x27, 0x2A, 0x2D, 0x30,
		0x33, 0x36, 0x39, 0x3C, 0x3F, 0x42, 0x45, 0x48, 0x4C, 0x50, 0x54, 0x58,
		0x5C, 0x60, 0x64, 0x68, 0x6C, 0x70, 0x74, 0x78, 0x7D, 0x82, 0x87, 0x8C,
		0x91, 0x96, 0x9B, 0xA0, 0xA5, 0xAA, 0xAF, 0xB4 };
const uint8_t UseBuiltInLinearLut[] = { 0xB9 };

static uint16_t lineBuf[64 * 3];

#if (OLED_PIX_DEPTH==3)
static uint16_t bgColor[3] = {0x0100, 0x0100, 0x0100};
static uint16_t fgColor[3] = {0x013f, 0x013f, 0x013f};
#else
static uint16_t bgColor[2] = { 0x0100, 0x0100 };
static uint16_t fgColor[2] = { 0x01ff, 0x01ff };
#endif


#ifdef OLED_USE_SEND_BUF
#define OLED_SEND_BUF_SIZE		(200000)
static uint16_t oled_send_buf[OLED_SEND_BUF_SIZE];
static uint32_t oled_send_buf_ptr = 0;
static void oled_InitSendBuf(void);
static inline void oled_PushSendBuf(uint16_t *buf, uint32_t length);
static void oled_FlushSendBuf(void);
#endif

// private functions
static void oled_IssueCommand(const uint8_t *bytes, uint32_t length);
static void oled_SetScanDirection(int direction);
static void oled_EncodeColor(uint32_t color, uint16_t *ptr);
static inline void oled_SendData(uint16_t *buf, uint32_t length);
static void oled_DrawChar(uint8_t c, int x, int y);
static void oled_DrawCharB(uint8_t c, int x, int y);
static inline void oled_SendPix(int x0, int y0, color_t color);
static inline void oled_DrawHorizontalLine(int x1, int x2, int y, color_t color);

// MACROS

#define ISSUE_COMMAND(command) oled_IssueCommand(command, sizeof(command)/sizeof(command[0]))

#define SWAP(a,b) {\
	int tmp;\
	tmp = b;\
	b = a;\
	a = tmp;}

#define SET_RECT_MACRO(x1, y1, x2, y2) {\
	uint8_t cmd[3];\
	cmd[0] = 0x15;\
	cmd[1] = (x1);\
	cmd[2] = (x2);\
	ISSUE_COMMAND(cmd);\
	cmd[0] = 0x75;\
	cmd[1] = (y1);\
	cmd[2] = (y2);\
	ISSUE_COMMAND(cmd);}

#define LENGTH_OF(table) (sizeof(table)/sizeof(table[0]))

#define SET_RECT(x1, y1, x2, y2) oled_SetRect(x1, y1, x2, y2)

#ifdef OLED_USE_SEND_BUF
#define INIT_BUF() oled_InitSendBuf()
#define FLUSH_BUF()	oled_FlushSendBuf()
#else
#define INIT_BUF()
#define FLUSH_BUF()
#endif

static void oled_EncodeColor(color_t color, uint16_t *ptr) {
#if (OLED_PIX_DEPTH == 3)
	ptr[0] = ((color >> (24 + 2)) & 0x3f) + 0x0100;
	ptr[1] = ((color >> (16 + 2)) & 0x3f) + 0x0100;
	ptr[2] = ((color >> (8 + 2)) & 0x3f) + 0x0100;
#else
	ptr[0] = ((color >> 24) & 0xf8) + ((color >> (16 + 5)) & 0x07) + 0x100;
	ptr[1] = ((color >> (16 - 3)) & 0xe0) + ((color >> (8 + 3)) & 0x1f) + 0x100;
#endif
}

#ifdef OLED_USE_SEND_BUF
static void oled_InitSendBuf(void)
{
	oled_send_buf_ptr = 0;
}

static inline void oled_PushSendBuf(uint16_t *buf, uint32_t length)
{
	for( ;length > 0; length-- ) {
		if(oled_send_buf_ptr >= OLED_SEND_BUF_SIZE) {
			break;
		}
		oled_send_buf[oled_send_buf_ptr++] = *buf++;
	}
}

#ifdef OLED_SPI_8BIT
/*
static uint32_t oled_ConvertBuff( uint16_t *inBuf, uint32_t inSize, uint8_t *outBuf )
{
	uint32_t i, k;
	uint8_t j, cycleCount, data8 = 0;
	uint16_t data16;
	
	cycleCount = 0;
	data16 = inBuf[0];
	data8 = 0;
	k = 0;
	
	for ( i = 0; i < inSize; i++ ) {
		data16 = inBuf[i];
		for ( j = 0; j < 9; j++ ) {
			data8 <<= 1;
			if( data16 & 0x0100 ) {
				data8 += 1;
			}
			cycleCount ++;
			if( cycleCount == 8 ) {
				cycleCount = 0;
				*(outBuf+k) = data8;
				k++;
				data8 = 0;
			}
			data16 <<= 1;
		}
	}
	if ( cycleCount > 0 ) {
		data8 <<= (8-cycleCount);
		*(outBuf+k) = data8;
		k++;
	}
	return k;

}
*/

static uint32_t oled_ConvertBuff( uint16_t *inBuf, uint32_t inSize, uint8_t *outBuf )
{
	uint8_t fb;
	uint32_t data32;
	uint32_t i, k;
	
	data32 = 0;
	fb = 0;
	k = 0;
	
	for ( i = 0; i < inSize; i++ ) {
		data32 |= ( *inBuf++ << (16+7-fb) );
		fb += 9;
		while( fb >= 8 ) {
			*outBuf++ = data32 >> 24;
			k++;
			data32 <<= 8;
			fb -= 8;
		}
	}
	
	if ( fb != 0 ) {
		*outBuf++ = data32 >> 24;
		k++;
	}
	
	return k;

}
#endif

static void oled_FlushSendBuf(void)
{
#ifdef OLED_SPI_8BIT

	uint32_t residue;
	uint8_t *sendPtr = (uint8_t*)oled_send_buf;
	int lastBlock;
	
	residue = oled_ConvertBuff( oled_send_buf, oled_send_buf_ptr, sendPtr );
	
	while ( residue > 0 ) {
		uint32_t bytesToSend;
		if(residue > 4096 ){
			bytesToSend = 4096;
			lastBlock = 1;
			perror("Error - size over");
			exit(1);
		} else {
			bytesToSend = residue;
			lastBlock = 1;
		}

		OLED_SPI_SEND(sendPtr, bytesToSend);
//		printf("sendBytes = %d\n", bytesToSend);
		sendPtr += bytesToSend;
		residue -= bytesToSend;
	}	
	oled_send_buf_ptr = 0;

	
#else

	uint32_t residue = oled_send_buf_ptr;
	uint16_t *sendPtr = oled_send_buf;

	while( residue > 0 ) {
		uint32_t wordsToSend;
		if(residue > 2048 ){
			wordsToSend = 2048;
		} else {
			wordsToSend = residue;
		}
		OLED_SPI_SEND(sendPtr, wordsToSend);
		sendPtr += wordsToSend;
		residue -= wordsToSend;
	}
	
	oled_send_buf_ptr = 0;
#endif

}

#endif

static void oled_IssueCommand(const uint8_t *bytes, uint32_t length)
{
	uint32_t i;
	for (i = 0; i < length; i++) {
		lineBuf[i] = (uint16_t) bytes[i];
		if (i > 0) {
			lineBuf[i] |= 0x0100;
		}
	}
	oled_SendData(lineBuf, length);
}

//
static void oled_SetRect(int x1, int y1, int x2, int y2) {
	SET_RECT_MACRO(x1, y1, x2, y2);
}

static inline void oled_SendData(uint16_t *buf, uint32_t length) {
#ifdef OLED_USE_SEND_BUF
	oled_PushSendBuf(buf, length);
#else
    OLED_SPI_SEND(buf, length);
#endif
}

static void oled_SetAddressIncrement(int vertical) {
    uint8_t cmd[2];
    cmd[0] = SetRemapAndColorDepth[0];
    cmd[1] = SetRemapAndColorDepth[1];
    
    if(vertical != 0) {
        cmd[1] |= 0x01;
    } else {
        cmd[1] &= ~0x01;
    }
    ISSUE_COMMAND(cmd);
}

void OLED_Init(void) {
	// Initialize GPIO
    OLED_GPIO_INIT();

    // Open SPI
    OLED_SPI_OPEN();
    
	INIT_BUF();
    // OLED Vcc OFF
    OLED_VCC_OFF();
    
    // OLED_Reset
    OLED_RESET_ASSERT();
    OLED_DELAY(100);
    OLED_RESET_RELEASE();
    
    // Initialize Panel Settings
    OLED_PanelInit();

    // OLED Vcc ON
    OLED_VCC_ON();
    
    OLED_DELAY(200);
    
	ISSUE_COMMAND(SetSleepModeOff);
    FLUSH_BUF();

	// Close SPI
	OLED_SPI_CLOSE();
	
    OLED_DELAY(100);

//	OLED_SPI_CLOSE();
}

void OLED_PanelInit(void) {
	
	ISSUE_COMMAND(SetDisplayModeAllOff);
	ISSUE_COMMAND(CommandLock1);
	ISSUE_COMMAND(CommandLock2);
	ISSUE_COMMAND(SetSleepModeOn);
	ISSUE_COMMAND(SetDisplayClockDivideRatio);
	ISSUE_COMMAND(SetMultiplexRatio);
	ISSUE_COMMAND(SetDisplayOffset);
	ISSUE_COMMAND(SetDisplayStartLine);
	ISSUE_COMMAND(SetRemapAndColorDepth);
	ISSUE_COMMAND(SetGPIO);
	ISSUE_COMMAND(FunctionSelection);
	ISSUE_COMMAND(SetSegmentLowVoltage);
	ISSUE_COMMAND(SetContrastCurrent);
	ISSUE_COMMAND(SetMasterCurrentControl);
	// set gamma UseBuiltInLinearLut
	ISSUE_COMMAND(SetGammaLut);
//	ISSUE_COMMAND(UseBuiltInLinearLut);
	ISSUE_COMMAND(SetPhaseLength);
	ISSUE_COMMAND(EnhanceDrivingSchemeCapability);
	ISSUE_COMMAND(SetPreChargeVoltage);
	ISSUE_COMMAND(SetSecondPreChargePeriod);
	ISSUE_COMMAND(SetVCOMHVoltage);
	ISSUE_COMMAND(SetDisplayModeReset);
	FLUSH_BUF();

	// clear screen
	OLED_ClearScreen(C_BLACK);

}

void OLED_SetSleepModeOff(void)
{
	ISSUE_COMMAND(SetSleepModeOff);
	FLUSH_BUF();
}

void OLED_SetSleepModeOn(void)
{
	ISSUE_COMMAND(SetSleepModeOn);
	FLUSH_BUF();
}

// direction 0: horizontal, 1:vertical
static inline void oled_SetScanDirection(int direction)
{
	uint8_t cmd[sizeof(SetRemapAndColorDepth)];
	cmd[0] = SetRemapAndColorDepth[0];
	cmd[1] = SetRemapAndColorDepth[0] & 0xfe;
	if (direction != 0) {
		cmd[1] |= 0x01;
	}
	ISSUE_COMMAND(cmd);
}

void OLED_SetMasterCurrentControl(uint8_t current_value)
{
	uint8_t cmd[sizeof(SetMasterCurrentControl)];
	cmd[0] = SetMasterCurrentControl[0];
	cmd[1] = (current_value & 0x0f);
	ISSUE_COMMAND(cmd);
    FLUSH_BUF();
}

void OLED_SetContrastCurrent(uint8_t red, uint8_t green, uint8_t blue)
{
	// default 0xc8 0x80 0xc8
	uint8_t cmd[sizeof(SetContrastCurrent)];
	cmd[0] = SetContrastCurrent[0];
	cmd[1] = red;
	cmd[2] = green;
	cmd[3] = blue;
	ISSUE_COMMAND(cmd);
    FLUSH_BUF();
}

void OLED_SetDefaultContrast(void)
{
	ISSUE_COMMAND(SetContrastCurrent);
    FLUSH_BUF();
}

void OLED_SetHorizontalScroll( uint8_t direction, uint8_t startRow, uint8_t numRow, uint8_t mode )
{
    uint8_t cmd[sizeof(SetHorizontalScroll)];

    memcpy( cmd, SetHorizontalScroll, sizeof(cmd) );
    cmd[1] = direction; // H-scroll direction
    cmd[2] = startRow;  // start row address
    cmd[3] = numRow;    // number of rows to be H-scrolled
    cmd[4] = 0x00;      // scrolling time interval
    cmd[5] = mode;      // mode
    
    ISSUE_COMMAND(cmd);
    FLUSH_BUF();
}

void OLED_StartHorizontalScroll(void)
{
    ISSUE_COMMAND(StartHorizontalScroll);
    FLUSH_BUF();
}

void OLED_StopHorizontalScroll(void) {
    ISSUE_COMMAND(StopHorizontalScroll);
    FLUSH_BUF();
}

void OLED_ClearScreen(color_t color) {
	int i;
	uint16_t drawColor[OLED_PIX_DEPTH];

	oled_EncodeColor(color, drawColor);
	// reset pointers
	ISSUE_COMMAND(ResetColumnAddress);
	ISSUE_COMMAND(ResetRowAddress);
	// WriteRam Command
	ISSUE_COMMAND(WriteRam);

	for (i = 0; i < OLED_PIXELS; i++) {
		// FIFO 8frames halfempty 4frames
		oled_SendData(drawColor, LENGTH_OF(drawColor));
		if ( i % OLED_DATA_COUNT_THRES == OLED_DATA_COUNT_THRES - 1 ) {
			FLUSH_BUF();
			ISSUE_COMMAND(WriteRam);
		}
	}
	
	FLUSH_BUF();
}

void OLED_DrawChar(uint8_t c, int x, int y)
{
	oled_DrawChar(c, x, y);
	FLUSH_BUF();
}


static void oled_DrawChar(uint8_t c, int x, int y)
{
	const uint8_t *font_p;
	uint8_t mask;
	int i, j;

	font_p = Font8x5[c];

	// 表示領域
	SET_RECT(x, y, x+5, y+7);

	// WriteRam Command
	ISSUE_COMMAND(WriteRam);

	// 48pixels
	mask = 0x01;
	for (j = 0; j < 8; j++) {
		for (i = 0; i < 6; i++) {
			if ((font_p[i] & mask) != 0) {
				oled_SendData(fgColor, LENGTH_OF(fgColor));
			} else {
				oled_SendData(bgColor, LENGTH_OF(fgColor));
			}
		}
		mask <<= 1;
	}
}

void OLED_DrawCharB(uint8_t c, int x, int y)
{
	oled_DrawCharB(c, x, y);
	FLUSH_BUF();
}

static void oled_DrawCharB(uint8_t c, int x, int y)
{
	const uint8_t *font_p;
	uint8_t mask;
	int i, j;
    uint8_t currBit, prevBit;

	font_p = Font8x5[c];

	// 表示領域
	SET_RECT(x, y, x+6, y+7);

	// WriteRam Command
	ISSUE_COMMAND(WriteRam);

	// 48pixels
	mask = 0x01;
	for (j = 0; j < 8; j++) {
        prevBit = 0;
		for (i = 0; i < 6; i++) {
            currBit = font_p[i] & mask;
			if (currBit != 0 || prevBit != 0) {
				oled_SendData(fgColor, LENGTH_OF(fgColor));
			} else {
				oled_SendData(bgColor, LENGTH_OF(fgColor));
			}
            prevBit = currBit;
		}
        if ( prevBit != 0 ) {
			oled_SendData(fgColor, LENGTH_OF(fgColor));
		} else {
			oled_SendData(bgColor, LENGTH_OF(fgColor));
		}
		mask <<= 1;
	}
}

void OLED_DrawKanji(uint8_t c, int x, int y)
{
	const uint8_t *font_p;
	uint16_t shift;
	int i, j;

	font_p = font16x16[c];

	// SetRect
	SET_RECT(x, y, x+15, y+15);

	// WriteRam Command
	ISSUE_COMMAND(WriteRam);

	// 256pixels
	for (i = 0; i < 16; i++) {
		shift = ((uint16_t) font_p[i * 2] << 8) + (uint16_t) font_p[i * 2 + 1];
		for (j = 0; j < 16; j++) {
			if ((shift & 0x8000) != 0) {
				oled_SendData(fgColor, LENGTH_OF(fgColor));
			} else {
				oled_SendData(bgColor, LENGTH_OF(bgColor));
			}
			shift <<= 1;
		}
	}
	FLUSH_BUF();
}

void OLED_TextOut(const char *string, int col, int raw)
{
	OLED_DrawString(string, col*6, raw*8);
}

void OLED_TextOutBold(const char *string, int col, int raw)
{
	OLED_DrawStringBold(string, col*7, raw*8);
}

void OLED_DrawString(const char *string, int x, int y)
{
	int length;
	length = strlen(string);
	OLED_DrawChars((const uint8_t*) string, length, x, y);
}

void OLED_DrawStringBold(const char *string, int x, int y)
{
	int length;
	length = strlen(string);
	OLED_DrawCharsBold((const uint8_t*) string, length, x, y);
}

void OLED_DrawChars(const uint8_t *chars, int length, int x, int y)
{
	int i;
	for (i = 0; i < length; i++) {
		oled_DrawChar(chars[i], x+i*6, y);
	}
	FLUSH_BUF();
}

void OLED_DrawCharsBold(const uint8_t *chars, int length, int x, int y)
{
	int i;
	for (i = 0; i < length; i++) {
		oled_DrawCharB(chars[i], x+i*7, y);
	}
	FLUSH_BUF();
}

void OLED_SetFgColor(uint32_t color)
{
	oled_EncodeColor(color, fgColor);
}

void OLED_SetBgColor(uint32_t color)
{
	oled_EncodeColor(color, bgColor);
}

void OLED_SetFgBgColor(uint32_t fg, uint32_t bg)
{
	oled_EncodeColor(fg, fgColor);
	oled_EncodeColor(bg, bgColor);
}

void OLED_FillRect(int x1, int y1, int x2, int y2, uint32_t color)
{
	int i, length;
	uint16_t drawColor[OLED_PIX_DEPTH];

	oled_EncodeColor(color, drawColor);
	if (x1 > x2) {
		SWAP(x1, x2);
	}
	if (y1 > y2) {
		SWAP(y1, y2);
	}
	length = (x2 - x1 + 1) * (y2 - y1 + 1);

	// Set rect
	SET_RECT(x1, y1, x2, y2);

	// WriteRam Command
	ISSUE_COMMAND(WriteRam);

	for (i = 0; i < length; i++) {
		oled_SendData(drawColor, LENGTH_OF(drawColor));
		if ( i % OLED_DATA_COUNT_THRES == OLED_DATA_COUNT_THRES - 1 ) {
			FLUSH_BUF();
			ISSUE_COMMAND(WriteRam);
		}
	}

	FLUSH_BUF();
}

void OLED_DrawRect(int x1, int y1, int x2, int y2, uint32_t color)
{
	int i;
	uint16_t drawColor[OLED_PIX_DEPTH];

	oled_EncodeColor(color, drawColor);
	if (x1 > x2) {
		SWAP(x1, x2);
	}
	if (y1 > y2) {
		SWAP(y1, y2);
	}
	// Top line
	SET_RECT(x1, y1, x2, y1);
	ISSUE_COMMAND(WriteRam);
	for (i = x1; i <= x2; i++) {
		oled_SendData(drawColor, LENGTH_OF(drawColor));
	}
	// Bottom line
	SET_RECT(x1, y2, x2, y2);
	ISSUE_COMMAND(WriteRam);
	for (i = x1; i <= x2; i++) {
		oled_SendData(drawColor, LENGTH_OF(drawColor));
	}

	// Change direction
//	oled_SetScanDirection(1);

	// Left line
	SET_RECT(x1, y1, x1, y2);
	ISSUE_COMMAND(WriteRam);
	for (i = y1; i <= y2; i++) {
		oled_SendData(drawColor, LENGTH_OF(drawColor));
	}

	// Right line
	SET_RECT(x2, y1, x2, y2);
	ISSUE_COMMAND(WriteRam);
	for (i = y1; i <= y2; i++) {
		oled_SendData(drawColor, LENGTH_OF(drawColor));
	}
	
	FLUSH_BUF();
}

void OLED_DrawVerticalLine(int x, int y1, int y2, color_t color)
{
	uint16_t drawColor[OLED_PIX_DEPTH];
	int i;

	oled_EncodeColor(color, drawColor);
	if (y1 > y2) {
		SWAP(y1, y2);
	}
	SET_RECT(x, y1, x, y2);
	ISSUE_COMMAND(WriteRam);
	for (i = y1; i <= y2; i++) {
		oled_SendData(drawColor, LENGTH_OF(drawColor));
	}

	FLUSH_BUF();
}

void OLED_SetColumn(int x)
{
	uint8_t cmd[3];
	cmd[0] = 0x15;
	cmd[1] = (uint8_t)x;
	cmd[2] = (uint8_t)x;
    ISSUE_COMMAND(cmd);
    FLUSH_BUF();
}    

void OLED_DrawVerticalLine2(int y1, int y2, color_t color)
{
	uint16_t drawColor[OLED_PIX_DEPTH];
	int i;
    uint8_t cmd[3];

	oled_EncodeColor(color, drawColor);
	if (y1 > y2) {
		SWAP(y1, y2);
	}
//	SET_RECT(x, y1, x, y2);
    cmd[0] = 0x75;
	cmd[1] = (uint8_t)y1;
	cmd[2] = (uint8_t)y2;
	ISSUE_COMMAND(cmd);

	ISSUE_COMMAND(WriteRam);
	for (i = y1; i <= y2; i++) {
		oled_SendData(drawColor, LENGTH_OF(drawColor));
	}
	FLUSH_BUF();
}


static inline void oled_DrawHorizontalLine(int x1, int x2, int y, color_t color)
{
	uint16_t drawColor[OLED_PIX_DEPTH];
	int i;

	oled_EncodeColor(color, drawColor);
	if (x1 > x2) {
		SWAP(x1, x2);
	}
	SET_RECT(x1, y, x2, y);
	ISSUE_COMMAND(WriteRam);
	for (i = x1; i <= x2; i++) {
		oled_SendData(drawColor, LENGTH_OF(drawColor));
	}
}

void OLED_DrawHorizontalLine(int x1, int x2, int y, color_t color)
{
	oled_DrawHorizontalLine( x1, x2, y, color );
	FLUSH_BUF();
}

void OLED_DrawBitmap(const uint16_t *bmp, int x, int y, int width, int height) {
	uint16_t pix[OLED_PIX_DEPTH];
	int i;
	int length;
	uint16_t color;

	length = width * height;

	SET_RECT(x, y, x+width-1, y+height-1);
	ISSUE_COMMAND(WriteRam);
	for (i = 0; i < length; i++) {
		color = *bmp++;
		if (color == 0x0000) {
			oled_SendData(bgColor, LENGTH_OF(bgColor));
		} else {
#if (OLED_PIX_DEPTH == 3)
			pix[0] = ((color >> 10) & 0x3e) + 0x0100;
			pix[1] = ((color >> 5) & 0x3f) + 0x0100;
			pix[2] = ((color << 1) & 0x3e) + 0x0100;
#elif (OLED_PIX_DEPTH == 2)
			pix[0] = (color >> 8) + 0x0100;
			pix[1] = (color & 0xff) + 0x0100;
#endif
			oled_SendData(pix, LENGTH_OF(pix));
		}
		if ( i % OLED_DATA_COUNT_THRES == OLED_DATA_COUNT_THRES - 1 ) {
			FLUSH_BUF();
			ISSUE_COMMAND(WriteRam);
		}
	}
	FLUSH_BUF();
}

void OLED_DrawBitmap256(const uint8_t *bmp, const uint16_t *palette, int x,
		int y, int width, int height) {
	uint16_t pix[OLED_PIX_DEPTH];
	int i;
	int length;
	uint16_t color;

	length = width * height;

	SET_RECT(x, y, x+width-1, y+height-1);
	ISSUE_COMMAND(WriteRam);
	for (i = 0; i < length; i++) {
		color = palette[*bmp++];
		if (color == 0x0000) {
			oled_SendData(bgColor, LENGTH_OF(bgColor));
		} else {
#if (OLED_PIX_DEPTH == 3)
			pix[0] = ((color >> 10) & 0x3e) + 0x0100;
			pix[1] = ((color >> 5) & 0x3f) + 0x0100;
			pix[2] = ((color << 1) & 0x3e) + 0x0100;
#elif (OLED_PIX_DEPTH == 2)
			pix[0] = (color >> 8) + 0x0100;
			pix[1] = (color & 0xff) + 0x0100;
#endif
			oled_SendData(pix, LENGTH_OF(pix));
		}
		if ( i % OLED_DATA_COUNT_THRES == OLED_DATA_COUNT_THRES - 1 ) {
			FLUSH_BUF();
			ISSUE_COMMAND(WriteRam);
		}
	}
	FLUSH_BUF();
}

void OLED_DrawBitmap16(const uint8_t *bmp, const uint16_t *palette, int x,
		int y, int width, int height) {
	uint16_t pix[OLED_PIX_DEPTH];
	int i, j;
	uint16_t color;
	uint8_t idx;
	uint8_t shift = 0;

	SET_RECT(x, y, x+width-1, y+height-1);
	ISSUE_COMMAND(WriteRam);
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			if ((i & 1) == 0) {
				shift = *bmp++;
			} else {
				shift <<= 4;
			}
			idx = shift >> 4;
			color = palette[idx];
			if (color == 0x0000) {
				oled_SendData(bgColor, LENGTH_OF(bgColor));
			} else {
#if (OLED_PIX_DEPTH == 3)
				pix[0] = ((color >> 10) & 0x3e) + 0x0100;
				pix[1] = ((color >> 5) & 0x3f) + 0x0100;
				pix[2] = ((color << 1) & 0x3e) + 0x0100;
#elif (OLED_PIX_DEPTH == 2)
				pix[0] = (color >> 8) + 0x0100;
				pix[1] = (color & 0xff) + 0x0100;
#endif
				oled_SendData(pix, LENGTH_OF(pix));
			}
		}
	}
	FLUSH_BUF();
}

void OLED_DrawBitmap4(const uint8_t *bmp, const uint16_t *palette, int x, int y,
		int width, int height) {
	uint16_t pix[OLED_PIX_DEPTH];
	int i, j;
	uint16_t color;
	uint8_t idx;
	uint8_t shift = 0;

	SET_RECT(x, y, x+width-1, y+height-1);
	ISSUE_COMMAND(WriteRam);
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			if ((i & 0x3) == 0) {
				shift = *bmp++;
			} else {
				shift <<= 2;
			}
			idx = shift >> 6;
			color = palette[idx];
			if (color == 0x0000) {
				oled_SendData(bgColor, LENGTH_OF(bgColor));
			} else {
#if (OLED_PIX_DEPTH == 3)
				pix[0] = ((color >> 10) & 0x3e) + 0x0100;
				pix[1] = ((color >> 5) & 0x3f) + 0x0100;
				pix[2] = ((color << 1) & 0x3e) + 0x0100;
#elif (OLED_PIX_DEPTH == 2)
				pix[0] = (color >> 8) + 0x0100;
				pix[1] = (color & 0xff) + 0x0100;
#endif
				oled_SendData(pix, LENGTH_OF(pix));
			}
		}
	}
	FLUSH_BUF();
}

/* 傾きが１の場合、最も時間がかかる. to draw n dots 
   words_to_send = 3+(3+1+2)*n = 3+6n
   duration = (3+6n)*10 * 1/16.5MHz
        10dots -> 38usec
*/        
void OLED_DrawLine(int x0, int y0, int x1, int y1, color_t color)
{
    int i;

    int dx, dy; /* 二点間の距離 */
    int sx, sy; /* 二点の方向 */
	uint16_t drawColor[OLED_PIX_DEPTH];
	uint8_t cmd[3];

	oled_EncodeColor(color, drawColor);

    dx = ( x1 > x0 ) ? x1 - x0 : x0 - x1;
    dy = ( y1 > y0 ) ? y1 - y0 : y0 - y1;

//    sx = ( x1 > x0 ) ? 1 : -1;
//    sy = ( y1 > y0 ) ? 1 : -1;

    /* 傾きが1より小さい場合 */
    if ( dx > dy ) {
        int E = -dx;
        int prev_y = -1;
        if( x0 >= x1 ) {
            SWAP(x0, x1);
            SWAP(y0, y1);
        }
        sx = 1;
        sy = ( y1 > y0 ) ? 1 : -1;
//        oled_SetAddressIncrement(0); // Horizontal
    	cmd[0] = 0x15;
    	cmd[1] = (uint8_t)x0;
    	cmd[2] = (uint8_t)x1;
        ISSUE_COMMAND(cmd);

        for ( i = 0 ; i <= dx ; i++ ) {
            if ( prev_y != y0 ) {
            	cmd[0] = 0x75;
            	cmd[1] = (uint8_t)y0;
            	cmd[2] = (uint8_t)y0;
                ISSUE_COMMAND(cmd);
        	    ISSUE_COMMAND(WriteRam);
                prev_y = y0;
            }
		    oled_SendData(drawColor, LENGTH_OF(drawColor));
            x0 += sx;
            E += 2 * dy;
            if ( E >= 0 ) {
                y0 += sy;
                E -= 2 * dx;
            }
        }
    /* 傾きが1以上の場合 */
    } else {
        int E = -dy;
        int prev_x = -1;
        if( y0 >= y1 ) {
            SWAP(x0, x1);
            SWAP(y0, y1);
        }
        sx = ( x1 > x0 ) ? 1 : -1;
        sy = 1;
//        oled_SetAddressIncrement(1); // Vertical
    	cmd[0] = 0x75;
    	cmd[1] = (uint8_t)y0;
    	cmd[2] = (uint8_t)y1;
        ISSUE_COMMAND(cmd);
        for ( i = 0 ; i <= dy ; i++ ) {
            if ( prev_x != x0 ) {
            	cmd[0] = 0x15;
            	cmd[1] = (uint8_t)x0;
            	cmd[2] = (uint8_t)x0;
                ISSUE_COMMAND(cmd); // 3 words
        	    ISSUE_COMMAND(WriteRam); // 1 word
                prev_x = x0;
            }
		    oled_SendData(drawColor, LENGTH_OF(drawColor)); // 2words
            y0 += sy;
            E += 2 * dx;
            if ( E >= 0 ) {
                x0 += sx;
                E -= 2 * dy;
            }
        }
    }
//    oled_SetAddressIncrement(0); // Horizontal
	FLUSH_BUF();
}

void OLED_FillCircle(int x0, int y0, int r, color_t color)
{
    int x = r;
    int y = 0;
    int F = -2 * r + 3;
    int prev_x = 0;

    while ( x >= y ) {
        oled_DrawHorizontalLine( x0 - x, x0 + x, y0 + y, color);
        oled_DrawHorizontalLine( x0 - x, x0 + x, y0 - y, color);
        if( prev_x != x ) {
            oled_DrawHorizontalLine( x0 - y, x0 + y, y0 + x, color);
            oled_DrawHorizontalLine( x0 - y, x0 + y, y0 - x, color);
            prev_x = x;
        }
        FLUSH_BUF();
        if ( F >= 0 ) {
            x--;
            F -= 4 * x;
        }
        y++;
        F += 4 * y + 2;
    }
}

void OLED_DrawCircle(int x0, int y0, int r, color_t color)
{
    int x = r;
    int y = 0;
    int F = -2 * r + 3;

    while ( x >= y ) {
        oled_SendPix( x0 + x, y0 + y, color );
        oled_SendPix( x0 - x, y0 + y, color );
        oled_SendPix( x0 + x, y0 - y, color );
        oled_SendPix( x0 - x, y0 - y, color );
        oled_SendPix( x0 + y, y0 + x, color );
        oled_SendPix( x0 - y, y0 + x, color );
        oled_SendPix( x0 + y, y0 - x, color );
        oled_SendPix( x0 - y, y0 - x, color );
        if ( F >= 0 ) {
            x--;
            F -= 4 * x;
        }
        y++;
        F += 4 * y + 2;
    }
	FLUSH_BUF();

}

static inline void oled_SendPix(int x0, int y0, color_t color)
{
	uint16_t drawColor[OLED_PIX_DEPTH];
	oled_EncodeColor(color, drawColor);
    if(x0 < 0 || x0 >= OLED_WIDTH || y0 < 0 || y0 >= OLED_HEIGHT){
        return;
    }
    SET_RECT(x0, y0, x0, y0); // 6 words
    ISSUE_COMMAND(WriteRam); // 1 word
    oled_SendData(drawColor, LENGTH_OF(drawColor)); // 2 words
}

void OLED_DrawPix(int x0, int y0, color_t color)
{
	oled_SendPix(x0, y0, color);
	FLUSH_BUF();
}

int OLED_DrawBitmapFile( const char *filename, int x, int y, int width, int height )
{
	FILE *fp;
	uint16_t *pixbuf = NULL;
	int ret = 0;
	uint8_t tmpbuf[2];
	int i;
	int pixnum;
	
	pixnum = width * height;
	
	fp = fopen( filename, "rb" );
	if ( fp == NULL ) {
		perror( "Cannot open input file" );
		ret = -1;
		goto error_exit;
	}
	
	pixbuf = (uint16_t *)malloc( pixnum * sizeof(uint16_t) );
	if ( pixbuf == NULL ) {
		perror( "Cannot allocate buffer" );
		ret = -2;
		goto error_exit;
	}
	
	// read data from specified file
	for ( i = 0; i < pixnum; i++ ) {
		uint16_t pix;
		if ( fread( tmpbuf, 1, 2, fp ) != 2 ) {
			// pix count is not enough for width,heght
			break;
		}
		pix = (uint16_t)((tmpbuf[1]<<8)+tmpbuf[0]);
		pixbuf[i] = pix;
	}
	
	// Draw bitmap
	OLED_DrawBitmap( pixbuf, x, y, width, height );
	
	
error_exit:
	if ( fp ) {
		fclose( fp );
	}
	if ( pixbuf ) {
		free( pixbuf );
	}
	return ret;
}

void OLED_StartControl( void )
{
	OLED_SPI_OPEN();
}

void OLED_EndControl( void )
{
	OLED_SPI_CLOSE();
}

/* [] END OF FILE */
