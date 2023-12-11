#include "../config.h"
#if USE_BACKEND == BACKEND_CUDA
#include <cassert>
#include <fstream>
#include <iostream>
#include "Model.h"
#include "../NeuralNet/CudaBackendKernel.h"

using namespace std;


#pragma comment(lib, CUDA_LIBROOT "cublas.lib")
#pragma comment(lib, CUDA_LIBROOT "cuda.lib")
#pragma comment(lib, CUDA_LIBROOT "cudart_static.lib")
#ifdef _DEBUG
#pragma comment(lib, "C:/Users/hzy/source/repos/UmaSimulator/x64/Debug/CudaBackendKernel_debug.lib")
//#pragma comment(lib, "./NeuralNet/CudaBackendKernel_debug.lib")
#else
#pragma comment(lib, "C:/Users/hzy/source/repos/UmaSimulator/x64/Release/CudaBackendKernel.lib")
//#pragma comment(lib, "./NeuralNet/CudaBackendKernel.lib")
#endif

//copy from KataGo
static const char* cublasGetErrorString(const cublasStatus_t status)
{
  switch (status)
  {
  case CUBLAS_STATUS_SUCCESS: return "CUBLAS_STATUS_SUCCESS";

  case CUBLAS_STATUS_ALLOC_FAILED: return "CUBLAS_STATUS_ALLOC_FAILED";
  case CUBLAS_STATUS_ARCH_MISMATCH: return "CUBLAS_STATUS_ARCH_MISMATCH";
  case CUBLAS_STATUS_EXECUTION_FAILED: return "CUBLAS_STATUS_EXECUTION_FAILED";
  case CUBLAS_STATUS_INTERNAL_ERROR: return "CUBLAS_STATUS_INTERNAL_ERROR";
  case CUBLAS_STATUS_INVALID_VALUE: return "CUBLAS_STATUS_INVALID_VALUE";
  case CUBLAS_STATUS_LICENSE_ERROR: return "CUBLAS_STATUS_LICENSE_ERROR";
  case CUBLAS_STATUS_MAPPING_ERROR: return "CUBLAS_STATUS_MAPPING_ERROR";
  case CUBLAS_STATUS_NOT_INITIALIZED: return "CUBLAS_STATUS_NOT_INITIALIZED";
  case CUBLAS_STATUS_NOT_SUPPORTED: return "CUBLAS_STATUS_NOT_SUPPORTED";
  default:
    return "UNKNOWN CUBLAS ERROR";
  }
}

static void checkCublasError(const cublasStatus_t status, const char* opName, const char* file, const char* func, int line) {
  (void)checkCublasError;
  if (status != CUBLAS_STATUS_SUCCESS)
  {
    cout<< std::string("CUBLAS Error, for ") + opName + " file " + file + ", func " + func + ", line " + to_string(line) + ", error " + cublasGetErrorString(status) << endl;
    throw std::string("CUBLAS Error, for ") + opName + " file " + file + ", func " + func + ", line " + to_string(line) + ", error " + cublasGetErrorString(status);
  }
}
#define CUBLAS_ERR(opName,x) { checkCublasError((x),opName,__FILE__,#x,__LINE__); }

static void checkCudaError(const cudaError_t status, const char* opName, const char* file, const char* func, int line) {
  if (status != cudaSuccess)
  {
    cout << std::string("CUDA Error, for ") + opName + " file " + file + ", func " + func + ", line " + to_string(line) + ", error " + cudaGetErrorString(status)<<endl;
    throw std::string("CUDA Error, for ") + opName + " file " + file + ", func " + func + ", line " + to_string(line) + ", error " + cudaGetErrorString(status);
  }
}
#define CUDA_ERR(opName,x) { checkCudaError((x),opName,__FILE__,#x,__LINE__); }


