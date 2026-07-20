#ifndef __SRTiny26_h_INCLUDED__
#define __SRTiny26_h_INCLUDED__

#include "SRUSB.h"
class SRTiny26 : public SRUSB {
public:

	void WriteIO(int addr, int data) ;
	int ReadIO(int addr) ; 
	void WriteEEPROM(int addr, unsigned char byte);
	unsigned char ReadEEPROM(int addr);
	
	SRTiny26();
	~SRTiny26();
	
	double PWMSetup(int pin, double frequency = 0);
	double PWM(int pin, double duty_cycle = 0);
	int ADCRead(int pin);
	int pin_to_IC(int pin) ;

};
	
#endif
