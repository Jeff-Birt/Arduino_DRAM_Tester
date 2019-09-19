/*
 Arduino_DRAM_Tester_0.6, 4164 and 41256 DRAM tester 
 Jeffrey T. Birt (Hey Birt!) http://www.soigeneris.com , http://www.youtube.com/c/HeyBirt 
 Based on project by ChronWorks http://www.chronworks.com/DRAM/
*/

// NOTES:
// Need to draw up schematics

#include "All_Defs.h"
#ifdef ARDUINO_AVR_MEGA2560
#include "Mega_Defs.h"
#elif ARDUINO_AVR_UNO
#include "Uno_Defs.h"
#endif

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

UISTATES uistate = BEGIN;
LEDSTATES ledstate = LED_OFF;
REFRESH_STATES refstate = REF_ON;
int ref_row; // ROW address for refresh interrupt handler
int blinkCounter; // LED blink counter

// the setup function runs once when you press reset or power the board
// TCCR2B sets Timer2 divisor, overflow = (1/F)*2^8*divisor
void setup() 
{
  initStandby();
  
  ref_row = 0;
  blinkCounter = 0;
  ledstate = LED_OFF;
  ENABLE_REFRESH; // enable refresh timer
  TCCR2B = (TCCR2B & B11111000) | 0x03; // set refresh rate to 1ms
  
  Serial.begin(9600);
}


// the loop function runs over and over again forever
// UI states - 0:Begin, 1:Select_Test, 3:Again, 4:Done
void loop() 
{

  if (uistate == BEGIN)
  {
    unsigned int miss; // miss counter, incorrect bits
    ledstate = LED_OFF;
  
    Serial.println("Hi, DRAM Tester Ready!");
    Serial.println("Choose DRAM type");
    Serial.println("1 - 4164");
    Serial.println("2 - 41256");

    uistate = SELECT;
  }

  if (uistate == SELECT && Serial.available())
  {
    char ch = Serial.read();
    if (ch == '1')
    {
      Serial.println("Testing 4164...");
      ledstate = LED_BLINK;
      doTests(255, 2552);
      Serial.println();
      Serial.println("Again? Y/N");
      uistate = AGAIN;
    }
    else if (ch == '2')
    {
      Serial.println("Testing 41256...");
      ledstate = LED_BLINK;
      doTests(511, 511);
      Serial.println();
      Serial.println("Again? Y/N");
      uistate = AGAIN;
    }
  }

  if (uistate == AGAIN && Serial.available())
  {
    char ch = Serial.read();
    if (ch == 'Y' || ch == 'y')
    {
      Serial.println();
      Serial.println();
      uistate = BEGIN;
    }
    else if (ch == 'N' || ch == 'n')
    {
      Serial.println("BYE!");
      uistate = DONE;
    }
  }
  
}

void doTests(int maxRow, int maxCol)
{
  bool failure = false;
  initTest();
  
  writePattern(maxRow, maxCol, 0x00);
  delay(1000);
  failure |= showResults( readPattern(maxRow, maxCol, 0x00), "Fill 0x00 - ");
  
  writePattern(maxRow, maxCol, 0xFF);
  delay(1000);
  failure |= showResults( readPattern(maxRow, maxCol, 0xFF), "Fill 0xFF - ");

  writePattern(maxRow, maxCol, 0x55);
  delay(1000);
  failure |= showResults( readPattern(maxRow, maxCol, 0x55), "Fill 0x55 - ");

  writePattern(maxRow, maxCol, 0xAA);
  delay(1000);
  failure |= showResults( readPattern(maxRow, maxCol, 0xAA), "Fill 0xAA - ");

  writePattern(maxRow, maxCol, 0x00);
  delay(1000);
  failure |= (showResults( readPattern(maxRow, maxCol, 0x55), "Seeded Failure #1 - ") == false);

  writePattern(maxRow, maxCol, 0xFF);
  delay(1000);
  failure |= (showResults( readPattern(maxRow, maxCol, 0x55), "Seeded Failure #2 - ") == false);

  if (failure) 
  {
    Serial.println();
    Serial.println("At least one test failed...");
  }
  else
  {
    Serial.println();
    Serial.println("All tests passed!");
  }

  ledstate = failure ? LED_OFF : LED_ON;
  initStandby();
}

bool showResults(unsigned int miss, String lable)
{
  Serial.println();
  Serial.print(lable);

  if ( miss > 0 )
  {
    Serial.print("Test failed! Misses: ");
    Serial.print(miss);
    return true;
  }
  else
  {
    Serial.print("Test passed! Misses: ");
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
// NOTE: Serial writes can interfere with this ISR
ISR(TIMER2_OVF_vect)
{
  // Refresh DRAM is turned on
  if (refstate == REF_ON)
  {
    for (int i = 64; i; i--)
    { 
      refreshRow(ref_row);
      ref_row++;
    }
    if (ref_row >=512) ref_row = 0;
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
