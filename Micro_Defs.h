#pragma once

//PORTB bit         Ard Prt Pin  DRAM
#define A0TOA6  0xFF // PB0 #19  #05
//                      PB1 #17  #07
//                      PB2 #18  #06
//                      PB3 #16  #12
//                      PB4 #30  #11
//                      PB5 #31  #10
//                      PB6 #32  #13
//  RXLED               PB7 

//PORTC bit         Ard Prt Pin  DRAM
//                      PC0
//                      PC1
//                      PC2
//                      PC3
//                      PC4
//                      PC5
//#define A7TOA8  0xC0 // PC6 #01  #01
//                      PC7 #27  #09

//PORTD bit         Ard Prt Pin  DRAM
// SDA                  PD0
// SCL                  PD1
//                      PD2
//                      PD3
#define GRN_LED 0x10 // PD4
//                      PD5
#define A7TOA8  0xC0 // PC6 #34  #01
//                      PC7 #28  #09

//PORTF bits        Ard Prt Pin  DRAM
#define CAS     0x01 // PF0 #09  #15
#define RAS     0x02 // PF1 #08  #04
//              0x04    PF2
//              0x08    PF3
#define WE      0x10 // PF4 #07  #03
#define DIN     0x20 // PF5 #06  #02
#define DOUT    0x40 // P65 #05  #14 
#define DUT_PWR 0x80 // PF7 #04


// to set pins to HIZ set DDRx bits to zero and set corresponding PORTx pin low
void initTest()
{
  DDRB = DDRB | A0TOA6;
  DDRD = DDRD | A7TOA8;
  DDRD = DDRD | GRN_LED;
  DDRF = DDRF | CAS | RAS | WE | DIN | DUT_PWR; // outputs
  DDRF = DDRF & ~DOUT; // input

  PORTD = PORTD & ~GRN_LED; // turn off LEDs
  PORTF = PORTF | RAS | CAS | WE;  // set HIGH
  PORTF = PORTF & ~DUT_PWR; // turn ON +5V to DUT 
}

// For HIZ set DDRx bits to zero and set corresponding PORTx pins low
void initStandby()
{
  DDRB = DDRB & ~A0TOA6;
  DDRD = DDRD & ~A7TOA8;
  DDRD = DDRD | GRN_LED;
  DDRF = DDRF & ~(DIN | RAS | CAS | WE);
  DDRF = DDRF & ~DOUT;

  PORTB = PORTB & ~A0TOA6;
  PORTD = PORTD & ~A7TOA8;
  PORTF = PORTF | DUT_PWR; // turn OFF +5V to DUT
  PORTF = PORTF & ~(DIN | RAS | CAS | WE);
  PORTF = PORTF & ~DOUT;
}

inline void setDIN(byte pattern, byte bitMask)
{
  PORTF = (PORTF & ~DIN) | ( ((pattern & bitMask)>0) ? DIN : 0); // clear DIN bit, conditinally set
}

inline void setCol(int col)
{
  PORTB = (byte)(col << 1);   // Write out COL address
  PORTD = (PORTD & ~A7TOA8) | ((col & 0x180) >> 1); //
//  Serial.print("Col:");
//  Serial.print(col, HEX); Serial.print(", ");
//  Serial.print((byte)(col << 1), HEX); Serial.print(", ");
//  Serial.print( (PORTC & ~A7TOA8) | ((col & 0x180) >> 1), HEX);
//  Serial.println();
}

inline void setRow(int row)
{
  PORTB = (byte)(row << 1);    // Write out ROW address
  PORTD = (PORTD & ~A7TOA8) | ((row & 0x180) >> 1); // 
//  Serial.print("Row:");
//  Serial.print(row, HEX); Serial.print(", ");
//  Serial.print((byte)(row << 1), HEX); Serial.print(", ");
//  Serial.print((byte)((PORTC & ~A7TOA8) | ((row >> 2) & A7TOA8, HEX)));
//  Serial.print(" - ");
}

inline void resetCAS()
{
  PORTF = PORTF | CAS;  // Set CAS HIGH
}

inline void setCAS()
{
  PORTF = PORTF & ~CAS; // Set CAS LOW
}

inline void setRAS()
{
  PORTF = PORTF & ~RAS; // set RAS low
}

inline void resetRAS()
{
  PORTF = PORTF | RAS;  // Set RAS HIGH
}

inline void setWE()
{
  PORTF = PORTF & ~WE;  // Set WE low for write
}

inline void resetWE()
{
  PORTF = PORTF | WE;   // Set WE high to complete write
}

inline bool readBit()
{
  return ((PINF & DOUT) != 0);
}

inline void refreshRow(int row)
{
  PORTB = (byte)(row << 1);    // Write out ROW address
  PORTD = (PORTD & ~A7TOA8) | ((row & 0x180) >> 1); // 
  PORTF = PORTF & ~RAS; // set RAS low
  PORTF = PORTF | RAS;  // set RAS high
}

inline void ledOFF()
{
  PORTD = PORTD & ~GRN_LED; // turn off grn LED
}

inline void ledON()
{
  PORTD = PORTD | GRN_LED; // turn off grn LED
}

inline void ledToggle()
{
  PORTD = PORTD ^ GRN_LED; // toggle grn LED
}
