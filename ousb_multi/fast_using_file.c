/*========================== fast_using_file.c ==============================

  PURPOSE - demonstrate driving ousb fast using a file of input commands.

    USAGE - compile as "gcc -o fast_using_file fast_using_file.c".
          - to run on Linux ./fast_using_file
            to run on windows fast_using_file
          - takes commands from ousb_commands.txt, executed them
            and places any result in ousb_result.txt
          - works under Linux or Windows.

  COPYRIGHT Dr. Pj Radcliffe 2010.

*/
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
 #define CMD "..\\ousb -file ousb_commands.txt > ousb_result.txt"
#else
 #define CMD "../ousb -file ousb_commands.txt > ousb_result.txt"
#endif 

//============================ main ==================================

int main(int argc, char **argv)
{  system( CMD) ;
}
