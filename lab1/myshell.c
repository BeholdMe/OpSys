/*
    Name:       Balaputradewa Ratuwina
    ID:         1001761950
    Course:     3320
    Section:    4
    Lab:        1
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/*
    Create a menu consisting of
        - Current directory
        - Files in current directory
        - Directories connected to current directory
            - Show previous directory and any subdirectory of working directory
            - Previous directory displayed as ...
        - List of command options
    Read [COMMAND] from user (1 char)
        E == Open text editor with file
            Read [file name] from user
        R == Run [file name], must be executable
            Read [file name] from user
        C == Change working directory.
        S == sorts listing by either size or date (prompt user)
    Implement
        Prev, and Next Operations so that the menu fits on one screen.
        Store names in an array, Only one pass through directory.
    Need
        More error handling
        Rely less on fixed paths
        check for return values
        handle parameters
        watch out for buffer overflows,
        security problems
        use environment variables
    
    Bonus
        Show additional file information (date, size, read/execute)
        Use file name completion as well as a number.
        Create/use a pull-down menu.

    Current issues:
        For wahtever reason, can't access curses.h so did not use it at all
        Error handling works... only on first time, then it becomes an almost infinite loop of menu choices
        quit not working
        loop not looping properly
            I can only perform one choice/operation and then it just breaks lol

*/

typedef struct
{
    char name[1024];
    //got this from stackOverflow https://stackoverflow.com/questions/53728346/how-to-get-metadata-from-a-file-in-c
    //MUST TEST; not sure if work
    struct stat info;
} FileStuff;

//function to get and list directories on a single page
void listDir(const FileStuff files[], int fileCount, int currPage)
{
    int start = currPage * 5,
        end = start + 5;
    
    //check if the end will be greater than the fileCount and if so, change it so that it equals it
    if(end > fileCount)
    {
        end = fileCount;
    }

    for(int i = start; i < end; i++)
    {
        //give name and size of file
        printf("(%d) %s (Size: %ld bytes, Last Modified: %s)\n", i, files[i].name, files[i].info.st_size, ctime(&files[i].info.st_mtime));
    }
    printf("Page %d/%d\n", currPage + 1, (fileCount + 4) / 5);
}


//compare sizes of files for qsort
//I have been told this is not the best way, BUT
//This is the only one I remembered bc of how silly it looks
//found on Reddit but cannot find the post anymore, might have been archived or deleted
//Here's a tutorialspoint that I'm kinda using https://www.tutorialspoint.com/c_standard_library/c_function_qsort.htm
int compSize(const void *a, const void *b)
{
    FileStuff *fileA = (FileStuff *)a;
    FileStuff *fileB = (FileStuff *)b;

    return (int)(fileB->info.st_size - fileA->info.st_size);
}

//Basically above but with time
int compTime(const void* a, const void *b)
{
    FileStuff *fileA = (FileStuff *)a;
    FileStuff *fileB = (FileStuff *)b;

    //compares for time of last modification
    return (int)(fileB->info.st_mtime - fileA->info.st_size);
}

//use default qsort bc I am too lazy to implement a better sort algorithm
void sortFiles(FileStuff files[], int fileCount, char how)
{
    if(how == 'S')
    {
        qsort(files, fileCount, sizeof(FileStuff), compSize);
    }else if(how == 'D')
    {
        qsort(files, fileCount, sizeof(FileStuff), compTime);
    }else
    {
        perror("Wrong input for sorting files");
    }
}

//read directory as a function of its own instead of as part of main
//error handling; -1 = something messed up; 0 = okay
// https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
// Trying to combine answers together
int readDir(const char *path, FileStuff files[], int *pageCount)
{
    DIR *d = opendir(path);
    //check if directory is able to be opened or null
    if(d == NULL)
    {
        perror("opendir");
        return -1;
    }

    struct dirent *de; //directory entry
    int index = 0;

    //This is very experimental and haven't fully determine/test it out to see if fully working
    //as intended
    while( (de = readdir(d)) != NULL )
    {
        if(de->d_type == DT_REG || de->d_type == DT_DIR) //check for only regular directories and files
        {
            //copy name of directory entry to FileStuff object's name field
            //index used to make sure name matches the file being "named"
            strcpy(files[index].name, de->d_name);

            //store info of the files into the FileStuffy object's info field
            //if -1 then something went wrong so get perror
            if(stat(de->d_name, &files[index].info) == -1)
            {
                perror("stat library");
            }
            index++;
        }
    }
    
    //close directory
    closedir(d);
    *pageCount = index; //set count of pages as last index
    return 0;
}

