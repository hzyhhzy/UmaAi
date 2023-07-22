#include <random>
#include <cassert>
#include <iostream>
#include <sstream>
#include <thread>
#include <atomic>
#include <future>
#include "Search.h"
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

void Search::runSearch(const Game& game, Evaluator* evaluators, int eachSamplingNum, int maxDepth, int targetScore, int threadNum)
{
  //cout << endl;
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
     // cout << "- 生涯比赛" << endl;
      allChoicesValue[useVenus][0] = evaluateSingleAction(game, evaluators, eachSamplingNum, maxDepth, targetScore,
        rand, -1, useVenus, -1, -1, threadNum, -1);
    }
    else
    {
      //五个训练
     //if (useVenus)
     // cout << endl << "- 分析女神Buff下的情况：" << endl;
     //cout << "- 正在分析训练";
      //对应位置01234，女神三选一事件对应8 9 10 11（不出，红，蓝，黄），休息5，外出6和12~17，比赛7
      for (int item = 0; item < 5; item++)
      {
        assert(GameDatabase::AllSupportCards[game.cardId[0]].cardType == 5 && "神团卡不在第一个位置");
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
                game, evaluators, eachSamplingNum, maxDepth, targetScore,
                rand, item, useVenus, chosenSpiritColor, -1, threadNum, 1);
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
              game, evaluators, eachSamplingNum, maxDepth, targetScore,
              rand, item, useVenus, -1, -1, threadNum, -1);
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
            game, evaluators, eachSamplingNum, maxDepth, targetScore,
            rand, item, useVenus, -1, -1, threadNum, -1);
        }
      }

      //休息
      //cout << endl << "- 正在分析休息";
      allChoicesValue[useVenus][5] = evaluateSingleAction(
        game, evaluators, eachSamplingNum, maxDepth, targetScore,
        rand, 5, useVenus, -1, -1, threadNum, -1);

      //比赛
      //cout << endl << "- 正在分析比赛";
      if (game.turn > 12)
      {
        allChoicesValue[useVenus][7] = evaluateSingleAction(
          game, evaluators, eachSamplingNum, maxDepth, targetScore,
          rand, 7, useVenus, -1, -1, threadNum, -1);
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
            game, evaluators, eachSamplingNum, maxDepth, targetScore,
            rand, 6, useVenus, -1, chosenOutgoing, threadNum, -1);
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
}



