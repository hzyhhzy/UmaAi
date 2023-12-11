#pragma once

#include "../config.h"
#if USE_BACKEND == BACKEND_CUDA
#include <cublas_v2.h>
#include <cuda_runtime.h>
cudaError_t reluOutOfPlace(const float* input, float* output, int n);
cudaError_t reluInPlace(float* data, int n);
cudaError_t cropMatrixRowsCUDA(float* d_out, const float* d_in, int batchSize, int c, int start, int end);
cudaError_t addThreeFeatures(float* A, float* B, float* C, int batchsize, int N_Person, int N_Card, int encoderC);
cudaError_t broadcastDim1Add(float* target, float* x, int batchsize, int N_Person, int encoderC);
cudaError_t addInPlace(float* A, const float* B, int numElements);
cudaError_t sumDim1(float* output, float* input, int batchSize, int dim1Size, int channel);
#endif