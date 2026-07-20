/*================ Open-USB-IO ===============================================

  PURPOSE - command line program for the PC to drive the Open-USB-IO board.
 
  BASED ON SRUSB by Daniel Salby and Bowen Rees 2007
  License: GPL 2.
  Dr. Pj Radcliffe 2009, 2010
 */

//------ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <cmath>
#include <dirent.h>
#include <math.h>
#include <cstdint>
#ifdef _WIN32
 #include <windows.h>
#endif

#include "SRUSB/service.h"

//------ define DEVICE to be the relevant device, based on makefile definitions.
#ifdef SRT26
#include "SRUSB/SRTiny26.h"
#define DEVICE SRTiny26
#endif

#ifdef SRM32
#include "SRUSB/SRMega32.h"
#define DEVICE SRMega32
#endif

using namespace std;

//================== variables and constants ===========================

DEVICE* device;

#define VERSION "1.6"

/*================== usage_exit ========================================

  PURPOSE - print program usage and quit program.
*/

void usage_exit(char *name)
{ fprintf(stderr, "\n ====== Open-USB-IO Command Line Controller: V%s ============\n\n", VERSION) ;

  fprintf(stderr, "  Usage:\n");
  fprintf(stderr, "    %s adc pin\n",name); 
  fprintf(stderr, "    %s io memory-address [new value] #rd/wr RAM.\n", name);
  fprintf(stderr, "    %s io register-name [new value]  #rd/wr registers.\n", name);
  fprintf(stderr, "    %s pwm-freq pin [off|new value]  #set frequency.\n",name);
  fprintf(stderr, "    %s pwm pin [off|new %% value]     #set %% duty cycle.\n",name);
#ifdef SRM32
  fprintf(stderr, "\n");
  fprintf(stderr, "  The following require V3 firmware or better.\n");
  fprintf(stderr, "    %s bp number|-number|clear|cont  #breakpojnt control.\n",name);
  fprintf(stderr, "    %s ee memory-address [new value] #rd/wr EEPROM.\n", name);
  fprintf(stderr, "    %s symr symbol_name | address #read ram/ee variables.\n",name);
  fprintf(stderr, "    %s symw symbol_name | address #write ram/ee variables.\n",name);
  fprintf(stderr, "    %s user 8bit 16_bit 16_bit    #user commands.\n",name);
  fprintf(stderr, "    %s ver                        #tell version numbers.\n",name);
#endif
  fprintf(stderr, "\n");
  fprintf(stderr, "  To change the base of printed values add option-\n");
  fprintf(stderr, "    -b : binary\n");
  fprintf(stderr, "    -h or -x : hex\n");
  fprintf(stderr, "    -r : raw (decimal with no other text)\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  To start in multi-line mode type \"ousb -multi\" then \"quit\" to finish.\n");
  fprintf(stderr, "  To use commands from a file type \"ousb -file file_name\".\n");
  fprintf(stderr, "  For more details visit www.pjradcliffe.wordpress.com\n\n") ;

  exit(1);
}


//===================== Links and interfaces ====================================

/**************************************************
String to Double
Slightly intelligent version of atoi(), allows using scientific notation e.g. 1.0e3
**************************************************/

double strtod(string str) //------------------------------------------------------
{
  double base, exp;

  string::size_type loc = str.find('e',0);
  if(loc == string::npos)
    loc = str.find('E',0);
  if(loc != string::npos)
   { sscanf((str.substr(0,loc)).c_str(),"%lf", &base);
     sscanf((str.substr(loc+1)).c_str(),"%lf", &exp);
     return base * pow((double)10,exp);
   }
  sscanf(str.c_str(),"%lf", &base);
  return base;
}



void print2(string var, int num, int base) //------------------------------------
{char str [200] ;
 STRNCPY( str, var.c_str(), sizeof(str)) ;
 print_sdxb(str, num, base) ;
}


//===================== SYMBOL ROUTINES ========================================


//---------------------- get_symbol_file ---------------------------------------

