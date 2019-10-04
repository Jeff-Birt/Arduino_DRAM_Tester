#pragma once

//PORTD bit         Ard Prt Pin  DRAM
#define A0TOA5  0xFC // PD2 #20  #05
//                      PD3 #21  #07
//                      PD4 #22  #06
//                      PD5 #23  #12
//                      PD6 #24  #11
//                      PD7 #25  #10

//PORTB bit         Ard Prt Pin  DRAM
#define A6TOA8  0x07 // PB0 #26  #13
//                      PB1 #27  #09
//                      PB2 #28  #01
#define GRN_LED 0x20 // PB3 #29 
#define BLU_LED 0x80 // PB4 #30
#define DUT_PWR 0x20 // PB5 #01

// PORTC bits       Ard Prt Pin  DRAM
#define CAS     0x01 // PC0 #04  #15
#define RAS     0x02 // PC1 #05  #04
#define WE      0x04 // PC2 #06  #03
#define DIN     0x08 // PC3 #07  #02
//      SDA     0x10 // PC4 #08
//      SCL     0x20 // PC5 #09
#define DOUT    0x40 // PC6 #10  #14
#define BTNS    0x80 // PC7 #11

 
// to set pins to HIZ set DDRx bits to zero and set corresponding PORTx pin low
void initTest()
{
  DDRD = DDRD | A0TOA5;
  DDRB = DDRB | A6TOA8;
  DDRB = DDRB | GRN_LED | BLU_LED | DUT_PWR; // outputs
  DDRC = DDRC | CAS | RAS | WE | DIN; // outputs
  DDRC = DDRC & ~DOUT; // input

  PORTC = PORTC | RAS | CAS | WE;  // set HIGH
  PORTB = PORTB & ~(GRN_LED | BLU_LED); // turn off LEDs
  PORTC = PORTC & ~DUT_PWR; // turn ON +5V to DUT
}

// For HIZ set DDRx bits to zero and set corresponding PORTx pins low
void initStandby()
{
  DDRD = DDRD & ~A0TOA5;
  DDRB = DDRB & ~A6TOA8;
  DDRC = DDRC & ~(DIN | RAS | CAS | WE);
  DDRC = DDRC & ~DOUT;

  PORTD = PORTD & ~A0TOA5;
  PORTB = PORTB & ~A6TOA8;
  PORTC = PORTC | DUT_PWR; // turn OFF +5V to DUT
  PORTC = PORTC & ~(DIN | RAS | CAS | WE);
  PORTC = PORTC & ~DOUT;
}

inline void setDIN(byte pattern, byte bitMask)
{
  PORTC = (PORTC & ~DIN) | ( ((pattern & bitMask)>0) ? DIN : 0); // clear DIN bit, conditinally set
}

//  A0TOA5 PORTD bits 2-7; A6TOA8 PORTB bits 0-2;
inline void setCol(int col)
{
  PORTD = (PORTD & ~A0TOA5) | (byte)(col << 2);    // Write out COL address
  PORTB = (PORTB & ~A6TOA8) | (col >> 6); // 
}

inline void setRow(int row)
{
  PORTD = (PORTD & ~A0TOA5) | (byte)(row << 2);    // Write out ROW address
  PORTB = (PORTB & ~A6TOA8) | (row >> 6); // 
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
  return ((PINC & DOUT) != 0);
}

inline void refreshRow(int row)
{
  PORTD = (PORTD & ~A0TOA5) | (byte)(row << 2);    // Write out ROW address
  PORTB = (PORTB & ~A6TOA8) | (row >> 6); // 
  PORTC = PORTC & ~RAS; // set RAS low
  PORTC = PORTC | RAS;  // set RAS high
}

inline void ledOFF()
{
  PORTB = PORTB & ~GRN_LED; // turn off grn LED
}

inline void ledON()
{
  PORTB = PORTB | GRN_LED; // turn off grn LED
}

inline void ledToggle()
{
  PORTB = PORTB ^ GRN_LED; // toggle grn LED
}
