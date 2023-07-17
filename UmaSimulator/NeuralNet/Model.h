#pragma once
#include <vector>
#include "NNInput.h"
#include "../Game/Game.h"


struct ModelOutputPolicyV1
{
  float trainingPolicy[5 + 3];
  float useVenusPolicy;
  float outgoingPolicy[6];
  float threeChoicesEventPolicy[3];
};
static_assert(sizeof(ModelOutputPolicyV1) == sizeof(float) * NNOUTPUT_CHANNELS_POLICY_V1, "NNOUTPUT_CHANNELS_POLICY_V1错误");


struct ModelOutputValueV1
{
  float winrate;
  float avgScoreMinusTarget;
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