FILE* get_symbol_file()
{//--- find a .sym file in the working directory.
   struct dirent *dp;
   DIR *dirp ;
   dirp = opendir( "." );
   while ( (dp = readdir( dirp )) != NULL )
    { if ( strstr( dp->d_name, ".sym" ) == NULL )
         continue ; // not right file, try next
      break ;       // got the desired file so quit while loop.
    }
   closedir( dirp );
   if (dp == NULL)
     { printf("\n   ABORTING : no *.sym file found in working directory.\n\n") ;
       return NULL ;
     }
 //--- If got here found symbol file, name in dp->d_name.
 //      Now try to open the file.
 //    For live-DVD only had to copy dp->d_name to local string
 //    as fopen appeared to clear dp->d_name.
   FILE* fp ;
   char xstr [200] ;
   STRNCPY(xstr, dp->d_name, sizeof(xstr)) ;
   fp = fopen(xstr, "r");
   if (fp == NULL)
     { printf("\n   ABORTING : could not open %s.\n\n", xstr) ;
       return NULL ;
     }
 //--- if got here then got symbol file and opened it.
   return fp ;
}


//---------------------- get_sym_addr ----------------------------------------
//
//  PURPOSE get symbol address from sym file, else return 0xFFFF.

int get_sym_addr(FILE* fp, char* symbol_name) 
{  // int read;
   char * paddr ;
   char * ptype ;
   char * pname ;
   //size_t len = 0;
   char line [200] ;
   line[0] = (char) 0 ;
   char * linep = &line[0] ;
//--- search through file.
   rewind(fp) ;
   //while ((read = getline(&linep, &len, fp)) != -1)  // minGW lacks getline
   while ((fgets(line, sizeof(line), fp)) != NULL)
    {  paddr = strtok(linep, " ") ;  trim_str_all(paddr) ;
       ptype = strtok(NULL, " ") ;   trim_str_all(ptype) ;
       pname = strtok(NULL, " ") ;   trim_str_all(pname) ;
       if ( strcmp(pname, symbol_name) != 0 )
         continue ; // wrong symbol, try next line
       if ( ( *ptype != 'B' )  && ( *ptype != 'D' ))
         return(0xFFFF)   ; // right symbol but not of reliable type.
     //--- if got here then found symbol of right type.
     //--- fix up address,  remove feral 8.
       paddr += 2 ;
       *paddr = 'x' ;
       paddr -= 1 ;
       return (strtoi( paddr)) ;
    }//while
 //--- if got here no match.
   return(0xFFFF) ;
}


//---------------------- symbol_print_value ------------------------------------

