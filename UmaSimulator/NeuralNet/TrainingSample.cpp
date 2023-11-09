#include "TrainingSample.h"
#include "../Search/Search.h"
using namespace std;
TrainingSample Search::exportTrainingSample(float policyDelta)
{
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

  //buy50pPolicy
  for (int j = 0; j < 5; j++)
  {
    double bestValue1 = 0.0;
    for (int i = 0; i < bfcn; i++)
    {
      bestValue1 = std::max(bestValue1, double(allChoicesValue[i][j].value));
    }

    double totalValue1 = 0.0;
    double buy50pValue = 0.0;
    for (int i = 0; i < bfcn; i++)
    {
      Action action = buyBuffAction(i, gameLastSearch.turn);
      double v = exp((allChoicesValue[i][j].value - bestValue1) * policyDeltaInv);
      totalValue1 += v;
      if (action.buy50p)
      {
        buy50pValue += v;
      }
    }
    pt.buy50pPolicy[j] = buy50pValue / totalValue1;
  }

  double totalPolicyExceptRest = 0.0;
  pt.buyPt10 = 0.0;
  pt.buyTwoUpgrades = 0.0;
  pt.buyVital20 = 0.0;
  for (int j = 0; j < 5; j++)
  {
    int bfcn = buyBuffChoiceNum(gameLastSearch.turn);
    for (int i = 0; i < bfcn; i++)
    {
      double v = values[i][j]; 
      totalPolicyExceptRest += v;
      Action action = buyBuffAction(i, gameLastSearch.turn);
      if (action.buyPt10)
      {
        pt.buyPt10 += v;
      }
      if (action.buyVital20)
      {
        pt.buyVital20 += v;
      }
      if (action.buy50p && (action.buyPt10 || action.buyVital20))
      {
        pt.buyTwoUpgrades += v;
      }
    }
  }
  if (totalPolicyExceptRest < 1e-8)totalPolicyExceptRest = 1e-8;
  pt.buyPt10 /= totalPolicyExceptRest;
  pt.buyTwoUpgrades /= totalPolicyExceptRest;
  pt.buyVital20 /= totalPolicyExceptRest;


  gameLastSearch.getNNInputV1(tdata.nnInputVector,param);
  return tdata;
}