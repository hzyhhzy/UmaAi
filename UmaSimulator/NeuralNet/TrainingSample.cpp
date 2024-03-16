#include "TrainingSample.h"
#include "../Search/Search.h"
using namespace std;
TrainingSample Search::exportTrainingSample(float policyDelta)
{
  TrainingSample tdata;
  rootGame.getNNInputV1(tdata.nnInputVector, param);



  double bestValue = -1e100; 
  double values[Action::MAX_ACTION_TYPE];
  for (int i = 0; i < Action::MAX_ACTION_TYPE; i++)
  {
    values[i] = -1e7;
    const auto& res = allActionResults[i];
    assert(res.upToDate);
    if (!res.isLegal)
      continue;
    assert(res.num > 0);

    double v = res.lastCalculate.value;
    values[i] = v;
    if (v > bestValue)
    {
      tdata.valueTarget = res.lastCalculate;
      bestValue = v;
    }
  }

  //policy target--------------------------

  double totalPolicy = 0.0;
  float policyDeltaInv = 1.0 / policyDelta;
  for (int i = 0; i < Action::MAX_ACTION_TYPE; i++)
  {
    values[i] = exp((values[i] - bestValue) * policyDeltaInv);
    totalPolicy += values[i];
  }
  totalPolicy = 1 / totalPolicy;
  for (int i = 0; i < Action::MAX_ACTION_TYPE; i++)
  {
    values[i] *= totalPolicy;
    tdata.policyTarget.actionPolicy[i] = values[i];
    assert(allActionResults[i].isLegal || values[i] < 1e-7);
  }
  return tdata;
  
}


