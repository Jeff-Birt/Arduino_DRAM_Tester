/*
 Arduino_DRAM_Tester_0.6, 4164 and 41256 DRAM tester
 Jeffrey T. Birt (Hey Birt!) http://www.soigeneris.com , http://www.youtube.com/c/HeyBirt
 Based on project by ChronWorks http://www.chronworks.com/DRAM/
*/

// NOTES:
// Mega 8VDC external input, draw 100ma idle, 150ma testing 41256
// 4116 -5V->200ua, +5V->1ma, 12V->35ma

#include "All_Defs.h"
#ifdef ARDUINO_AVR_MEGA2560
#include "Mega_Defs.h"
#elif ARDUINO_AVR_UNO
#include "Uno_Defs.h"
#elif ARDUINO_AVR_NANO
#include "Nano_Defs.h"
#else //ARDUINO_AVR_MICRO
#include "Micro_Defs2.h"
#endif

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h> 

LEDSTATES ledstate = LED_OFF;
UISTATES uistate = SPLASH;					// start out showing our splash screen
volatile REFRESH_STATES refstate = REF_OFF; // auto reffresh off
volatile int ref_maxRow = 512;				// maxium rows of DRAM (for refresh)
int ref_row = 0;							// ROW address for auto refresh
int blinkCounter = 0;						// Blink counter for green LED
static const int keypadPin = A5;			// analog input for keypad

// about 3.5 lines, 11-12 columns with fount used
#define OLED_RESET     -1					// OLED Reset pin#, -1 menas none
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // 

String DRAM_NAME[3] = { "4116", "4164", "41256" };	// could use PROGMEM to put these in flash
int DRAM_MAX_ROW[3] = { 127, 255, 511 };			// corresponding max #rows
int dramSel = DRAM_4116;							// selected DRAM type


// the setup function runs once when you press reset or power the board
// Initalize IO to standby state, enable refresh timer
void setup()
{
	initStandby();

	TCCR3B = (TCCR3B & B11111000) | 0x03; // set refresh rate to 2ms
	ENABLE_REFRESH;					// enable refresh interrupt timer

	Serial.begin(9600);				// Only used for debugging
	Serial.println("Setup");

	// set up OLED display
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
	{
		Serial.println(F("SSD1306 allocation failed"));
		for (;;); // Don't proceed, loop forever
	}

	display.display();				// Display Adafruit Splash
	delay(2000);					// Pause for 2 seconds

	display.setFont(&FreeMono9pt7b);// Nice sized font
	display.setTextSize(1);			// Normal 1:1 pixel scale
	display.setTextColor(WHITE);	// Draw white text
	display.cp437(true);			// Use full 256 char 'Code Page 437' font
}

// We use the loop function for the UI
void loop()
{
	int btn = 0;
	delay(200);		// key repeat timer

	switch (uistate)
	{
	case SPLASH:
		clearDisplay();
		display.println(F("DRAM Test"));
		display.display();
		delay(2000);

		dramSel = DRAM_4116;
		uistate = BEGIN;
		break;

	case BEGIN:
		unsigned int miss;	// miss counter, incorrect bits
		ledstate = LED_OFF;

		clearDisplay();
		display.println(F("DRAM type"));

		// print out DRAM list starting with currently selected
		// also, put a ">" beside currently selected 
		for (int i = 0; i < DRAM_END; i++)
		{
			if (i >= dramSel)
			{
				i == dramSel ? display.print(">") : display.print(" ");
				display.println(DRAM_NAME[i]);
			}
		}
		display.display();
		uistate = SELECT;
		break;

	case SELECT:
		btn = getBtn();
		if (btn == 1)
		{
			dramSel++;
			if (dramSel > 2) { dramSel = 0; }
			uistate = BEGIN;
		}
		else if (btn == 2)
		{
			uistate = TEST;
		}
		break;

	case TEST:
		clearDisplay();
		display.println("Test " + DRAM_NAME[dramSel]);
		display.display();

		ref_maxRow = DRAM_MAX_ROW[dramSel];
		ledstate = LED_BLINK;
		doTests(ref_maxRow, ref_maxRow);

		display.println(F("Again?"));
		display.println(F("A-Yes  B-No"));
		display.display();
		uistate = AGAIN;
		break;

	case AGAIN:
		btn = getBtn();
		if (btn == 1)
		{
			uistate = TEST;
		}
		else if (btn == 2)
		{
			uistate = SPLASH;
		}
		break;
	}

}