ModelOutputValueV1 Search::evaluateSingleAction(const Game& game, Evaluator* evaluators, int eachSamplingNum, int maxDepth, int targetScore,
  std::mt19937_64& rand, int chosenTrain, bool useVenus, int chosenSpiritColor, int chosenOutgoing, int threadNum, int forceThreeChoicesEvent)
{
    //cout << "."; cout.flush();
  if (threadNum == 1)
  {

    int batchsize = evaluators->maxBatchsize;
    int batchNum = (eachSamplingNum - 1) / evaluators->maxBatchsize + 1;
    eachSamplingNum = batchNum * evaluators->maxBatchsize;//凑够整数个batchsize

    std::vector<float> targetScores;
    targetScores.assign(batchsize, targetScore);

    double totalScore = 0;
    double totalWinrate = 0;
    double totalNum = 0;
    Game gameCopy = game;
    gameCopy.playerPrint = false;
    std::vector<Game> gamesBuf;

    for (int batch = 0; batch < batchNum; batch++)
    {
      gamesBuf.assign(batchsize, gameCopy);

      //先走第一步
      for (int i = 0; i < batchsize; i++)
      {
        gamesBuf[i].applyTrainingAndNextTurn(rand, chosenTrain, useVenus, chosenSpiritColor, chosenOutgoing, forceThreeChoicesEvent);
      }


      for (int depth = 0; depth < maxDepth; depth++)
      {
        evaluators->evaluate(gamesBuf.data(), targetScores.data(), 1, batchsize);//计算policy
        bool distributeCards = (depth != maxDepth - 1);//最后一层就不分配卡组了，直接调用神经网络估值


        bool allFinished = true;
        for (int i = 0; i < batchsize; i++)
        {
          Search::runOneTurnUsingPolicy(rand, gamesBuf[i], evaluators->policyResults[i], distributeCards);
          if (!gamesBuf[i].isEnd())allFinished = false;
        }
        if (allFinished)break;
      }
      evaluators->evaluate(gamesBuf.data(), targetScores.data(), 0, batchsize);//计算value
      for (int i = 0; i < batchsize; i++)
      {
        totalScore += evaluators->valueResults[i].avgScoreMinusTarget;
        totalWinrate += evaluators->valueResults[i].winrate;
      }
      totalNum += batchsize;

    }
    ModelOutputValueV1 out;
    out.avgScoreMinusTarget = totalScore / totalNum;
    out.winrate = totalWinrate / totalNum;
    return out;
  }

  else
  {

    int eachSamplingNumEveryThread = eachSamplingNum / threadNum;
    if (eachSamplingNumEveryThread <= 0)eachSamplingNumEveryThread = 1;

    std::vector<std::mt19937_64> rands;
    for (int i = 0; i < threadNum; i++)
      rands.push_back(std::mt19937_64(rand()));



    std::vector<std::future<ModelOutputValueV1>> futures(threadNum); // 用于存储每个线程的结果

    // 创建线程并执行函数f
    for (int i = 0; i < threadNum; ++i) {
      futures[i] = std::async(std::launch::async,
        [this, i, &game, &evaluators, eachSamplingNumEveryThread, maxDepth, targetScore, &rands, chosenTrain, useVenus, chosenSpiritColor, chosenOutgoing, forceThreeChoicesEvent]() {
          
          return this->evaluateSingleAction(game, evaluators + i, eachSamplingNumEveryThread, maxDepth, targetScore,
          rands[i],
          chosenTrain, useVenus, chosenSpiritColor, chosenOutgoing, 1, forceThreeChoicesEvent);
        });
     
    }

    ModelOutputValueV1 totalResult = { 0,0 };
    // 获取每个线程的结果
    for (int i = 0; i < threadNum; ++i) {
      auto result = futures[i].get(); 
      totalResult.avgScoreMinusTarget += result.avgScoreMinusTarget;
      totalResult.winrate += result.winrate;
    }
    totalResult.avgScoreMinusTarget /= threadNum;
    totalResult.winrate /= threadNum;
    return totalResult;
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

  bool useVenus;//先看看用不用女神
  {
    float bestValueNotUseVenus = -1e30;
    for (int i = 0; i < 8; i++)
    {
      if (allChoicesValue[0][i].extract(mode) > bestValueNotUseVenus)bestValueNotUseVenus = allChoicesValue[0][i].extract(mode);
    }
    float bestValueUseVenus = -1e30;
    for (int i = 0; i < 8; i++)
    {
      if (allChoicesValue[1][i].extract(mode) > bestValueUseVenus)bestValueUseVenus = allChoicesValue[1][i].extract(mode);
    }
    useVenus = bestValueUseVenus > bestValueNotUseVenus;

    float useVenusPolicy[2] = { bestValueUseVenus * deltaInv,bestValueNotUseVenus * deltaInv };
    softmax(useVenusPolicy, 2);
    policy.useVenusPolicy = useVenusPolicy[0];
  }

  //训练8选1
  for (int i = 0; i < 8; i++)
    policy.trainingPolicy[i] = deltaInv * allChoicesValue[int(useVenus)][i].extract(mode);
  softmax(policy.trainingPolicy, 8);

  //三选一事件
  for (int i = 0; i < 3; i++)
    policy.threeChoicesEventPolicy[i] = deltaInv * allChoicesValue[int(useVenus)][8 + 1 + i].extract(mode);
  softmax(policy.threeChoicesEventPolicy, 3);

  //选外出
  for (int i = 0; i < 6; i++)
    policy.outgoingPolicy[i] = deltaInv * allChoicesValue[int(useVenus)][8 + 4 + i].extract(mode);
  softmax(policy.outgoingPolicy, 6);

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
  useVenus = policy.useVenusPolicy >= 0.5;
  {
    float bestPolicy = 0.001;
    for (int i = 0; i < 3; i++)
    {
      float p = policy.threeChoicesEventPolicy[i];
      if (p > bestPolicy)
      {
        chosenSpiritColor = i;
        bestPolicy = p;
      }
    }
  }
  {
    float bestPolicy = 0.001;
    for (int i = 0; i < 6; i++)
    {
      float p = policy.outgoingPolicy[i];
      if (p > bestPolicy)
      {
        chosenOutgoing = i;
        bestPolicy = p;
      }
    }
  }
  if(distributeCards)
    game.applyTrainingAndNextTurn(rand, chosenTrain, useVenus, chosenSpiritColor, chosenOutgoing);
  else
    game.applyTraining(rand, chosenTrain, useVenus, chosenSpiritColor, chosenOutgoing);


}
