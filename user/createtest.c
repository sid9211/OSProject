#include <inc/lib.h>

void
umain(int argc, char **argv)
{
        int fd;
        char buf[512];
        int n, r;

/*	#if (REPLAY==0 && CHECK==2)
	cprintf("\n-----------Now trying to open testfile after Panic and without Journal Replay---------\n");

	 if ((fd = open("/testfile", O_RDONLY )) < 0)
                panic("\n------------------Couldn't open /testfile: Doesn't Exist --------\n");
	
	#endif	
*/
	#if (REPLAY==1 && CHECK==2)
	
	cprintf("\n---------------------------------------------------------Now Replaying the Journal and try to open the file again-------------\n");

	 if ((fd = open("/testfile", O_RDONLY )) < 0)
                panic("\n---------Couldn't open /testfile: -----------%e \n", fd);
	
	cprintf("\n----------------------------------------------------------File opened! Created during Journal Replay-------------\n");

	cprintf("\n .........\n");
		
	close(fd);
	#endif

}