void symbol_print_value( FILE* fp,          char* symbol_name, 
                         char* symbol_type, int read_length, int base, bool quiet,
                         bool ee) 
{//--- attempt to open the 
   char line [200] ;
   line[0] = (char) 0 ;
   int  the_addr ;
   unsigned char* cptr ;
     //--- find symbol address.
       if ( *symbol_name == '=')
            the_addr = strtoi( ++symbol_name) ;
       else the_addr = get_sym_addr(fp, symbol_name) ;
       if (the_addr == 0xFFFF)
         { printf("  Symbol %s not found.  Variable without volatile?\n", symbol_name) ;
           return ;
         }
     //--- make default string length 200 characters.
       if ( strcmp("s", symbol_type) == 0 )
        if ( read_length < 200)
          read_length = 200 ;

     //--- print out values.
       if ( !quiet)
         printf("  %s (type '%s' at 0x%x) length %i is: ", symbol_name, symbol_type, the_addr, read_length) ;
       int j ;
       long long int jl ;
       for (int i = 1 ; i <= read_length ; i++ ) 
        {//--- read from RAM or EEPROM
                if (ee)
                     cptr = device->ReadEE(the_addr) ;
                else cptr = device->read_ram_5(0,the_addr,0) ;
         //--- given symbol type decide what to do.
                if ( strcmp("c", symbol_type) == 0 )
                 { printf("%c ", *cptr) ;
                   ++the_addr ;
                 }
           else if ( strcmp("s", symbol_type) == 0 )
                 { if ( *cptr == (char) 0)
                    break ; // terminating null
                   printf("%c", *cptr) ;
                   ++the_addr ;
                 }
           else if (( strcmp("uc", symbol_type) == 0 ) || ( strcmp("u08", symbol_type) == 0 ))
                 { j = (int) *cptr ;
                   print_dbx(j, base) ;
                   the_addr += 1 ;
                 }
           else if (( strcmp("s08", symbol_type) == 0 ) || ( strcmp("sc", symbol_type) == 0))
                 { j = (int) *cptr ;
                   base = 10 ;
                   if ( j > 0x80)
                     j = j - 0x100 ;
                   print_dbx(j, base) ;
                   the_addr += 1 ;
                 }
           else if (( strcmp("u16", symbol_type) == 0) || ( strcmp("ui", symbol_type) == 0))
                 { j = (int) *cptr++ ;
                   j += (int)*cptr << 8 ;
                   print_dbx(j, base) ;
                   the_addr += 2 ;
                 }
           else if (( strcmp("i", symbol_type) == 0 ) || ( strcmp("si", symbol_type) == 0 ) || ( strcmp("s16", symbol_type) == 0))
                 { j = (int) *cptr++ ;
                   j += (int)*cptr << 8 ;
                   base = 10 ;
                   if ( j >= 0x8000 )
                      j = j - 0x10000 ;
                   print_dbx(j, base) ;
                   the_addr += 2 ;
                 }
           else if (( strcmp("uli", symbol_type) == 0 ) || ( strcmp("u32", symbol_type) == 0))
                 { jl  = (unsigned char)*cptr++ ;
                   jl |= (unsigned char)*cptr++ << 8 ;
                   jl |= (unsigned char)*cptr++ << 16 ;
                   jl |= (unsigned char)*cptr   << 24 ;
                   print_dbx(jl, base) ;
                   the_addr += 4 ;
                 }
           else if (( strcmp("li", symbol_type) == 0 ) || ( strcmp("s32", symbol_type) == 0))
                 { jl =  (unsigned char) *cptr++ ;
                   jl += (unsigned char)*cptr++ << 8 ;
                   jl += (unsigned char)*cptr++ << 16 ;
                   jl += (unsigned char)*cptr   << 24 ;
                   base = 10 ;
                   if ( jl >= 0x80000000 )
                      jl = jl -  0xffffffff -1 ;
                   print_dbx(jl, base) ;
                   the_addr += 4 ;
                 }
           else if (( strcmp("df", symbol_type) == 0 ) || ( strcmp("f", symbol_type) == 0) || ( strcmp("e", symbol_type) == 0))
                 {union{ float f; unsigned char i[4] ; } fsi ;
                   fsi.i[0] = (int) *cptr++ ;
                   fsi.i[1] = (int) *cptr++ ;
                   fsi.i[2] = (int) *cptr++ ;
                   fsi.i[3] = (int) *cptr++ ;
                   if ( strcmp("e", symbol_type) == 0)
                        printf(" %e ",fsi.f) ;
                   else printf(" %f ",fsi.f) ;
                   the_addr += 4 ;
                 }
            else // not a valid base.
                   printf(" Invalid Type ") ;
        } //for

       printf("\n") ;
       return ; // printed this symbol so return. 
}

//====================================== main ==================================

