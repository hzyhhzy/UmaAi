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

#include "../External/cnpy/cnpy.h"

#if USE_BACKEND == BACKEND_ONNX
#include <onnxruntime_cxx_api.h>
using namespace std;

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
void main_testOnnx()
{
  Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "test");
  Ort::Session session{env, L"./db/model.onnx", Ort::SessionOptions{nullptr} };
  auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

  // 准备输入数据
  std::string filename = "./db/example_data.npz";
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
  vector<int64_t> input_shape = { 8,num_cols };
  std::vector<const char*> input_node_names;
  const char inputName0[] = "input";
  input_node_names.push_back(inputName0);
  std::vector<const char*> output_node_names;
  const char outputName0[] = "output";
  output_node_names.push_back(outputName0);

  auto input_tensor = Ort::Value::CreateTensor<float>(memory_info, data.data(), data.size(), input_shape.data(), 2);
  auto output_tensors =
    session.Run(Ort::RunOptions{ nullptr }, input_node_names.data(), &input_tensor, 1, output_node_names.data(), 1);
  float* output = output_tensors.front().GetTensorMutableData<float>();
  for (int t = 0; t < 8; t++)
  {
    for (int i = 0; i < 5; i++)
      cout << output[NNOUTPUT_CHANNELS_V1 * t + i] << " ";
    cout << endl;
  }
}
#else

void main_testOnnx()
{
}
#endif