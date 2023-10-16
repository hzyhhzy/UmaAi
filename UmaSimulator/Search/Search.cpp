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

Search::Search(Model* model, int batchSize, int threadNumInGame):threadNumInGame(threadNumInGame), batchSize(batchSize)
{
  evaluators.resize(threadNumInGame);
  for (int i = 0; i < threadNumInGame; i++)
    evaluators[i] = Evaluator(model, batchSize);
  
}




void Search::runSearch(const Game& game,
  int samplingNum, int maxDepth, int targetScore)
{
  /*
  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 8 + 4 + 6; j++)
    {
      allChoicesValue[i][j].winrate = -1;
      allChoicesValue[i][j].avgScoreMinusTarget = -1e6;
    }

  std::random_device rd;
  auto rand = std::mt19937_64(rd());
  //gamesBuf.assign(eachSamplingNum,rootGame);
  int activateVenusChoices = game.venusAvailableWisdom > 0 ? 2 : 1;
  for (int useVenus = 0; useVenus < activateVenusChoices; useVenus++)
  {
    //先考虑是不是只有比赛
    if (game.isRacing)
    {
      allChoicesValue[useVenus][0] = evaluateSingleAction(game, evaluators, eachSamplingNum, maxDepth, targetScore, threadNum, radicalFactor,
        rand, -1, useVenus, -1, -1, -1);
    }
    else
    {
      //五个训练
     if (GameConfig::debugPrint && useVenus)
       cout << endl << "- 分析女神Buff：" << endl;

      //对应位置01234，女神三选一事件对应8 9 10 11（不出，红，蓝，黄），休息5，外出6和12~17，比赛7
      for (int item = 0; item < 5; item++)
      {
        assert(GameDatabase::AllCards[game.cardId[0]].cardType == 5 && "神团卡不在第一个位置");
        if (game.cardDistribution[item][0])//神团在这里，需要考虑三选一事件
        {
          double threeChoicesProb = game.getThreeChoicesEventProb(useVenus);
          //女神出来的情况
          double bestWinrateIfThreeChoices = -1;//三选一里面最好的那种情况
          double bestScoreIfThreeChoices = -1e6;
          if (threeChoicesProb > 0.0)
          {
            for (int chosenSpiritColor = 0; chosenSpiritColor < 3; chosenSpiritColor++)
            {
              auto value = evaluateSingleAction(
                game, evaluators, eachSamplingNum, maxDepth, targetScore, threadNum, radicalFactor,
                rand, item, useVenus, chosenSpiritColor, -1, 1);
              allChoicesValue[useVenus][8 + 1 + chosenSpiritColor] = value;
              if (value.avgScoreMinusTarget > bestScoreIfThreeChoices)
                bestScoreIfThreeChoices = value.avgScoreMinusTarget;
              if (value.winrate > bestWinrateIfThreeChoices)
                bestWinrateIfThreeChoices = value.winrate;
            }
          }
          else
          {
            for (int chosenSpiritColor = 0; chosenSpiritColor < 3; chosenSpiritColor++)
            {
              allChoicesValue[useVenus][8 + 1 + chosenSpiritColor].avgScoreMinusTarget = -1e6;
              allChoicesValue[useVenus][8 + 1 + chosenSpiritColor].winrate = 0.0;
            }
          }

          //女神不出来的情况
          if (threeChoicesProb < 1.0)
          {
            allChoicesValue[useVenus][8] = evaluateSingleAction(
              game, evaluators, eachSamplingNum, maxDepth, targetScore, threadNum, radicalFactor, 
              rand, item, useVenus, -1, -1,  -1);
          }
          else
          {
            allChoicesValue[useVenus][8].avgScoreMinusTarget = -1e6;
            allChoicesValue[useVenus][8].winrate = 0.0;
          }

          //出来和不出来取平均
          allChoicesValue[useVenus][item].avgScoreMinusTarget = allChoicesValue[useVenus][8].avgScoreMinusTarget * (1 - threeChoicesProb) + bestScoreIfThreeChoices * threeChoicesProb;
          allChoicesValue[useVenus][item].winrate = allChoicesValue[useVenus][8].winrate * (1 - threeChoicesProb) + bestWinrateIfThreeChoices * threeChoicesProb;

        }
        else//女神不在这个训练
        {
          allChoicesValue[useVenus][item] = evaluateSingleAction(
            game, evaluators, eachSamplingNum, maxDepth, targetScore, threadNum, radicalFactor,
            rand, item, useVenus, -1, -1, -1);
        }
      }

      //休息
      //cout << endl << "- 正在分析休息";
      allChoicesValue[useVenus][5] = evaluateSingleAction(
        game, evaluators, eachSamplingNum, maxDepth, targetScore, threadNum, radicalFactor,
        rand, 5, useVenus, -1, -1, -1);

      //比赛
      //cout << endl << "- 正在分析比赛";
      if (game.turn > 12 && game.turn < 72)
      {
        allChoicesValue[useVenus][7] = evaluateSingleAction(
          game, evaluators, eachSamplingNum, maxDepth, targetScore, threadNum, radicalFactor,
          rand, 7, useVenus, -1, -1, -1);
      }

      //外出
      //cout << endl << "- 正在分析出行";
      if(!game.isXiaHeSu())
      {
        double bestWinrate = -1;//三选一里面最好的那种情况
        double bestScore = -1e6;
        for (int chosenOutgoing= 0; chosenOutgoing < 6; chosenOutgoing++)
        {
          if (!game.isOutgoingLegal(chosenOutgoing))
            continue;
          auto value = evaluateSingleAction(
            game, evaluators, eachSamplingNum, maxDepth, targetScore, threadNum, radicalFactor,
            rand, 6, useVenus, -1, chosenOutgoing, -1);
          allChoicesValue[useVenus][8 + 4 + chosenOutgoing] = value;
          if (value.avgScoreMinusTarget > bestScore)
            bestScore = value.avgScoreMinusTarget;
          if (value.winrate > bestWinrate)
            bestWinrate = value.winrate;
        }
        allChoicesValue[useVenus][6].avgScoreMinusTarget = bestScore;
        allChoicesValue[useVenus][6].winrate = bestWinrate;
      }

    }
  }
  */
}


