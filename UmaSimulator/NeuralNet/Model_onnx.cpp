#include "../config.h"
#if USE_BACKEND == BACKEND_ONNX
#include <cassert>
#include <onnxruntime_cxx_api.h>
#include <dml_provider_factory.h>
#include <filesystem>
#pragma comment(lib,"./external/onnx/onnxruntime.lib")
#include "Model.h"
#include "iostream"
using namespace std;


Model::Model(std::string path, int batchsize):batchSize(batchsize)
{
  if (path == "")
    return;
  auto pathWstr = std::filesystem::path(path).wstring();
  Ort::SessionOptions sessionOptions;
  Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_DML(sessionOptions, 0));
  onnxSession = Ort::Session(onnxEnv, pathWstr.data(), sessionOptions);
  onnxMemoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
}

void Model::evaluate(Evaluator* eva, float* inputBuf, float* outputBuf, int gameNum)
{
  const char* input_names[] = { "input" };
  const char* output_names[] = { "output" };
  std::lock_guard<std::mutex> lock(mtx);

  vector<int64_t> input_shape = { batchSize,NNINPUT_CHANNELS_V1 };
  onnxInput = Ort::Value::CreateTensor<float>(onnxMemoryInfo, inputBuf, batchSize * NNINPUT_CHANNELS_V1, input_shape.data(), 2);
  auto onnxOutput =
    onnxSession.Run(Ort::RunOptions{ nullptr }, input_names, &onnxInput, 1, output_names, 1);

  float* output = onnxOutput.front().GetTensorMutableData<float>();
  std::memcpy(outputBuf, output, NNOUTPUT_CHANNELS_V1 * batchSize * sizeof(float));
}
void Model::printBackendInfo()
{
    cout << "AI版本：神经网络，后端：Onnx-DirectML " << endl;
}

#endif