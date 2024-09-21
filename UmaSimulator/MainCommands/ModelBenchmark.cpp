#include "../config.h"
#ifdef USE_BACKEND_LIBTORCH
#include <torch/script.h>
#include <torch/torch.h>
#endif
#include <iostream>
#include <vector>
#include <chrono>
#include "MainCommands.h"
#include "../NeuralNet/Model.h"
#include "../NeuralNet/Evaluator.h"
using namespace std;
void main_modelBenchmark() {


  const int threadNum = 1;
  const int batchSize = 2048;
  int64_t N = 3000000;

  int batchNumEveryThread = 1 + (N - 1) / (batchSize * threadNum);
  N = (batchSize * threadNum) * batchNumEveryThread;


  cout << "N=" << N << " thread=" << threadNum << " batchsize=" << batchSize << " batchNumEveryThread=" << batchNumEveryThread << endl;


#if USE_BACKEND == BACKEND_LIBTORCH
  std::string modelpath = "../training/example/model_traced.pt";
  // 加载模型
  torch::jit::script::Module model;
  try {
    model = torch::jit::load(modelpath);
  }
  catch (const c10::Error& e) {
    std::cerr << "模型加载失败。" << e.what() << std::endl;
    return;
  }
  if (LIBTORCH_USE_GPU) {
    // 将模型移动到GPU
    model.to(torch::kCUDA);
  }
  else
  {
    model.to(torch::kCPU);
  }

  std::cout << "Benchmark:" << std::endl;
  auto start = std::chrono::high_resolution_clock::now();

  auto modelthread = [&model](int batchNumEveryThread)
    {
      int nc = NNINPUT_CHANNELS_V1;
      vector<float> data(batchSize * nc);
      for (int b = 0; b < batchNumEveryThread; b++)
      {

        torch::Tensor input = torch::from_blob(data.data(), { batchSize, nc });
        if (LIBTORCH_USE_GPU) {
          input = input.to(at::kCUDA);
        }

        // 运行模型
        at::Tensor output = model.forward({ input }).toTensor().to(at::kCPU);

        // 转换输出为浮点数数组
        float* output_data = output.data_ptr<float>();

      }
    };

  std::vector<std::thread> threads;
  for (int i = 0; i < threadNum; i++)
    threads.emplace_back(modelthread, batchNumEveryThread);

  // 等待所有线程完成
  for (auto& th : threads) {
    th.join();
  }

  auto stop = std::chrono::high_resolution_clock::now();

  // 计算持续时间
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  double duration_s = 0.000001 * duration.count();
  // 输出持续时间
  std::cout << "Time: " << duration_s << " s, speed=" << N / duration_s << std::endl;
#endif

#if USE_BACKEND == BACKEND_CUDA
  static_assert(threadNum == 1);
  const string modelpath = "../training/example/model.txt";
  Model model(modelpath, batchSize);

  std::vector<float> data(batchSize * NNINPUT_CHANNELS_V1), output(batchSize * NNOUTPUT_CHANNELS_V1);

  std::cout << "Benchmark:" << std::endl;
  auto start = std::chrono::high_resolution_clock::now();

  Evaluator evaBuf(&model, batchSize);//只用于提供一些缓存区

  for (int b = 0; b < batchNumEveryThread; b++)
  {
    model.evaluate(&evaBuf, data.data(), output.data(), batchSize);
  }


  auto stop = std::chrono::high_resolution_clock::now();

  // 计算持续时间
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  double duration_s = 0.000001 * duration.count();
  // 输出持续时间
  std::cout << "Time: " << duration_s << " s, speed=" << N / duration_s << std::endl;
#endif
  cout << "按任意键继续" << endl;
  cin.get();
  return;
}