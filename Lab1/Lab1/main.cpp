#include <cstdio>
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#include <stdlib.h>
int main(int argc, char* argv[]) {
    cl_int ret = CL_SUCCESS;
    cl_uint platform_count = 0;
    clGetPlatformIDs(0, nullptr, &platform_count);

    cl_platform_id* platforms = new cl_platform_id[platform_count];
    clGetPlatformIDs(platform_count, platforms, nullptr);
    for (cl_uint i = 0; i < platform_count; ++i) {
        char platformName[128];
        clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 128, platformName, nullptr);
    }
    if (platform_count == 0) {
        fprintf(stderr, "No platform found!\n");
    }

    cl_context_properties properties[3] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0], 0
    };
    cl_context context = clCreateContextFromType(properties, CL_DEVICE_TYPE_ALL, nullptr, nullptr, nullptr);
    size_t device_count = 0;
    clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, 0, &device_count);
    if (device_count == 0) {
        fprintf(stderr, "No platform found!\n");

    }
    cl_device_id* devices = new cl_device_id[device_count];  
    clGetContextInfo(context, CL_CONTEXT_DEVICES, device_count, devices, nullptr);

    cl_command_queue queue = clCreateCommandQueue(context, devices[0], 0, nullptr);

    FILE* kernel_file = fopen("lab1.cl", "r");
    fseek(kernel_file, 0, SEEK_END);
    size_t source_len = ftell(kernel_file);
    fseek(kernel_file, 0, SEEK_SET);
    char* source = (char*)calloc(1, source_len + 1);
    fread(source, 1, source_len, kernel_file);

    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&source, &source_len, nullptr);
    if (clBuildProgram(program, 1, &devices[0], nullptr, nullptr, nullptr) == CL_BUILD_PROGRAM_FAILURE) {
        size_t log_size;
        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char* log = (char*)malloc(log_size);

        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        fprintf(stderr, "%s\n", log);

    }

    cl_kernel kernel = clCreateKernel(program, "print_info", nullptr);

    size_t workgroup_size;
    clGetKernelWorkGroupInfo(kernel, devices[0], CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &workgroup_size, nullptr);

    size_t global_work_size = workgroup_size;
    clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_work_size, &workgroup_size, 0, nullptr, nullptr);
    clFinish(queue);

    free(source);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    return 0;
}
