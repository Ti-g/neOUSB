#include "SRMega32.h"

SRMega32::SRMega32(){
	#open(0x16C0, "rasusb.googlepages.com", 1503, "SR USB Mega32");
	
	reg_map["SREG"] = 0x5F;
	reg_map["SPH"] = 0x5E;
	reg_map["SPL"] = 0x5D;
	reg_map["OCR0"] = 0x5C;
	reg_map["GICR"] = 0x5B;
	reg_map["GIFR"] = 0x5A;
	reg_map["TIMSK"] = 0x59;
	reg_map["TIFR"] = 0x58;
	reg_map["SPMCR"] = 0x5F;
	reg_map["TWCR"] = 0x56;	
	reg_map["MCUCR"] = 0x55;
	reg_map["MCUCSR"] = 0x54;
	reg_map["TCCR0"] = 0x53;
	reg_map["TCNT0"] = 0x52;
	reg_map["OSCCAL"] = 0x51;
	reg_map["SFIOR"] = 0x50;
	reg_map["TCCR1A"] = 0x4F;
	reg_map["TCCR1B"] = 0x4E;
	reg_map["TCNT1H"] = 0x4D;
	reg_map["TCNT1L"] = 0x4C;
	reg_map["OCR1AH"] = 0x4B;
	reg_map["OCR1AL"] = 0x4A;
	reg_map["OCR1BH"] = 0x49;
	reg_map["OCR1BL"] = 0x48;
	reg_map["ICR1H"] = 0x47;
	reg_map["ICR1L"] = 0x46;
	reg_map["TCCR2"] = 0x45;
	reg_map["TCNT2"] = 0x44;
	reg_map["OCR2"] = 0x43;
	reg_map["ASSR2"] = 0x42;
	reg_map["WDTCR"] = 0x41;
	reg_map["UCSRC"] = 0x40;
	reg_map["EEARH"] = 0x3F;
	reg_map["EEARL"] = 0x3E;
	reg_map["EEDR"] = 0x3D;
	reg_map["EECR"] = 0x3C;
	reg_map["PORTA"] = 0x3B;
	reg_map["DDRA"] = 0x3A;
	reg_map["PINA"] = 0x39;
	reg_map["PORTB"] = 0x38;
	reg_map["DDRB"] = 0x37;
	reg_map["PINB"] = 0x36;
	reg_map["PORTC"] = 0x35;
	reg_map["DDRC"] = 0x34;
	reg_map["PINC"] = 0x33;
	reg_map["PORTD"] = 0x32;
	reg_map["DDRD"] = 0x31;
	reg_map["PIND"] = 0x30;
	reg_map["SPDR"] = 0x2F;
	reg_map["SPSR"] = 0x2E;
	reg_map["SPCR"] = 0x2D;
	reg_map["UDR"] = 0x2C;
	reg_map["UCSRA"] = 0x2B;
	reg_map["UCSRB"] = 0x2A;
	reg_map["UBRRL"] = 0x29;
	reg_map["ACSR"] = 0x28;
	reg_map["ADMUX"] = 0x27;
	reg_map["ADCSRA"] = 0x26;
	reg_map["ADCH"] = 0x25;
	reg_map["ADCL"] = 0x24;
	reg_map["TWDR"] = 0x23;
	reg_map["TWAR"] = 0x22;
	reg_map["TWSR"] = 0x21;
	reg_map["TWBR"] = 0x20;
	
	//16 bit aliases
	reg_map["TCNT1"] = 0x4C;
	reg_map["OCR1A"] = 0x4A;
	reg_map["OCR1B"] = 0x48;
	reg_map["ICR1"] = 0x46;
	reg_map["EEAR"] = 0x3E;
	reg_map["ADC"] = 0x24;

}

SRMega32::~SRMega32(){

}

void SRMega32::WriteIO(int addr, int data)
{
	unsigned char report[6];
	report[0] = 2;
	report[1] = 1;
	report[2] = addr >> 8;
	report[3] = addr & 0xff;
	report[4] = data >> 8;
	report[5] = data & 0xff;
	sendreport(&report[0], 6);
}

