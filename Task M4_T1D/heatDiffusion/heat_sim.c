#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define GRID_SIZE 100
#define MAX_STEPS 500
#define ALPHA 0.1
#define MASTER 0
#define INITIAL_TEMP 20.0
#define SOURCE_TEMP 100.0

double** allocate_2d(int rows, int cols) {
    double* data = (double*)malloc(rows * cols * sizeof(double));
    double** array = (double**)malloc(rows * sizeof(double*));
    for (int i = 0; i < rows; i++)
        array[i] = &data[i * cols];
    return array;
}

void initialize(double** grid, int local_rows, int cols, int rank, int size) {
    for (int i = 0; i < local_rows + 2; i++) {
        for (int j = 0; j < cols; j++) {
            grid[i][j] = INITIAL_TEMP;
        }
    }

    int global_center_row = GRID_SIZE / 2;
    int start_row = rank * local_rows;
    int end_row = start_row + local_rows - 1;

    if (global_center_row >= start_row && global_center_row <= end_row) {
        int local_center_row = global_center_row - start_row + 1;
        grid[local_center_row][cols / 2] = SOURCE_TEMP;
    }
}

int main(int argc, char* argv[]) {
    int rank, size;
    int rows = GRID_SIZE;
    int cols = GRID_SIZE;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rows % size != 0) {
        if (rank == MASTER)
            printf("Grid rows not divisible by number of processes.\n");
        MPI_Finalize();
        return -1;
    }

    int local_rows = rows / size;
    double** current = allocate_2d(local_rows + 2, cols);
    double** next = allocate_2d(local_rows + 2, cols);

    initialize(current, local_rows, cols, rank, size);

    double start_time = MPI_Wtime();

    for (int step = 0; step < MAX_STEPS; step++) {
        if (rank > 0) {
            MPI_Send(current[1], cols, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
            MPI_Recv(current[0], cols, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        if (rank < size - 1) {
            MPI_Send(current[local_rows], cols, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
            MPI_Recv(current[local_rows + 1], cols, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        for (int i = 1; i <= local_rows; i++) {
            for (int j = 1; j < cols - 1; j++) {
                next[i][j] = current[i][j] + ALPHA * (
                    current[i + 1][j] + current[i - 1][j] +
                    current[i][j + 1] + current[i][j - 1] -
                    4 * current[i][j]
                );
            }
        }

        // Reapply heat source every step
        int global_center_row = GRID_SIZE / 2;
        int start_row = rank * local_rows;
        int end_row = start_row + local_rows - 1;

        if (global_center_row >= start_row && global_center_row <= end_row) {
            int local_center_row = global_center_row - start_row + 1;
            next[local_center_row][cols / 2] = SOURCE_TEMP;
        }

        for (int i = 1; i <= local_rows; i++) {
            next[i][0] = INITIAL_TEMP;
            next[i][cols - 1] = INITIAL_TEMP;
        }

        double** temp = current;
        current = next;
        next = temp;
    }

    double end_time = MPI_Wtime();
    double local_elapsed = end_time - start_time;
    double max_elapsed;
    MPI_Reduce(&local_elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, MASTER, MPI_COMM_WORLD);

    double* local_data = &current[1][0];
    double* full_grid = NULL;
    if (rank == MASTER) {
        full_grid = (double*)malloc(rows * cols * sizeof(double));
    }

    MPI_Gather(local_data, local_rows * cols, MPI_DOUBLE,
               full_grid, local_rows * cols, MPI_DOUBLE,
               MASTER, MPI_COMM_WORLD);

    if (rank == MASTER) {
        FILE* fp = fopen("output.txt", "w");
        if (!fp) {
            printf("Failed to write output.txt\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                fprintf(fp, "%.2f ", full_grid[i * cols + j]);
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
        printf("Output written to output.txt\n");
        printf("Max execution time: %.6f seconds\n", max_elapsed);
        free(full_grid);
    }

    free(current[0]);
    free(current);
    free(next[0]);
    free(next);

    MPI_Finalize();
    return 0;
}

