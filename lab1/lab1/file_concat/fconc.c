/*

						    _-`````-,           ,- '- .
						  .'   .- - |          | - -.  `.
						 /.'  /                     `.   \
						:/   :      _...   ..._      ``   :
						::   :     /._ .`:'_.._\.    ||   :
						::    `._ ./  ,`  :    \ . _.''   .
						`:.      /   |  -.  \-. \\_      /
						  \:._ _/  .'   .@)  \@) ` `\ ,.'
						     _/,--'       .- .\,-.`--`.
						       ,'/''     (( \ `  )    
						        /'/'  \    `-'  (      
						         '/''  `._,-----'
						          ''/'    .,---'
						           ''/'      ;:
						             ''/''  ''/
						               ''/''/''
						                 '/'/'
						                  `;
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>		/* strcmp */
#include <stdlib.h> 		/* exit	  */
#include <errno.h>

// ==================
void usage(void);
void warning(void);
ssize_t writep( int fd, const void* buf, size_t size );

// ==================
#ifndef BUFSIZE			// allow gcc to override size using -DBUFSIZE <SIZE>
#define BUFSIZE 1024
#endif
// ==================

int main( int argc, char *argv[] )
{
	if ((argc != 3 && argc != 4) || strcmp(argv[1], "--help") == 0) {
		usage();
	}
	char *outf = "fconc.out";
	if (argc == 4) { 
		outf = argv[3];
		if ( (strcmp(argv[3], argv[2]) == 0 || strcmp(argv[3], argv[1]) == 0) ) {
			warning();
			// Warning: outfile is the same as one of its input file
			// print Warning, and ask user to give an alternative output filename
		}
	}

	int fd[3] = {0, 0, 0};		/* file descriptors */
	int opflags = O_CREAT   	/* create */
			| O_WRONLY	/* write only */
			| O_APPEND;	/* append: set file offset at end-of-file */
	//		| O_TRUNC; 	// TRUNCATE existing file to zero length (we might need to use this flag)
	mode_t opmode = 0666; 		/* rw-rw-rw */
	ssize_t numRead; 		// bytes succesfully read
	
	char buffer[BUFSIZE];


	// open() 
	fd[0] = open(argv[1], O_RDONLY);
	if (fd[0] == -1) {	// error handling :: fd[0]
		perror("open[infile1]");
		exit(-1);
	}

	fd[1] = open(argv[2], O_RDONLY);
	if (fd[1] == -1) {	// error handling :: fd[1]
		close(fd[0]);
		perror("open[infile2]");
		exit(-1);
	}

	fd[2] = open(outf, opflags, opmode);
	if (fd[2] == -1) {	// error handling :: fd[2]
		close(fd[0]); close(fd[1]);
		perror("open[outfile]");
		exit(-1);
	}
	/*
		man 2 write: 
		The number of bytes written may be less than count if, for example,
		there  is  insufficient  space  on the underlying physical
	        medium, or the RLIMIT_FSIZE resource limit is  encountered  (see
	        setrlimit(2)),  or  the call was interrupted by a signal handler
	        after having written less than count bytes.
	    	Thus we provide a wrapper, which is persistent.
	*/

	// read() / write() [infile1]
	while((numRead = read(fd[0], buffer, BUFSIZE)) > 0) {
		// 0 indicates EOF
		writep(fd[2], buffer, (size_t)numRead);
	}

	// read() / write() [infile2]
	while((numRead = read(fd[1], buffer, BUFSIZE)) > 0) {
		// 0 indicates EOF
		writep(fd[2], buffer, (size_t)numRead);
	}

	// close()
	int cnt;
	for (cnt = 0; cnt < 3; cnt++) {
		if (close(fd[cnt]) == -1) {
	 	  perror("close");
		  exit(-1); 
		}
	}

	return 0;
}

void usage( void )	// prints help message to STDOUT
{
	printf ("Usage: ./fconc <infile1> <infile2> [<outfile>]\n");
	exit(0);
}

void warning(void){
	printf("Warning: outfile is the same as one of the input file\n");
	printf("Please give alternative output file name\n");
	exit(0);
}

ssize_t writep( int fd, const void* buf, size_t size )
{
	/*  EINTR           Interrupted function call
	 *  EIO             Input/output error
	 *  EAGAIN          Resource temporarily unavailable
	 */

    ssize_t numWritten;			// bytes actually written
    while (size > 0) {			// while there is something to write
        
        do {				// write to file at least once, retry on error
             numWritten = write(fd, buf, size);
        } while ( (numWritten < 0) && (errno == EINTR || errno == EAGAIN) );
        
        if (numWritten < 0) {	// in case of other error
            return numWritten;	// return the actual value from write()
	}

        size -= numWritten; 	// in case of incomplete write, continue again until size = 0
        buf += numWritten;	// move pointer to the right offset
    }
    return 0;
}
