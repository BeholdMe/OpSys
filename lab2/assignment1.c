/*
    Name:       Balaputradewa Ratuwina
    ID:         1001761950
    Course:     Operating System
    Section:    004
*/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>  // For shared memory
#include <string.h>
#include <time.h>



//So I found this interesting video on mmap and shared memory
// https://www.youtube.com/watch?v=rPV6b8BUwxM
//I decided to use it in order to not have to merge multiple 

// Structure to hold each tuple of data
typedef struct
{
    char text[256];
    int number;
} DataTuple;

// Function to read data from a CSV file into an array of DataTuple
int read_data_from_csv(const char *filename, DataTuple data[])
{
    FILE *file = fopen("data.csv", "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file) && count < 10000) {
        sscanf(line, "%[^,],%d", data[count].text, &data[count].number);
        count++;
    }

    fclose(file);
    return count; // Return the number of data tuples read
}

// Bubble sort implementation for sorting the data based on the numeric value
void bubble_sort(DataTuple data[], int size)
{
    int i, j;
    DataTuple temp;
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - i - 1; j++) {
            if (data[j].number > data[j + 1].number)
            {
                temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }
}

// Function to merge two sorted sections of the shared memory data
void merge(DataTuple data[], int left, int mid, int right) {
    int i = left, j = mid + 1, k = 0;
    DataTuple temp[right - left + 1];

    while (i <= mid && j <= right) {
        if (data[i].number <= data[j].number) {
            temp[k++] = data[i++];
        } else {
            temp[k++] = data[j++];
        }
    }

    while (i <= mid) temp[k++] = data[i++];
    while (j <= right) temp[k++] = data[j++];

    for (i = left, k = 0; i <= right; i++, k++) {
        data[i] = temp[k];
    }
}

// Recursive merge sort function for the data in shared memory
void merge_sort(DataTuple data[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort(data, left, mid);
        merge_sort(data, mid + 1, right);
        merge(data, left, mid, right);
    }
}

// Function to handle the sorting by each child process
void child_process(DataTuple *shared_data, int start_index, int end_index) {
    bubble_sort(&shared_data[start_index], end_index - start_index + 1);
    exit(0); // Exit child process after sorting
}

int main() {
    //set default process number to 2
    //ask user how many processes you want
    int num_processes = 2;
    printf("How many processes would you like? ");
    scanf("%d", &num_processes);

    DataTuple *shared_data;
    int data_size;
    int chunk_size;
    pid_t pids[num_processes];

    // Allocate shared memory for the dataset using mmap
    shared_data = mmap(NULL, 10000 * sizeof(DataTuple), PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    data_size = read_data_from_csv("data.csv", shared_data); // Load data into shared memory
    chunk_size = data_size / num_processes;

    time_t start, end;
    double elapsed_time;

    // Record the start time of the sorting
    start = clock();

    // Create child processes and assign tasks
    for (int i = 0; i < num_processes; i++) {
        int start_index = i * chunk_size;
        int end_index = (i == num_processes - 1) ? data_size - 1 : (start_index + chunk_size - 1);

        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pids[i] == 0) { // Child process
            child_process(shared_data, start_index, end_index);
        }
    }

    // Wait for all child processes to complete their sorting
    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    // Perform a final merge pass on the sorted chunks in shared memory
    for (int i = 1; i < num_processes; i++) {
        int mid = (i * chunk_size) - 1;
        int end_index = (i == num_processes - 1) ? data_size - 1 : ((i + 1) * chunk_size) - 1;
        merge(shared_data, 0, mid, end_index);
    }

    // Record the end time of the sorting
    end = clock();
    elapsed_time = end - start;

    // Display the elapsed time for sorting
    printf("Parallel merge sort time with %d processes: %.2f tics\n", num_processes, elapsed_time);
    
    /*
    for (int i = 0; i < data_size; i++)
    {
        printf("%s, %d\n", shared_data[i].text, shared_data[i].number);
    }
    */

    // Clean up shared memory
    munmap(shared_data, 10000 * sizeof(DataTuple));

    return 0;
}