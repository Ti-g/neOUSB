INTODUCTION
~~~~~~~~~~~~~
This folder shows how to use the ousb "-multi" and "-file" options.  The options allow the ousb program to stay alive in memory and accept command line input from the keyboard,  a text file,  or from a C program.  Driving ousb from a script or C program results in a speed of about 25 commands per second because of the delay of starting the ousb program,  running it  and then terminating it.  The -multi option keeps ousb alive in memory, avoiding these delays and so the speed is increased to about 200-250 commands per second.


COMMAND LINE
~~~~~~~~~~~~~
  * Type "ousb -multi" then return.
  * Now type any command without ousb at the front and it will execute.
    For example "io portb 0x55"
  * Type "quit" to terminate ousb.


SCRIPT FILE
~~~~~~~~~~~~
  * Create a text file with ousb commands but without the "ousb" word.
    For example "io portb 1" rather than "ousb io portb 1".
    Note the existing ousb_commands.txt file.

  * For Linux only run ousb as follows: "ousb -multi < ousb_commands.txt "
    The ousb commands from the file will be executed and the display will
    go to the screen.  The "<" causes the input to come from the
    file instead of the keyboard.
    The last command should be "quit" so ousb properly terminates.

  * For Linux and Windows try "ousb -file ousb_commands.txt"
    The -file option behaves very similarly to the -multi option but
    does not rely on a pipe.

  * It is also possible to redirect the output to a file.
    For Linux only:         "ousb -multi < ousb_commands.txt > ousb_result.txt"
    For Linux and Windows:  "ousb -file ousb_commands.txt > ousb_result.txt"
    The file of commands will be executed and the result placed in
    the result file.
    You could use this as a really slow logic analyser.

  * Note the useful script file fill_file as a way to create a text
    file of commands.
        Linux:    ./fill_file ousb_commands.txt
        Windows:   fill_file ousb_commands.txt



DRIVING WITH C CODE FROM LINUX
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  It has proved quite difficult to get a C/C++ program to directly drive ousb at high speed.  Linux pipes and named pipes do not quite behave as we would like, notably the reading of a pipe does not always block as would be desired.
Under Windows this has proved to be impossible and the -file option must be used as shown below.

The program fast_c_ousb.c shows how a C/C++ program can drive the ousb program via named pipes.  Note it must be started with the script file run_fast.  Type the command "./run_fast" to see 1000 writes to the ousb portb in about 5 seconds.



DRIVING WITH C USING -FILE
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The following code frament comes from the file fast_using_file.c  and shows how a C/C++ program can make ousb run fast under Linux or Windows.  The commands must come from a  file,  and the results go to a file.

   #include <stdlib.h>
   #include <stdio.h>

   #ifdef _WIN32
     #define CMD "..\\ousb -file ousb_commands.txt > ousb_result.txt"
   #else
     #define CMD "..\/\/ousb -file ousb_commands.txt > ousb_result.txt"
    #endif 

  //============================ main ==================================

    int main(int argc, char **argv)
    {  system( CMD) ;
    }



