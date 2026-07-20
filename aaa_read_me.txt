Open-USB-IO Command Line
~~~~~~~~~~~~~~~~~~~~~~~~~~~
The code here runs on a PC and drives an Open_USB-IO 
board which has the Open-USB-IO firmware installed.
See the ousb reference manual for details on how to
use the ousb program.  Typeing ousb with no parameters
will give some useful help information.


FOR LINUX
~~~~~~~~~
To get a terminal pointed to this directory hit the
F4 key.

The program created here is called ousb, and a version 
is included in /usr/local/bin.  To use this second version
from any directory type-
  ousb io portb 255
and all LEDs should go on.

If you create a new version in this directory use instead-
  ./ousb io portb 255

The demo* files are bash script files that can do various things
to the board.  Read each first and start simply as-
   ./demo_rest_of_name

To recompile the code first make a copy of this directory.
Next type "make clean", make your changes,  then type "make".



OUSB_MULTI
~~~~~~~~~~~~~
See the folder ousb-multi to discover how to drive the ousb
program very quickly from script files and C code files.



FOR WINDOWS
~~~~~~~~~~~~~
See the Open-USB-IO manual for how to download the source code
and compile ousb.


COPYRIGHT
~~~~~~~~~~~
All files in this directory and sub-directories are copyright 
to Dr. Pj Radcliffe or Objective Development but released as GPL 2.







