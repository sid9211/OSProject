#include <inc/lib.h>

void
umain(int argc, char **argv)
{
        int fd;
        char buf[512];
        int n, r;
	
	
	cprintf("\n----------------------------------------------------- Testing for 'File Remove' replay by opening the file----------------\n");
	//testCrash = 1;
        if ((fd = open("/testfile", O_RDWR )) < 0)
		      cprintf("\n-----------------------------------------------------Couldn't Open file : DOESN'T EXIST----------------\n");

	
	close(fd);
       
}
