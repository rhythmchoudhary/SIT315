#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <omp.h>

#define N 1000000  // Vector size

// Error checking utility
void checkErr(cl_int err, const char* name) {
    if (err != CL_SUCCESS) {
        std::cerr << "ERROR: " << name << " (" << err << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}

const char* kernelSource = R"(
__kernel void vector_add(__global const float* A,  // Input vector A
                        __global const float* B,  // Input vector B
                        __global float* C) {      // Output vector C
    int i = get_global_id(0);  // Get thread ID
    C[i] = A[i] + B[i];       // Perform addition
}
)";

int main() {
    // Initialize random vectors A and B
    std::vector<float> A(N), B(N), C(N), C_host(N);
    for (int i = 0; i < N; i++) {
        A[i] = static_cast<float>(rand()) / RAND_MAX;
        B[i] = static_cast<float>(rand()) / RAND_MAX;
    }

    // --- OpenCL Setup ---
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;

    // Get platform and device (using CPU as fallback)
    err = clGetPlatformIDs(1, &platform, nullptr);
    checkErr(err, "clGetPlatformIDs");
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
    if (err != CL_SUCCESS) {
        std::cerr << "GPU not found. Using CPU instead." << std::endl;
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, nullptr);
        checkErr(err, "clGetDeviceIDs (CPU)");
    }

    // Create context and command queue
    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    checkErr(err, "clCreateContext");
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
    checkErr(err, "clCreateCommandQueue");

    // Build OpenCL program
    cl_program program = clCreateProgramWithSource(context, 1, &kernelSource, nullptr, &err);
    checkErr(err, "clCreateProgramWithSource");
    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::vector<char> log(logSize);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
        std::cerr << "Build error:\n" << log.data() << std::endl;
        exit(1);
    }

    // Create buffers
    cl_mem bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * N, A.data(), &err);
    cl_mem bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * N, B.data(), &err);
    cl_mem bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * N, nullptr, &err);
    checkErr(err, "clCreateBuffer");

    // Execute kernel
    cl_kernel kernel = clCreateKernel(program, "vector_add", &err);
    checkErr(err, "clCreateKernel");
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferA);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferB);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufferC);
    checkErr(err, "clSetKernelArg");

    size_t globalSize = N;
    auto start_gpu = std::chrono::high_resolution_clock::now();
    err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);
    checkErr(err, "clEnqueueNDRangeKernel");
    clFinish(queue);

    // Read results
    err = clEnqueueReadBuffer(queue, bufferC, CL_TRUE, 0, sizeof(float) * N, C.data(), 0, nullptr, nullptr);
    checkErr(err, "clEnqueueReadBuffer");
    auto end_gpu = std::chrono::high_resolution_clock::now();

    // --- OpenMP (CPU) Comparison ---
    auto start_cpu = std::chrono::high_resolution_clock::now();
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        C_host[i] = A[i] + B[i];
    }
    auto end_cpu = std::chrono::high_resolution_clock::now();

    // Verify results
    bool correct = true;
    for (int i = 0; i < N; i++) {
        if (fabs(C[i] - C_host[i]) > 1e-5) {
            correct = false;
            break;
        }
    }

    // --- Results ---
    std::cout << "Results are " << (correct ? "correct " : "incorrect ") << std::endl;
    auto duration_gpu = std::chrono::duration_cast<std::chrono::nanoseconds>(end_gpu - start_gpu).count();
    auto duration_cpu = std::chrono::duration_cast<std::chrono::nanoseconds>(end_cpu - start_cpu).count();
    std::cout << "OpenCL time: " << duration_gpu << " ns\n";
    std::cout << "OpenMP (CPU) time: " << duration_cpu << " ns\n";

    // Cleanup
    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}
