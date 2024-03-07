#include <random>
#include <cassert>
#include <iostream>
#include <sstream>
#include <thread>
#include <atomic>
#include <future>
#include <iostream>
#include "Search.h"
#include "../GameDatabase/GameConfig.h"
#include "../External/mathFunctions.h"
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

//根据回合数调整激进度
static double adjustRadicalFactor(double maxRf, int turn)
{
  //计算该取的激进度
  double remainTurns = turn >= 65 ? 1 : 65 - turn;
  double factor = (remainTurns <= 5 ? 5 * remainTurns : remainTurns + 20) / (67.0 + 20);//分段折线
  return factor * maxRf;
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

  assert(false && "todo");

  return action;
}

Search::Search(Model* model, int batchSize, int threadNumInGame):threadNumInGame(threadNumInGame), batchSize(batchSize)
{
  evaluators.resize(threadNumInGame);
  for (int i = 0; i < threadNumInGame; i++)
    evaluators[i] = Evaluator(model, batchSize);

  //正态分布累积分布函数的反函数在0~1上均匀取点
  for (int i = 0; i < NormDistributionSampling; i++)
  {
    double x = (i + 0.5) / NormDistributionSampling;
    normDistributionCdfInv[i] = normalCDFInverse(x);
  }

  param.samplingNum = 0;
}

Search::Search(Model* model, int batchSize, int threadNumInGame, SearchParam param0) :Search(model, batchSize, threadNumInGame)
{
  setParam(param0);
}
void Search::setParam(SearchParam param0)
{
  param = param0;

  //让param.samplingNum是整batch
  int batchEveryThread = (param.samplingNum - 1) / (threadNumInGame * batchSize) + 1;//相当于向上取整
  if (batchEveryThread <= 0)batchEveryThread = 1;
  int samplingNumEveryThread = batchSize * batchEveryThread;
  param.samplingNum = threadNumInGame * samplingNumEveryThread;
  NNresultBuf.resize(param.samplingNum);
}



Action Search::runSearch(const Game& game,
  std::mt19937_64& rand)
{
  assert(param.samplingNum >= 0 && "Search.param not initialized");

  gameLastSearch = game;
  ModelOutputValueV1 illegalValue;
  {
    illegalValue.scoreMean = -1e5;
    illegalValue.scoreStdev = 0;
    illegalValue.value = -1e5;
  }

  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 10; j++)
    {
      allChoicesValue[i][j] = illegalValue;
    }
  assert(false && "todo");
  /*
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
      allChoicesValue[buyBuffChoice][t] = evaluateSingleAction(game, rand, action);
      assert(game.isLegal(action) == (allChoicesValue[buyBuffChoice][t].scoreMean > -1e4));//检查isLegal写的对不对
    }

  }
  */
  Action action;
  double bestValue = -5e4;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 10; j++)
    {
      double v = allChoicesValue[i][j].value;
      if (v > bestValue)
      {
        bestValue = v;
        action = buyBuffAction(i, game.turn);
        action.train = j;
      }
    }
  return action;
}


ModelOutputValueV1 Search::evaluateSingleAction(const Game& game, std::mt19937_64& rand, Action action)
{
  //先检查action是否合法
  Game game1 = game;
  bool isLegal = game1.applyTraining(rand, action);
  if (!isLegal)
  {
    ModelOutputValueV1 illegalValue;
    illegalValue.scoreMean = -1e5;
    illegalValue.scoreStdev = 0;
    illegalValue.value = -1e5;
    return illegalValue;
  }


  assert(param.samplingNum % (threadNumInGame * batchSize) == 0);
  assert(NNresultBuf.size() == param.samplingNum);
  int samplingNumEveryThread = param.samplingNum / threadNumInGame;

  if (threadNumInGame > 1)
  {
    std::vector<std::mt19937_64> rands;
    for (int i = 0; i < threadNumInGame; i++)
      rands.push_back(std::mt19937_64(rand()));

    std::vector<std::thread> threads;
    for (int i = 0; i < threadNumInGame; ++i) {
      threads.push_back(std::thread(

        [this, i, samplingNumEveryThread, &game, &rands, action]() {
          evaluateSingleActionThread(
            i,
            NNresultBuf.data() + samplingNumEveryThread * i,
            game,
            samplingNumEveryThread,
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
      NNresultBuf.data(),
      game,
      samplingNumEveryThread,
      rand,
      action
    );
  }


  double rf = adjustRadicalFactor(param.maxRadicalFactor, game.turn);

  //整合所有结果
  for (int i = 0; i < MAX_SCORE; i++)
    finalScoreDistribution[i] = 0;
  for (int i = 0; i < param.samplingNum; i++)
  {
    addNormDistribution(NNresultBuf[i].scoreMean, NNresultBuf[i].scoreStdev);
  }

  double N = 0;//总样本量
  double scoreTotal = 0;//score的和
  double scoreSqrTotal = 0;//score的平方和
  //double winNum = 0;//score>=target的次数期望

  double valueWeightTotal = 0;//sum(n^p*x[n]),x[n] from small to big
  double valueTotal = 0;//sum(n^p)
  double totalNinv = 1.0 / (param.samplingNum * NormDistributionSampling);
  for (int s = 0; s < MAX_SCORE; s++)
  {
    double n = finalScoreDistribution[s]; //当前分数的次数
    double r = (N + 0.5 * n) * totalNinv; //当前分数的排名比例
    N += n;
    scoreTotal += n * s;
    scoreSqrTotal += n * s * s;

    //按排名加权平均
    double w = pow(r, rf);
    valueWeightTotal += w * n;
    valueTotal += w * n * s;
  }

  ModelOutputValueV1 v;
  v.scoreMean = scoreTotal / N;
  v.scoreStdev = sqrt(scoreSqrTotal * N - scoreTotal * scoreTotal) / N;
  v.value = valueTotal / valueWeightTotal;
  return v;
}

void Search::evaluateSingleActionThread(int threadIdx, ModelOutputValueV1* resultBuf, const Game& game, int samplingNum, std::mt19937_64& rand, Action action)
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

    for (int depth = 0; depth < param.maxDepth; depth++)
    {
      eva.evaluateSelf(1, param);//计算policy
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
    eva.evaluateSelf(0, param);//计算value
    for (int i = 0; i < batchSize; i++)
    {
      resultBuf[batch * batchSize + i] = eva.valueResults[i];
    }

  }
}

void Search::addNormDistribution(double mean, double stdev)
{
  for (int i = 0; i < NormDistributionSampling; i++)
  {
    int y = int(mean + stdev * normDistributionCdfInv[i] + 0.5);
    if (y < 0)y = 0;
    if (y >= MAX_SCORE)y = MAX_SCORE - 1;
    finalScoreDistribution[y] += 1;
  }
}
