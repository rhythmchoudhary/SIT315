#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <chrono>
#include <omp.h>

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
    
    auto start = high_resolution_clock::now();
    
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    auto stop = high_resolution_clock::now();
    
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "Execution time: " << duration.count() << " ms" << endl;
    
    writeMatrixToFile(C, "output_matrix_openmp.txt");
    
    return 0;
}
