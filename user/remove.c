#include <inc/lib.h>

void umain(int argc, char **argv)
{
        int fd;
        char buf[512];
        int n, r;

	
	#if (REPLAY==1 && CHECK==4)

	        cprintf("\n-------------------------------------------Journal Test for removing files----------------------------\n");

	
	
       if ((fd = open("/testfile", O_RDWR | O_CREAT)) < 0)
                panic("open /testfile failed: %e", fd);

        cprintf("\n-------------------------------------------------------The string being written :: This is cse 506 project------- \n");

        strcpy(buf,"This is cse 506 project");

        n = 23;

        if ((r = write(fd, buf, n)) != n)
        {
                cprintf(" Error in writing \n");
        }
	
	
        seek(fd, 0);
	
	
	    cprintf("\n-----------------------------------------------------------Trying to delete the file and crashing in between ------\n");
	
	remove("/testfile");

	

	
	close(fd);
	#endif
	
	
}
