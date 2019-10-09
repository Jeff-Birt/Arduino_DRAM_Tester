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

UISTATES uistate = BEGIN;
LEDSTATES ledstate = LED_OFF;
REFRESH_STATES refstate = REF_OFF;
volatile int ref_row = 0;;		// ROW address for refresh interrupt handler
volatile int blinkCounter = 0;	// LED blink counter
int ref_maxRow = 512;			// maxium rows of DRAM (for refresh)

// the setup function runs once when you press reset or power the board
// Initalize IO to standby state, enable refresh timer
void setup() 
{
  initStandby();

  TCCR3B = (TCCR3B & B11111000) | 0x03; // set refresh rate to 2ms
  //ENABLE_REFRESH; // enable refresh timer ***can't start refresh timer until part size known
  
  Serial.begin(9600);
}


// the loop function runs over and over again forever
// We use the loop function for the UI
void loop() 
{
	char ch;

	switch (uistate)
	{
		case BEGIN :
			delay(2000);		// give the Micro serial time to come up
			unsigned int miss;	// miss counter, incorrect bits
			ledstate = LED_OFF;
  
			Serial.println(F("Hi, DRAM Tester Ready!"));
			Serial.println(F("Choose DRAM type"));
			Serial.println(F("1 - 4164"));
			Serial.println(F("2 - 41256"));

			uistate = SELECT;
			break;

		case SELECT :
			ch = Serial.read();
			if (ch == '1')
			{
				ref_maxRow = 256;
				Serial.println(F("Testing 4164..."));
				ledstate = LED_BLINK;
				doTests(255, 255);
				Serial.println();
				Serial.println(F("Again? Y/N"));
				uistate = AGAIN;
			}
			else if (ch == '2')
			{
				ref_maxRow = 512;
				Serial.println(F("Testing 41256..."));
				ledstate = LED_BLINK;
				doTests(511, 511);
				Serial.println();
				Serial.println(F("Again? Y/N"));
				uistate = AGAIN;
			}
			break;

		case AGAIN :
			ch = Serial.read();
			if (ch == 'Y' || ch == 'y')
			{
				Serial.println();
				Serial.println();
				uistate = BEGIN;
			}
			else if (ch == 'N' || ch == 'n')
			{
				Serial.println(F("BYE!"));
				uistate = DONE;
			}
			break;
    }
  
}

// Runs a series of tests with various bit patterns
// maxRow, maxCol -> maximum row and column DRAM has
void doTests(int maxRow, int maxCol)
{
  bool failure = false;
  initTest();

  writePattern(maxRow, maxCol, 0x00);
  delay(1000);
  failure |= showResults( readPattern(maxRow, maxCol, 0x00), F("Fill 0x00 - "));
  
  writePattern(maxRow, maxCol, 0xFF);
  delay(1000);
  failure |= showResults( readPattern(maxRow, maxCol, 0xFF), F("Fill 0xFF - "));

  writePattern(maxRow, maxCol, 0x55);
  delay(1000);
  failure |= showResults( readPattern(maxRow, maxCol, 0x55), F("Fill 0x55 - "));

  writePattern(maxRow, maxCol, 0xAA);
  delay(1000);
  failure |= showResults( readPattern(maxRow, maxCol, 0xAA), F("Fill 0xAA - "));

  writePattern(maxRow, maxCol, 0x00);
  delay(1000);
  failure |= (showResults( readPattern(maxRow, maxCol, 0x55), F("Seeded Failure #1 - ")) == false);

  writePattern(maxRow, maxCol, 0xFF);
  delay(1000);
  failure |= (showResults( readPattern(maxRow, maxCol, 0x55), F("Seeded Failure #2 - ")) == false);

  if (failure) 
  {
    Serial.println();
    Serial.println(F("At least one test failed..."));
  }
  else
  {
    Serial.println();
    Serial.println(F("All tests passed!"));
  }

  refstate = REF_OFF;
  ledstate = failure ? LED_OFF : LED_ON;
  initStandby();
}

// display results over sreial link
bool showResults(unsigned int miss, String lable)
{
  Serial.println();
  Serial.print(lable);

  if ( miss > 0 )
  {
    Serial.print(F("Test failed! Misses: "));
    Serial.print(miss);
    return true;
  }
  else
  {
    Serial.print(F("Test passed! Misses: "));
    Serial.print(miss);
    return false;
  }
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
  
  for(col = 0 ; col < maxCol ; col++)
  {
    for(row = 0 ; row < maxRow ; row++)
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
  
  for(col = 0 ; col < maxCol ; col++)
  {
    for(row = 0 ; row < maxRow ; row++)
    {
      setRow(row);   // set ROW address
      setRAS();      
      
      setCol(col);   // set COLUMN address
      setCAS();
      NOP;           // delay 62.5ns
      NOP;           // delay 62.5ns, need second one for micro
      
      // verify bit value based on pattern passed
      if ( readBit() !=  ((pattern & bitMask) != 0)) { miss++; }
      bitMask = bitMask << 1; if (bitMask == 0) bitMask = 1; // shift pattern mask

      resetCAS();    // reset control lines
      resetRAS();
    } 
  }

  refstate = REF_ON; // enable referesh timer
  return miss;
  
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
    for (int i = 64; i; i--)
    { 
      refreshRow(ref_row);
      ref_row++;
    }
    if (ref_row >= ref_maxRow) ref_row = 0; // todo: need max row adjusted for part
  }

//  // update Green LED
//  if (ledstate == LED_BLINK)
//  {  
//    if (blinkCounter > 500) 
//    {   
//      ledToggle();
//      blinkCounter = 0;
//    }   
//    blinkCounter++;
//  } 
//  else if (ledstate == LED_ON)
//  {
//    ledON();
//  }
//  else if (ledstate == LED_OFF)
//  {
//    ledOFF();
//  }
}
