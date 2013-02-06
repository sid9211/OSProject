#include <inc/lib.h>

void umain(int argc, char **argv)
{
        int fd;
        char buf[512];
        int n, r;

	
	#if (REPLAY==1 && CHECK==1)

	        cprintf("\n-------------------------------------------Journal Test for writing on files----------------------------\n");

	
	
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
	

	cprintf("\n -----------------------------------------------------------Now, reading the contents of the file------------\n");
        while ((n = read(fd, buf, sizeof buf-1)) > 0)
        {
                cprintf("\n================================================");
                sys_cputs(buf, n);
                cprintf("=====================================\n");
        }

        seek(fd, 0);


	ftruncate(fd, 10);
	  
/*      while ((n = read(fd, buf, sizeof buf-1)) > 0)
	{
		cprintf("\n==========");
                sys_cputs(buf, n);
		cprintf("\n");
	}
	
	seek(fd, 0);
        cprintf("\n");
	
	    cprintf("\n-----------------------------------------------------------Trying to over-write the file and crashing in between ------\n");
	

		        cprintf("\n-------------------------------------------The string being written :: This is a Journalling project------- \n");
	n=30;

	        strcpy(buf,"This is a Journalling project");

	
	if ((r = write(fd, buf, n)) != n)
        {
                cprintf(" Error in writing \n");
        }

*/	

	
	close(fd);
	#endif
	
	
}