// returns button pressed, no btn=0, A=1, B=2, A&B=3
int getBtn()
{
	int aIn = analogRead(keypadPin);
	int result = 0;

	// if > 975 no key pressed
	if (aIn < 975)	
	{
		delay(10); // check again to filter out noise
		aIn = analogRead(keypadPin);

		if (aIn < 975)			// no key
		{
			if (aIn > 875)		// 'A' key
			{
				result = 1;
			}
			else if (aIn > 775)	// 'B' key
			{
				result = 2;
			}
			else if (aIn > 675)	// 'A' & 'B' keys
			{
				result = 3;
			}
		}
	}

	return result;
}

// Runs a series of checkboard and walking 1/0 tests
// maxRow, maxCol -> maximum row and column DRAM has
void doTests(int maxRow, int maxCol)
{
	bool failure = false;
	int miss = 0;
	int reps = 1;

	initTest();
	refstate = REF_ON; // need refresh timer running before starting tests
	delay(1000);	   // time for DC-DC converter to come up

	// test with checkerboard patterns
	failure |= runTest(reps, 0x55, maxRow, maxCol, F("Fill 0x55"));
	delay(2000);

	failure |= runTest(reps, 0xAA, maxRow, maxCol, F("Fill 0xAA"));
	delay(2000);

	// test with walking 1/0 tests
	failure |= walkTest(reps, maxRow, maxCol, true, F("March 0x01"));
	delay(2000);

	failure |= walkTest(reps, maxRow, maxCol, false, F("March 0x00"));
	delay(2000);

	// display final results
	clearDisplay();
	failure ? display.println(F("Failed")) : display.println(F("Passed"));
	display.display();

	refstate = REF_OFF;
	ledstate = failure ? LED_OFF : LED_ON;
	initStandby();
}

// runs an individual checkerboard type test
bool runTest(int reps, int pattern, int maxRow, int maxCol, String lable)
{
	unsigned int miss = 0;
	bool fail;

	clearDisplay();
	display.println(lable);
	display.display();

	// run test 'reps' times
	for (int i = 0; i < reps; i++)
	{
		writePattern(maxRow, maxCol, pattern);
		delay(1000);
		miss += readPattern(maxRow, maxCol, pattern);
	}

	fail = (miss != 0) ? true : false;	// miss > 0 is test failure
	fail ? display.println(F("Fail ")) : display.println(F("Passed "));
	
	miss = (fail) ? miss / reps : miss;	// average misses/test
	display.println(miss);
	display.display();
	
	return fail;
}

// fill memory with 1 or 0, step through each cell check for filled value
// write opposite value, check for new value. state == fill value
bool walkTest(int reps, int maxRow, int maxCol, int state, String lable)
{
	int row, col, miss = 0;
	bool fail = false;

	// first, fill all with state
	int value = (state == 0) ? 0x00 : 0xFF;
	String tempLable = " Fill 0x" + String(value, HEX);
	runTest(1, value, maxRow, maxCol, tempLable);

	clearDisplay();
	display.println(lable);
	display.display();

	refstate = REF_OFF; // turn off auto refresh
	resetCAS();
	resetRAS();
	resetWE();

	int walkValue = 1 - state; // walking value is state inverted
	for (col = 0; col < maxCol; col++)
	{
		for (row = 0; row < maxRow; row++)
		{
			if (readBit(row, col) == state)
			{
				writeBit(row, col, walkValue);
				if (readBit(row, col) != walkValue)
				{
					miss++;
				}
			}
			else
			{
				miss++;
			}		
		}
	}

	fail = (miss != 0) ? true : false;	// miss > 0 is test failure
	fail ? display.println(F("Fail ")) : display.println(F("Passed "));
	miss = (fail) ? miss / reps : miss; // average misses/test

	display.println(miss);
	display.display();

	return fail;
}

