#include <random>
#include <cassert>
#include <iostream>
#include <sstream>
#include <thread>
#include <atomic>
#include <future>
#include "Search.h"
#include "../GameDatabase/GameConfig.h"
#include <iostream>
using namespace std;

static void softmax(float* f, int n)
{
  float max = -1e30;
  for (int i = 0; i < n; i++)
    if (f[i] > max)max = f[i];

  float total = 0;
  for (int i = 0; i < n; i++)
  {
    f[i] = exp(f[i] - max);
    total += f[i];
  }

  float totalInv = 1 / total;
  for (int i = 0; i < n; i++)
    f[i] *= totalInv;
}

int Search::buyBuffChoiceNum(int turn)
{
  return
    (turn <= 35 || (43 <= turn && turn <= 59)) ? 1 :
      turn == 41 ? 2 :
      ((36 <= turn && turn <= 39) || turn >= 60) ? 4 :
      -1;
}

Action Search::buyBuffAction(int idx, int turn)
{
  Action action;
  action.train = -1;
  action.buy50p = false;
  action.buyFriend20 = false;
  action.buyPt10 = false;
  action.buyVital20 = false;

  if (36 <= turn && turn <= 39)
  {
    action.buy50p = idx % 2;
    action.buyPt10 = idx / 2;
  }
  else if (turn >= 60)
  {
    action.buy50p = idx % 2;
    action.buyVital20 = idx / 2;
  }
  else if (turn == 41)
  {
    action.buy50p = idx % 2;
  }

  return action;
}

Search::Search(Model* model, int batchSize, int threadNumInGame):threadNumInGame(threadNumInGame), batchSize(batchSize)
{
  evaluators.resize(threadNumInGame);
  for (int i = 0; i < threadNumInGame; i++)
    evaluators[i] = Evaluator(model, batchSize);
  
}




Action Search::runSearch(const Game& game,
  int samplingNum, int maxDepth, int targetScore,
  std::mt19937_64& rand)
{

  ModelOutputValueV1 illegalValue;
  {
    illegalValue.scoreMean = -1e5;
    illegalValue.scoreOverTargetMean = -1e5;
    illegalValue.winRate = 0;
    illegalValue.scoreStdev = 0;
    illegalValue.scoreOverTargetStdev = 0;
  }

  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 10; j++)
    {
      allChoicesValue[i][j] = illegalValue;
    }



  for (int buyBuffChoice = 0; buyBuffChoice < buyBuffChoiceNum(game.turn); buyBuffChoice++)
  {
    Action action0=buyBuffAction(buyBuffChoice,game.turn);
    int trainNumToConsider = //如果买训练buff，则一定不休息
      buyBuffChoice == 0 ? 10 :
      action0.buyVital20 ? 4 : //买体力消耗减少的buff肯定不点智力
      5;
    for (int t = 0; t < trainNumToConsider; t++)
    {
      Action action = action0;
      action.train = t;
      allChoicesValue[buyBuffChoice][t] = evaluateSingleAction(game, samplingNum, maxDepth, targetScore, rand, action);
    }

  }

  Action action;
  double bestValue = -5e4;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 10; j++)
    {
      double v = allChoicesValue[i][j].scoreOverTargetMean;
      if (v > bestValue)
      {
        bestValue = v;
        action = buyBuffAction(i, game.turn);
        action.train = j;
      }
    }
  return action;
}


