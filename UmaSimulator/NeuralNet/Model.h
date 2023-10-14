#pragma once
#include <vector>
#include "NNInput.h"
#include "../Game/Game.h"


struct ModelOutputPolicyV1
{
  float trainingPolicy[10];
  float buy50p;//买不买对应训练的+50%
  float buyPt10;//买不买pt+10
  float buyFriend20;//买不买友情+20%
  float buyVital20;//买不买体力消耗-20%
  float expectShixingPtCost;//适性pt消耗的期望值。加这个的目的是避免只能买一个但不能买两个的情况下，两个buy都恰好大于50%
};
static_assert(sizeof(ModelOutputPolicyV1) == sizeof(float) * NNOUTPUT_CHANNELS_POLICY_V1, "NNOUTPUT_CHANNELS_POLICY_V1错误");


struct ModelOutputValueV1
{
  float scoreMean;//score的平均值
  float scoreStdev;//score的标准差
  float winRate;//score>=target的概率
  float scoreOverTargetMean;//max(target,score)的平均值
  float scoreOverTargetStdev;//max(target,score)的标准差
  float extract(int i);
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