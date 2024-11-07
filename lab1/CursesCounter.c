
#include <sys/types.h>
#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>


int main()
{
        int i;
        initscr();                      /* Start curses mode              */
        for (i = 0; i<1000; i++) {
          move(4, 8);
          printw("%d",i);       /* Print Hello World              */
          refresh();                    /* Print it on to the real screen */
          sleep( 1 );
          }
        getch();                        /* Wait for user input */
        endwin();                       /* End curses mode                */

        return 0;
}