ModelOutputValueV1 Search::evaluateSingleAction(const Game& game, int samplingNum, int maxDepth, int targetScore, std::mt19937_64& rand, Action action)
{
  //先检查action是否合法
  Game game1 = game;
  bool isLegal = game1.applyTraining(rand, action);
  if (!isLegal)
  {
    ModelOutputValueV1 v;
    v.scoreMean = -1e5;
    v.scoreOverTargetMean = -1e5;
    v.winRate = 0;
    v.scoreStdev = 0;
    v.scoreOverTargetStdev = 0;
    return v;
  }


  int batchEveryThread = (samplingNum - 1) / (threadNumInGame * batchSize) + 1;//相当于向上取整
  if (batchEveryThread <= 0)batchEveryThread = 1;
  int samplingNumEveryThread = batchSize * batchEveryThread;
  samplingNum = threadNumInGame * samplingNumEveryThread;
  resultBuf.resize(samplingNum);




  if (threadNumInGame > 1)
  {
    std::vector<std::mt19937_64> rands;
    for (int i = 0; i < threadNumInGame; i++)
      rands.push_back(std::mt19937_64(rand()));

    std::vector<std::thread> threads;
    for (int i = 0; i < threadNumInGame; ++i) {
      threads.push_back(std::thread(

        [this, i, samplingNumEveryThread, &game, maxDepth, targetScore, &rands, action]() {
          evaluateSingleActionThread(
            i,
            resultBuf.data() + samplingNumEveryThread * i,
            game,
            samplingNumEveryThread,
            maxDepth,
            targetScore,
            rands[i],
            action
          );
        })
      );


    }
    for (auto& thread : threads) {
      thread.join();
    }
  }
  else
  {
    evaluateSingleActionThread(
      0,
      resultBuf.data(),
      game,
      samplingNumEveryThread,
      maxDepth,
      targetScore,
      rand,
      action
    );
  }


  //整合所有结果
  double scoreTotal = 0;//score的和
  double scoreSqrTotal = 0;//score的平方和
  double winNum = 0;//score>=target的次数期望
  double scoreOverTargetTotal = 0;//max(target,score)的和
  double scoreOverTargetSqrTotal = 0;//max(target,score)的平方和
  for (int i = 0; i < samplingNum; i++)
  {
    ModelOutputValueV1 v = resultBuf[i];
    scoreTotal += v.scoreMean;
    scoreSqrTotal += double(v.scoreMean) * v.scoreMean + double(v.scoreStdev) * v.scoreStdev;
    scoreOverTargetTotal += v.scoreOverTargetMean;
    //cout << v.scoreMean << " ";
    scoreOverTargetSqrTotal += double(v.scoreOverTargetMean) * v.scoreOverTargetMean + double(v.scoreOverTargetStdev) * v.scoreOverTargetStdev;
    winNum += v.winRate;
  }

  ModelOutputValueV1 v;
  v.scoreMean = scoreTotal / samplingNum;
  v.scoreStdev = sqrt(scoreSqrTotal * samplingNum - scoreTotal * scoreTotal) / samplingNum;
  v.scoreOverTargetMean = scoreOverTargetTotal / samplingNum;
  v.scoreOverTargetStdev = sqrt(scoreOverTargetSqrTotal * samplingNum - scoreOverTargetTotal * scoreOverTargetTotal) / samplingNum;
  v.winRate = winNum / samplingNum;
  return v;
}

void Search::evaluateSingleActionThread(int threadIdx, ModelOutputValueV1* resultBuf, const Game& game, int samplingNum, int maxDepth, int targetScore, std::mt19937_64& rand, Action action)
{
  Evaluator& eva = evaluators[threadIdx];
  assert(eva.maxBatchsize == batchSize);
  assert(samplingNum % batchSize == 0);
  int batchNum = samplingNum / batchSize;



  for (int batch = 0; batch < batchNum; batch++)
  {
    eva.gameInput.assign(batchSize, game);

    //先走第一步
    for (int i = 0; i < batchSize; i++)
    {
      eva.gameInput[i].applyTrainingAndNextTurn(rand, action);
    }

    for (int depth = 0; depth < maxDepth; depth++)
    {
      eva.evaluateSelf(1, targetScore);//计算policy
      //bool distributeCards = (depth != maxDepth - 1);//最后一层就不分配卡组了，直接调用神经网络估值


      bool allFinished = true;
      for (int i = 0; i < batchSize; i++)
      {
        if(!eva.gameInput[i].isEnd())
          eva.gameInput[i].applyTrainingAndNextTurn(rand, eva.actionResults[i]);
        //Search::runOneTurnUsingPolicy(rand, gamesBuf[i], evaluators->policyResults[i], distributeCards);
        if (!eva.gameInput[i].isEnd())allFinished = false;
      }
      if (allFinished)break;
    }
    eva.evaluateSelf(0, targetScore);//计算value
    for (int i = 0; i < batchSize; i++)
    {
      resultBuf[batch * batchSize + i] = eva.valueResults[i];
    }

  }
}

ModelOutputPolicyV1 Search::extractPolicyFromSearchResults(int mode, float delta)
{
  ModelOutputPolicyV1 policy;
  if (delta == 0)
  {
    if (mode == 0)delta = 0.02;
    else delta = 30;
  }
  float deltaInv = 1 / delta;


  assert(false);
  //训练8选1
  for (int i = 0; i < 8; i++)
    policy.trainingPolicy[i] = deltaInv * allChoicesValue[0][i].extract(mode);
  softmax(policy.trainingPolicy, 8);

  return policy;
}

void Search::runOneTurnUsingPolicy(std::mt19937_64& rand, Game& game, const ModelOutputPolicyV1& policy, bool distributeCards)
{
  if (game.isEnd())return;
  bool useVenus = false;
  int chosenSpiritColor = -1;
  int chosenTrain = -1;
  int chosenOutgoing = -1;

  //auto policy = Evaluator::handWrittenPolicy(game);
  {
    float bestPolicy = 0.001;
    for (int i = 0; i < 8; i++)
    {
      float p = policy.trainingPolicy[i];
      if (p > bestPolicy)
      {
        chosenTrain = i;
        bestPolicy = p;
      }
    }
  }
  Action action;
  if(distributeCards)
    game.applyTrainingAndNextTurn(rand, action);
  else
    game.applyTraining(rand, action);

}
