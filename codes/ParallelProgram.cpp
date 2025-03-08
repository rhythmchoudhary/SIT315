#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <pthread.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

const int N = 100; // Matrix size
const int NUM_THREADS = 4; // Number of threads

vector<vector<int>> A(N, vector<int>(N));
vector<vector<int>> B(N, vector<int>(N));
vector<vector<int>> C(N, vector<int>(N, 0));

// Function to generate a random matrix
void generateMatrix(vector<vector<int>>& matrix) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = rand() % 5; // Random values between 0-4
        }
    }
}

// Structure to pass multiple arguments to thread function
struct ThreadData {
    int thread_id;
};

// Thread function for matrix multiplication
void* multiplyRows(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int thread_id = data->thread_id;
    
    int rows_per_thread = N / NUM_THREADS;
    int start_row = thread_id * rows_per_thread;
    int end_row = (thread_id == NUM_THREADS - 1) ? N : start_row + rows_per_thread;
    
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    pthread_exit(nullptr);
}

// Function to write matrix to a file
void writeMatrixToFile(const vector<vector<int>>& matrix, const string& filename) {
    ofstream file(filename);
    for (const auto& row : matrix) {
        for (int val : row) {
            file << val << " ";
        }
        file << "\n";
    }
    file.close();
}

int main() {
    srand(time(0));
    generateMatrix(A);
    generateMatrix(B);
    
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        pthread_create(&threads[i], nullptr, multiplyRows, (void*)&thread_data[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], nullptr);
    }
    
    auto stop = high_resolution_clock::now();
    
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "Execution time: " << duration.count() << " ms" << endl;
    
    writeMatrixToFile(C, "output_matrix_parallel.txt");
    
    return 0;
}