static void mallocOnDevice(const string& name, int numWeights, float*& deviceBuf) {
   size_t floatBytes = numWeights * sizeof(float);
   CUDA_ERR(name.c_str(), cudaMalloc(&deviceBuf, floatBytes));
}

static void mallocAndCopyToDevice(const string& name, const float* weights, int numWeights, float*& deviceBuf) {
  size_t floatBytes = numWeights * sizeof(float);
  CUDA_ERR(name.c_str(), cudaMalloc(&deviceBuf, floatBytes));
  CUDA_ERR(name.c_str(), cudaMemcpy(deviceBuf, weights, floatBytes, cudaMemcpyHostToDevice));
}

static void mallocAndCopyToDevice(const string& name, const vector<float>& weights, float*& deviceBuf) {
  size_t floatBytes = weights.size() * sizeof(float);
  CUDA_ERR(name.c_str(), cudaMalloc(&deviceBuf, floatBytes));
  CUDA_ERR(name.c_str(), cudaMemcpy(deviceBuf, weights.data(), floatBytes, cudaMemcpyHostToDevice));
}

void ModelCudaBuf::init(const ModelWeight& weight, int batchSize)
{
  assert(batchSize <= MAX_BATCHSIZE_CUDA);
  CUBLAS_ERR("CudaHandles", cublasCreate(&cublas));

  //复制权重到GPU
  mallocAndCopyToDevice("inputheadGlobal1", weight.inputheadGlobal1, inputheadGlobal1);
  mallocAndCopyToDevice("inputheadGlobal2", weight.inputheadGlobal2, inputheadGlobal2);
  mallocAndCopyToDevice("inputheadCard", weight.inputheadCard, inputheadCard);
  mallocAndCopyToDevice("inputheadPerson", weight.inputheadPerson, inputheadPerson);

  for(int i=0;i<ModelWeight::encoderLayer;i++)
  {
    mallocAndCopyToDevice("encoder_Q", weight.encoder_Q[i], encoder_Q[i]);
    mallocAndCopyToDevice("encoder_V", weight.encoder_V[i], encoder_V[i]);
    mallocAndCopyToDevice("encoder_global", weight.encoder_global[i], encoder_global[i]);
  }

  mallocAndCopyToDevice("linBeforeMLP1", weight.linBeforeMLP1, linBeforeMLP1);
  mallocAndCopyToDevice("linBeforeMLP2", weight.linBeforeMLP2, linBeforeMLP2);
  for (int i = 0; i < ModelWeight::mlpBlock; i++)
  {
    mallocAndCopyToDevice("mlp_lin1", weight.mlp_lin[i][0], mlp_lin[i][0]);
    mallocAndCopyToDevice("mlp_lin2", weight.mlp_lin[i][1], mlp_lin[i][1]);
  }
  mallocAndCopyToDevice("outputhead_w", weight.outputhead_w, outputhead_w);
  mallocAndCopyToDevice("outputhead_b", weight.outputhead_b, outputhead_b);

  mallocOnDevice("input", batchSize * NNINPUT_CHANNELS_V1, input);
  mallocOnDevice("inputGlobal", batchSize * NNINPUT_CHANNELS_V1, inputGlobal);
  mallocOnDevice("inputCard", batchSize * NNINPUT_CHANNELS_V1, inputCard);
  mallocOnDevice("inputPerson", batchSize * NNINPUT_CHANNELS_V1, inputPerson);
  mallocOnDevice("gf", batchSize * ModelWeight::globalCh, gf);
  mallocOnDevice("encoderInput_gf", batchSize * ModelWeight::encoderCh, encoderInput_gf);
  mallocOnDevice("encoderInput_cardf", batchSize * NN_Game_Card_Num * ModelWeight::encoderCh, encoderInput_cardf);
  mallocOnDevice("encoderInput", batchSize * NN_Game_Person_Num * ModelWeight::encoderCh, encoderInput);
  mallocOnDevice("encoderQ", batchSize * NN_Game_Person_Num * ModelWeight::encoderCh, encoderQ);
  mallocOnDevice("encoderV", batchSize * NN_Game_Person_Num * ModelWeight::encoderCh, encoderV);
  mallocOnDevice("encoderAtt", batchSize * NN_Game_Person_Num * NN_Game_Person_Num, encoderAtt);
  mallocOnDevice("encoderGf", batchSize * ModelWeight::encoderCh, encoderGf);
  mallocOnDevice("encoderOutput", batchSize * NN_Game_Person_Num * ModelWeight::encoderCh, encoderOutput);
  mallocOnDevice("encoderSum", batchSize * ModelWeight::encoderCh, encoderSum);
  mallocOnDevice("mlpInput", batchSize * ModelWeight::mlpCh, mlpInput);
  mallocOnDevice("mlpMid", batchSize * ModelWeight::mlpCh, mlpMid);
  mallocOnDevice("mlpMid2", batchSize * ModelWeight::mlpCh, mlpMid2);
  mallocOnDevice("output", batchSize * NNOUTPUT_CHANNELS_V1, output);


  mallocOnDevice("outputhead_b_copied", batchSize * NNOUTPUT_CHANNELS_V1, outputhead_b_copied);
  for (int i = 0; i < batchSize; i++)
  {
    CUDA_ERR("", cudaMemcpy(outputhead_b_copied + i * NNOUTPUT_CHANNELS_V1, outputhead_b, sizeof(float) * NNOUTPUT_CHANNELS_V1, cudaMemcpyDeviceToDevice));//batchsize*NNInputC矩阵
  }

}

