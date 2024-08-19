#pragma once
#include "NNInput.h"
#include "../config.h"
#include "../Game/Action.h"
#include <string>
#include <vector>
#include <mutex>

#define NO_NN_GAME_PERSON //uaf剧本没有单独的person，如果是类似凯旋门的剧本则注释掉这行，仅限cuda
//#define COMPRESS_NNINPUT //压缩输入，节省pcie带宽，仅限cuda

const int NN_Game_Card_Num = 6;//7张卡（每次有且仅有一个位置为空）
const int NN_Game_Person_Num = 0;//20个人头（每次有且仅有二个位置为空）

#ifdef NO_NN_GAME_PERSON
static_assert(NN_Game_Person_Num == 0);
const int NN_TF_NUM = NN_Game_Card_Num;//transformer有几个项
#else
static_assert(NN_Game_Person_Num != 0);
const int NN_TF_NUM = NN_Game_Person_Num;//transformer有几个项
#endif

#if USE_BACKEND == BACKEND_LIBTORCH
#include <torch/torch.h>
#include <torch/script.h>
#endif

#if USE_BACKEND == BACKEND_CUDA
#include <cublas_v2.h>
#include <cuda_runtime.h>
#endif

#if USE_BACKEND == BACKEND_ONNX
#include <onnxruntime_cxx_api.h>
#endif

//用-1e7表示不合法操作，在神经网络训练时直接忽略掉这些值
struct ModelOutputPolicyV1
{
  float actionPolicy[Action::MAX_ACTION_TYPE];//直接按顺序列举

};
static_assert(sizeof(ModelOutputPolicyV1) == sizeof(float) * NNOUTPUT_CHANNELS_POLICY_V1, "NNOUTPUT_CHANNELS_POLICY_V1错误");


struct ModelOutputValueV1
{
  float scoreMean;//score的平均值
  float scoreStdev;//score的标准差
  //float winRate;//score>=target的概率
  float value;//考虑激进度之后的打分
  //float extract(int i);
  static const ModelOutputValueV1 illegalValue;
};
static_assert(sizeof(ModelOutputValueV1) == sizeof(float) * NNOUTPUT_CHANNELS_VALUE_V1, "NNOUTPUT_CHANNELS_VALUE_V1错误");

struct ModelOutputV1
{
  ModelOutputValueV1 value;
  ModelOutputPolicyV1 policy;
};
static_assert(sizeof(ModelOutputV1) == sizeof(float) * NNOUTPUT_CHANNELS_V1,"NNOUTPUT_CHANNELS_V1错误");

#if USE_BACKEND != BACKEND_NONE && USE_BACKEND != BACKEND_LIBTORCH 
struct ModelWeight
{
  static const int encoderLayer = 1;
  static const int encoderCh = 256;
  static const int mlpBlock = 2;
  static const int globalCh = 256;
  static const int mlpCh = 256;

  //为了省事，绝大部分线性变换没有bias只有weight
  //内存排列方式是输入通道优先
  std::vector<float> inputheadGlobal1;// [globalCh * (NNINPUT_CHANNELS_GAMEGLOBAL_V1 + NNINPUT_CHANNELS_SEARCHPARAM_V1)] ;
  std::vector<float> inputheadGlobal2;// [encoderCh * globalCh];
  std::vector<float> inputheadCard;// [encoderCh * NNINPUT_CHANNELS_CARD_V1];
#ifndef NO_NN_GAME_PERSON
  std::vector<float> inputheadPerson;// [encoderCh * NNINPUT_CHANNELS_PERSON_V1];
#endif // !NO_NN_GAME_PERSON


  std::vector<float> encoder_Q[encoderLayer];// [encoderCh * encoderCh];
  std::vector<float> encoder_V[encoderLayer];// [encoderCh * encoderCh];
  std::vector<float> encoder_global[encoderLayer];// [encoderCh * globalCh];

  std::vector<float> linBeforeMLP1;// [mlpCh * encoderCh];
  std::vector<float> linBeforeMLP2;// [mlpCh * globalCh];

  std::vector<float> mlp_lin[mlpBlock][2];// [mlpCh * mlpCh];

  std::vector<float> outputhead_w;// [NNOUTPUT_CHANNELS_V1 * mlpCh];
  std::vector<float> outputhead_b;// [NNOUTPUT_CHANNELS_V1];

  void load(std::string path);
};
#endif


#if USE_BACKEND == BACKEND_CUDA
struct ModelCudaBuf
{
  cublasHandle_t cublas;
  //cudnnHandle_t cudnn;


  //内存排列方式是输入通道优先
  float* inputheadGlobal1;
  float* inputheadGlobal2;
  float* inputheadCard;
#ifndef NO_NN_GAME_PERSON
  float* inputheadPerson;
#endif // !NO_NN_GAME_PERSON

  float* encoder_Q[ModelWeight::encoderLayer];
  float* encoder_V[ModelWeight::encoderLayer];
  float* encoder_global[ModelWeight::encoderLayer];

  float* linBeforeMLP1;
  float* linBeforeMLP2;

  float* mlp_lin[ModelWeight::mlpBlock][2];

  float* outputhead_w;
  float* outputhead_b;

  //中间变量
  uint16_t* inputOnesIdx;
  uint16_t* inputFloatIdx;
  float* inputFloatValue;

  float* input;
  float* inputGlobal;
  float* inputCard;
#ifndef NO_NN_GAME_PERSON
  float* inputPerson;
#endif // !NO_NN_GAME_PERSON
  float* gf;
  float* encoderInput_gf;
#ifndef NO_NN_GAME_PERSON
  float* encoderInput_cardf;
#endif // !NO_NN_GAME_PERSON
  float* encoderInput;//self.inputheadPerson(x3)直接保存在这里然后加上gf和cardf，然后relu

  float* encoderQ;
  float* encoderV;
  float* encoderAtt;
  float* encoderGf;
  float* encoderOutput;

  float* encoderSum;
  float* mlpInput;
  float* mlpMid;
  float* mlpMid2;

  float* outputhead_b_copied;
  float* output;


  void init(const ModelWeight& weight, int batchSize);
  ~ModelCudaBuf();
};
#endif


class Evaluator;



class Model
{
public:
  //static lock;//所有的evaluator共用一个lock
  Model(std::string path, int batchsize);
  void evaluate(Evaluator* eva, float* inputBuf, float* outputBuf, int gameNum);//计算gamesBuf中gameNum局游戏的输出，输出到outputBuf
  static void printBackendInfo();



private:
#if USE_BACKEND == BACKEND_ONNX
  Ort::Env onnxEnv;
  Ort::Session onnxSession{ nullptr };
  Ort::Value onnxInput{ nullptr };
  Ort::MemoryInfo onnxMemoryInfo{ nullptr }; 

#endif

#if USE_BACKEND == BACKEND_LIBTORCH
  torch::jit::script::Module model;
#endif

#if USE_BACKEND != BACKEND_NONE && USE_BACKEND != BACKEND_LIBTORCH 
  const int batchSize;
  std::mutex mtx; // 互斥锁
#endif

#if USE_BACKEND == BACKEND_CUDA
  ModelWeight modelWeight;
  ModelCudaBuf cb;
#endif
};