int old_main(int argc, char **argv) {
try{
  //--- set default parameters.
  int syntax_err = 1;           // by default its an error.
  int  base = 10 ;              // base used for output or symbol work.

  //for (int z = 0 ; z < argc ; z++)
  //  printf(" %s ", argv[z]) ;
  //printf("\n") ;

  //--- loop through command line, starting at 1 to skip name of program.
  for(int i = 1; i < argc; i++)
   {strtolower( argv[i]) ;
    if(strcmp(argv[i], "-b")==0)
      base=2;
    if(strcmp(argv[i], "-h")==0)
      base=16;
    if(strcmp(argv[i], "-x")==0)
      base=16;
    if(strcmp(argv[i], "-r")==0)
      base=0;

  //------ io command.
    if(strcmp(argv[i], "io")==0)
     { syntax_err = 0;
       if(argc < i + 2)
         { unsigned char* cptr = device->read_io_3() ;
           print_sdxb( "   PINA", *(cptr++), base) ;
           print_sdxb( "   PINB", *(cptr++), base) ;
           print_sdxb( "   PINC", *(cptr++), base) ;
           print_sdxb( "   PIND", *(cptr), base) ;
           return 0 ;
         }
       //need to check if we have a read "io x" or a write "io x y", y will always start with an numeric character.
       if(argc > i + 2) // we have at least 2 more variables after io
         if(isdigit(argv[i+2][0]))
           {//first character of the variable will be a digit if it's a number (incl hex/binary), 
            //  but will not be for any commands
            if(isdigit(argv[i+1][0]))  //write to memory address, as opposed to a named register
              (*device)[strtoi(argv[i+1])] = strtoi(argv[i+2]); 
              //convert parameters to integers and call member functions
            else
              (*device)[ strtoupper( argv[i+1]) ] = strtoi(argv[i+2]); 

            print2(argv[i+1],strtoi(argv[i+2]),base);
            i += 2; //Increment i by 2 to reflect the extra 2 parameters we have processed.
            continue;
           }
       // if get here must be read.
       int result;
       if(isdigit(argv[i+1][0]))  //memory address, as opposed to a named register
         result = (*device)[strtoi(argv[i+1])];
       else
         result = (*device)[ strtoupper(argv[i+1]) ];
       print2(argv[i+1],result,base);
       return 0 ;
     } //io

  //------ ee command.
    if(strcmp(argv[i], "ee")==0)
     { syntax_err = 0;
       if(argc < i + 2)
          usage_exit(argv[0]);
       if (argc == (i + 2))
            { int addr = strtoi(argv[i+1]) ;
              unsigned char * cptr = device->ReadEE( addr) ;
              printf("  EEPROM address %s:", argv[i+1]) ;
              for (int k=1; k <= 5 ; k++)
                print_dbx( *(cptr++), base) ;
              printf("\n") ;
            }
       else device->WriteEE( strtoi(argv[i+1]), strtoi(argv[i+2]) ) ;
       return 0 ;
     } //ee

    //------ pwm freq
    if(strcmp(argv[i], "pwm-freq")==0){
      syntax_err = 0;
      if(argc < i + 2) // abort if not enough parameters.
        usage_exit(argv[0]);

      //check if we are changing or just reading
      double freq = 0;
      int pin = strtoi(argv[i+1]);
      if (( pin > 4) || (pin < 1))
        usage_exit(argv[0]);

      if(argc > i + 2){ //if this is false then freq stays at 0 and we issue a read
        if(isdigit(argv[i+2][0])){
          freq = strtod(argv[i+2]);
          i++;
        }
        else if(strcmp(argv[i+2],"off")==0){
          freq = -1;
          i++;
        }
      }
      i++;
      freq = device->PWMSetup(pin, freq);
      if(base==0) //raw
        printf("%f",freq);
      else
        printf("   PWM #%d on pin %d operating at %f Hz\n",pin, device->pin_to_IC(pin), freq);
      return 0 ;
    }//pwm-freq

    //------ pwm duty cycle.
    if(strcmp(argv[i], "pwm")==0)
      {  syntax_err = 0;
         if(argc < i + 2)
           usage_exit(argv[0]);

         //check if we are changing or just reading
         double duty = 2; //setting a value >1 results in a read operation being performed
         int pin = strtoi(argv[i+1]);
         if (( pin > 4) || (pin < 1))
           usage_exit(argv[0]);

         if(argc > i + 2){
           if(isdigit(argv[i+2][0])){
             duty = strtod(argv[i+2])/100;
             i++;
          }
         else if(strcmp(argv[i+2],"off")==0){
               duty = -1;
            i++;
          }
         }
         i++;
         duty = device->PWM(pin,duty);
         if(base == 0) //raw
           printf("%f",duty);
         else
           printf("   PWM #%d on pin %d operating at a duty cycle of %f\n", pin, device->pin_to_IC(pin), duty);
         return 0 ;
      }//pwm

    //------ adc
    if(strcmp(argv[i], "adc")==0)
     {  syntax_err = 0;
        if(argc < i + 2)
          usage_exit(argv[0]);
        print2(string("   ADC")+argv[i+1], device->ADCRead(strtoi(argv[i+1])), base);
        return 0 ;
     }//adc

    //------ version
    if(strcmp(argv[i], "ver")==0)
     {  unsigned char* cptr = device->read_io_3() ;
        cptr += 4 ;
        printf("   ATMEGA32 firmware version %i.   PC ousb version %s\n", *cptr, VERSION);
        return 0 ;
     }//ver

    //------ user commands
    if(strcmp(argv[i], "user")==0)
     {  syntax_err = 0;
        if(argc < i + 4)
          usage_exit(argv[0]);
        if (base != 0)
          printf("   user code return: ") ;
        unsigned char* cptr = device->user_command(strtoi(argv[i+1]),
                                                  strtoi(argv[i+2]),
                                                  strtoi(argv[i+3])) ;
        print_dbx( *cptr++, base) ;
        int i ;
        i = *cptr++ << 8 ; i += *cptr++ & 0xFF ;
        print_dbx( i, base) ;
        i = *cptr++ << 8 ; i += *cptr++ & 0xFF ;
        print_dbx( i, base) ;
        printf("\n") ;
        return 0 ;
     }//user

    //------ symbol read command
      if(strcmp(argv[i], "symr")==0)
       {//--- find a symbol file in the working directory.
          FILE* fp = get_symbol_file() ;
          if ( fp == NULL)
            return 0 ;
        //--- Now parse command line to get symbols to print out.
          syntax_err = 0;
          int got_sym = 0 ;             // T/F if got symbol name.
          char symbol_type[5] = "uc" ;  // default display type of symbol display.
          char symbol_name[50] = "" ;   // symbol name to print/write.
          int  read_length  = 1 ;       // number of items to print in symbol read.
          bool quiet = 0 ;              // T/F if printout in quiet mode.
          bool ee = 0 ;                 // if read really from eeprom.
          i++ ;                         // move onto next argv
          while ( i < argc)
           {  if ( argv[i][0] == '-')
               {//--- it may be a parameter.
                  ++argv[i] ; // get over the '-', its a base or a type.
                       if ( strcmp( argv[i], "h") == 0 )   base = 16 ;
                  else if ( strcmp( argv[i], "x") == 0 )   base = 16 ;
                  else if ( strcmp( argv[i], "d") == 0 )   base = 10 ;
                  else if ( strcmp( argv[i], "b") == 0 )   base = 2 ;
                  else if ( strcmp( argv[i], "q") == 0 )   quiet= 1 ;
                  else if ( strcmp( argv[i], "r") == 0 )   quiet= 1 ;
                  else if ( strcmp( argv[i], "ee") == 0 )  ee = 1 ;
                       else STRNCPY( symbol_type, argv[i], sizeof(symbol_type)) ;
                  i++ ; // to next argv
                  continue ;
               }
             if ( isdigit(argv[i][0]) )
              {// its a length to print.
                read_length = strtoi( argv[i]) ; 
              }
             else
              {//--- its an identifier to find in the symbol file.
                 if (got_sym == 0 )
                  { got_sym = 1 ; ;
                    STRNCPY( symbol_name, argv[i], sizeof(symbol_name) ) ;
                  }
                 else 
                  {//--- must be 2nd symbol to print,  so print first then save new value.
                     symbol_print_value( fp, symbol_name, symbol_type, read_length, base, quiet, ee) ;
                     STRNCPY( symbol_name, argv[i], sizeof(symbol_name) ) ;
                     read_length = 1 ; 
                  }
              }
             //--- move onto next command line parameter.
             i++ ;
           } // end looking at command line parameters.
        //--- clean up from symr
          if ( got_sym == 1 )
            symbol_print_value( fp, symbol_name, symbol_type, read_length, base, quiet, ee) ;
          fclose( fp) ;
          return 0 ;
         }//symr

    //------ symbol write command
      if(strcmp(argv[i], "symw")==0)
       {//--- find a symbol file in the working directory.
          FILE* fp = get_symbol_file() ;
          if ( fp == NULL)
            return 0 ;
        //--- symbol file now open and ready to parse, now parse command line to get write.
          syntax_err = 0;
          bool got_sym = 0 ;            // T/F if got symbol name.
          char symbol_type[5] = "uc" ;  // default display type of symbol display.
          bool quiet = 0 ;              // T/F if printout in quiet mode.
          bool ee = 0 ;                 // if read really from eeprom.
          uint16_t addr = 0xFFFF ;
          i++ ;                         // move onto next argv
          while ( i < argc)
           { if ( ( argv[i][0] == '-') && !isdigit(argv[i][1]))
               {//--- its a parameter.
                  ++argv[i] ; // get over the '-', its a base or a type.
                       if ( strcmp( argv[i], "h") == 0 )   base = 16 ;
                  else if ( strcmp( argv[i], "x") == 0 )   base = 16 ;
                  else if ( strcmp( argv[i], "d") == 0 )   base = 10 ;
                  else if ( strcmp( argv[i], "b") == 0 )   base = 2 ;
                  else if ( strcmp( argv[i], "q") == 0 )   quiet= 1 ;
                  else if ( strcmp( argv[i], "ee") == 0 )  ee = 1 ;
                       else STRNCPY( symbol_type, argv[i], sizeof(symbol_type)) ;
                  i++ ; // to next argv
                  continue ;
               }
             if (argv[i][0] == '"') // string write.
               {  if (!got_sym)
                    { printf("  No symbol, aborting.\n") ;
                      return 0 ;
                    }
                  int j = 0 ;
                  do device->write_ram_ee( 1, addr++, argv[i][++j], ee) ; 
                  while ( argv[i][j] != (char) 0 ) ;
                  i++ ; // on to next parameter.
                  continue ;
               }
             if (argv[i][0] == '/') // char write.
               {  if (!got_sym)
                    { printf("  No symbol, aborting.\n") ;
                      return 0 ;
                    }
                  int j = 0 ;
                  while ( argv[i][++j] != (char) 0 ) 
                    device->write_ram_ee( 1, addr++, argv[i][j], ee) ;
                  i++ ; // on to next parameter.
                  continue ;
               }
             if ( !isdigit(argv[i][0])  && (argv[i][0] != '-')) // not a parameter, not number, its a var name.
              { if ( argv[i][0] == '=')
                     addr = strtoi( ++argv[i]) ; // get over =
                else addr = get_sym_addr(fp, argv[i]) ;
                if (addr != 0xFFFF)
                     got_sym = 1 ;
                else got_sym = 0 ;
              }
             else // is a digit, value to write.
              { if (!got_sym)
                 { printf("  No symbol, aborting.\n") ;
                   return 0 ;
                 }
                     if ((strcmp( symbol_type, "uc")  == 0) ||
                         (strcmp( symbol_type, "u08") == 0) ||
                         (strcmp( symbol_type, "sc") == 0) ||
                         (strcmp( symbol_type, "s08") == 0))
                          device->write_ram_ee( 1, addr++, strtoi(argv[i]), ee) ;
                else if ((strcmp( symbol_type, "u16") == 0) ||
                         (strcmp( symbol_type, "s16") == 0) ||
                         (strcmp( symbol_type, "si")  == 0) ||
                         (strcmp( symbol_type, "ui")  == 0) ||
                         (strcmp( symbol_type, "i")   == 0))
                          { device->write_ram_ee( 2, addr, strtoi(argv[i]), ee) ;
                            addr += 2 ;
                          }
                else if ((strcmp( symbol_type, "li")  == 0) ||
                         (strcmp( symbol_type, "uli") == 0) ||
                         (strcmp( symbol_type, "u32") == 0) ||
                         (strcmp( symbol_type, "s32") == 0))
                          { int z = strtoi( argv[i]) ;
                            device->write_ram_ee( 2, addr, z & 0xFFFF, ee) ;
                            addr += 2 ;
                            device->write_ram_ee( 2, addr, z >> 16, ee) ;
                            addr += 2 ;
                          }
                else if ((strcmp( symbol_type, "df")  == 0) ||
                         (strcmp( symbol_type, "e")   == 0) ||
                         (strcmp( symbol_type, "f")   == 0) )
                          { union{ float f; unsigned char i[4] ; } fsi ;
                            fsi.f = atof(argv[i]) ;
                            device->write_ram_ee( 1, addr++, fsi.i[0], ee) ;
                            device->write_ram_ee( 1, addr++, fsi.i[1], ee) ;
                            device->write_ram_ee( 1, addr++, fsi.i[2], ee) ;
                            device->write_ram_ee( 1, addr++, fsi.i[3], ee) ;
                          }
                else printf(" Invalid Type ") ;
              }
             //--- move onto next command line parameter.
             i++ ;
           } // end looking at command line parameters.
        //--- clean up from symw
          fclose( fp) ;
          return 0 ;
         }//symw

    //------ break point commands
      if(strcmp(argv[i], "bp")==0)
       {//--- find a symbol file in the working directory.
          FILE* fp = get_symbol_file() ;
          if ( fp == NULL)
            return 0 ;
        //--- get addr and values of of breakpoint and stop variables.
          int addr_bp   = get_sym_addr(fp, "breakpoint") ;
          if ( addr_bp == 0xFFFF)
            { printf("\n\n   Cannot find symbol file.\n\n") ;
              return 0 ;
            }
          int addr_stop = get_sym_addr(fp, "stop") ;
          if (addr_stop == 0xFFFF)
            { printf("\n\n   Cannot find variables breakpoint and stop.\n\n") ;
              return 0 ;
            }
          unsigned char* cptr ;
          cptr = device->read_ram_5(0,addr_bp,0) ;
          int breakpoint = (int) *cptr ;
          cptr = device->read_ram_5(0,addr_stop,0) ;
          int stop  = (int) *cptr ;
          int j ;
        //--- analyse command line.
          if (argc == (i + 1)) // just bp, show breakpoints and status.
            {  if (stop)
                    printf("   At breakpoint #%i.   ", stop) ;
               else printf("   Not at breakpoint.   ") ;
               if ( breakpoint ==0)
                       printf("   No breakpoints active.") ;
               else {  printf("Active breakpoints ( 1 to 8) = ") ;
                       for ( j = 1 ; j <= 8 ; j++)
                        {  if ( breakpoint & 1)
                           printf(" %i", j) ;
                            breakpoint = breakpoint >> 1 ;
                        }
                    }
               printf("\n") ;
            }
          for ( ;  i < argc ; i++ ) // work through command line.
           { if (strcmp(argv[i], "cont")==0) // continue from existing bp.
               {  device->write_ram_ee( 1, addr_stop, 0, 0) ;
                  continue ;
               }
             if (strcmp(argv[i], "clear")==0) // clear all bp.
               {  device->write_ram_ee( 1, addr_bp, 0, 0) ;
                  continue ;
               }
             if ( isdigit( argv[i][0])) // bp set
               {  j = atoi(argv[i]) ;
                  if ( (j<1) || (j>8))
                    { printf("\n\n   Invalid breakpoint %s\n\n", argv[i]) ;
                      continue ;
                    }
                  breakpoint |= (1 << (j-1)) ;
                  device->write_ram_ee( 1, addr_bp, breakpoint, 0) ;
                  continue ;
               }
             if ( (argv[i][0] == '-') && ( isdigit( argv[i][1]))) // bp clear
               {  j = -atoi(argv[i]) ;
                  if ( (j<1) || (j>8))
                    { printf("\n\n   Invalid breakpoint %s\n\n", argv[i]) ;
                      continue ;
                    }
                  breakpoint &= ~(1 << (j-1)) ;
                  device->write_ram_ee( 1, addr_bp, breakpoint, 0) ;
                  continue ;
               }
           } 
        //--- clean up from bp
          fclose( fp) ;
          return 0 ;
       }//bp

}

  if(syntax_err)
    usage_exit(argv[0]);

} //try
catch(SRUSBErr x)
{ std::cout << "Fatal Exception!\n";
  std::cout << "USB Error #: " << x.getUSBErr() << std::endl;
  std::cout << "USB Error Text: " << x.getUSBErrMsg() << std::endl;
  std::cout << "SRUSB Error #: " << x.getSRUSBErr() << std::endl;
  std::cout << "SRUSB Error Text: " << x.getSRUSBErrMsg() 
  << std::endl;
}
  return 0;
}


