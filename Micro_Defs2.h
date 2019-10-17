#pragma once

//                      Ard      4116		4164       41256		4464
//PORTB bit             Prt Pin  Func PIN	Func Pin   Func PIN		Func PIN
//  RXLED               PB0 
#define A0TOA6  0xFE // PB1 17	 A0   05	A0   05		A0   05		A0   14
//                      PB2 18	 A1   07	A1   07		A1   07		A1   13
//                      PB3 16	 A2   06	A2   06		A2   06		A2   12
//                      PB4 30	 A3   12	A3   12		A3   12		A3   11
//                      PB5 31	 A4   11	A4   11		A4   11		A4   08
//                      PB6 32	 A5   10	A5   10		A5   10		A5   07
//                      PB7 33	 A6   13	A6   13		A6   13		A6   06


//                      Ard      4116		4164       41256		4464
//PORTC bit             Prt Pin  Func Pin   Func PIN	Func PIN	Func PIN
//                      PC0
//                      PC1
//                      PC2
//                      PC3
//                      PC4
//                      PC5
#define DUT_PWR 0x40 // PC6 27
#define WE      0x80 // PC7 01	 WE   03	WE   03		WE   03		WE   04

//                      Ard      4116		4164       41256		4464
//PORTD bit             Prt Pin  Func Pin   Func PIN	Func PIN	Func PIN
// SDA                  PD0 24   
// SCL                  PD1 25
#define CAS     0x04 // PD2 21   CAS  15	CAS  15		CAS	 15		CAS	 16
#define RAS     0x08 // PD3 20   RAS  04	RAS  04		RAS	 04		RAS	 05
// open              // PD4 
// TXLED                PD5
#define A7TOA8  0xC0 // PC6 34							A7   01
//                      PC7 28							A8   09
#define A7      0xC0 // PC6 34										A7   10
#define NOTG         // PC7 28										/G   01


//                      Ard      4116		4164		41256		4464
//PORTE bit             Prt Pin  Func Pin   Func PIN	Func PIN	Func PIN
//					    PE0
//					    PE1
//					    PE2
//					    PE3
//					    PE4
//						PE5
#define GRN_LED	0x40 // PE6 29
//					    PE7


//                      Ard      4116		4164       41256		4464
//PORTF bits            Prt Pin  Func Pin   Func PIN	Func PIN	Func PIN
#define KEYPAD  0x01 // PF0 09	(A5) 
#define MOD_ID  0x02 // PF1 08	(A4)
//              0x04    PF2
//              0x08    PF3
#define DIN     0x10 // PF4 07	 DIN  02	DIN  02		DIN  02		DQ1	02
#define DOUT    0x20 // PF5 06	 DOUT 14	DOUT 14		DOUT 14		DQ2	03
#define DIO0    0x10 // PF4 07	 DIN  02	DIN  02		DIN  02		DQ1	02
#define DIO1    0x20 // PF5 06	 DOUT 14	DOUT 14		DOUT 14		DQ2	03
#define DIO2    0x40 // P65 05										DQ3	15
#define DIO3    0x80 // PF7 04										DQ4	17




// to set pins to HIZ set DDRx bits to zero and set corresponding PORTx pin low
void initTest()
{
	DDRB = DDRB | A0TOA6;			// outputs
	DDRC = DDRC | DUT_PWR | WE;		// outputs
	DDRD = DDRD | A7TOA8 | CAS | RAS; // outputs
	DDRE = DDRE | GRN_LED;			// outputs
	DDRF = DDRF | DIN;				// output
	DDRF = DDRF & ~DOUT;			// input

	PORTC = PORTC & ~DUT_PWR;		// turn ON +5V to DUT 
	PORTC = PORTC | WE;				// WE HIGH (off)
	PORTD = PORTD | RAS | CAS;		// set HIGH (off)
	PORTE = PORTE & ~GRN_LED;		// turn off LED	
}

// For HIZ set DDRx bits to inputs (zero) and set corresponding PORTx pins low
void initStandby()
{
	DDRB = DDRB & ~A0TOA6;			// inputs
	DDRC = DDRC & ~WE;				// inputs
	DDRD = DDRD & ~(A7TOA8 | RAS | CAS); // inputs
	DDRE = DDRE | GRN_LED;			// output even in init
	DDRF = DDRF & ~(DIN | DOUT);	// inputs

	PORTB = PORTB & ~A0TOA6;		// set low
	PORTC = PORTC & ~WE;			// set low
	PORTD = PORTD & ~(A7TOA8 | RAS | CAS);	// set low
	PORTF = PORTF | DUT_PWR;		// turn OFF +5V to DUT
	PORTF = PORTF & ~(DIN | DOUT);	// set low
}

// set Data IN pin of DRAM
//inline void setDIN(byte pattern, byte bitMask)
inline void setDIN(bool state)
{
	PORTF = (PORTF & ~DIN) | (state ? DIN : 0); // clear DIN bit, conditinally set
}

// set column address, could combine with row as well
inline void setCol(int col)
{
	PORTB = (byte)(col << 1);   // Write out COL address
	PORTD = (PORTD & ~A7TOA8) | ((col & 0x180) >> 1); //
}

inline void setRow(int row)
{
	PORTB = (byte)(row << 1);    // Write out ROW address
	PORTD = (PORTD & ~A7TOA8) | ((row & 0x180) >> 1); // 
}

// set Column Address Select HIGH (off)
inline void resetCAS()
{
	PORTD = PORTD | CAS;  // Set CAS HIGH
}

// set Column Address Select LOW (on)
inline void setCAS()
{
	PORTD = PORTD & ~CAS; // Set CAS LOW
}

// set Row Address Select HIGH (off)
inline void resetRAS()
{
	PORTD = PORTD | RAS;  // Set RAS HIGH
}

// set Row Address Select LOW (on)
inline void setRAS()
{
	PORTD = PORTD & ~RAS; // set RAS low
}

// set Write Enable HIGH (off)
inline void resetWE()
{
	PORTC = PORTC | WE;   // Set WE high to complete write
}

// set Write Enable LOW (on)
inline void setWE()
{
	PORTC = PORTC & ~WE;  // Set WE low for write
}

// read bit state from DRAM DOUT pin
inline bool readDOUT()
{
	return ((PINF & DOUT) != 0);
}

// refresh row to maintain DRAM data
inline void refreshRow(int row)
{
	PORTB = (byte)(row << 1);    // Write out ROW address
	PORTD = (PORTD & ~A7TOA8) | ((row & 0x180) >> 1); // 
	PORTD = PORTD & ~RAS; // set RAS low
	PORTD = PORTD | RAS;  // set RAS high
}

// turn LED off
inline void ledOFF()
{
	PORTE = PORTE & ~GRN_LED; // turn off grn LED
}

// turn LED on
inline void ledON()
{
	PORTE = PORTE | GRN_LED; // turn off grn LED
}

// toggle LED state
inline void ledToggle()
{
	PORTE = PORTE ^ GRN_LED; // toggle grn LED
}
