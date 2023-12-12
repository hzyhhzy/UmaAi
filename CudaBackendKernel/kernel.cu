#include "cuda_runtime.h"
#include <stdio.h>
#include <cstdint>

// 非原位ReLU核函数
__global__ void reluOutOfPlaceKernel(const float* input, float* output, int n) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i < n) {
    output[i] = max(0.0f, input[i]);
  }
}

// 调用非原位ReLU核函数的接口
cudaError_t reluOutOfPlace(const float* input, float* output, int n) {
  int blockSize = 512;
  int numBlocks = (n + blockSize - 1) / blockSize;
  reluOutOfPlaceKernel << <numBlocks, blockSize >> > (input, output, n);
  cudaDeviceSynchronize();
  return cudaGetLastError();
}

// 原位ReLU核函数
__global__ void reluInPlaceKernel(float* data, int n) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i < n) {
    data[i] = max(0.0f, data[i]);
  }
}

// 调用原位ReLU核函数的接口
cudaError_t reluInPlace(float* data, int n) {
  int blockSize = 512;
  int numBlocks = (n + blockSize - 1) / blockSize;
  reluInPlaceKernel << <numBlocks, blockSize >> > (data, n);
  cudaDeviceSynchronize();
  return cudaGetLastError();
}
/*
//裁剪矩阵，batchSize列c行，列优先
__global__ void cropMatrixRowsKernel(float* d_out, const float* d_in, int c, int batchSize, int start, int end) {
  int col = blockIdx.x * blockDim.x + threadIdx.x;
  int row = blockIdx.y * blockDim.y + threadIdx.y + start;  // 起始于第a行

  if (col < start && row < end) {
    d_out[col * (end - start) + (row - start)] = d_in[col * c + row];
  }
}
void cropMatrixRowsCUDA(float* d_out, const float* d_in, int c, int batchSize, int start, int end) {
  // 检查a和b的合法性，确保它们在矩阵范围内
  if (start < 0 || end >= c || start > end) {
    return; // 可能需要一些错误处理
  }

  int newRowSize = end - start;

  dim3 blockSize(32, 16); // 根据实际情况调整
  dim3 gridSize((batchSize + blockSize.x - 1) / blockSize.x, (newRowSize + blockSize.y - 1) / blockSize.y);

  cropMatrixRowsKernel <<<gridSize, blockSize>>> (d_out, d_in, c, batchSize, start, end);

  // 调用者应检查CUDA调用后的错误
}
*/
__global__ void cropMatrixRowsKernel(float* d_out, const float* d_in, int batchsize, int c, int start, int end) {
  int batchIdx = blockIdx.x * blockDim.x + threadIdx.x;
  int fIdx = blockIdx.y * blockDim.y + threadIdx.y + start;  // 从第start行开始

  if (fIdx >= start && fIdx < end) {
    // 计算源矩阵和目标矩阵中的索引
    int idx_in = fIdx + batchIdx * c;               // 源矩阵列优先索引
    int idx_out = (fIdx - start) + batchIdx * (end - start);  // 输出矩阵列优先索引
    d_out[idx_out] = d_in[idx_in];
  }
}
cudaError_t cropMatrixRowsCUDA(float* d_out, const float* d_in, int batchsize, int c, int start, int end) {
  // 检查start和end的合法性
  if (start < 0 || end > c || start >= end) {
    return cudaErrorInvalidValue; // 错误处理
  }

  int cropSize = end - start;  // 裁剪区域的行数

  // 定义每个块的线程维度
  dim3 blockSize(32, 16);

  // 计算所需的块数来覆盖矩阵
  dim3 gridSize((batchsize + blockSize.x - 1) / blockSize.x, (cropSize + blockSize.y - 1) / blockSize.y);

  // 启动内核
  cropMatrixRowsKernel << <gridSize, blockSize >> > (d_out, d_in, batchsize, c, start, end);

  // 等待 GPU 完成
  cudaDeviceSynchronize();

  return cudaGetLastError();
}

__global__ void addThreeFeaturesKernel(float* A, float* B, float* C, int batchsize, int N_Person, int N_Card, int encoderC) {
  int encoderIdx = blockIdx.x * blockDim.x + threadIdx.x;
  int personIdx = blockIdx.y * blockDim.y + threadIdx.y;
  int batchIdx = blockIdx.z * blockDim.z + threadIdx.z;

  if (encoderIdx < encoderC && personIdx < N_Person && batchIdx < batchsize) {
    // 对于列优先存储，encoderC变化最快
    int idxC = encoderIdx + personIdx * encoderC + batchIdx * encoderC * N_Person;
    int idxA = encoderIdx + batchIdx * encoderC;  // A的索引

    // 对C[:,:,:] += A.unsqueeze(1)操作
    C[idxC] += A[idxA];

    // 对C[:,0:N_Card,:] += B操作
    if (personIdx < N_Card) {
      int idxB = encoderIdx + personIdx * encoderC + batchIdx * encoderC * N_Card;
      C[idxC] += B[idxB];
    }
  }
}
cudaError_t addThreeFeatures(float* A, float* B, float* C, int batchsize, int N_Person, int N_Card, int encoderC) {
  // 定义block和grid的大小
  dim3 blockSize(16, 1, 32);
  dim3 gridSize((encoderC + blockSize.x - 1) / blockSize.x,
    (N_Person + blockSize.y - 1) / blockSize.y,
    (batchsize + blockSize.z - 1) / blockSize.z);

  // 调用核函数
  addThreeFeaturesKernel << <gridSize, blockSize >> > (A, B, C, batchsize, N_Person, N_Card, encoderC);

  // 检查CUDA错误和同步
  cudaDeviceSynchronize();
  return cudaGetLastError();
}

