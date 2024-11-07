/*
    Balaputradewa Ratuwina
    1001761950
    Section 4
    Lab 2 — Part 1
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>


/*
    This is essentially a copy-and-paste from assignment 1 with modification
    Instead of multiple processes, there is only 1 process with multiple threads

    Initial version plan was to have
        1 process with 1 thread
        1 process with 2 threads
        1 process with 4 threads
        1 process with 10 threads

    Decided against it as I now believe the point of the project is to compare/contrast multiprocessing and multithreading
    Use mutex for synchronization purposes
*/

pthread_mutex_t mutex;

// Structure to hold each tuple of data
typedef struct 
{
    char text[256];
    int number;
} Tuple;

// Structure to pass data to each thread
typedef struct 
{
    Tuple *data;
    int initial;
    int final;
} ThreadData;

// Function to read data from a CSV file into an array of Tuple
void loadData(Tuple data[]) 
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
void bubble_sort(Tuple data[], int size) 
{
    int i, j;
    Tuple temp;
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

// Function to be run by each thread for sorting its portion of data
void* thread_sort(void* arg) 
{
    ThreadData* thread_data = (ThreadData*)arg;
    bubble_sort(thread_data->data, thread_data->final - thread_data->initial);
    return NULL;
}

int main() 
{
    Tuple data[10000];
    loadData(data); // Load data from CSV file
    Tuple sorted_data[10000]; // Store the final sorted data
    time_t start, end;
    double elapsed_time;

    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    // Sort using a single-threaded approach
    start = clock();
    bubble_sort(data, 10000);
    end = clock();
    elapsed_time = end - start;
    printf("Single-threaded sorting time: %.2f tics\n", elapsed_time);

    // Sort using multiple threads
    int numThreads[] = {2, 4, 10};
    int index = 0;
    int part = 10000;
    
    for (int i = 0; i < 3; i++)
    {
        pthread_t threads[10];
        ThreadData threadData[10];
        part = 10000 / numThreads[i];

        // Record the start time of the sorting
        start = clock();

        // Create threads to sort chunks of data
        for (int j = 0; j < numThreads[i]; j++) 
        {
            threadData[j].data = &data[j * part];
            threadData[j].initial = j * part;
            threadData[j].final = (j == numThreads[i] - 1) ? 10000 : (j + 1) * part;
            pthread_create(&threads[j], NULL, thread_sort, &threadData[i]);
        }

        // Wait for all threads to finish
        for (int n = 0; n < numThreads[i]; n++) 
        {
            pthread_join(threads[n], NULL);
        }

        // Critical section due to need to combine data into shared pool
        pthread_mutex_lock(&mutex);
        index = 0;
        for (int k = 0; k < numThreads[i]; k++) 
        {
            for (int m = threadData[i].initial; m < threadData[i].final; m++)
            {
                sorted_data[index++] = data[m]; // Add the sorted data chunks back to sorted_data
            }
        }
        pthread_mutex_unlock(&mutex);


        // Sort the merged result again to ensure complete sorting
        bubble_sort(sorted_data, 10000);

        // Record the end time of the sorting
        end = clock();
        elapsed_time = end - start;
        printf("Parallel sorting time with %d threads: %.2f tics\n", numThreads[i], elapsed_time);
    }

    // Destroy the mutex
    pthread_mutex_destroy(&mutex);
    return 0;
}