int SRMega32::ReadIO(int addr)
{
	unsigned char report[6];
	report[0] = 2;
	report[1] = 0;
	report[2] = addr >> 8;
	report[3] = addr & 0xff;
	report[4] = 0;
	report[5] = 0;
	sendreport(&report[0], 6);
	recvreport(&report[0], 6);
	return report[4]*256+report[5];
}
/***************************************************
PWM On the ATMega32.

The ATMega32 features many PWM options on a total of 4 different outputs, in order to simplify things for the end user
we make several restrictions and assumptions

Pin 1 is the outputs of Timer0, an 8 bit timer which is configured for fast PWM mode (WGM01:0 = 11).
In fast PWM mode TCNT0 counts from 0 to 255, and is compared against OCR0, if OCR0 < TCNT0, the output is high.
Hence our duty cycle is controlled by seting OCR0.
Frequency is controlled by selecting the clock divider which increments TCNT0, which gives us 5 fixed frequencies of 
47 KHz, 5.9KHz, 732 Hz, 183 Hz and 46 Hz. 
The output is controlled on/off by selecting COM01:0 = 10 (non-inverting), and disabled by setting COM01:0 = 00 (pin disconnected)

Pin 4 is the output of Timer2 which is similar to Timer0, with different register names/bit names.

Pins 2 and 3 are controlled by Timer 1, a 16 bit timer.
It is configured to operate in mode #14 (WGM13:10 = 1110) which is fast PWM with a top value of ICR1.
TCTNT1 counts from 0 to ICR1, and is compared against OCR1A and OCR1B for pins 2 and 3.
We use a fixed divider of 1 (TCNT1 increments every clock) and set ICR1 to select a frequency between 183 (ICR1=65535)
and 3MHz (ICR=3). 

Enabling/disabling of outputs is set similiarly to Pin1/4 by setting the COM1A1:0 and COM1B1:0 bits to 10 for on and 00 for off.
***************************************************/
double SRMega32::PWMSetup(int pin, double frequency)
{
	double fPCK = 12000000;
	
	if(pin < 1 || pin > 4) // Out of range.
		return -1;
		
	if(frequency < 0)
	{	if(pin == 1)
			(*this)["TCCR0"] = 0;
		else if(pin==4)
			(*this)["TCCR2"] = 0;
		else {
			(*this)["TCCR1A"] = 0;
			(*this)["TCCR1B"] = 0;
		}
		return 0;
	}
	else if(frequency == 0 ){ //just want to read it
		unsigned char divider;
		int TOP = 255;
		if(pin == 1)
			divider = (*this)["TCCR0"]&0x07;
		else if(pin == 4)
			divider = (*this)["TCCR2"]&0x07;
		else{
			
			if(pin == 2)
				TOP = (*this)["OCR1A"];
			else if(pin == 3)
				TOP = (*this)["OCR1B"];
			return fPCK/(TOP+1);
		}
		if (divider == 1)	
			return fPCK/(TOP+1);
		else if (divider == 2)	
			return fPCK/(8*(TOP+1));
		else if (divider == 3)	
			return fPCK/(64*(TOP+1));
		else if (divider == 4)	
			return fPCK/(256*(TOP+1));
		else if (divider == 5)	
			return fPCK/(1024*(TOP+1));
	}
	else{//--- want to set the frequency.
		if(pin == 1 || pin == 4){
			//pin 1 and 4 can only take on fixed frequencies, we round to the nearest.
			unsigned char divider = 0; 
			float fdivider;
			if(frequency > 26367){
				divider = 1; //46875
				fdivider = 1;}
			else if(frequency > 3296){
				divider = 2; //5859
				fdivider = 8;
			}
			else if(frequency > 458){
				divider = 3; //732
				fdivider = 64;
			}
			else if(frequency > 114){
				divider = 4; //183
				fdivider = 256;
			}
			else{
				divider = 5; //46
				fdivider = 1024;
			}
			if(pin == 1)
				(*this)["TCCR0"] = 0x48 | divider; //0x48 = WGM1 + WGM0
			else
				(*this)["TCCR2"] = 0x48 | divider; //0x48 = WGM1 + WGM0

			return fPCK/(fdivider*256);
		}
		else{// pin 2 or 3
						
			//we enforce a fixed divider of 1 gives limits from 183Hz to 4MHz
			if(frequency < 183)
				frequency = 183;
			if(frequency > 3000000)
				frequency = 3000000;
			

			int TOP = (int)round(fPCK/frequency - 1);
			unsigned char TCCR1A, TCCR1B;
			TCCR1A = (*this)["TCCR1A"];
			TCCR1B = (*this)["TCCR1B"];
			(*this)["TCCR1A"] = TCCR1A & 0x02; //set WGM11
			(*this)["TCCR1B"] = TCCR1B & 0x19; //set WGM13, WGM12 and CS10
			(*this)["ICR1"] = TOP;

			return fPCK/(TOP+1);
		}
	}
 return(-1) ;
}


