#================ Open-USB-IO Command Line ======================

# Make file for Open Open-USB-IO Commandline
# License: GPL 2.0

#====== variables that may need altering.
#       For Windows ensure there are no blanks after "WINDOWS".
# OS either WINDOWS LINUX MAC
OS= LINUX
# For Linux only, if want libraries included in executable add "-static"
# Non-static gives a 100kB program,  static is 1300kB program.
# Windows always adds library statically.
STATIC= -static
#STATIC = 
#DEBUG= -g
DEBUG=


#====== OS dependencies

ifeq ($(OS),LINUX)
USBFLAGS=   `libusb-config --cflags`
USBLIBS=    `libusb-config --libs`
EXE_SUFFIX=
endif

ifeq ($(OS),WINDOWS)
USBFLAGS=
USBLIBS=    -lhid -lusb-1.0 -lsetupapi
EXE_SUFFIX= .exe
endif


#====== Variable initializations.
CC=			gcc
CXX=			g++
CFLAGS=			-O2 -Wall $(USBFLAGS)
#CFLAGS=			-g -Wall $(USBFLAGS)
LIBS=			$(USBLIBS)

OBJ=		SRUSB/SRUSB.o SRUSB/SRTiny26.o SRUSB/SRMega32.o SRUSB/usbcalls.o  SRUSB/service.o
HEADERS=	SRUSB/hidsdi.h SRUSB/SRMega32.h SRUSB/SRUSB.h SRUSB/usbcalls.h SRUSB/service.h
PROGRAMm32=	ousb$(EXE_SUFFIX)
PROGRAMt26=	t26$(EXE_SUFFIX)


#====== Dependencies and actions.

$(PROGRAMm32): $(OBJ) m32.o
	rm -f main.o
	$(CXX) $(CFLAGS) $(OBJ) m32.o $(STATIC) $(LIBS)  -o $(PROGRAMm32) 

#$(PROGRAMt26): t26.o $(OBJ)
#	$(CXX) $(CFLAGS) t26.o $(OBJ) $(LIBS) -o $(PROGRAMt26) 

m32.o: main.cpp $(HEADERS)
	$(CXX) $(CFLAGS) $(DEBUG) -DSRM32 -c main.cpp -o m32.o

#t26.o: main.cpp $(HEADERS)
#	$(CXX) $(CFLAGS) -DSRT26 -c main.cpp -o t26.o

	
#===== pattern based rules

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $*.c -o $*.o

%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) $(DEBUG) -c $*.cpp -o $*.o


#===== PHONY rules

.PHONY:	strip	
strip: 
	strip $(PROGRAM)

.PHONY:	clean
clean:
	rm -f $(OBJ) *.o *~ SRUSB/*~ SRUSB/*.o
	
.PHONY: all
all:
	make clean
	make

