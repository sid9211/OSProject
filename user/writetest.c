#include <inc/lib.h>

void
umain(int argc, char **argv)
{
        int fd;
        char buf[512];
        int n, r;
	
	
	cprintf("\n------------------------------------------------- Testing for 'File Write' replay by opening and reading it----------------\n");
	//testCrash = 1;
        if ((fd = open("/testfile", O_RDWR )) < 0)
                panic("\n open /testfile failed: %e", fd);
	
         
	      cprintf("\n------------------------------------------------- Testing for 'File Write' replay by opening and reading it----------------\n");

	
	cprintf("\n-----------------------------------------------------The contents of the file being read are : ------------------------");
	seek(fd, 0);
	
	while ((n = read(fd, buf, sizeof buf-1)) > 0)
	{
		cprintf("\n=============================================================");
                sys_cputs(buf, n);
                cprintf("=====================================================\n");
	}
        cprintf("\n===\n");


	close(fd);
       
}
