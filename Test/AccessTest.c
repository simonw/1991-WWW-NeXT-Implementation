/*	Test Access methods
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>		/* Unix io */
#include "HTAccess.h"
#define SIZE 1024		/* Read this buffer from the stream */
int main(int argc, char *argv[])
{
    if(argc<2) {
    	printf("Usage: AccessText <FullReference>\n");
	return -1;
    } else {
	int fd = HTAccess(argv[1]);
	if (fd<0) {
	    printf("Failed to access `%s'\n", argv[1]);
	    return fd;
	}
	printf("File descriptor is %i\n", fd);
	{
	    char buffer[SIZE];
	    int status = read(fd, buffer, SIZE);
	    printf("Read(%i) returned %i\n", fd, status);
	    close(fd);
	    if (status<=0) {
		return status;
	    }
	    printf("Buffer read was:\n%s\n", buffer);
	}
    }
    return 0;
} /* main */
