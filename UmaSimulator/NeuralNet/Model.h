#pragma once
#include <vector>
#include "NNInput.h"
#include "../Game/Game.h"

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

class Model
{
  //Model* model;
  //static lock;//所有的evaluator共用一个lock
  //void evaluate(const Game* gamesBuf, const float* otherInputsBuf, int gameNum);//计算gamesBuf中gameNum局游戏的输出，输出到outputBuf

};