ModelCudaBuf::~ModelCudaBuf()
{
  cublasDestroy(cublas);
}























Model::Model(std::string path, int batchsize) :batchSize(batchsize)
{
  if (path == "")
    return;
  modelWeight.load(path);
  cb.init(modelWeight, batchSize);
}

static void loadWeight(string name, ifstream& fs, vector<float>& arr)
{
  string name1;
  fs >> name1;
  if (name1 != name)
  {
    cout << "wrong model weight name, expected \"" << name << "\", got \"" << name1 << "\"" << endl;
  }
  int num;
  fs >> num;
  assert(num > 0 && num < 1000000000);
  arr.resize(num);
  for (int i = 0; i < num; i++)
    fs >> arr[i];
}

void ModelWeight::load(std::string path)
{
  ifstream fs(path);
  assert(fs.good());
  string modelType;
  fs >> modelType;
  assert(modelType == "ems");
  int p1, p2, p3, p4, p5;
  fs >> p1 >> p2 >> p3 >> p4 >> p5;
  assert(p1 == encoderLayer);
  assert(p2 == encoderCh);
  assert(p3 == mlpBlock);
  assert(p4 == globalCh);
  assert(p5 == mlpCh);

  loadWeight("inputheadGlobal1", fs, inputheadGlobal1);
  loadWeight("inputheadGlobal2", fs, inputheadGlobal2);
  loadWeight("inputheadCard", fs, inputheadCard);
  loadWeight("inputheadPerson", fs, inputheadPerson);
  for (int i = 0; i < encoderLayer; i++)
  {
    loadWeight("encoder_" + to_string(i) + ".lin_Q", fs, encoder_Q[i]);
    loadWeight("encoder_" + to_string(i) + ".lin_V", fs, encoder_V[i]);
    loadWeight("encoder_" + to_string(i) + ".lin_global", fs, encoder_global[i]);
  }
  loadWeight("linBeforeMLP1", fs, linBeforeMLP1);
  loadWeight("linBeforeMLP2", fs, linBeforeMLP2);
  for (int i = 0; i < mlpBlock; i++)
  {
    loadWeight("mlp_" + to_string(i) + ".lin1", fs, mlp_lin[i][0]);
    loadWeight("mlp_" + to_string(i) + ".lin2", fs, mlp_lin[i][1]);
  }
  loadWeight("outputhead_w", fs, outputhead_w);
  loadWeight("outputhead_b", fs, outputhead_b);
}

