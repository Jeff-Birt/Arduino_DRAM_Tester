#pragma once

//PORTA bit         Ard Prt Pin  DRAM
#define A0TOA7  0xFF // PA0 #22  #5
//                      PA1 #23  #7
//                      PA2 #24  #6
//                      PA3 #25  #12
//                      PA4 #26  #11
//                      PA5 #27  #10
//                      PA6 #28  #13
//                      PA7 #29  #9  

// PORTC bits       Ard Prt Pin  DRAM
#define A8      0x01 // PC0 #37  #1
#define DIN     0x02 // PC1 #36  #2
#define CAS     0x04 // PC2 #35  #15
#define RAS     0x08 // PC3 #34  #4
#define WE      0x10 // PC4 #33  #3
#define GRN_LED 0x20 // PC5 #32  
#define DUT_PWR 0x40 // PC6 #31
#define BLU_LED 0x80 // PC7 #30

//PORTL bit         Ard Prt Pin  DRAM
#define DOUT    0x01 // PL0 #49  #14
 
// to set pins to HIZ set DDRx bits to zero and set corresponding PORTx pin low
void initTest()
{
  DDRA = DDRA | A0TOA7;
  DDRC = DDRC | A8;
  DDRC = DDRC | DIN | RAS | CAS | WE | GRN_LED | DUT_PWR | BLU_LED; // outputs
  DDRL = DDRL & ~DOUT; // input

  PORTC = PORTC | RAS | CAS | WE;  // set HIGH
  PORTC = PORTC & ~(GRN_LED | BLU_LED); // turn off LEDs
  PORTC = PORTC & ~DUT_PWR; // turn ON +5V to DUT
}

// For HIZ set DDRx bits to zero and set corresponding PORTx pins low
void initStandby()
{
  DDRA = DDRA & ~A0TOA7;
  DDRC = DDRC & ~A8;
  DDRC = DDRC & ~(DIN | RAS | CAS | WE);
  DDRL = DDRL & ~DOUT;

  PORTA = PORTA & ~A0TOA7;
  PORTC = PORTC & ~A8;
  PORTC = PORTC | DUT_PWR; // turn OFF +5V to DUT
  PORTC = PORTC & ~(DIN | RAS | CAS | WE);
  PORTL = PORTL & ~DOUT;
}

inline void setDIN(byte pattern, byte bitMask)
{
  PORTC = (PORTC & ~DIN) | ( ((pattern & bitMask)>0) ? DIN : 0); // clear DIN bit, conditinally set
}

inline void setCol(int col)
{
  PORTA = (byte)col;    // Write out COL address
  PORTC = (PORTC & ~A8) | (col >> 8); // not using rest of PORTC only bit0 
}

inline void setRow(int row)
{
  PORTA = (byte)row;    // 
  PORTC = (PORTC & ~A8) | (row >> 8); // not using rest of PORTC only bit0
}

inline void resetCAS()
{
  PORTC = PORTC | CAS;  // Set CAS HIGH
}

inline void setCAS()
{
  PORTC = PORTC & ~CAS; // Set CAS LOW
}

inline void setRAS()
{
  PORTC = PORTC & ~RAS; // set RAS low
}

inline void resetRAS()
{
  PORTC = PORTC | RAS;  // Set RAS HIGH
}

inline void setWE()
{
  PORTC = PORTC & ~WE;  // Set WE low for write
}

inline void resetWE()
{
  PORTC = PORTC | WE;   // Set WE high to complete write
}

inline bool readBit()
{
  return ((PINL & DOUT) != 0);
}

inline void refreshRow(int row)
{
    PORTA = (byte)row;      // & 0xFF implicit
    PORTC = (PORTC & ~A8) | (row >> 8); // not using rest of PORTC only bit0
    PORTC = PORTC & ~RAS; // set RAS low
    PORTC = PORTC | RAS;  // set RAS high
}

inline void ledOFF()
{
  PORTC = PORTC & ~GRN_LED; // turn off grn LED
}

inline void ledON()
{
  PORTC = PORTC | GRN_LED; // turn off grn LED
}

inline void ledToggle()
{
  PORTC = PORTC ^ GRN_LED; // toggle grn LED
}
