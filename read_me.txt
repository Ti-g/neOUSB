Open-USB-IO Command Line
~~~~~~~~~~~~~~~~~~~~~~~~~~~
The code here runs on a PC and drives an Open_USB-IO 
board which has the Open-USB-IO firmware installed.

To get a terminal pointed to this directory hit the
F4 key.

The program created here is called ousb, and a version 
is included in /usr/local/bin.  To use this version type
  ousb io portb 255
and all LEDs should go on.

If you create a new version in this directory use instead-
  ./ousb io portb 255


Type "make help" to see all the make file can do.

The demo* files are bash script files that can do various things
to the board.  Read each first and start simply as-
   ./demo_rest_of_name






