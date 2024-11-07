/* Utility program, print time, name, PID, wait, print again, exit
 */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

int main(void)
{
	time_t t;
	char s[256];
	int i, j;
	pid_t p,pp;

   	t = time(NULL);
	printf( "Util Time: %s\n", ctime(&t));
	p = getpid();
	pp  = getppid();
	printf("Util PID: %d   PPID: %d \n", p, pp);
	sleep(10);
      	t = time(NULL);
	printf( "Util again: %s\n", ctime(&t));
	exit(0);
}
	
