#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

void quickSortSequential(std::vector<int>& arr, int left, int right) {
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

        quickSortSequential(arr, left, partitionIndex - 1);
        quickSortSequential(arr, partitionIndex + 1, right);
    }
}

int main() {
    std::vector<int> arr(1000000);
    std::srand(std::time(0));
    for (int& num : arr) {
        num = std::rand() % 1000000;
    }

    clock_t start = clock();
    quickSortSequential(arr, 0, arr.size() - 1);
    clock_t end = clock();

    double timeTaken = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "Sequential QuickSort took: " << timeTaken << " seconds" << std::endl;

    return 0;
}