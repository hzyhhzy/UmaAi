#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <cassert>
#include <thread>
#include <atomic>
#include <mutex>
#include <cmath>
#include "../Game/Game.h"
#include "../NeuralNet/Evaluator.h"
#include "../Search/Search.h"
#include "../External/termcolor.hpp"

#include "../GameDatabase/GameDatabase.h"
#include "../GameDatabase/GameConfig.h"
#include "../Tests/TestConfig.h"

#include "../NeuralNet/CudaBackendKernel.h"
#include "../External/cnpy/cnpy.h"

using namespace std;

#if USE_BACKEND == BACKEND_CUDA
// 主机端打印函数
void printMatrix(const float* matrix, int m, int n) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      printf("%0.2f ", matrix[j * m + i]);
    }
    printf("\n");
  }
  printf("\n");
}
void main_testCuda()
{
  const int bs = 32;
  const string modelpath = "./db/model.txt";
  Model model(modelpath, bs);

  // 准备输入数据
  std::string filename = "./db/example_data.npz";
  cnpy::npz_t my_npz = cnpy::npz_load(filename);
  cnpy::NpyArray arr = my_npz["x"];

  // 将数据转换为适当的格式并提取前N列
  float* loaded_data = arr.data<float>();
  std::vector<float> data,output;
  output.resize(1000000);
  int num_rows = arr.shape[0];
  int num_cols = arr.shape[1];

  for (size_t i = 0; i < bs; ++i) { // 只取前2行
    for (size_t j = 0; j < num_cols; ++j) {
      data.push_back(loaded_data[i * num_cols + j]);
    }
  }

  Evaluator evaBuf(&model, bs);//只用于提供一些缓存区
  model.evaluate(&evaBuf, data.data(), output.data(), bs);
  for (int t = 0; t < 32; t++)
  {
    for (int i = 0; i < 5; i++)
      cout << output[NNOUTPUT_CHANNELS_V1 * t + i] << " ";
    cout << endl;
  }
}
#else

void main_testCuda()
{
}
#endif