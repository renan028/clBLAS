/* ************************************************************************
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>

/* Include CLBLAS header. It automatically includes needed OpenCL header,
 * so we can drop out explicit inclusion of cl.h header.
 */
#include <clBLAS.h>
/* This example uses predefined matrices and their characteristics for
 * simplicity purpose.
 */
static const clblasOrder order = clblasColumnMajor;

static const size_t N = 5;

static const cl_double2 alpha = {{10,10}};
static const clblasUplo uplo = clblasUpper;
static const cl_double2 A[] = {
	{{ 1.0, 00.0}}, {{ 2.0, 02.0}}, {{ 4.0,  4.0}}, {{ 7.0,  7.0}}, {{11.0, 11.0}},
    {{00.0, 00.0}}, {{ 3.0, 03.0}}, {{ 5.0,  5.0}}, {{ 8.0,  8.0}}, {{12.0, 12.0}},
    {{00.0, 00.0}}, {{00.0, 00.0}}, {{ 6.0,  6.0}}, {{ 9.0,  9.0}}, {{13.0, 13.0}},
    {{00.0, 00.0}}, {{00.0, 00.0}}, {{00.0, 00.0}}, {{10.0, 10.0}}, {{14.0, 14.0}},
	{{00.0, 00.0}}, {{00.0, 00.0}}, {{00.0, 00.0}}, {{00.0, 00.0}}, {{15.0, 15.0}}
};
static const size_t lda = 5;    /* i.e. lda = N */

static const cl_double2 X[] = {
	{{1.0, 0.0}},
	{{2.0, 0.0}},
	{{3.0, 0.0}},
	{{4.0, 0.0}},
	{{5.0, 0.0}}
};
static const int incx = 1;

static const cl_double2 beta = {{20.0, 20.0}};

static cl_double2 Y[] = {
	{{1.0, 0.0}},
    {{2.0, 0.0}},
    {{3.0, 0.0}},
    {{4.0, 0.0}},
    {{5.0, 0.0}}
};
static const int incy = 1;

static void
printResult(void)
{
    size_t i, nElements;

    printf("Result:\n");

    nElements = (sizeof(Y) / sizeof(cl_double2)) / incy;
    for (i = 0; i < nElements; i++) {
        printf("(%9.2f, %-9.2f)\n", CREAL(Y[i * incy]), CIMAG(Y[i * incy]));
    }
}

int
main(void)
{
    cl_int err;
    cl_platform_id platform = 0;
    cl_device_id device = 0;
    cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
    cl_context ctx = 0;
    cl_command_queue queue = 0;
    cl_mem bufA, bufX, bufY;
    cl_event event = NULL;
    int ret = 0;

    /* Setup OpenCL environment. */
    err = clGetPlatformIDs(1, &platform, NULL);
    if (err != CL_SUCCESS) {
        printf( "clGetPlatformIDs() failed with %d\n", err );
        return 1;
    }

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err != CL_SUCCESS) {
        printf( "clGetDeviceIDs() failed with %d\n", err );
        return 1;
    }

    props[1] = (cl_context_properties)platform;
    ctx = clCreateContext(props, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        printf( "clCreateContext() failed with %d\n", err );
        return 1;
    }

    queue = clCreateCommandQueue(ctx, device, 0, &err);
    if (err != CL_SUCCESS) {
        printf( "clCreateCommandQueue() failed with %d\n", err );
        clReleaseContext(ctx);
        return 1;
    }

    /* Setup clblas. */
    err = clblasSetup();
    if (err != CL_SUCCESS) {
        printf("clblasSetup() failed with %d\n", err);
        clReleaseCommandQueue(queue);
        clReleaseContext(ctx);
        return 1;
    }

    /* Prepare OpenCL memory objects and place matrices inside them. */
    bufA = clCreateBuffer(ctx, CL_MEM_READ_ONLY, N * lda * sizeof(*A),
                          NULL, &err);
    bufX = clCreateBuffer(ctx, CL_MEM_READ_ONLY, N * sizeof(*X),
                          NULL, &err);
    bufY = clCreateBuffer(ctx, CL_MEM_READ_WRITE, N * sizeof(*Y),
                          NULL, &err);

    err = clEnqueueWriteBuffer(queue, bufA, CL_TRUE, 0,
        N * lda * sizeof(*A), A, 0, NULL, NULL);
    err = clEnqueueWriteBuffer(queue, bufX, CL_TRUE, 0,
        N * sizeof(*X), X, 0, NULL, NULL);
    err = clEnqueueWriteBuffer(queue, bufY, CL_TRUE, 0,
        N * sizeof(*Y), Y, 0, NULL, NULL);

    /* Call clblas function. */
    err = clblasZhemv(order, uplo, N, alpha, bufA, 0 /*offA */, lda,
												bufX, 0 /*offx*/, incx, beta,
												bufY, 0 /*offx*/, incy, 1, &queue, 0, NULL, &event);

//	blasZhemv(order, uplo, N, alpha, (DoubleComplex*)A, 0, lda, (DoubleComplex*)X, 0, incx, beta, (DoubleComplex*)Y, 0, incy);
//	err = CL_SUCCESS;
	//err = clblasZtrmv(order, uplo, clblasNoTrans, clblasNonUnit, N, bufA, 0 /*offA */, lda,
    //                                      bufX, 0 /*offx*/, incx,
    //                                      bufY, 1, &queue, 0, NULL, &event);

   	if (err != CL_SUCCESS) {
        printf("clblasZhemv() failed with %d\n", err);
        ret = 1;
    }
    else {
        /* Wait for calculations to be finished. */
        err = clWaitForEvents(1, &event);
		printResult();
        /* Fetch results of calculations from GPU memory. */
        err = clEnqueueReadBuffer(queue, bufY, CL_TRUE, 0, N * sizeof(*Y),
                                  Y, 0, NULL, NULL);
        /* At this point you will get the result of SSYMV placed in Y array. */
        printResult();
    }

    /* Release OpenCL events. */
    clReleaseEvent(event);

    /* Release OpenCL memory objects. */
    clReleaseMemObject(bufY);
    clReleaseMemObject(bufX);
    clReleaseMemObject(bufA);

    /* Finalize work with clblas. */
    clblasTeardown();

    /* Release OpenCL working objects. */
    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);

    return ret;
}
