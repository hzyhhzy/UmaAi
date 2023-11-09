#pragma once
#include "Model.h"



struct TrainingSample
{
  float nnInputVector[NNINPUT_CHANNELS_V1];
  ModelOutputPolicyV1 policyTarget;
  ModelOutputValueV1 valueTarget;
};