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

    auto start = high_resolution_clock::now();
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < size; i++) {
        v3[i] = v1[i] + v2[i];
    }

    auto stop = high_resolution_clock::now();
    cout << "Time taken with static scheduling: " << duration_cast<microseconds>(stop - start).count() << " microseconds\n";

    delete[] v1;
    delete[] v2;
    delete[] v3;

    return 0;
}
