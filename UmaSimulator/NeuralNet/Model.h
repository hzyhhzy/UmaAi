#pragma once
#include "NNInput.h"
#include "../config.h"
#include <string>
#include <vector>

const int NN_Game_Card_Num = 7;//7张卡（每次有且仅有一个位置为空）
const int NN_Game_Person_Num = 20;//20个人头（每次有且仅有二个位置为空）

#if USE_BACKEND == BACKEND_LIBTORCH
#include <torch/torch.h>
#include <torch/script.h>
#endif

#if USE_BACKEND == BACKEND_CUDA
#include <cublas_v2.h>
#include <cuda_runtime.h>
#endif

//用-1e7表示不合法操作，在神经网络训练时直接忽略掉这些值
struct ModelOutputPolicyV1
{
  float trainingPolicy[10];
  float buy50pPolicy[5];//在已确定训练的条件下，买对应训练的+50%的概率
  float buyPt10;//买不买pt+10
  float buyVital20;//买不买体力消耗-20%

  //同时买两个升级的概率。
  //加这个的目的是，假如买50%和买pt+10的分数差不多，但同时买会导致无法消除debuff过不去凯旋门，这时神经网络输出的两个值应该在50%附近，如果恰好都大于50%，则容易误判为同时买两个。
  //因此加入此项来辨别是否应该买两个
  float buyTwoUpgrades;
};
static_assert(sizeof(ModelOutputPolicyV1) == sizeof(float) * NNOUTPUT_CHANNELS_POLICY_V1, "NNOUTPUT_CHANNELS_POLICY_V1错误");


struct ModelOutputValueV1
{
  float scoreMean;//score的平均值
  float scoreStdev;//score的标准差
  //float winRate;//score>=target的概率
  float value;//考虑激进度之后的打分
  //float extract(int i);
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
  static const int encoderCh = 128;
  static const int mlpBlock = 3;
  static const int globalCh = 256;
  static const int mlpCh = 256;

  //为了省事，绝大部分线性变换没有bias只有weight
  //内存排列方式是输入通道优先
  std::vector<float> inputheadGlobal1;// [globalCh * (NNINPUT_CHANNELS_GAMEGLOBAL_V1 + NNINPUT_CHANNELS_SEARCHPARAM_V1)] ;
  std::vector<float> inputheadGlobal2;// [encoderCh * globalCh];
  std::vector<float> inputheadCard;// [encoderCh * NNINPUT_CHANNELS_CARD_V1];
  std::vector<float> inputheadPerson;// [encoderCh * NNINPUT_CHANNELS_PERSON_V1];

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
  float* inputheadPerson;

  float* encoder_Q[ModelWeight::encoderLayer];
  float* encoder_V[ModelWeight::encoderLayer];
  float* encoder_global[ModelWeight::encoderLayer];

  float* linBeforeMLP1;
  float* linBeforeMLP2;

  float* mlp_lin[ModelWeight::mlpBlock][2];

  float* outputhead_w;
  float* outputhead_b;

  //中间变量
  float* input;
  float* inputGlobal;
  float* inputCard;
  float* inputPerson;
  float* gf;
  float* encoderInput_gf;
  float* encoderInput_cardf;
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






class Model
{
public:
  //static lock;//所有的evaluator共用一个lock
  Model(std::string path, int batchsize);
  void evaluate(float* inputBuf, float* outputBuf, int gameNum);//计算gamesBuf中gameNum局游戏的输出，输出到outputBuf
  static void printBackendInfo();



private:
#if USE_BACKEND == BACKEND_LIBTORCH
  torch::jit::script::Module model;
#endif

#if USE_BACKEND != BACKEND_NONE && USE_BACKEND != BACKEND_LIBTORCH 
  const int batchSize;
  ModelWeight modelWeight;
#endif

#if USE_BACKEND == BACKEND_CUDA
  ModelCudaBuf cb;
#endif
};