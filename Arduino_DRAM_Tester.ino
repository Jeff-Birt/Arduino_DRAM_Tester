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
#include "Micro_Defs.h"
#endif

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h> 

LEDSTATES ledstate = LED_OFF;
volatile REFRESH_STATES refstate = REF_OFF;
int ref_row = 0;;		// ROW address for refresh interrupt handler
int blinkCounter = 0;	// LED blink counter
volatile int ref_maxRow = 512;	// maxium rows of DRAM (for refresh)

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// about 3.5 lines, 11-12 columns with fount used
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //7.8K

String DRAM_NAME[3] = { "4116", "4164", "41256" };	// could use PROGMEM to put these in flash
int DRAM_MAX_ROW[3] = { 127, 255, 511 };			// corresponding max #rows
int dramSel = DRAM_4116;
static const int keypadPin = A6;    // analog input for keypad
UISTATES uistate = SPLASH;

// the setup function runs once when you press reset or power the board
// Initalize IO to standby state, enable refresh timer
void setup()
{
	initStandby();

	TCCR3B = (TCCR3B & B11111000) | 0x03; // set refresh rate to 2ms
	ENABLE_REFRESH; // enable refresh timer, does not enable CAS strobing

	Serial.begin(9600);
	Serial.println("Setup");

	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) //3.6k
	{
		Serial.println(F("SSD1306 allocation failed"));
		for (;;); // Don't proceed, loop forever
	}

	display.display();            // Display Adafruit Splash
	delay(2000);                  // Pause for 2 seconds

	display.setFont(&FreeMono9pt7b);
	display.setTextSize(1);       // Normal 1:1 pixel scale
	display.setTextColor(WHITE);  // Draw white text
	display.cp437(true);          // Use full 256 char 'Code Page 437' font
}


// the loop function runs over and over again forever
// We use the loop function for the UI
void loop()
{
	int aIn = 0;

	switch (uistate)
	{
	case SPLASH:
		clearDisplay();
		display.println(F("DRAM Test"));
		display.display();
		delay(2000);

		uistate = BEGIN;
		break;

	case BEGIN:
		unsigned int miss;	// miss counter, incorrect bits
		ledstate = LED_OFF;

		clearDisplay();
		display.println(F("DRAM type"));

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
		delay(200);	// key repeat timer
		aIn = analogRead(keypadPin);
		if (aIn < 975)		// no key
		{
			delay(10); // check again to filter out noise
			aIn = analogRead(keypadPin);
			if (aIn < 975)			// no key
			{
				if (aIn > 875)		// 'A' key
				{
					dramSel++;
					if (dramSel > 2) { dramSel = 0; }
					uistate = BEGIN;
				}
				else if (aIn > 775)	// 'B' key
				{
					uistate = TEST;
				}
				//else if (aIn > 675)	// 'A' & 'B' keys
				//{
				//	//Serial.println(F("A&B btns "));
				//}
			}
		}
		break;

	case TEST:
		clearDisplay();
		display.println("Test " + DRAM_NAME[dramSel]);
		display.display();

		ref_maxRow = DRAM_MAX_ROW[dramSel];
		ledstate = LED_BLINK;
		doTests(ref_maxRow, ref_maxRow);

		//clearDisplay();
		display.println("Again?");
		display.println("A-Yes  B-No");
		display.display();
		uistate = AGAIN;
		break;

	case AGAIN:
		delay(200);	// key repeat timer
		aIn = analogRead(keypadPin);
		if (aIn < 975)		// no key
		{
			delay(10); // check again to filter out noise
			aIn = analogRead(keypadPin);
			if (aIn < 975)			// no key
			{
				if (aIn > 875)		// 'A' key == YES
				{
					uistate = TEST;
				}
				else if (aIn > 775)	// 'B' key == NO
				{
					uistate = SPLASH;
				}
			}
		}
		break;
	}

}