void Model::evaluate(float* inputBuf, float* outputBuf, int gameNum)
{
  //内存中是batchsize*NNInputC矩阵，行优先
  CUDA_ERR("", cudaMemcpy(cb.input, inputBuf, sizeof(float) * batchSize * NNINPUT_CHANNELS_V1, cudaMemcpyHostToDevice)); //batchsize*NNInputC矩阵

  const int sliceIdx1 = NNINPUT_CHANNELS_GAMEGLOBAL_V1 + NNINPUT_CHANNELS_SEARCHPARAM_V1;
  const int sliceIdx2 = sliceIdx1 + NN_Game_Card_Num * NNINPUT_CHANNELS_CARD_V1;
  const int sliceIdx3 = sliceIdx2 + NN_Game_Person_Num * NNINPUT_CHANNELS_PERSON_V1;
  static_assert(sliceIdx3 == NNINPUT_CHANNELS_V1);

  CUDA_ERR("", cropMatrixRowsCUDA(cb.inputGlobal, cb.input, batchSize, NNINPUT_CHANNELS_V1, 0, sliceIdx1));//[batch,c1]
  CUDA_ERR("", cropMatrixRowsCUDA(cb.inputCard, cb.input, batchSize, NNINPUT_CHANNELS_V1, sliceIdx1, sliceIdx2));//[batch,7,c2]
  CUDA_ERR("", cropMatrixRowsCUDA(cb.inputPerson, cb.input, batchSize, NNINPUT_CHANNELS_V1, sliceIdx2, sliceIdx3));//[batch,20,c3]
  
  const float one = 1.0;
  const float zero = 0.0;

  CUBLAS_ERR("", cublasSgemm(cb.cublas, CUBLAS_OP_T, CUBLAS_OP_N, ModelWeight::globalCh, batchSize, sliceIdx1, &one, cb.inputheadGlobal1, sliceIdx1, cb.inputGlobal, sliceIdx1, &zero, cb.gf, ModelWeight::globalCh));
  CUBLAS_ERR("", cublasSgemm(cb.cublas, CUBLAS_OP_T, CUBLAS_OP_N, ModelWeight::encoderCh, batchSize * NN_Game_Card_Num, NNINPUT_CHANNELS_CARD_V1, &one, cb.inputheadCard, NNINPUT_CHANNELS_CARD_V1, cb.inputCard, NNINPUT_CHANNELS_CARD_V1, &zero, cb.encoderInput_cardf, ModelWeight::encoderCh));
  CUBLAS_ERR("", cublasSgemm(cb.cublas, CUBLAS_OP_T, CUBLAS_OP_N, ModelWeight::encoderCh, batchSize * NN_Game_Person_Num, NNINPUT_CHANNELS_PERSON_V1, &one, cb.inputheadPerson, NNINPUT_CHANNELS_PERSON_V1, cb.inputPerson, NNINPUT_CHANNELS_PERSON_V1, &zero, cb.encoderInput, ModelWeight::encoderCh));
  CUDA_ERR("", reluInPlace(cb.gf, ModelWeight::globalCh * batchSize));
  CUBLAS_ERR("", cublasSgemm(cb.cublas, CUBLAS_OP_T, CUBLAS_OP_N, ModelWeight::encoderCh, batchSize, ModelWeight::globalCh, &one, cb.inputheadGlobal2, ModelWeight::globalCh, cb.gf, ModelWeight::globalCh, &zero, cb.encoderInput_gf, ModelWeight::encoderCh));

  CUDA_ERR("", addThreeFeatures(cb.encoderInput_gf, cb.encoderInput_cardf, cb.encoderInput, batchSize, NN_Game_Person_Num, NN_Game_Card_Num, ModelWeight::encoderCh));
  CUDA_ERR("", reluInPlace(cb.encoderInput, ModelWeight::encoderCh * NN_Game_Person_Num * batchSize));

  //接下来是encoder部分
  for (int layer = 0; layer < ModelWeight::encoderLayer; layer++)
  {
    CUBLAS_ERR("", cublasSgemm(cb.cublas, CUBLAS_OP_T, CUBLAS_OP_N,
      ModelWeight::encoderCh, batchSize * NN_Game_Person_Num, ModelWeight::encoderCh, &one,
      cb.encoder_Q[layer], ModelWeight::encoderCh,
      cb.encoderInput, ModelWeight::encoderCh, &zero,
      cb.encoderQ, ModelWeight::encoderCh));

    CUBLAS_ERR("", cublasSgemm(cb.cublas, CUBLAS_OP_T, CUBLAS_OP_N,
      ModelWeight::encoderCh, batchSize * NN_Game_Person_Num, ModelWeight::encoderCh, &one,
      cb.encoder_V[layer], ModelWeight::encoderCh,
      cb.encoderInput, ModelWeight::encoderCh, &zero,
      cb.encoderV, ModelWeight::encoderCh));

    //Q*x计算attention，只能每个矩阵分开算
    CUBLAS_ERR("", cublasSgemmStridedBatched(cb.cublas, CUBLAS_OP_T, CUBLAS_OP_N,
      NN_Game_Person_Num, NN_Game_Person_Num, ModelWeight::encoderCh, &one,
      cb.encoderQ, ModelWeight::encoderCh, NN_Game_Person_Num * ModelWeight::encoderCh,
      cb.encoderInput, ModelWeight::encoderCh, NN_Game_Person_Num * ModelWeight::encoderCh,
      &zero,
      cb.encoderAtt, NN_Game_Person_Num, NN_Game_Person_Num * NN_Game_Person_Num, 
      batchSize));

    CUDA_ERR("", reluInPlace(cb.encoderAtt, NN_Game_Person_Num * NN_Game_Person_Num * batchSize));

    //attention*v，只能每个矩阵分开算
    CUBLAS_ERR("", cublasSgemmStridedBatched(cb.cublas, CUBLAS_OP_N, CUBLAS_OP_N,
      ModelWeight::encoderCh, NN_Game_Person_Num, NN_Game_Person_Num, &one,
      cb.encoderV, ModelWeight::encoderCh, NN_Game_Person_Num * ModelWeight::encoderCh,
      cb.encoderAtt, NN_Game_Person_Num, NN_Game_Person_Num * NN_Game_Person_Num,
      &zero,
      cb.encoderOutput, ModelWeight::encoderCh, NN_Game_Person_Num * ModelWeight::encoderCh,
      batchSize));

    CUBLAS_ERR("", cublasSgemm(cb.cublas, CUBLAS_OP_T, CUBLAS_OP_N, ModelWeight::encoderCh, batchSize, ModelWeight::globalCh, &one, cb.encoder_global[layer], ModelWeight::globalCh, cb.gf, ModelWeight::globalCh, &zero, cb.encoderGf, ModelWeight::encoderCh));

    CUDA_ERR("", broadcastDim1Add(cb.encoderOutput, cb.encoderGf, batchSize, NN_Game_Person_Num, ModelWeight::encoderCh));
    CUDA_ERR("", reluInPlace(cb.encoderOutput, batchSize * NN_Game_Person_Num * ModelWeight::encoderCh));
    CUDA_ERR("", addInPlace(cb.encoderInput, cb.encoderOutput, batchSize * NN_Game_Person_Num * ModelWeight::encoderCh));//ResNet结构，放回input
  }

  //h=h.mean(dim=1)
  CUDA_ERR("", sumDim1(cb.encoderSum, cb.encoderInput, batchSize, NN_Game_Person_Num, ModelWeight::encoderCh));

  //linBeforeMLP2(h)
  CUBLAS_ERR("", cublasSgemm(cb.cublas, CUBLAS_OP_T, CUBLAS_OP_N, ModelWeight::mlpCh, batchSize, ModelWeight::encoderCh, &one, cb.linBeforeMLP2, ModelWeight::encoderCh, cb.encoderSum, ModelWeight::encoderCh, &zero, cb.mlpInput, ModelWeight::mlpCh));
  
  //+=linBeforeMLP1(gf)
  CUBLAS_ERR("", cublasSgemm(cb.cublas, CUBLAS_OP_T, CUBLAS_OP_N, ModelWeight::mlpCh, batchSize, ModelWeight::globalCh, &one, cb.linBeforeMLP1, ModelWeight::globalCh, cb.gf, ModelWeight::globalCh, &one, cb.mlpInput, ModelWeight::mlpCh));


  CUDA_ERR("", reluInPlace(cb.mlpInput, batchSize * ModelWeight::mlpCh));

  //mlp
  for (int layer = 0; layer < ModelWeight::mlpBlock; layer++)
  {
    CUBLAS_ERR("", cublasSgemm(cb.cublas, CUBLAS_OP_T, CUBLAS_OP_N, ModelWeight::mlpCh, batchSize, ModelWeight::mlpCh, &one, cb.mlp_lin[layer][0], ModelWeight::mlpCh, cb.mlpInput, ModelWeight::mlpCh, &zero, cb.mlpMid, ModelWeight::mlpCh));
    CUDA_ERR("", reluInPlace(cb.mlpMid, batchSize * ModelWeight::mlpCh));
    CUBLAS_ERR("", cublasSgemm(cb.cublas, CUBLAS_OP_T, CUBLAS_OP_N, ModelWeight::mlpCh, batchSize, ModelWeight::mlpCh, &one, cb.mlp_lin[layer][1], ModelWeight::mlpCh, cb.mlpMid, ModelWeight::mlpCh, &zero, cb.mlpMid2, ModelWeight::mlpCh));
    CUDA_ERR("", reluInPlace(cb.mlpMid2, batchSize * ModelWeight::mlpCh));

    CUDA_ERR("", addInPlace(cb.mlpInput, cb.mlpMid2, batchSize * ModelWeight::mlpCh));//ResNet结构，放回input
  }


  CUBLAS_ERR("", cublasSgemm(cb.cublas, CUBLAS_OP_T, CUBLAS_OP_N, NNOUTPUT_CHANNELS_V1, batchSize, ModelWeight::mlpCh, &one, cb.outputhead_w, ModelWeight::mlpCh, cb.mlpInput, ModelWeight::mlpCh, &zero, cb.output, NNOUTPUT_CHANNELS_V1));
  CUDA_ERR("", addInPlace(cb.output, cb.outputhead_b_copied, batchSize * NNOUTPUT_CHANNELS_V1));//ResNet结构，放回input

  
  CUDA_ERR("", cudaMemcpy(outputBuf, cb.output, sizeof(float) * batchSize * NNOUTPUT_CHANNELS_V1, cudaMemcpyDeviceToHost)); //batchsize*NNInputC矩阵

  /*
  
  CUDA_ERR("", cudaMemcpy(f.data(), cb.encoderInput, sizeof(float) * batchSize * ModelWeight::encoderCh * NN_Game_Person_Num, cudaMemcpyDeviceToHost)); //batchsize*NNInputC矩阵

  for (int i = 0; i < batchSize; i++)
  {
    for (int p = 0; p < NN_Game_Person_Num; p++)
    {
      cout << f[i * ModelWeight::encoderCh * NN_Game_Person_Num + p * ModelWeight::encoderCh + 30] << " ";
    }
    cout << endl;
  }
*/
}

void Model::printBackendInfo()
{
  cout << "AI版本：神经网络，后端：CUDA";
}
#endif