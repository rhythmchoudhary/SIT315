#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 16         
#define MASTER 0

void quicksort(int *arr, int left, int right) {
    int i = left, j = right;
    int tmp;
    int pivot = arr[(left + right) / 2];

    while (i <= j) {
        while (arr[i] < pivot) i++;
        while (arr[j] > pivot) j--;
        if (i <= j) {
            tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            i++;
            j--;
        }
    }

    if (left < j) quicksort(arr, left, j);
    if (i < right) quicksort(arr, i, right);
}

void merge(int *a, int size_a, int *b, int size_b, int *result) {
    int i = 0, j = 0, k = 0;

    while (i < size_a && j < size_b) {
        if (a[i] < b[j]) result[k++] = a[i++];
        else result[k++] = b[j++];
    }

    while (i < size_a) result[k++] = a[i++];
    while (j < size_b) result[k++] = b[j++];
}

int main(int argc, char *argv[]) {
    int rank, size;
    int *data = NULL;
    int *local_data;
    int chunk_size;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    chunk_size = ARRAY_SIZE / size;
    local_data = (int *)malloc(chunk_size * sizeof(int));

    if (rank == MASTER) {
        data = (int *)malloc(ARRAY_SIZE * sizeof(int));
        srand(time(NULL));

        printf("Original Array: ");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = rand() % 100;
            printf("%d ", data[i]);
        }
        printf("\n");

        start_time = MPI_Wtime();  // Start timing
    }

    MPI_Scatter(data, chunk_size, MPI_INT, local_data, chunk_size, MPI_INT, MASTER, MPI_COMM_WORLD);

    quicksort(local_data, 0, chunk_size - 1);

    int *sorted = NULL;
    if (rank == MASTER) {
        sorted = (int *)malloc(ARRAY_SIZE * sizeof(int));
    }

    MPI_Gather(local_data, chunk_size, MPI_INT, sorted, chunk_size, MPI_INT, MASTER, MPI_COMM_WORLD);

    if (rank == MASTER) {
        int *temp = sorted;
        int *merged = (int *)malloc(ARRAY_SIZE * sizeof(int));
        for (int i = chunk_size; i < ARRAY_SIZE; i += chunk_size) {
            merge(temp, i, sorted + i, chunk_size, merged);
            for (int j = 0; j < i + chunk_size; j++) temp[j] = merged[j];
        }

        end_time = MPI_Wtime();  // End timing

        printf("Sorted Array:   ");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", temp[i]);
        }
        printf("\n");

        printf("\n Execution Time (MPI only): %.6f seconds\n", end_time - start_time);

        free(data);
        free(sorted);
        free(merged);
    }

    free(local_data);
    MPI_Finalize();
    return 0;
}
