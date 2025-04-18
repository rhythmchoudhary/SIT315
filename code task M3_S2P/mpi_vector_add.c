#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    int rank, size;
    int n = 8;
    int *v1 = NULL, *v2 = NULL, *v3 = NULL;
    int *local_v1, *local_v2, *local_v3;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int local_n = n / size;

    local_v1 = (int*)malloc(local_n * sizeof(int));
    local_v2 = (int*)malloc(local_n * sizeof(int));
    local_v3 = (int*)malloc(local_n * sizeof(int));

    if (rank == 0) {
        v1 = (int*)malloc(n * sizeof(int));
        v2 = (int*)malloc(n * sizeof(int));
        v3 = (int*)malloc(n * sizeof(int));

        for (int i = 0; i < n; i++) {
            v1[i] = i + 1;
            v2[i] = 1;
        }
    }

    MPI_Scatter(v1, local_n, MPI_INT, local_v1, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(v2, local_n, MPI_INT, local_v2, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < local_n; i++) {
        local_v3[i] = local_v1[i] + local_v2[i];
    }

    MPI_Gather(local_v3, local_n, MPI_INT, v3, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    int local_sum = 0, total_sum = 0;
    for (int i = 0; i < local_n; i++) {
        local_sum += local_v3[i];
    }

    MPI_Reduce(&local_sum, &total_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Final result vector v3:\n");
        for (int i = 0; i < n; i++) {
            printf("%d ", v3[i]);
        }
        printf("\nTotal sum of v3: %d\n", total_sum);

        free(v1); free(v2); free(v3);
    }

    free(local_v1); free(local_v2); free(local_v3);
    MPI_Finalize();
    return 0;
}
