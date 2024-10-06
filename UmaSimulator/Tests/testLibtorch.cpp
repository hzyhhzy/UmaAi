#include "tests.h"
#ifdef UMAAI_TESTLIBTORCH
//if you switch debug/release. you should also change PATH and restart visual studio
#include <torch/script.h>
#include <torch/torch.h>
#include <iostream>
#include <vector>
#include <chrono>
#include "../External/cnpy/cnpy.h"
const bool useCuda = true;
void main_testLibtorch() {

  // 加载模型
  torch::jit::script::Module model;
  try {
    model = torch::jit::load("../training/example/model_traced.pt");
  }
  catch (const c10::Error& e) {
    std::cerr << "模型加载失败。" << e.what() << std::endl;
    return;
  }
  if (useCuda) {
    // 将模型移动到GPU
    model.to(torch::kCUDA);
  }
  else
  {
    model.to(torch::kCPU);
  }

  // 准备输入数据
  std::string filename = "../training/example/256.npz";
  cnpy::npz_t my_npz = cnpy::npz_load(filename);
  cnpy::NpyArray arr = my_npz["x"];

  // 将数据转换为适当的格式并提取前N列
  float* loaded_data = arr.data<float>();
  std::vector<float> data;
  int num_rows = arr.shape[0];
  int num_cols = arr.shape[1];

  for (size_t i = 0; i < 8; ++i) { // 只取前N行
    for (size_t j = 0; j < num_cols; ++j) {
      data.push_back(loaded_data[i * num_cols + j]);
    }
  }
  torch::Tensor input = torch::from_blob(data.data(), { 8, num_cols });
  if (useCuda) {
    input = input.to(at::kCUDA);
  }
  // 运行模型
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back(input);
  at::Tensor output = model.forward(inputs).toTensor().to(at::kCPU);

  // 转换输出为浮点数数组
  float* output_data = output.data_ptr<float>();

  // 输出前几个结果以进行验证
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 5; ++j) {
      std::cout << output_data[i * 21 + j] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << "Benchmark:" << std::endl;
  std::vector<int> toTestBatchsize = { 1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768 };
  for (int bsi = 0; bsi < toTestBatchsize.size(); bsi++)
  {
    int batchSize = toTestBatchsize[bsi];
    int epoches = 100;
    auto start = std::chrono::high_resolution_clock::now();
    for (int epoch = 0; epoch < epoches; epoch++)
    {

      torch::Tensor input = torch::randn({ batchSize, 2341 });
      if (useCuda) {
        input = input.to(at::kCUDA);
      }

      // 运行模型
      std::vector<torch::jit::IValue> inputs;
      inputs.push_back(input);
      at::Tensor output = model.forward(inputs).toTensor().to(at::kCPU);

      // 转换输出为浮点数数组
      float* output_data = output.data_ptr<float>();

    }
    auto stop = std::chrono::high_resolution_clock::now();

    // 计算持续时间
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    double duration_s = 0.000001 * duration.count();
    // 输出持续时间
    std::cout << "Time: " << duration_s << " s, batchsize=" << batchSize << " batchnum=" << epoches << " speed=" << batchSize * epoches / duration_s << std::endl;
  }
  return;
}
#else
void main_testLibtorch() {
}
#endif