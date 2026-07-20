#include "SRTiny26.h"

SRTiny26::SRTiny26(){
	reg_map["SREG"] = 0x5F;
	reg_map["SP"] = 0x5D;
	reg_map["GIMSK"] = 0x5B;
	reg_map["GIFR"] = 0x5A;
	reg_map["TIMSK"] = 0x59;
	reg_map["TIFR"] = 0x58;
	reg_map["MCUCR"] = 0x55;
	reg_map["MCUSR"] = 0x54;
	reg_map["TCCR0"] = 0x53;
	reg_map["TCNT0"] = 0x52;
	reg_map["OSCCAL"] = 0x51;
	reg_map["TCCR1A"] = 0x50;
	reg_map["TCCR1B"] = 0x4F;
	reg_map["TCNT1"] = 0x4E;
	reg_map["OCR1A"] = 0x4D;
	reg_map["OCR1B"] = 0x4C;
	reg_map["OCR1C"] = 0x4B;
	reg_map["PLLSCR"] = 0x49;
	reg_map["WDTCR"] = 0x41;
	reg_map["EEAR"] = 0x3E;
	reg_map["EEDR"] = 0x3D;
	reg_map["EECR"] = 0x3C;
	reg_map["PORTA"] = 0x3B;
	reg_map["DDRA"] = 0x3A;
	reg_map["PINA"] = 0x39;
	reg_map["PORTB"] = 0x38;
	reg_map["DDRB"] = 0x37;
	reg_map["PINB"] = 0x36;
	reg_map["USIDR"] = 0x2F;
	reg_map["USISR"] = 0x2E;
	reg_map["USICR"] = 0x2D;
	reg_map["ACSR"] = 0x28;
	reg_map["ADMUX"] = 0x27;
	reg_map["ADCSR"] = 0x26;
	reg_map["ADCH"] = 0x25;
	reg_map["ADCL"] = 0x24;
	
	open(0x16C0, "rasusb.googlepages.com", 1503, "SR USB Tiny26");
}

SRTiny26::~SRTiny26(){
}

void SRTiny26::WriteIO(int addr, int data)
{
	unsigned char report[4];
	report[0] = 1;
	report[1] = 1;
	report[2] = addr;
	report[3] = data & 0xff;
	sendreport(&report[0], 4);
}

int SRTiny26::ReadIO(int addr)
{
	unsigned char report[4];
	report[0] = 1;
	report[1] = 0;
	report[2] = addr;
	report[3] = 0;
	sendreport(&report[0], 4);
	recvreport(&report[0], 4);
	return report[3];
}

double SRTiny26::PWMSetup(int pin, double frequency){
	if(pin < 1 || pin >2)
		return -1;
		
	double fPCK = 16500000;
	unsigned char OCR1C;
	unsigned char divider = 0; //this is mostly to shut the compiler up.
	if(frequency < 0){ //disable it
		(*this)["TCCR1A"] = 0;
		(*this)["TCCR1B"] = 0;
		return 0;
	}
	else if(frequency == 0 ){		//just want to read it
		divider = (*this)["TCCR1B"]&15;
		OCR1C   = (*this)["OCR1C"];
	}
	else{
		if(frequency > 128000)	//Set limits.
			frequency = 128000;
		if(frequency < 4)
			frequency = 4;

		double fdiv = 0.5;
		double fOCR1C = 256;
		while(fOCR1C > 255) {
			fdiv = fdiv * 2;
			fOCR1C = round(fPCK/(frequency * fdiv) - 1);
		}
		divider = (int)fdiv; //cast it back to integer so we can use bitwise ops.
		OCR1C   = (int)fOCR1C;
		(*this)["TCCR1B"] = 0x80 | divider & 15; //set CTC1
		(*this)["OCR1C"] = OCR1C;
	}
	
	return fPCK/((double)divider*(OCR1C+1));
}


double SRTiny26::PWM(int pin, double duty_cycle){
	unsigned char TCCR1A, OCR1C, OCR1AB=0 ;
	if(pin < 1 || pin > 2)
		return -1;

	//Read duty cycle
	if(duty_cycle > 1){
		if(pin == 1)
			OCR1AB = (*this)["OCR1A"];
		else if (pin == 2)
			OCR1AB = (*this)["OCR1B"];
		OCR1C = (*this)["OCR1C"];
		return (double)OCR1AB/(double)OCR1C;
	}
	
	//Enable/disable PWM outptus
	TCCR1A = (*this)["TCCR1A"];
	if(pin == 1){
		TCCR1A &= 0x3E; //00xxxxx0
		if (duty_cycle > 0)
			TCCR1A |= 0xC1; //11xxxxx1
	}
	else if(pin == 2){
		TCCR1A &= 0xCD; //xx00xx0x
		if (duty_cycle > 0)
			TCCR1A |= 0x32; //xx11xx1x
	}
	
	(*this)["TCCR1A"] = TCCR1A;

	if(duty_cycle < 0)  //If we were just disabling an output we can return.
		return 0;
	
	OCR1C = (*this)["OCR1C"]; //this was backwards
	OCR1AB = (int)round((double)OCR1C * duty_cycle); //does that cast need to be in here
	if(pin == 1)
		(*this)["OCR1A"] = OCR1AB;
	else if(pin == 2)
		(*this)["OCR1B"] = OCR1AB;
		
	return (double)OCR1AB/(double)OCR1C;
}
	
	
int SRTiny26::ADCRead(int pin)
{	
	unsigned char ADCL, ADCH;
	(*this)["ADMUX"] = pin & 0x1F;
	//we need a cross platform delay here, of the order of a millisecond.
	ADCL = (*this)["ADCL"];
	ADCH = (*this)["ADCH"];
	return ADCH<<8 + ADCL;
}

//================= pin # to IC pin # ================================================

int SRTiny26::pin_to_IC(int pin) 
{ switch (pin)
   { case 1:  return(2)  ; break ;
     case 2:  return(4) ;  break ;
     default: return(-1) ;
   }
}
