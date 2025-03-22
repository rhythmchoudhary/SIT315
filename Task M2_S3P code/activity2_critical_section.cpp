#include <iostream>
#include <chrono>
#include <cstdlib>
#include <omp.h>

using namespace std;
using namespace chrono;

void randomVector(int *vector, int size) {
    for (int i = 0; i < size; i++) {
        vector[i] = rand() % 100;
    }
}

int main() {
    int size = 100000;
    srand(time(0));

    int *v1 = new int[size];
    int *v2 = new int[size];
    int *v3 = new int[size];

    randomVector(v1, size);
    randomVector(v2, size);

    long long total = 0;

    auto start = high_resolution_clock::now();

    #pragma omp parallel
    {
        long long local_sum = 0;
        #pragma omp for
        for (int i = 0; i < size; i++) {
            local_sum += v3[i];
        }
        #pragma omp critical
        total += local_sum;
    }

    auto stop = high_resolution_clock::now();
    cout << "Total sum: " << total << "\nTime taken: " << duration_cast<microseconds>(stop - start).count() << " microseconds\n";

    delete[] v1;
    delete[] v2;
    delete[] v3;

    return 0;
}
