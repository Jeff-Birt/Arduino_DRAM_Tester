#pragma once

// used to add NOP delay between CAS and READ
#define NOP __asm__ __volatile__ ("nop\n\t")
#define DISABLE_REFRESH TIMSK2 = (TIMSK2 & B11111110) // disable refresh timer
#define ENABLE_REFRESH TIMSK2 = (TIMSK2 & B11111110) | 0x01 // enable refresh timer

void writePattern(int maxRow, int maxCol, byte pattern);
unsigned int readPattern(int maxRow, int maxCol, byte pattern);
bool showResults(unsigned int miss, String lable);
void doTests();
