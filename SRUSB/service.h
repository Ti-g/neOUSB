/*---------- service.h ------------------------------------------------

  PURPOSE - this module contains odds and sods of generally useful routines
            used by a variety of routines.

*/

#ifndef SERVICE
#define SERVICE

//====== string routines.
char* strtolower( char* str) ;      // convert string to lower case.
char* strtoupper( char* str) ;      // convert string to upper case.

void  trim_str_end( char *str) ;    // eliminate chars <= ' ' on the end of a string.
void  trim_str_start( char *str) ;  // eliminate chars <= ' ' on the start of a string.
void  trim_str_all( char *str) ;    // eliminate chars <= ' ' from start and end.

int   strtoi(char* str) ;          // str to int, copes with 0x and 0b.

void print_sdxb( char* var, int num, int base) ; // pretty print decimal, hex, binary.
void print_dbx(long long int num, int base) ;         // print number in the right base.

//Safe strncpy.
#define STRNCPY( dest, src, num)  {strncpy(dest,src,num) ; dest[num-1] = '0' ;}


#endif
