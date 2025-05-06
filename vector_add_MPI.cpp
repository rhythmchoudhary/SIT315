#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>

#define N 1000000  // Total vector size

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int chunk_size = N / size;
    std::vector<float> A_chunk(chunk_size), B_chunk(chunk_size), C_chunk(chunk_size);
    std::vector<float> A, B, C;

    if (rank == 0) {
        A.resize(N);
        B.resize(N);
        C.resize(N);
        for (int i = 0; i < N; i++) {
            A[i] = static_cast<float>(rand()) / RAND_MAX;
            B[i] = static_cast<float>(rand()) / RAND_MAX;
        }
    }

    double start_time = MPI_Wtime();

    MPI_Scatter(A.data(), chunk_size, MPI_FLOAT, A_chunk.data(), chunk_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Scatter(B.data(), chunk_size, MPI_FLOAT, B_chunk.data(), chunk_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < chunk_size; i++) {
        C_chunk[i] = A_chunk[i] + B_chunk[i];
    }

    MPI_Gather(C_chunk.data(), chunk_size, MPI_FLOAT, C.data(), chunk_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();

    if (rank == 0) {
        std::cout << "MPI Vector Addition Completed.\n";
        std::cout << "MPI Execution Time: " << (end_time - start_time) * 1e9 << " ns\n";
    }

    MPI_Finalize();
    return 0;
}
