#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

void quickSortParallel(std::vector<int>& arr, int left, int right) {
    if (left < right) {
        int pivot = arr[right];
        int i = left - 1;

        for (int j = left; j < right; ++j) {
            if (arr[j] < pivot) {
                ++i;
                std::swap(arr[i], arr[j]);
            }
        }
        std::swap(arr[i + 1], arr[right]);

        int partitionIndex = i + 1;

        #pragma omp parallel sections
        {
            #pragma omp section
            {
                quickSortParallel(arr, left, partitionIndex - 1);
            }
            #pragma omp section
            {
                quickSortParallel(arr, partitionIndex + 1, right);
            }
        }
    }
}

int main() {
    std::vector<int> arr(1000000);
    std::srand(std::time(0));
    for (int& num : arr) {
        num = std::rand() % 1000000;
    }

    double start = omp_get_wtime();
    quickSortParallel(arr, 0, arr.size() - 1);
    double end = omp_get_wtime();

    double timeTaken = end - start;
    std::cout << "Parallel QuickSort took: " << timeTaken << " seconds" << std::endl;

    return 0;
}