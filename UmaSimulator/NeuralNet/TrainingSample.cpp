#include "TrainingSample.h"
#include "../Search/Search.h"
using namespace std;
TrainingSample Search::exportTrainingSample(float policyDelta)
{
  assert(false && "todo");
  return TrainingSample();
  /*
  TrainingSample tdata;
  int bfcn = buyBuffChoiceNum(gameLastSearch.turn);
  double values[4][10];

  double bestValue = -1e100;
  for (int i = 0; i < bfcn; i++)
    for (int j = 0; j < 10; j++)
    {
      double v = allChoicesValue[i][j].value;
      values[i][j] = v;
      if (v > bestValue)
      {
        tdata.valueTarget = allChoicesValue[i][j];
        bestValue = v;
      }
    }

  //policy target--------------------------
  auto& pt = tdata.policyTarget;
  //softmax
  double totalPolicy = 0.0;
  float policyDeltaInv = 1.0 / policyDelta;
  for (int i = 0; i < bfcn; i++)
    for (int j = 0; j < 10; j++)
    {
      values[i][j] = exp((values[i][j] - bestValue) * policyDeltaInv);
      totalPolicy += values[i][j];
    }
  totalPolicy = 1 / totalPolicy;
  for (int i = 0; i < bfcn; i++)
    for (int j = 0; j < 10; j++)
    {
      values[i][j] *= totalPolicy;
    }

  //trainingPolicy
  for (int j = 0; j < 10; j++)
  {
    pt.trainingPolicy[j] = 0.0;
    for (int i = 0; i < bfcn; i++)
    {
      pt.trainingPolicy[j] += values[i][j];
    }
  }


  gameLastSearch.getNNInputV1(tdata.nnInputVector,param);
  return tdata;
  */
}


