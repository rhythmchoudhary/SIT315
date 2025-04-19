#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

#define N 100

// Function to fill matrix with random numbers
void fillMatrix(int mat[N][N]) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            mat[i][j] = rand() % 5;
}

int main(int argc, char *argv[]) {
    int rank, size;
    int A[N][N], B[N][N], C[N][N] = {0};

    MPI_Init(&argc, &argv);                      // Initialize MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);        // Get process rank
    MPI_Comm_size(MPI_COMM_WORLD, &size);        // Get total processes

    int rows_per_proc = N / size;
    int start = rank * rows_per_proc;
    int end = (rank == size - 1) ? N : start + rows_per_proc;

    if (rank == 0) {
        srand(0);
        fillMatrix(A);
        fillMatrix(B);
    }

    // Broadcast matrices to all processes
    MPI_Bcast(A, N * N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    double start_time = MPI_Wtime();  // Start timing

    // Parallel region (OpenMP)
    #pragma omp parallel for num_threads(4)
    for (int i = start; i < end; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    double end_time = MPI_Wtime();  // End timing
    double local_time = end_time - start_time;

    // Gather results at root
    MPI_Gather(&C[start][0], rows_per_proc * N, MPI_INT,
               &C[start][0], rows_per_proc * N, MPI_INT,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Hybrid MPI+OpenMP multiplication complete.\n");
        printf("Execution Time: %.6f seconds\n", local_time);
    }

    MPI_Finalize(); // Finalize MPI
    return 0;
}