ModelOutputValueV1 Search::evaluateSingleAction(const Game& game, int samplingNum, int maxDepth, int targetScore, std::mt19937_64& rand, Action action)
{
  int batchEveryThread = (samplingNum - 1) / (threadNumInGame * batchSize) + 1;//相当于向上取整
  if (batchEveryThread <= 0)batchEveryThread = 1;
  int samplingNumEveryThread = threadNumInGame * batchSize;
  samplingNum = batchEveryThread * samplingNumEveryThread;
  resultBuf.resize(samplingNum);




  if (threadNumInGame > 1)
  {
    std::vector<std::mt19937_64> rands;
    for (int i = 0; i < threadNumInGame; i++)
      rands.push_back(std::mt19937_64(rand()));

    std::vector<std::thread> threads;
    for (int i = 0; i < threadNumInGame; ++i) {
      threads.push_back(std::thread(

        [&]() {
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
  double scoreTotal;//score的和
  double scoreSqrTotal;//score的平方和
  double winNum;//score>=target的次数期望
  double scoreOverTargetTotal;//max(target,score)的和
  double scoreOverTargetSqrTotal;//max(target,score)的平方和
  for (int i = 0; i < samplingNum; i++)
  {
    ModelOutputValueV1 v = resultBuf[i];
    scoreTotal += v.scoreMean;
    scoreSqrTotal += double(v.scoreMean) * v.scoreMean + double(v.scoreStdev) * v.scoreStdev;
    scoreOverTargetTotal += v.scoreOverTargetMean;
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
      Action action;
      eva.gameInput[i].applyTrainingAndNextTurn(rand, action);
    }

    for (int depth = 0; depth < maxDepth; depth++)
    {
      eva.evaluateSelf(1, targetScore);//计算policy
      bool distributeCards = (depth != maxDepth - 1);//最后一层就不分配卡组了，直接调用神经网络估值


      bool allFinished = true;
      for (int i = 0; i < batchSize; i++)
      {
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