double SRMega32::PWM(int pin, double duty_cycle){
//	unsigned char TCCR1A, OCRx, OCR1C;
	if (pin < 1 || pin > 4) // out of range PWM.
		return -1;
		
	//--- Read duty cycle
	if(duty_cycle >1 ){
		int OCRx;
		int TOP = 255;
		if(pin == 1)
			OCRx = (*this)["OCR0"];
		else if (pin == 4)
			OCRx = (*this)["OCR2"];
		else{
			TOP = (*this)["ICR1"];
			if(pin == 2)
				OCRx = (*this)["OCR1A"];
			else //pin == 3
				OCRx = (*this)["OCR1B"];
		}
		return (double)OCRx/(double)TOP;
	}
	
	//Enable/disable PWM outptus
	if(pin == 1){
		unsigned char TCCR0 = (*this)["TCCR0"] & 0xCF; //xx00xxxx
		if (duty_cycle > 0)
			TCCR0 |= 0x20; //xx10xxxx
		(*this)["TCCR0"] = TCCR0;
	}
	else if(pin == 4){
		unsigned char TCCR2 = (*this)["TCCR2"] & 0xCF; //xx00xxxx
		if (duty_cycle > 0)
			TCCR2 |= 0x20; //xx10xxxx
		(*this)["TCCR2"] = TCCR2;
	}
	else{// pin 2,3 can have different duty cycles.
		unsigned char TCCR1A = (*this)["TCCR1A"];
		if(pin == 2){
			TCCR1A &= 0x3F; //00xxxxxx
			if(duty_cycle > 0)
				TCCR1A |= 0x80; //10xxxxxx
		}
		else {//pin == 3
			TCCR1A &= 0xCF;	//xx00xxxx
			if(duty_cycle > 0)
				TCCR1A |= 0x20; //xx10xxxx
		}
		(*this)["TCCR1A"] = TCCR1A;
	}
	
	if(duty_cycle < 0)  //If we were just disabling an output we can return.
		return 0;
	
	
	if(pin == 1){
		(*this)["OCR0"] = (int)round(255.0*duty_cycle);
		return round(255.0*duty_cycle)/255.0;
	}
	else if(pin == 4){
		(*this)["OCR2"] = (int)round(255.0*duty_cycle);
		return round(255.0*duty_cycle)/255.0;
	}
	else if(pin == 2){
		int TOP = (*this)["ICR1"];
		(*this)["OCR1A"] = (int)round((double)TOP*duty_cycle);
		return round((double)TOP*duty_cycle)/TOP;
	}
	else if(pin == 3){
		int TOP = (*this)["ICR1"];
		(*this)["OCR1B"] = (int)round((double)TOP*duty_cycle);
		return round((double)TOP*duty_cycle)/TOP;
	}
		
	return -1;
}
	
	
int SRMega32::ADCRead(int pin)
{	
	(*this)["ADMUX"] = pin & 0x1F;                 // select ADC pint ot use.
        (*this)["ADCSRA"] = (*this)["ADCSRA"] | 0x40 ; // start conversion
	return (*this)["ADC"];                         // read conversion result.
}

//================= pin # to IC pin # ================================================

int SRMega32::pin_to_IC(int pin) 
{ switch (pin)
   { case 1:  return(4)  ; break ;
     case 2:  return(19) ; break ;
     case 3:  return(18) ; break ;
     case 4:  return(21) ; break ;
     default: return(-1) ;
   }
}