// Runs a series of tests with various bit patterns
// maxRow, maxCol -> maximum row and column DRAM has
void doTests(int maxRow, int maxCol)
{
	bool failure = false;
	int miss = 0;
	initTest();
	refstate = REF_ON; // need refresh timer running before starting tests
	delay(1000);	   // time for DC-DC converter to come up

	writePattern(maxRow, maxCol, 0x00);
	delay(1000);
	miss = readPattern(maxRow, maxCol, 0x00);
	failure |= showResults( (miss > 0), miss, F("Fill 0x00"));
	delay(2000);

	writePattern(maxRow, maxCol, 0xFF);
	delay(1000);
	miss = readPattern(maxRow, maxCol, 0xFF);
	failure |= showResults((miss > 0), miss, F("Fill 0xFF"));
	delay(2000);

	writePattern(maxRow, maxCol, 0x55);
	delay(1000);
	miss = readPattern(maxRow, maxCol, 0x55);
	failure |= showResults((miss > 0), miss, F("Fill 0x55"));
	delay(2000);

	writePattern(maxRow, maxCol, 0xAA);
	delay(1000);
	miss = readPattern(maxRow, maxCol, 0xAA);
	failure |= showResults((miss > 0), miss, F("Fill 0xAA"));
	delay(2000);

	// 'miss > 0' for Seeded failure tests, i.e. (miss > 0) == PASS
	writePattern(maxRow, maxCol, 0x00);
	delay(1000);
	miss = readPattern(maxRow, maxCol, 0x55);
	failure |= showResults((miss == 0), miss, F("Seeded #1"));
	delay(2000);

	writePattern(maxRow, maxCol, 0xFF);
	delay(1000);
	miss = readPattern(maxRow, maxCol, 0x55);
	failure |= showResults((miss == 0), miss, F("Seeded #2"));
	delay(2000);

	if (failure)
	{
		clearDisplay();
		display.println(F("Failed"));
		display.display();
	}
	else
	{
		clearDisplay();
		display.println(F("Passed"));
		display.display();
	}

	refstate = REF_OFF;
	ledstate = failure ? LED_OFF : LED_ON;
	initStandby();
}

// display results over sreial link
bool showResults(bool fail, unsigned int miss, String lable)
{
	bool result = false;

	clearDisplay();
	display.println(lable);
	display.display();

	/*if (miss > 0)*/
	if (fail)
	{
		display.println(F("Fail "));
		display.println(miss);
		result = true;
	}
	else
	{
		display.println(F("Passed "));
		display.println(miss);
	}

	display.display();
	return result;
}

// Fills DRAM with pattern of bits
void writePattern(int maxRow, int maxCol, byte pattern)
{
	int row, col, miss = 0;
	byte bitMask = 0x01;

	refstate = REF_OFF;
	resetCAS();
	resetRAS();
	resetWE();

	for (col = 0; col < maxCol; col++)
	{
		for (row = 0; row < maxRow; row++)
		{
			// write bit value based on pattern passed
			setDIN(pattern, bitMask);
			bitMask = bitMask << 1; if (bitMask == 0) bitMask = 1;   // shift pattern mask

			setRow(row);  // set ROW address
			setRAS();
			setWE();

			setCol(col);  // set COLUMN address
			setCAS();
			NOP;          // delay 62.5ns

			resetWE();    // reset all control lines
			resetCAS();
			resetRAS();
		}
	}

	refstate = REF_ON; // enable referesh timer
	return;
}

// verifies that pattern of bits is in DRAM
unsigned int readPattern(int maxRow, int maxCol, byte pattern)
{
	int row, col;
	unsigned int miss = 0;
	byte bitMask = 0x01;

	refstate = REF_OFF;
	resetCAS();
	resetRAS();
	resetWE();

	for (col = 0; col < maxCol; col++)
	{
		for (row = 0; row < maxRow; row++)
		{
			setRow(row);   // set ROW address
			setRAS();

			setCol(col);   // set COLUMN address
			setCAS();
			NOP;           // delay 62.5ns
			NOP;           // delay 62.5ns, need second one for micro

			// verify bit value based on pattern passed
			if (readBit() != ((pattern & bitMask) != 0)) { miss++; }
			bitMask = bitMask << 1; if (bitMask == 0) bitMask = 1; // shift pattern mask

			resetCAS();    // reset control lines
			resetRAS();
		}
	}

	refstate = REF_ON; // enable referesh timer
	return miss;

}

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

	 // update Green LED
	 if (ledstate == LED_BLINK)
	{  
		if (blinkCounter > 500) 
		{   
			ledToggle();
			blinkCounter = 0;
		}   
		blinkCounter++;
	} 
	else if (ledstate == LED_ON)
	{
		ledON();
	}
	else if (ledstate == LED_OFF)
	{
		ledOFF();
	}
}


