/*
    Name:       Balaputradewa Ratuwina
    ID:         1001761950
    Course:     Operating System
    Section:    004
*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>


typedef struct {
    char text[256];
    int number;
} DataTuple;

typedef struct {
    DataTuple *data;
    int start;
    int end;
} ThreadData;

//mutex for the merge
pthread_mutex_t merge_mutex;

// Function to read data from the CSV file
int read_data_from_csv(const char *filename, DataTuple data[]) {
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
    return count;
}

// Bubble sort function
void bubble_sort(DataTuple data[], int start, int end) {
    DataTuple temp;
    for (int i = start; i < end - 1; i++) {
        for (int j = start; j < end - (i - start) - 1; j++) {
            if (data[j].number > data[j + 1].number) {
                temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }
}

// Thread function for sorting
void *sort_thread(void *arg) {
    ThreadData *thread_data = (ThreadData *)arg;
    bubble_sort(thread_data->data, thread_data->start, thread_data->end);
    pthread_exit(NULL);
}

// Function to merge two sorted sections of the array
void merge(DataTuple data[], int start, int mid, int end) {
    int left_size = mid - start + 1;
    int right_size = end - mid;
    DataTuple *left = malloc(left_size * sizeof(DataTuple));
    DataTuple *right = malloc(right_size * sizeof(DataTuple));

    for (int i = 0; i < left_size; i++)
        left[i] = data[start + i];
    for (int i = 0; i < right_size; i++)
        right[i] = data[mid + 1 + i];

    int i = 0, j = 0, k = start;
    while (i < left_size && j < right_size) {
        if (left[i].number <= right[j].number) {
            data[k++] = left[i++];
        } else {
            data[k++] = right[j++];
        }
    }

    while (i < left_size) {
        data[k++] = left[i++];
    }
    while (j < right_size) {
        data[k++] = right[j++];
    }

    free(left);
    free(right);
}

// Main merge function for the sorted chunks
void merge_sorted_chunks(DataTuple data[], int num_threads, int chunk_size, int data_size) {
    int step = chunk_size;
    while (step < data_size) {
        for (int i = 0; i < data_size; i += 2 * step) {
            int mid = i + step - 1;
            int end = (i + 2 * step - 1 < data_size) ? i + 2 * step - 1 : data_size - 1;
            if (mid < end) {
                pthread_mutex_lock(&merge_mutex); // Synchronize merge operation
                merge(data, i, mid, end);
                pthread_mutex_unlock(&merge_mutex);
            }
        }
        step *= 2;
    }
}

int main() {
    DataTuple data[10000];
    int data_size = read_data_from_csv(FILENAME, data);
    int num_threads = 2;
    printf("How many threads would you like? ");
    scanf("%d", &num_threads);
    int chunk_size = data_size / num_threads;

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    pthread_mutex_init(&merge_mutex, NULL); // Initialize the mutex

    time_t start, end;
    double elapsed_time;
    start = clock();

    // Create threads to sort the data chunks
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].data = data;
        thread_data[i].start = i * chunk_size;
        thread_data[i].end = (i == num_threads - 1) ? data_size : (i + 1) * chunk_size;
        pthread_create(&threads[i], NULL, sort_thread, &thread_data[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Merge the sorted chunks
    merge_sorted_chunks(data, num_threads, chunk_size, data_size);

    end = clock();
    elapsed_time = end - start;
    printf("Parallel sorting time with %d threads: %.2f tics\n", num_threads, elapsed_time);

    // Optionally print the sorted result to verify correctness
    /*for (int i = 0; i < data_size; i++) {
        printf("%s, %d\n", data[i].text, data[i].number);
    }*/

    pthread_mutex_destroy(&merge_mutex); // Destroy the mutex
    return 0;
}