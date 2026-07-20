/*========================== fast_c_ousb.c ==============================

  PURPOSE - demonstrate bidirectional link to ousb under Linux.  This does
            not work under Windows.

    USAGE - compile as "gcc -o fast_c_ousb fast_c_ousb.c".
          - must start by using a script,  see run_fast, start
            as "./run_fast".

     NOTE - two fifos are used, one to the ousb and one from the ousb,
            these are passed as parameters into the program.
          - write of each command to the ousb via the fifo requires 
            the fifo to be opened, written, then closed.
          - read from the ousb fifo requires a delay to stop the process 
            using excessive CPU time in a tight loop, typically 40% CPU!
          - in the main() function find the for loop that does write and
            read from the ousb.  Replace this with your own code.

  COPYRIGHT Dr. Pj Radcliffe 2010.

*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>


//======================= vars and constants ==========================

char buf [8000] ;         // buffer for send and rx from fifo.

struct timespec sleep_time ;

int   fdr ;              // read pipe file no.
char* write_fifo_name ;



//======================== rd and wr pipe =============================

void wr_to_pipe() //---------------------------------------------------
{//--- open fifo
   int fd, res ;
   fd = open( write_fifo_name, O_WRONLY);
   if ( fd < 0)
     { printf("   Couldn't open the write fifo.\n") ;
       exit(1) ;
     }
 //--- try write.
   res = write(fd, &buf, strlen(buf));
   if (res < 0) // if failed write.
     { printf("    Write to pipe failed");
       exit(1);
     }
 //--- close fifo to ensure receiver can now read.
   res=close(fd);
   if (res !=0)
     { printf("   Failed to close pipe\n") ;
       exit(1);
     }
}

void rd_from_pipe() //------------------------------------------------
{  int res ;
   do
     { memset(buf, (char) 0, sizeof(buf)) ;
       res = read(fdr, &buf, sizeof(buf));
       nanosleep( &sleep_time, NULL) ; // avoids excessive CPU usage.
     }
   while ( strlen(buf) == 0 ) ;        // block until get reply.
}



//============================ main ==================================

int main(int argc, char **argv)
{//--- checks.
   if (argc < 3)
    { printf("\n  ABORTING: start with 'fast_c_ousb fifo_name fifo_name'\n\n") ;
      exit(1) ;
    }
   write_fifo_name = argv[1] ;

 //--- inits
   sleep_time.tv_sec = 0 ;
   sleep_time.tv_nsec = 900000 ;

 //--- open rx fifo
   fdr = open(argv[2], O_RDONLY | O_NONBLOCK);
   if ( fdr < 0)
     { perror("   Couldn't open the read fifo.") ;
       exit(1) ;
     }

 //--- loop to send commands and get responses.
 //    *** replace this with your own code.
   int i ;
   for (i = 0 ; i < 1000 ; i++ )
     { sprintf(buf,"-r io portb %i\n",i&0xFF) ;
       wr_to_pipe() ;
       rd_from_pipe() ;
       printf("return: %s\n", buf) ;
     }

 //--- clean up.
   sprintf(buf,"quit") ;
   wr_to_pipe() ;
   close(fdr) ;
   printf("\n\n Just wrote 1000 times to portb.\n\n") ;
   return 0;
}
