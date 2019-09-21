#pragma once
  
// PORTB bit        Ard Prt Pin  DRAM
#define A0TOA5  0x3F // PB0 #8   #5
                     // PB1 #9   #7
                     // PB2 #10  #6
                     // PB3 #11  #12
                     // PB4 #12  #11
                     // PB5 #13  #10

// PORTC bit        Ard Prt Pin  DRAM
#define A6TOA8  0x07 // PC0 #23  #13
                     // PC1 #24  #9
                     // PC2 #25  #1
#define DUT_PWR 0x10 // PC4 #27
#define RED_LED 0x20 // PC5 #28

// PORTD bit        Ard Prt Pin  DRAM
#define DIN     0x04 // PD2 #4   #2
#define DOUT    0x08 // PD3 #5   #14
#define GRN_LED 0x10 // PD4 #6  
#define CAS     0x20 // PD5 #11  #15
#define RAS     0x40 // PD6 #12  #4
#define WE      0x80 // PD7 #13  #3

// Set Ports/Pins to correrct settings to running test
void initTest()
{
  DDRB = DDRB | A0TOA5;
  DDRC = DDRC | A6TOA8;
  DDRC = DDRC | BLU_LED | RED_LED;
  DDRD = (DDRD & ~DOUT); // set DOUT as input
  DDRD = DDRD | DIN | GRN_LED | CAS | RAS | WE; // set all outputs
   
  PORTD = PORTD | RAS | CAS | WE;  // set high
  PORTD = PORTD & ~GRN_LED; // turn off green LED
  PORTC = PORTC & ~(RED_LED | BLU_LED); // turn off red/blue LEDs
}

// Set all pins connected to DRAM to HIZ duriny standby
// For HIZ set DDRx bits to zero and set corresponding PORTx pins low
void initStandby()
{
  DDRB = DDRB & ~A0TOA5;
  DDRC = DDRC & ~A6TOA8;
  DDRD = DDRD & ~DOUT; // set DOUT as input
  DDRD = DDRD & ~(DIN | CAS | RAS | WE);

  PORTB = PORTB & ~A0TOA5;
  PORTC = PORTC & ~A6TOA8;
  PORTD = PORTD & ~(DIN | RAS | CAS | WE);
}

inline void setDIN(byte pattern, byte bitMask)
{
  PORTD= (PORTD & ~DIN) | ( ((pattern & bitMask)>0) ? DIN : 0); // clear DIN bit, conditinally set
}

inline void setCol(int col)
{
  PORTB = (byte)((PORTB & ~A0TOA5) | (col & A0TOA5));    // Write out COL address
  PORTC = (byte)( PORTC & ~A6TOA8) | ((col >> 6) & A6TOA8)); // not using rest of PORTC only bit0 
}

inline void setRow(int row)
{
  PORTB = (byte)((PORTB & ~A0TOA5) | (row & A0TOA5));    // Write out COL address
  PORTC = (byte)( PORTC & ~A6TOA8) | ((row >> 6) & A6TOA8)); // not using rest of PORTC only bit0 
}

inline void setRow(int row)
{
  PORTA = (byte)row;    // 
  PORTC = (PORTC & ~A8) | (row >> 8); // not using rest of PORTC only bit0
}

inline void resetCAS()
{
  PORTD = PORTD | CAS;  // Set CAS HIGH
}

inline void setCAS()
{
  PORTD = PORTD & ~CAS; // Set CAS LOW
}

inline void setRAS()
{
  PORTD = PORTD & ~RAS; // set RAS low
}

inline void resetRAS()
{
  PORTD = PORTD | RAS;  // Set RAS HIGH
}

inline void setWE()
{
  PORTD = PORTD & ~WE;  // Set WE low for write
}

inline void resetWE()
{
  PORTD = PORTD | WE;   // Set WE high to complete write
}

inline bool readBit()
{
  return ((PIND & DOUT) != 0);
}

inline void refeshRow(int row)
{
  PORTB = (byte)((PORTB & ~A0TOA5) | (row & A0TOA5));    // Write out COL address
  PORTC = (byte)( PORTC & ~A6TOA8) | ((row >> 6) & A6TOA8)); // not using rest of PORTC only bit0 
  PORTD = PORTD & ~RAS; // set RAS low
  PORTD = PORTD | RAS;  // set RAS high
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