int main(int argc, char *argv[])
{
    //variable to keep count of files in current directory and keep track of current pages for "scrolling"
    int fileCount = 0, currPage = 0;
    //variable for string to be changed, command, and user choice from menu
    //TODO: change to dynamic mem if enough time
    char stuff[256];
    char cmd[256];
    char choice;
    time_t t;
    //variable for the array of files
    FileStuff files[1024];

    //if executable is ran with a directory attached, use as directory
    if(argc > 1)
    {
        //GeekFromGeek for perror info: https://www.geeksforgeeks.org/error-handling-in-c/
        //Idea is to try and change directory if provided
        //If change of directory failed for whatever reason, initiate perror func and exit
        if(chdir(argv[1]) != 0)
        {
            perror("Can't change directory");
            exit(EXIT_FAILURE);
        }
    }

    while(1)
    {
        //display time
        t = time( NULL );
        printf( "Time: %s\n", ctime( &t ));
        printf("-----------------------------------------------\n" );

        //display the current directory
        getcwd(stuff, sizeof(stuff));
        printf( "\nCurrent Directory: %s \n", stuff);

        //read and display directory
        if(readDir(".", files, &fileCount) == -1)
        {
            continue;
        }
        listDir(files, fileCount, currPage);

        //choice menu
        printf("\nOperations: \nN - Next Page\nP - Previous Page\nE - Edit File\nR - Run file\nC - Change Directory\nS - Sort files\nM - Move File\nX - Remove File\nQ - Quit\n");
        printf("Enter choice: ");
        choice = toupper(getchar()); getchar();
        //convert to uppercase so no issue with case sensitivity

        switch (choice)
        {
        //Quit not working for some reason
        case 'Q':
            exit(0);
            break;
        case 'N':
            currPage = (currPage + 1) % ((fileCount + 4) / 5);
            break;
        case 'P':
            currPage = (currPage - 1 + (fileCount + 4) / 5) % ((fileCount + 4) / 5);
            break;
        case 'E':
            printf( "Edit what?: " );
            scanf( "%s", stuff);
            strcpy( cmd, "pico ");
            strcat( cmd, stuff );
            system( cmd );
            break;
        case 'R':
            printf( "Run what?: " );
            scanf( "%s", cmd );
            system( cmd );
            break;
        case 'C':
            printf( "Change To?: " );
            scanf( "%s", cmd );
            if (chdir( cmd ) == -1)
            {
                perror("change directory is not working");
            }
            break;
        case 'S':
            printf("Sort by (S)ize or (D)ate? ");
            char how = toupper(getchar()); getchar();
            sortFiles(files, fileCount, how);
            currPage = 0;
            break;
        case 'M':
            printf("Enter file to move: ");
            char src[256], dest[256];
            scanf("%s", src); //source file
            printf("Enter destination: ");
            scanf("%s", dest); //destination directory
            getchar();
            //error handling if attempting to rename
            if(rename(src, dest) == -1)
            {
                perror("renaming attempt");
            }
            break;
        case 'X':
            printf("Enter file to remove: ");
            scanf("%s", cmd);
            getchar();
            if(remove(cmd) == -1)
            {
                perror("Attempt to remove failed");
            }
            break;
        default:
            printf("Bruh.\n");
            break;
      }
    }
}

/*
    Personal reference for template code

    pid_t child;
    DIR * d;
    struct dirent * de;
    int i, c, k;
    char currDir[256], cmd[256];
    time_t t;

    while (1) {

      t = time( NULL );
      printf( "Time: %s\n", ctime( &t ));
      printf("-----------------------------------------------\n" );

      getcwd(currDir, 200);
      printf( "\nCurrent Directory: %s \n", s);

      d = opendir( "." );
      c = 0;
      while ((de = readdir(d))){
          if ((de->d_type) & DT_DIR) 
             printf( " ( %d Directory:  %s ) \n", c++, de->d_name);	  
      }
      closedir( d );
      printf( "-----------------------------------------\n" );
 
      d = opendir( "." );
      c = 0;                    
      while ((de = readdir(d))){                    
          if (((de->d_type) & DT_REG))                              
             printf( " ( %d File:  %s ) \n", c++, de->d_name);
          if ( ( c % 5 ) == 0 ) {
             printf( "Hit N for Next\n" );
             k = getchar( );
             }
      }
      closedir( d );
      printf( "-----------------------------------------\n" );
  
      c = getchar( ); getchar( );
      switch (c) {
        case 'q': exit(0); // quit
        case 'e': printf( "Edit what?:" );
                  scanf( "%s", s );
                  strcpy( cmd, "pico ");
                  strcat( cmd, s );
                  system( cmd );
                  break;
        case 'r': printf( "Run what?:" );
                  scanf( "%s", cmd );
                  system( cmd );
                  break;
        case 'c': printf( "Change To?:" );
                  scanf( "%s", cmd );
                  chdir( cmd );   
                  break; 
      }
       
    }
*/