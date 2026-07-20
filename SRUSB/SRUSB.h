//HIDpoke.h
#ifndef __SRUSB_h_INCLUDED__
#define __SRUSB_h_INCLUDED__

 extern "C" {
 #include "usbcalls.h"
 }

#include <string>
#include <map>
#include <cmath>

#define SRUSB_ERROR_NO_ERROR 0
//return "usb error" in the error code function, for niceness
#define	SRUSB_ERROR_DEVICE_NOT_FOUND 1
#define SRUSB_ERROR_BAD_LENGTH 2
#define SRUSB_ERROR_SET 3
#define SRUSB_ERROR_GET 4
#define SRUSB_BAD_REPORT 5
#define SRUSB_BAD_PORT 6
#define SRUSB_BAD_REG 7

class SRTiny26;

//need a constructor/destructor
class SRUSBErr
{
	friend class SRUSB;
	
	friend class SRTiny26;	//this needs to go
	int usberror;
	int srusberr;
public:
	int getUSBErr() {return usberror;}
	int getSRUSBErr() {return srusberr;}
	std::string getUSBErrMsg() {
		switch (usberror){
		case 2:
			return "Device not found";
		case 5:
			return "I/O error";
		default:
			return "No USB error";
		}
	}
	std::string getSRUSBErrMsg() {
		switch (srusberr){
		case 1:
			return "Device not found";
		case 2:
			return "Bad length of report";
		case 3:
			return "Bad set";
		case 4:
			return "Bad get";
		case 5:
			return "Bad report";
		case 6:
			return "Invalid port";
		case 7:
			return "Invalid Register";
		default:
			return "No SRUSB error";
		}
	}
};

class SRUSB
{
protected:
	usbDevice_t *dev;	//handle to a usb device
	//next we define the subclasses used for registor/memory and eeprom control.
	class reg;
	class eeprom;
	friend class reg;
	friend class eeprom;
	class reg {
		friend class SRUSB;
		unsigned int addr;
		SRUSB *parent;
	public:
		SRUSB::reg& operator=(const int& right);
		operator int() const;
	};
	class eeprombyte {
		friend class SRUSB;
		unsigned int addr;
		SRUSB *parent;
	public:
		SRUSB::eeprombyte& operator=(const unsigned char& right);
		operator int() const;
	};
	class eeprom_t {
		friend class SRUSB;
		SRUSB *parent;
	public:	
		SRUSB::eeprombyte operator[](int addr);
	};	
	int usberror;
	int srusberr;
	std::map<std::string, int> reg_map;
	
	void recvreport(unsigned char* report, int length);
	void sendreport(unsigned char* report, int length);
	
	void open(int vendor_n, std::string vendor_s, int product_n, std::string product_s);
	
public:
	eeprom_t eeprom;
	virtual void WriteIO(int addr, int data) = 0;
	virtual int ReadIO(int addr) = 0; 
	/*virtual*/ void WriteEEPROM(int addr, unsigned char byte);
	/*virtual*/ unsigned char ReadEEPROM(int addr);
	
	SRUSB::reg operator[](std::string sreg);
	SRUSB::reg operator[](int addr);
	
	SRUSB();
	virtual ~SRUSB();
	
};







#endif