__global__ void broadcastDim1AddKernel(float* target, float* x, int batchsize, int N_Person, int encoderC) {
  int encoderIdx = blockIdx.x * blockDim.x + threadIdx.x;
  int personIdx = blockIdx.y * blockDim.y + threadIdx.y;
  int batchIdx = blockIdx.z * blockDim.z + threadIdx.z;

  if (encoderIdx < encoderC && personIdx < N_Person && batchIdx < batchsize) {
    // 对于列优先存储，encoderC变化最快
    int idxC = encoderIdx + personIdx * encoderC + batchIdx * encoderC * N_Person;
    int idxA = encoderIdx + batchIdx * encoderC;  // A的索引

    // 对C[:,:,:] += A.unsqueeze(1)操作
    target[idxC] += x[idxA];

  }
}
cudaError_t broadcastDim1Add(float* target, float* x, int batchsize, int N_Person, int encoderC) {
  // 定义block和grid的大小
  dim3 blockSize(16, 1, 32);
  dim3 gridSize((encoderC + blockSize.x - 1) / blockSize.x,
    (N_Person + blockSize.y - 1) / blockSize.y,
    (batchsize + blockSize.z - 1) / blockSize.z);

  // 调用核函数
  broadcastDim1AddKernel << <gridSize, blockSize >> > (target, x, batchsize, N_Person, encoderC);

  // 检查CUDA错误和同步
  cudaDeviceSynchronize();
  return cudaGetLastError();
}

__global__ void addInPlaceKernel(float* A, const float* B, int numElements) {
  int i = blockDim.x * blockIdx.x + threadIdx.x;
  if (i < numElements) {
    A[i] += B[i];
  }
}

cudaError_t addInPlace(float* A, const float* B, int numElements) {
  int blockSize = 512;
  int numBlocks = (numElements + blockSize - 1) / blockSize;
  addInPlaceKernel << <numBlocks, blockSize >> > (A, B, numElements);

  // 检查CUDA错误和同步
  cudaDeviceSynchronize();
  return cudaGetLastError();
}

__global__ void sumDim1Kernel(float* output, const float* input, int batchSize, int dim1Size, int channel) {
  int batch = blockIdx.x * blockDim.x + threadIdx.x;
  int ch = blockIdx.y * blockDim.y + threadIdx.y;

  if (batch < batchSize && ch < channel) {
    float sum = 0.0f;
    for (int i = 0; i < dim1Size; ++i) {
      sum += input[(batch * dim1Size * channel) + (i * channel) + ch];
    }
    output[batch * channel + ch] = sum;
  }
}

cudaError_t sumDim1(float* output, float* input, int batchSize, int dim1Size, int channel) {
  dim3 blockDim(32, 16);
  dim3 gridDim((batchSize + blockDim.x - 1) / blockDim.x, (channel + blockDim.y - 1) / blockDim.y);

  sumDim1Kernel << <gridDim, blockDim >> > (output, input, batchSize, dim1Size, channel);

  // 检查是否有任何错误发生
  cudaDeviceSynchronize();
  return cudaGetLastError();
}

__global__ void sparseToDenseKernel(uint32_t* idx, float* value, float* output, int m) {
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < m) {
    output[idx[index]] = value[index];
  }
}

cudaError_t sparseToDense(uint32_t* idx, float* value, float* output, int m) {
  // 假设output已经分配并初始化为0

  // 设置线程块和网格大小
  int threadsPerBlock = 512;
  int blocksPerGrid = (m + threadsPerBlock - 1) / threadsPerBlock;

  // 调用核函数
  sparseToDenseKernel << <blocksPerGrid, threadsPerBlock >> > (idx, value, output, m);

  // 等待GPU完成
  cudaDeviceSynchronize();
  return cudaGetLastError();
}

const int NNINPUT_MAX_FLOAT = 128; //nninput里面最多有多少个非0非1的数
const int NNINPUT_MAX_ONES = 272; //nninput里面最多有多少个1
__global__ void decompressNNInputKernel(uint16_t* onesIdx, uint16_t* floatIdx, float* floatValue, float* output, int m, int nninputSize) {
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index < m) {
    for (int i = 0; i < NNINPUT_MAX_FLOAT; ++i) {
      int fIdx = NNINPUT_MAX_FLOAT * index + i;
      int cIdx = floatIdx[fIdx];
      if (cIdx < nninputSize)
        output[nninputSize * index + cIdx] = floatValue[fIdx];
    }
    for (int i = 0; i < NNINPUT_MAX_ONES; ++i) {
      int cIdx = onesIdx[NNINPUT_MAX_ONES * index + i];
      if (cIdx < nninputSize)
        output[nninputSize * index + cIdx] = 1.0;
    }
  }
}

cudaError_t decompressNNInput(uint16_t* onesIdx, uint16_t* floatIdx, float* floatValue, float* output, int m, int nninputSize) {
  // 假设output已经分配并初始化为0

  // 设置线程块和网格大小
  int threadsPerBlock = 512;
  int blocksPerGrid = (m + threadsPerBlock - 1) / threadsPerBlock;

  cudaMemset(output, 0, sizeof(float) * m * nninputSize);
  // 调用核函数
  decompressNNInputKernel << <blocksPerGrid, threadsPerBlock >> > (onesIdx, floatIdx, floatValue, output, m, nninputSize);

  // 等待GPU完成
  cudaDeviceSynchronize();
  return cudaGetLastError();
}
