/*---------- service.cpp ------------------------------------------------

  PURPOSE - this module contains odds and sods of generally useful routines
            used by a variety of routines.

  ROUTINES - see header for a summary.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>



#include "service.h"


//===================== Trim functions ==================================
//
//  PURPOSE - trim < ' ' from start, end, and both ends of a string.

void trim_str_end( char *str)
{ int str_pos ;
  str_pos = strlen( str) -1;
  while (    (*(str + str_pos) < ' ')
          && ( str_pos >=0)
        )
   { *(str + str_pos) = 0 ;
     --str_pos ;
   } ;
} ;

void trim_str_start( char *str)
{
  while ( ( *str <= ' ') && ( strlen( str) > 0) )
   { strcpy( str, str+1) ;
   } ;
} ;

void trim_str_all( char *str)
{ trim_str_start( str) ;
  trim_str_end( str) ;
}


//===================== Case conversions ==============================


char* strtolower( char* str)
{   int i ;
    for( i = 0; str[ i ]; i++)
       str[ i ] = tolower( str[ i ] );
    return( str) ;
}

char* strtoupper( char* str)
{   int i ;
    for( i = 0; str[ i ]; i++)
       str[ i ] = toupper( str[ i ] );
    return( str) ;
}

//===================== Conversions ==================================
//
//  PURPOSE - Converts a string to integer, 
//            can use 0b or 0x to specify binary and hex

int strtoi(char* str)
{ int bin,n,r,i,s=0;
  if((str[0] == '0') && (str[1] == 'x'))
    sscanf(str, "0x%x", &s);
  else if((str[0] == '0') && (str[1] == 'b'))
         { sscanf(str, "0b%d", &bin);
           n=bin;
           for(i=0;n!=0;i++)
            {   r=n%10;
                s=s+r*(int)pow(float(2),i);
                n=n/10;
            }
         }
       else
           sscanf(str,"%d",&s);
  return s;
}


//==========================Pretty Printing ============================
//
//  PURPOSE - print "%s = " the hex, decimal, binary numbers.
//          - print just number.

void print_sdxb( char* var, int num, int base)
{ if (base == 10)
    printf("%s = %d\n", var, num);
  else if (base == 0)
    printf("%d\n",num);
  else if (base==16)
    printf("%s = 0x%x\n",var,num);
  else if(base==2)
   { printf("%s = 0b",var);
     int i = 7;
     if(num > 255)
       i=15;
     for(;i>-1;i--)
      { if(num & (1 << i))
         printf("1");
        else
         printf("0");
      }
     printf("\n");
   }
}

void print_dbx(long long int num, int base) //------ print number in the right base.
{ if (base == 10)
    printf(" %lld ", num);
  else if (base == 0)
    printf(" %lld",num);
  else if (base==16)
    printf(" 0x%llx ",num);
  else if(base==2)
   { printf(" 0b");
     long int i = 7;
     if(num > 0xFF)
       i=15;
     if ( i > 0xFFFF)
       i = 32 ;
     for(;i>-1;i--)
      { if(num & (1 << i))
         printf("1");
        else
         printf("0");
      }
     printf(" ");
   }
}


