#pragma once

// used to add NOP delay between CAS and READ
#define NOP __asm__ __volatile__ ("nop\n\t")
#define DISABLE_REFRESH TIMSK3 = (TIMSK3 & B11111110) // disable refresh timer
#define ENABLE_REFRESH  TIMSK3 = (TIMSK3 & B11111110) | 0x01 // enable refresh timer

enum UISTATES
{
	BEGIN,
	SELECT,
	AGAIN,
	DONE
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
