#include "../config.h"
#if USE_BACKEND == BACKEND_LIBTORCH
#include <cassert>
#include "Model.h"
#include "iostream"
using namespace std;

#ifdef _DEBUG 
#pragma comment(lib, TORCH_LIBROOT_DEBUG "torch.lib")
#pragma comment(lib, TORCH_LIBROOT_DEBUG "c10.lib")
#pragma comment(lib, TORCH_LIBROOT_DEBUG "torch_cuda.lib")
#pragma comment(lib, TORCH_LIBROOT_DEBUG "torch_cpu.lib")
#pragma comment(lib, TORCH_LIBROOT_DEBUG "c10_cuda.lib")
#else
#pragma comment(lib, TORCH_LIBROOT "torch.lib")
#pragma comment(lib, TORCH_LIBROOT "c10.lib")
#pragma comment(lib, TORCH_LIBROOT "torch_cuda.lib")
#pragma comment(lib, TORCH_LIBROOT "torch_cpu.lib")
#pragma comment(lib, TORCH_LIBROOT "c10_cuda.lib")
#endif

Model::Model(std::string path, int batchsize)
{
  if (path == "")
    return;
  model = torch::jit::load(path);
  
  if (LIBTORCH_USE_GPU) {
    // 将模型移动到GPU
    model.to(torch::kCUDA);
  }
  else
  {
    model.to(torch::kCPU);
  }
}

void Model::evaluate(float* inputBuf, float* outputBuf, int gameNum)
{
  torch::Tensor input = torch::from_blob(inputBuf, { gameNum, NNINPUT_CHANNELS_V1 });
  if (LIBTORCH_USE_GPU) {
    input = input.to(at::kCUDA);
  }
  // 运行模型
  at::Tensor output = model.forward({ input }).toTensor().to(at::kCPU);

  // 转换输出为浮点数数组
  float* output_data = output.data_ptr<float>();
  memcpy(outputBuf, output_data, gameNum * NNOUTPUT_CHANNELS_V1 * sizeof(float));
}

void Model::printBackendInfo()
{
    cout << "AI版本：神经网络，后端：Libtorch " << (LIBTORCH_USE_GPU ? "GPU版" : "CPU版");
}

#endif