/*
    Name:       Balaputradewa Ratuwina
    ID:         1001761950
    Course:     Operating System
    Section:    004
    Lab:        2
    Question:   1 — Multiprocessing
*/


/*
    Goal
    1. Load data from csv file
        a. data in tuple [1st half is a name or text, 2nd half is number]
        b. Wants to be sorted in ascending order via the number half
    2. Sort with bubble sort and time it
    3. Sort with 2 processes that does bubble sort and time it
        a. Divide in parts between the processes and merge at end
        b. Repeat for 4 and 10 parallel processes
    
    1st iteration
        Uses pipes to switch between processes and allow IPC
        
        Result — FAILED
            Pipes only works for 1 and 2 processes
            Segmentation core for 3 or more processes
            Not sure exactly why; current best guess is that pipes need to be adjusted for each amount of process

    2nd Iteration
        Divide into "parts" — there are 10,000 data so for 2 processes -> 2 parts, 4 processes -> 4 parts, etc.

        Result — Works?!?
            Not sure if there even is IPC

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>


// Structure to hold each tuple of data
typedef struct 
{
    char text[256];
    int number;
} DataTuple;

// Function to read data from a CSV file into an array of DataTuple
void loadData(DataTuple data[]) 
{
    FILE *file = fopen("data.csv", "r");
    if (!file) 
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file) && count < 10000) 
    {
        sscanf(line, "%[^,],%d", data[count].text, &data[count].number);
        count++;
    }

    fclose(file);
}

// Bubble sort; ascending value; uses the Tuple->number for sorting
void bubble_sort(DataTuple data[], int size) 
{
    int i, j;
    DataTuple temp;
    for (i = 0; i < size - 1; i++) 
    {
        for (j = 0; j < size - i - 1; j++) 
        {
            if (data[j].number > data[j + 1].number) 
            {
                temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }
}

// Function to handle the child process's task of sorting its portion of data
void child_process(DataTuple data[], int size) 
{
    // Sort the data chunk using bubble sort
    bubble_sort(data, size);
    exit(0); // Exit child process
}

int main()
{
    DataTuple data[10000];
    loadData(data); // Load data from CSV file
    DataTuple sorted_data[10000]; // Store the final sorted data
    time_t start, end;
    double timeCounter;

    // Sort using a single-threaded approach
    start = clock();
    bubble_sort(data, 10000);
    end = clock();
    timeCounter = end - start;
    printf("Single process sorting time: %.2f tics\n", timeCounter);

    // Sort using multiple processes
    int numProcesses[] = {2, 4, 10};
    int part = 10000;
    int initial = 0;
    int endPlace = 0;
    for (int i = 0; i < 3; i++)
    {
        part = 10000 / numProcesses[i];
        initial = 0;
        endPlace = 0;
        pid_t pids[numProcesses[i]];

        // Record the start time of the sorting
        start = clock();

        for (int j = 0; j < numProcesses[i]; j++) 
        {
            pids[j] = fork();
            if (pids[j] == 0) { // Child process
                // Calculate the start and end index for this process's data chunk
                initial = j * part;
                endPlace = (j == numProcesses[i] - 1) ? 10000 : initial + part;

                // Sort the chunk of data
                child_process(&data[initial], endPlace - initial);
                exit(0);
            }
        }

        // Wait for all child processes to finish
        for (int k = 0; k < numProcesses[i]; k++)
        {
            wait(NULL);
        }

        // Merge the sorted chunks into the final sorted result (a naive merge for simplicity)
        int index = 0;
        for (int m = 0; m < numProcesses[i]; m++) 
        {
            initial = m * part;
            endPlace = (m == numProcesses[i] - 1) ? 10000 : initial + part;
            for (int n = initial; n < endPlace; n++) 
            {
                sorted_data[index++] = data[n]; // Add the sorted data chunks back to sorted_data
            }
        }

        // Sort the merged result again since the data chunks may not be fully sorted
        bubble_sort(sorted_data, 10000);

        // Record the end time of the sorting
        end = clock();
        timeCounter = end - start;
        printf("Parallel sorting time with %d processes: %.2f tics\n", numProcesses[i], timeCounter);
    }

    return 0;
}