//---------------------------------------------------------------------------

int main(int argc, char **argv)
{//--- cope with multi-line ousb commands ( thanks ).
   char line[256];
   char *p;
   char *pline;
   char *paramv[10];
   int  paramc;
#ifdef _WIN32
#else
   struct timespec sleep_time ;
   sleep_time.tv_sec = 0 ;
   sleep_time.tv_nsec = 900000 ;
#endif
 //--- Print help info if no parameters, else start commands.
  if(argc < 2)
   usage_exit(argv[0]);

  device = new DEVICE ;

 //--- if first parameter is -multi the go for multi-line ousb.
   if (argc >= 2  &&  strcasecmp(argv[1], "-multi") == 0) //...................
    {//
       while (1) 
        {//--- get line to execute,  if empty string then delay to
         //      stop excessive CPU use, then try again.
           line[0] = (char) 0 ;
           pline = fgets(line, sizeof(line), stdin);
           fflush(stdin) ;
           trim_str_all( line) ;
           if ( strlen(line) == 0 )
              { 
#ifdef _WIN32
                Sleep(1) ; //Win32 Sleep is in ms.
#else
                nanosleep( &sleep_time, NULL) ;
#endif
                continue;
              }
           //printf(">>%s\n", line) ;
         //--- quit multi mode if get "quit".
           if (strcmp(line, "quit")==0)
             break ;
         //--- get params in line ready for call to old main.
           paramv[0] = argv[0];
           paramc = 1;
           p = strtok(pline, " ");
           while (p != NULL) 
             {  paramv[paramc ++] = p;
                p = strtok(NULL, " ");
             }
           old_main(paramc, paramv);
           fflush(stdout);
        }
    }
   else if  (argc >= 3  &&  strcasecmp(argv[1], "-file") == 0) //..............
    {//--- try to open the file
       FILE* fp ;
       fp = fopen(argv[2], "r");
       if (fp == NULL)
        { printf("\n   ABORTING : could not open file of commands %s.\n\n", argv[2]) ;
          return(0) ;
        }
     //--- loop to extract command lines from file and then excecute them.   
       while (1) 
        {//--- get line to execute,  if empty string then delay to
         //      stop excessive CPU use, then try again.
           line[0] = (char) 0 ;
           pline = fgets(line, sizeof(line), fp);
           trim_str_all( line) ;
           //printf(">>%s\n", line) ;
         //--- quit multi mode if get "quit".
           if (strcmp(line, "quit")==0)
             break ;
         //--- get params in line ready for call to old main.
           paramv[0] = argv[0];
           paramc = 1;
           p = strtok(pline, " ");
           while (p != NULL) 
             {  paramv[paramc ++] = p;
                p = strtok(NULL, " ");
             }
           old_main(paramc, paramv);
        }
       fclose(fp) ; 
    }
    //}
   else 
    { old_main(argc, argv); //.................................................
    }
    
   return(0) ; 
}




