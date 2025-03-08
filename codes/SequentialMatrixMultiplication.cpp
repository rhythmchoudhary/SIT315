#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Function to generate a random matrix
vector<vector<int>> generateMatrix(int N) {
    vector<vector<int>> matrix(N, vector<int>(N));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = rand() % 5; // Random values between 0-9
        }
    }
    return matrix;
}

// Function for matrix multiplication
vector<vector<int>> multiplyMatrices(const vector<vector<int>>& A, const vector<vector<int>>& B, int N) {
    vector<vector<int>> C(N, vector<int>(N, 0));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C;
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
    int N = 100; // Matrix size
    
    vector<vector<int>> A = generateMatrix(N);
    vector<vector<int>> B = generateMatrix(N);
    
    auto start = high_resolution_clock::now();
    vector<vector<int>> C = multiplyMatrices(A, B, N);
    auto stop = high_resolution_clock::now();
    
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "Execution time: " << duration.count() << " ms" << endl;
    
    writeMatrixToFile(C, "output_matrix.txt");
    
    return 0;
}