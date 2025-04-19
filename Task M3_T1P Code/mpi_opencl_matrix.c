#include <stdio.h>
#include <stdlib.h>
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#include <mpi.h>

#define N 100
#define MAX_SOURCE_SIZE (0x100000)

void fillMatrix(int mat[N][N]) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            mat[i][j] = rand() % 5;
}

int main(int argc, char* argv[]) {
    int rank, size;
    int A[N][N], B[N][N], C[N][N] = {0};

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows_per_proc = N / size;
    int start = rank * rows_per_proc;
    int end = (rank == size - 1) ? N : start + rows_per_proc;

    if (rank == 0) {
        srand(0);
        fillMatrix(A);
        fillMatrix(B);
    }

    MPI_Bcast(A, N * N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    double start_time = MPI_Wtime();  // Start timing

    // OpenCL Setup
    cl_platform_id platform_id;
    cl_device_id device_id;
    cl_context context;
    cl_command_queue command_queue;
    cl_program program;
    cl_kernel kernel;
    cl_int ret;

    clGetPlatformIDs(1, &platform_id, NULL);
    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, NULL);
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret); // Use OpenCL 1.2 version

    // Load kernel source
    FILE* f = fopen("kernel.cl", "r");
    char* source_str = (char*)malloc(MAX_SOURCE_SIZE);
    size_t source_size = fread(source_str, 1, MAX_SOURCE_SIZE, f);
    fclose(f);

    program = clCreateProgramWithSource(context, 1, (const char**)&source_str, &source_size, &ret);
    clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    kernel = clCreateKernel(program, "mat_mul", &ret);

    // Buffers
    cl_mem a_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, N * N * sizeof(int), NULL, &ret);
    cl_mem b_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, N * N * sizeof(int), NULL, &ret);
    cl_mem c_mem = clCreateBuffer(context, CL_MEM_WRITE_ONLY, N * N * sizeof(int), NULL, &ret);

    // Copy data to GPU
    clEnqueueWriteBuffer(command_queue, a_mem, CL_TRUE, 0, N * N * sizeof(int), A, 0, NULL, NULL);
    clEnqueueWriteBuffer(command_queue, b_mem, CL_TRUE, 0, N * N * sizeof(int), B, 0, NULL, NULL);

    // Set kernel args
    int size_n = N;
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &a_mem);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &b_mem);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &c_mem);
    clSetKernelArg(kernel, 3, sizeof(int), &size_n);
    clSetKernelArg(kernel, 4, sizeof(int), &start);

    size_t global_size[2] = {end - start, N};
    clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_size, NULL, 0, NULL, NULL);
    clFinish(command_queue);

    // Read back result
    clEnqueueReadBuffer(command_queue, c_mem, CL_TRUE, 0, N * N * sizeof(int), C, 0, NULL, NULL);

    double end_time = MPI_Wtime();  // End timing
    double local_time = end_time - start_time;

    // Gather results
    MPI_Gather(&C[start][0], rows_per_proc * N, MPI_INT,
               &C[start][0], rows_per_proc * N, MPI_INT,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Hybrid MPI+OpenCL matrix multiplication complete.\n");
        printf("Execution Time: %.6f seconds\n", local_time);
    }

    // Cleanup
    clReleaseMemObject(a_mem);
    clReleaseMemObject(b_mem);
    clReleaseMemObject(c_mem);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
    free(source_str);

    MPI_Finalize();
    return 0;
}

