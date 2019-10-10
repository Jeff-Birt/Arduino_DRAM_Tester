#pragma once

// used to add NOP delay between CAS and READ
#define NOP __asm__ __volatile__ ("nop\n\t")
#define DISABLE_REFRESH TIMSK3 = (TIMSK3 & B11111110) // disable refresh timer
#define ENABLE_REFRESH  TIMSK3 = (TIMSK3 & B11111110) | 0x01 // enable refresh timer

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define  DRAM_4116  0
#define  DRAM_4164  1
#define  DRAM_41256 2
#define  DRAM_END   3

//enum UISTATES
//{
//	BEGIN,
//	SELECT,
//	AGAIN,
//	DONE
//};

enum UISTATES
{
	SPLASH,
	BEGIN,
	SELECT,
	TEST,
	AGAIN
};

enum LEDSTATES
{
	LED_ON,
	LED_BLINK,
	LED_OFF
};

enum REFRESH_STATES
{
	REF_ON,
	REF_OFF
};

void writePattern(int maxRow, int maxCol, byte pattern);
unsigned int readPattern(int maxRow, int maxCol, byte pattern);
bool showResults(unsigned int miss, String lable);
void doTests(int maxRow, int maxCol);
