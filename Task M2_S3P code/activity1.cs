#include <chrono>
#include <cstdlib>
#include <iostream>
#include <omp.h>

using namespace std::chrono;
using namespace std;

void randomVector(int vector[], int size) {
    for (int i = 0; i < size; i++) {
        vector[i] = rand() % 100;
    }
}

int main() {
    unsigned long size = 100000000;
    srand(time(0));

    int *v1, *v2, *v3;
    v1 = (int *)malloc(size * sizeof(int));
    v2 = (int *)malloc(size * sizeof(int));
    v3 = (int *)malloc(size * sizeof(int));

    randomVector(v1, size);
    randomVector(v2, size);

    // Sequential Execution
    auto start_seq = high_resolution_clock::now();
    for (int i = 0; i < size; i++) {
        v3[i] = v1[i] + v2[i];
    }
    auto stop_seq = high_resolution_clock::now();
    auto duration_seq = duration_cast<microseconds>(stop_seq - start_seq);
    cout << "Time taken by sequential execution: " << duration_seq.count() << " microseconds" << endl;

    // OpenMP Parallel Execution
    auto start_omp = high_resolution_clock::now();
    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        v3[i] = v1[i] + v2[i];
    }
    auto stop_omp = high_resolution_clock::now();
    auto duration_omp = duration_cast<microseconds>(stop_omp - start_omp);
    cout << "Time taken by OpenMP parallel execution: " << duration_omp.count() << " microseconds" << endl;

    free(v1);
    free(v2);
    free(v3);

    return 0;
}
