#include "Model.h"
#include "iostream"
using namespace std;

Model::Model(std::string path, int batchsize)
{
#ifdef USE_BACKEND_LIBTORCH
  model = torch::jit::load(path);
  
  if (LIBTORCH_USE_GPU) {
    // 将模型移动到GPU
    model.to(torch::kCUDA);
  }
  else
  {
    model.to(torch::kCPU);
  }
#else
  assert(false);
#endif
}

void Model::evaluate(float* inputBuf, float* outputBuf, int gameNum)
{
#ifdef USE_BACKEND_LIBTORCH
  torch::Tensor input = torch::from_blob(inputBuf, { gameNum, NNINPUT_CHANNELS_V1 });
  if (LIBTORCH_USE_GPU) {
    input = input.to(at::kCUDA);
  }
  // 运行模型
  at::Tensor output = model.forward({ input }).toTensor().to(at::kCPU);

  // 转换输出为浮点数数组
  float* output_data = output.data_ptr<float>();
  memcpy(outputBuf, output_data, gameNum * NNOUTPUT_CHANNELS_V1 * sizeof(float));
#else
  assert(false);
#endif
}

// 打印torch版程序基本信息
void Model::detect(Model* md)
{
#ifdef USE_BACKEND_LIBTORCH
    cout << "AI版本：神经网络 + " << (LIBTORCH_USE_GPU ? "GPU" : "CPU");
    c10::Device device = torch::Device(torch::kCUDA, 0);
    cout << ", CUDA: " << (device.is_cuda() ? "On" : "Off")
        << (md ? ", 已载入模型" : ", 未载入模型") << endl;
#else
    cout << "AI版本：MonteCarlo" << endl;
#endif
}