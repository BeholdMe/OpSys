#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

#define MAX_DATA_SIZE 10000
#define MAX_LINE_LENGTH 256
#define FILENAME "data.csv" // The CSV file containing the dataset

// Structure to hold each tuple of data
typedef struct {
    char text[MAX_LINE_LENGTH];
    int number;
} DataTuple;

// Function to read data from a CSV file into an array of DataTuple
int read_data_from_csv(const char *filename, DataTuple data[]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) && count < MAX_DATA_SIZE) {
        sscanf(line, "%[^,],%d", data[count].text, &data[count].number);
        count++;
    }

    fclose(file);
    return count; // Return the number of data tuples read
}

// Bubble sort implementation for sorting the data based on the numeric value
void bubble_sort(DataTuple data[], int size) {
    int i, j;
    DataTuple temp;
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - i - 1; j++) {
            if (data[j].number > data[j + 1].number) {
                temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }
}

// Function to handle the child process's task of sorting its portion of data
void child_process(int pipe_fd[]) {
    close(pipe_fd[1]); // Close the write end of the pipe in the child

    int size;
    DataTuple data_chunk[MAX_DATA_SIZE];

    // Read the size of the data chunk
    if (read(pipe_fd[0], &size, sizeof(int)) <= 0) {
        perror("Child failed to read size from pipe");
        exit(EXIT_FAILURE);
    }

    // Read the data chunk
    if (read(pipe_fd[0], data_chunk, sizeof(DataTuple) * size) <= 0) {
        perror("Child failed to read data chunk from pipe");
        exit(EXIT_FAILURE);
    }

    // Sort the data chunk using bubble sort
    bubble_sort(data_chunk, size);

    // Close the read end before writing to prevent interference
    close(pipe_fd[0]);

    // Now write the sorted data back to the pipe
    // Open a new pipe for writing sorted data (using a different pipe for writing)
    int write_fd = pipe_fd[1];
    if (write(write_fd, data_chunk, sizeof(DataTuple) * size) == -1) {
        perror("Child failed to write sorted data");
        exit(EXIT_FAILURE);
    }

    close(write_fd); // Close the write end of the pipe in the child
    exit(0); // Exit child process
}

// Function to merge multiple sorted arrays into a single sorted array
void merge_sorted_chunks(DataTuple data[], int num_chunks, int chunk_sizes[], DataTuple result[], int total_size) {
    int indices[num_chunks]; // Array to track the current index of each chunk
    memset(indices, 0, sizeof(indices)); // Initialize indices to 0

    for (int i = 0; i < total_size; i++) {
        int min_value = __INT_MAX__;
        int min_chunk = -1;

        // Find the smallest element among the chunks
        for (int j = 0; j < num_chunks; j++) {
            if (indices[j] < chunk_sizes[j] && data[j * MAX_DATA_SIZE + indices[j]].number < min_value) {
                min_value = data[j * MAX_DATA_SIZE + indices[j]].number;
                min_chunk = j;
            }
        }

        // Add the smallest element to the result array and increment the index for that chunk
        result[i] = data[min_chunk * MAX_DATA_SIZE + indices[min_chunk]];
        indices[min_chunk]++;
    }
}

int main() {
    int num_processes = 2;
    printf("How many processes would you like? ");
    scanf("%d", &num_processes);
    DataTuple data[MAX_DATA_SIZE];
    DataTuple sorted_result[MAX_DATA_SIZE];
    int data_size = read_data_from_csv(FILENAME, data); // Load data from CSV file
    int chunk_size = data_size / num_processes;

    int pipes[num_processes][2]; // Array of pipes for each process
    pid_t pids[num_processes];
    int chunk_sizes[num_processes];

    time_t start, end;
    double elapsed_time;

    // Record the start time of the sorting
    start = clock();

    // Create child processes and assign tasks
    for (int i = 0; i < num_processes; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }

        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pids[i] == 0) { // Child process
            close(pipes[i][1]); // Close write end
            child_process(pipes[i]);
            exit(0); // Ensure child exits here
        } else { // Parent process
            close(pipes[i][0]); // Close read end

            // Send the chunk size and data to the child process
            int start_index = i * chunk_size;
            int end_index = (i == num_processes - 1) ? data_size : start_index + chunk_size;
            chunk_sizes[i] = end_index - start_index;

            // Error handling for write
            if (write(pipes[i][1], &chunk_sizes[i], sizeof(int)) == -1) {
                perror("Parent failed to write chunk size");
                exit(EXIT_FAILURE);
            }
            if (write(pipes[i][1], &data[start_index], sizeof(DataTuple) * chunk_sizes[i]) == -1) {
                perror("Parent failed to write data chunk");
                exit(EXIT_FAILURE);
            }
            close(pipes[i][1]); // Close write end after sending data
        }
    }

    // Wait for all child processes to finish and gather sorted chunks
    for (int i = 0; i < num_processes; i++) {
        wait(NULL); // Wait for child process to finish

        // Read the sorted chunk from the child process
        read(pipes[i][0], &data[i * MAX_DATA_SIZE], sizeof(DataTuple) * chunk_sizes[i]);
        close(pipes[i][0]); // Close read end in the parent
    }

    // Merge the sorted chunks into a final sorted result
    merge_sorted_chunks(data, num_processes, chunk_sizes, sorted_result, data_size);

    // Record the end time of the sorting
    end = clock();
    elapsed_time = end - start;

    // Display the elapsed time for sorting
    printf("Parallel sorting time with %d processes: %.2f tics\n", num_processes, elapsed_time);

    return 0;
}