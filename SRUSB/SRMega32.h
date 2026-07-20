#ifndef __SRMega32_h_INCLUDED__
#define __SRMega32_h_INCLUDED__

#include "SRUSB.h"

class SRMega32 : public SRUSB {
public:

	void WriteIO(int addr, int data) ;
	int ReadIO(int addr) ; 
	void WriteEEPROM(int addr, unsigned char byte);
	unsigned char ReadEEPROM(int addr);
	
	SRMega32();
	~SRMega32();
	
	double PWMSetup(int pin, double frequency = 0);
	double PWM(int pin, double duty_cycle = 0);
	int ADCRead(int pin);
	int pin_to_IC(int pin) ;
	unsigned char* user_command(int u08A, int u16A, int u16B) ;
	unsigned char* read_ram_5(int ctrl, int addr, int data) ;
	void write_ram(int len, int addr, int data) ;
        unsigned char* read_io_3() ;
        void WriteEE(int addr, int data) ;
        unsigned char* ReadEE(int addr) ;
        void write_ram_ee(int ctrl, int addr, int data, bool ee) ;

};
	
#endif
