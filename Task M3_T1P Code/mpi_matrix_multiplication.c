#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 100

// Function to fill matrix with random values between 0 and 4
void fillMatrix(int mat[N][N]) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            mat[i][j] = rand() % 5;
}

int main(int argc, char *argv[]) {
    int rank, size;
    int A[N][N], B[N][N], C[N][N] = {0};

    MPI_Init(&argc, &argv);                      // Initialize MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);        // Get current process rank
    MPI_Comm_size(MPI_COMM_WORLD, &size);        // Get total number of processes

    int rows_per_proc = N / size;
    int start = rank * rows_per_proc;
    int end = (rank == size - 1) ? N : start + rows_per_proc;

    if (rank == 0) {
        srand(0);           // Seed for reproducible results
        fillMatrix(A);      // Fill matrices A and B with random values
        fillMatrix(B);
    }

    // Broadcast matrices A and B to all processes
    MPI_Bcast(A, N * N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    // Start timing
    double start_time = MPI_Wtime();

    // Matrix multiplication for assigned rows
    for (int i = start; i < end; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    // End timing
    double end_time = MPI_Wtime();
    double local_time = end_time - start_time;

    // Gather results from all processes
    MPI_Gather(&C[start][0], rows_per_proc * N, MPI_INT,
               &C[start][0], rows_per_proc * N, MPI_INT,
               0, MPI_COMM_WORLD);

    // Print time on rank 0
    if (rank == 0) {
        printf("Matrix multiplication complete.\n");
        printf("Execution Time: %.6f seconds\n", local_time);
    }

    MPI_Finalize(); // Finalize MPI
    return 0;
}