// write a single bit
void writeBit(int row, int col, int state)
{
	setDIN( (state==1) );

	setRow(row);	// set ROW address
	setRAS();		// set RAS low (active)
	setWE();		// set WE low (active)

	setCol(col);	// set COLUMN address
	setCAS();		// set CAS low (active)

	resetWE();		// reset all control lines
	resetCAS();
	resetRAS();
}

// read a single bit
int readBit(int row, int col)
{
	int value = 0;

	setRow(row);	// set ROW address
	setRAS();		// set RAS low (active)

	setCol(col);	// set COLUMN address
	setCAS();		// set CAS low (active)
	NOP;			// delay 62.5ns
	NOP;			// delay 62.5ns, need second one for micro

	// verify bit value based on pattern passed
	value = readDOUT();

	resetCAS();		// reset control lines
	resetRAS();

	return value;
}

// Fills DRAM with pattern of bits
void writePattern(int maxRow, int maxCol, byte pattern)
{
	int row, col, miss = 0;
	byte bitMask = 0x01;

	refstate = REF_OFF; // turn off auto refresh
	resetCAS();			// make sure all control lines are reset
	resetRAS();
	resetWE();

	// bit value based on pattern,  shift pattern mask
	for (col = 0; col < maxCol; col++)
	{
		for (row = 0; row < maxRow; row++)
		{
			writeBit(row, col, ((pattern & bitMask) > 0));
			bitMask = bitMask << 1; if (bitMask == 0) bitMask = 1;
		}
	}

	refstate = REF_ON; // enable auto referesh
	return;
}

// verifies that pattern of bits is in DRAM
unsigned int readPattern(int maxRow, int maxCol, byte pattern)
{
	int row, col;
	unsigned int miss = 0;
	byte bitMask = 0x01;

	refstate = REF_OFF;	// turn off auto refresh
	resetCAS();			// make sure all control lines are reset
	resetRAS();
	resetWE();

	// verify bit value based on pattern, shift pattern mask
	for (col = 0; col < maxCol; col++)
	{
		for (row = 0; row < maxRow; row++)
		{
			if (readBit(row, col) != ((pattern & bitMask) != 0)) { miss++; }
			bitMask = bitMask << 1; if (bitMask == 0) bitMask = 1;
		}
	}

	refstate = REF_ON; // enable referesh timer
	return miss;
}

// helper to clear display and set correct cursor location
void clearDisplay()
{
	display.clearDisplay();   // Clear the buffer
	display.setCursor(0, 12); // Start at top-left corner
}

// Performs DRAM refresh, 64 Rows each interrupt
// Interrupt fires every 1ms so all 512 rows are done every 4ms
// Also, handles LED flashing
// NOTE: Serial writes can interfere with this ISR timing
ISR(TIMER3_OVF_vect)
{
	// Refresh DRAM if enabled
	if (refstate == REF_ON)
	{
		for (int i = 0; i < 64; i++)
		{
			refreshRow(ref_row);
			ref_row++;
		}
		if (ref_row > ref_maxRow) { ref_row = 0; } // todo: need max row adjusted for part
	}

	if (ledstate == LED_BLINK)	// update Green LED
	{  
		if (blinkCounter > 500) 
		{   
			ledToggle();
			blinkCounter = 0;
		}   
		blinkCounter++;
	} 
	else if (ledstate == LED_ON) // will turn LED on even if already on
	{
		ledON();
	}
	else if (ledstate == LED_OFF) // will turn LED off even if already off
	{
		ledOFF();
	}
}


