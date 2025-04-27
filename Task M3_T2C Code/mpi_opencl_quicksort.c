#include <mpi.h>
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 16  

// OpenCL kernel for partitioning and compaction
const char *kernelSource = 
"__kernel void partition_compact(__global int* data, int pivot, __global int* out, __global int* counts) {\n"
"   int id = get_global_id(0);\n"
"   int val = data[id];\n"
"   int pos = 0;\n"
"   \n"
"   if (val < pivot) {\n"
"       pos = atomic_inc(&counts[0]);\n"
"       out[pos] = val;\n"
"   } else if (val > pivot) {\n"
"       pos = atomic_inc(&counts[2]) + counts[0] + counts[1];\n"
"       out[pos] = val;\n"
"   } else {\n"
"       pos = atomic_inc(&counts[1]) + counts[0];\n"
"       out[pos] = val;\n"
"   }\n"
"}\n";

void serial_quicksort(int* arr, int left, int right) {
    if (left >= right) return;
    
    int pivot = arr[(left + right) / 2];
    int i = left, j = right;
    
    while (i <= j) {
        while (arr[i] < pivot) i++;
        while (arr[j] > pivot) j--;
        if (i <= j) {
            int tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            i++;
            j--;
        }
    }
    
    serial_quicksort(arr, left, j);
    serial_quicksort(arr, i, right);
}

int main(int argc, char** argv) {
    int rank, size;
    int *data = NULL;
    int chunk_size;
    int *chunk;
    double start, end;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Ensure N is divisible by size
    if (N % size != 0) {
        if (rank == 0) {
            printf("Error: Array size (%d) must be divisible by number of processes (%d)\n", N, size);
        }
        MPI_Finalize();
        return 1;
    }

    chunk_size = N / size;
    chunk = (int*)malloc(sizeof(int) * chunk_size);

    if (rank == 0) {
        data = (int*)malloc(sizeof(int) * N);
        srand(time(NULL));
        for (int i = 0; i < N; i++) {
            data[i] = rand() % 100;
        }
        printf("Unsorted array (%d elements):\n", N);
        for (int i = 0; i < N; i++) {
            printf("%d ", data[i]);
        }
        printf("\n");
    }

    start = MPI_Wtime();

    // Scatter chunks
    MPI_Scatter(data, chunk_size, MPI_INT, chunk, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Initialize OpenCL
    cl_platform_id platform_id;
    cl_device_id device_id;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem d_chunk, d_out, d_counts;
    size_t global_size = chunk_size;
    cl_int ret;

    ret = clGetPlatformIDs(1, &platform_id, NULL);
    if (ret != CL_SUCCESS) {
        printf("Error: No OpenCL platform found!\n");
        MPI_Finalize();
        exit(1);
    }

    // Try to get GPU device first
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    if (ret != CL_SUCCESS) {
        if (rank == 0) {
            printf("Warning: No GPU device found, trying CPU device...\n");
        }
        ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
        if (ret != CL_SUCCESS) {
            printf("Error: No OpenCL GPU or CPU device found!\n");
            MPI_Finalize();
            exit(1);
        }
    }

    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &ret);
    program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, &ret);
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

    if (ret != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = (char *)malloc(log_size);
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("Build log:\n%s\n", log);
        free(log);
        MPI_Finalize();
        exit(1);
    }

    kernel = clCreateKernel(program, "partition_compact", &ret);

    d_chunk = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * chunk_size, NULL, &ret);
    d_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * chunk_size, NULL, &ret);
    d_counts = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * 3, NULL, &ret);

    // Initialize counts to 0
    int counts[3] = {0, 0, 0};
    clEnqueueWriteBuffer(queue, d_counts, CL_TRUE, 0, sizeof(int) * 3, counts, 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, d_chunk, CL_TRUE, 0, sizeof(int) * chunk_size, chunk, 0, NULL, NULL);

    // Choose pivot - better to use a global pivot but using local for simplicity
    int pivot = chunk[chunk_size / 2];

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_chunk);
    clSetKernelArg(kernel, 1, sizeof(int), &pivot);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_out);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &d_counts);

    ret = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);

    // Read back the results
    int* partitioned = (int*)malloc(sizeof(int) * chunk_size);
    clEnqueueReadBuffer(queue, d_out, CL_TRUE, 0, sizeof(int) * chunk_size, partitioned, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, d_counts, CL_TRUE, 0, sizeof(int) * 3, counts, 0, NULL, NULL);
    clFinish(queue);

    // Sort the partitioned data
    if (counts[0] > 1) serial_quicksort(partitioned, 0, counts[0] - 1); // Sort low part
    if (counts[1] > 1) serial_quicksort(partitioned, counts[0], counts[0] + counts[1] - 1); // Sort equal part
    if (counts[2] > 1) serial_quicksort(partitioned, counts[0] + counts[1], chunk_size - 1); // Sort high part

    // Cleanup OpenCL
    clReleaseMemObject(d_chunk);
    clReleaseMemObject(d_out);
    clReleaseMemObject(d_counts);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    // Gather all sorted chunks
    MPI_Gather(partitioned, chunk_size, MPI_INT, data, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    end = MPI_Wtime();

    if (rank == 0) {
        printf("\nSorted array (%d elements):\n", N);
        for (int i = 0; i < N; i++) {
            printf("%d ", data[i]);
        }
        printf("\n");
        printf("Execution Time: %f seconds\n", end - start);
    }

    free(chunk);
    free(partitioned);
    if (rank == 0) free(data);

    MPI_Finalize();
    return 0;
}
