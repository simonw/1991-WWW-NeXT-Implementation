/*		Test Relative Filenames
**		-----------------------
*/
#include <libc.h>
#include "HTParse.h"

int main(int argc, char * argv[])
{
    char * result;
    
    if (argc<3) {
    	printf("Usage: %s  <name> <relativeName>\n");
	exit(2);
    }
    
    result = HTRelative(argv[1], argv[2]);
    printf("`%s' expressed relative to\n  `%s' is\n  `%s'.\n", argv[1], argv[2], result);
    free(result);
} /* main */
