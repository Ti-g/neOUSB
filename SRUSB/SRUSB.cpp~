//SRUSB.cpp
#include "SRUSB.h"

/*********************************************************************************************/
void SRUSB::recvreport(unsigned char* report, int length){
	int usberror;
	if ( (usberror = usbGetReport(dev, USB_HID_REPORT_TYPE_FEATURE, report[0], (reinterpret_cast<char*>(report)), &length))) {
		SRUSBErr err;	
		err.usberror = usberror;
		err.srusberr = SRUSB_ERROR_GET;
		throw err;
	}
//	if(length==6)
//		printf("recv: %x %x %x %x %x %x\n",report[0],report[1],report[2],report[3],report[4],report[5]);
}

void SRUSB::sendreport(unsigned char* report, int length){
	int usberror;
	if ( (usberror = usbSetReport(dev, USB_HID_REPORT_TYPE_FEATURE, (reinterpret_cast<char*>(report)), length))) {
		SRUSBErr err;	
		err.usberror = usberror;
		err.srusberr = SRUSB_ERROR_SET;
		throw err;
	}
}

SRUSB::reg SRUSB::operator[](std::string sreg){
	int addr = reg_map[sreg];
	if(!addr){
		SRUSBErr err;	
		err.usberror = 0;
		err.srusberr = SRUSB_BAD_REG;
		throw err;
	}
	SRUSB::reg reg;
	reg.addr = addr;
	reg.parent = this;
	return reg;
}

SRUSB::reg SRUSB::operator[](int addr){
	SRUSB::reg reg;
	reg.addr = addr;
	reg.parent = this;
	return reg;
}

SRUSB::SRUSB(){
	eeprom.parent = this;
//	printf("hooking up eeprom\n");
}

SRUSB::~SRUSB(){
//	printf("bye from ~SRUSB()\n");
}

void SRUSB::open(int vendor_n, std::string vendor_s, int product_n, std::string product_s){
	int err;
	//The usbOpenDevice function requires non-const strings, even though it doesn't modify them.
	char vendor[vendor_s.size()+1]; //plus one for \0
	strcpy(vendor, vendor_s.c_str());
	char product[product_s.size()+1];
	strcpy(product, product_s.c_str());
	
	if((err = usbOpenDevice(&dev, vendor_n, vendor, product_n, product, 1)) != 0){
		SRUSBErr err;	
		err.usberror = 0;
		err.srusberr = SRUSB_ERROR_DEVICE_NOT_FOUND;
		throw err;
    }
}



/*********************************************************************************************/

SRUSB::reg& SRUSB::reg::operator=(const int& right) {
	parent->WriteIO(addr,right);
	return *this;
}

SRUSB::reg::operator int() const {
	return parent->ReadIO(addr);
}
/*
SRMega32::SRMega32() {
	printf("new mega32 created\n");
	printf("open(.....)\n");
}*/



/*

*/

