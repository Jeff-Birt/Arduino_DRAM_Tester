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

// states the UI can be in
enum UISTATES
{
	SPLASH,
	BEGIN,
	SELECT,
	TEST,
	AGAIN
};

// states LED can be in
enum LEDSTATES
{
	LED_ON,
	LED_BLINK,
	LED_OFF
};

// DRAM refresh state
enum REFRESH_STATES
{
	REF_ON,
	REF_OFF
};


void writePattern(int maxRow, int maxCol, byte pattern); // write pattern to DRAM
unsigned int readPattern(int maxRow, int maxCol, byte pattern); // read DRAM, pattern match
bool showResults(bool fail, unsigned int miss, String lable); // display test results
void doTests(int maxRow, int maxCol); // do set of tests
void clearDisplay(); // helper to clear display set cursor position
int getBtn(); // returns analog button pressed
