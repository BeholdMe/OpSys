#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCK_SIZE 256
#define MAX_FILENAME_LEN 56
#define MAX_USERNAME_LEN 40
#define MAX_BLOCK_POINTERS 8

// Structure for File Name Table (FNT) entries
typedef struct {
    char filename[MAX_FILENAME_LEN];
    int inode;  // Pointer to DABPT entry
} FileEntry;

// Structure for Directory and Attribute/Block Pointer Table (DABPT) entries
typedef struct {
    int fileSize;
    time_t created;
    int blockPointers[MAX_BLOCK_POINTERS];
    char userName[MAX_USERNAME_LEN];
} DirectoryEntry;

// FS structure holding the essential data
typedef struct {
    FileEntry *fnt;
    DirectoryEntry *dabpt;
    int totalBlocks;
    int usedBlocks;
    FILE *diskFile;
} FileSystem;

FileSystem fs;

// Create a new filesystem
void createFS(const char *diskName, int numBlocks) {
    fs.totalBlocks = numBlocks;
    fs.usedBlocks = 0;

    // Allocate FNT and DABPT
    fs.fnt = (FileEntry *)calloc(numBlocks, sizeof(FileEntry));
    fs.dabpt = (DirectoryEntry *)calloc(numBlocks, sizeof(DirectoryEntry));

    // Open the disk file for writing and create it if necessary
    fs.diskFile = fopen(diskName, "wb+");
    if (fs.diskFile == NULL) {
        perror("Error creating disk file");
        exit(1);
    }

    printf("Filesystem created with %d blocks of %d bytes each\n", numBlocks, BLOCK_SIZE);
}

// Format filesystem w/given number of FNT and DABPT entries
void formatFS(int numFiles, int numDabptEntries) {
    // Initialize FNT and DABPT entries with default values
    for (int i = 0; i < numFiles; i++) {
        strcpy(fs.fnt[i].filename, "");
        fs.fnt[i].inode = -1;
    }

    for (int i = 0; i < numDabptEntries; i++) {
        fs.dabpt[i].fileSize = 0;
        fs.dabpt[i].created = time(NULL);
        memset(fs.dabpt[i].blockPointers, -1, sizeof(fs.dabpt[i].blockPointers));
        strcpy(fs.dabpt[i].userName, "");
    }

    printf("Filesystem formatted with %d FNT entries and %d DABPT entries\n", numFiles, numDabptEntries);
}

// List files
void listFiles()
{
    printf("Listing files in the filesystem:\n");
    for (int i = 0; i < fs.totalBlocks; i++) {
        if (fs.fnt[i].inode != -1) {  // Check if inode points to a valid file
            printf("File: %s | Size: %d bytes | User: %s\n",
                   fs.fnt[i].filename,
                   fs.dabpt[fs.fnt[i].inode].fileSize,
                   fs.dabpt[fs.fnt[i].inode].userName);
        }
    }
}


int main(int argc, char *argv[]) 
{
    if (argc < 2) {
        printf("Usage: %s <command> [arguments]\n", argv[0]);
        return 1;
    }


    if (strcmp(argv[1], "createfs") == 0 && argc == 4) {
        const char *diskName = argv[2];
        int numBlocks = atoi(argv[3]);
        createFS(diskName, numBlocks);
    } else if (strcmp(argv[1], "formatfs") == 0 && argc == 4) {
        int numFiles = atoi(argv[2]);
        int numDabptEntries = atoi(argv[3]);
        formatFS(numFiles, numDabptEntries);
    } else if (strcmp(argv[1], "list") == 0) {
        listFiles();
    } else {
        printf("Invalid command or arguments\n");
    }

    return 0;
}