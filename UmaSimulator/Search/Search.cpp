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

const ModelOutputValueV1 ModelOutputValueV1::illegalValue = { 1e-5,0,1e-5 };

const double Search::searchFactorStage[searchStageNum] = { 0.25,0.25,0.5 };
const double Search::searchThreholdStdevStage[searchStageNum] = { 6,5,0 };//5个标准差，非常保守

double SearchResult::normDistributionCdfInv[NormDistributionSampling];

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
  double remainTurns = TOTAL_TURN - turn;
  double factor = pow(remainTurns / TOTAL_TURN, 0.5);
  return factor * maxRf;
}

Search::Search(Model* model, int batchSize, int threadNumInGame):threadNumInGame(threadNumInGame), batchSize(batchSize)
{
  evaluators.resize(threadNumInGame);
  for (int i = 0; i < threadNumInGame; i++)
    evaluators[i] = Evaluator(model, batchSize);

  allActionResults.resize(Action::MAX_ACTION_TYPE);
  for (int i = 0; i < Action::MAX_ACTION_TYPE; i++)
    allActionResults[i].clear();

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
  //int batchEveryThread = (param.samplingNum - 1) / (threadNumInGame * batchSize) + 1;//相当于向上取整
  //if (batchEveryThread <= 0)batchEveryThread = 1;
  //int samplingNumEveryThread = batchSize * batchEveryThread;
  //param.samplingNum = threadNumInGame * samplingNumEveryThread;
  //NNresultBuf.resize(param.samplingNum);
}



Action Search::runSearch(const Game& game,
  std::mt19937_64& rand)
{
  assert(param.samplingNum >= 0 && "Search.param not initialized");

  rootGame = game;
  double radicalFactor = adjustRadicalFactor(param.maxRadicalFactor, rootGame.turn);


  bool shouldContinueSearch[Action::MAX_ACTION_TYPE];
  for (int actionInt = 0; actionInt < Action::MAX_ACTION_TYPE; actionInt++)
  {
    Action action = Action::intToAction(actionInt);

    allActionResults[actionInt].clear();
    allActionResults[actionInt].isLegal = rootGame.isLegal(action);
    shouldContinueSearch[actionInt] = allActionResults[actionInt].isLegal;
  }

  double bestValue = -1e4;
  Action bestAction = { -1,0 };
  for (int stage = 0; stage < searchStageNum; stage++)
  {
    bestValue = -1e4; 
    bestAction = { -1,0 };
    int searchN = searchFactorStage[stage] * param.samplingNum;

    for (int actionInt = 0; actionInt < Action::MAX_ACTION_TYPE; actionInt++)
    {
      if (!shouldContinueSearch[actionInt])continue;
      Action action = Action::intToAction(actionInt);

      searchSingleAction(searchN, rand, allActionResults[actionInt], action);
      ModelOutputValueV1 value = allActionResults[actionInt].getWeightedMeanScore(radicalFactor);//同时也保存到了allActionResults[actionInt].lastCalculate里
      if (value.value > bestValue)
      {
        bestValue = value.value;
        bestAction = action;
      }

    }

    double stdev = double(expectedSearchStdev) / sqrt(double(searchN));
    double continueSearchThrehold = bestValue - searchThreholdStdevStage[stage] * stdev;

    for (int actionInt = 0; actionInt < Action::MAX_ACTION_TYPE; actionInt++)
    {
      if (allActionResults[actionInt].lastCalculate.value < continueSearchThrehold)//比最高分选项低了好几个标准差，无需继续计算
        shouldContinueSearch[actionInt] = false;
    }

  }



  assert(rootGame.isLegal(bestAction));
  return bestAction;
}

ModelOutputValueV1 Search::evaluateNewGame(const Game& game, int searchN, double radicalFactor, std::mt19937_64& rand)
{
  rootGame = game;
  param.maxDepth = TOTAL_TURN;
  param.maxRadicalFactor = radicalFactor;
  param.samplingNum = searchN;
  allActionResults[0].clear();
  allActionResults[0].isLegal = true;
  searchSingleAction(searchN, rand, allActionResults[0], Action::Action_RedistributeCardsForTest);
  return allActionResults[0].getWeightedMeanScore(radicalFactor);
}


void Search::searchSingleAction(
  int searchN,
  std::mt19937_64& rand,
  SearchResult& searchResult,
  Action action)
{
  //先检查action是否合法
  assert(action.train == TRA_redistributeCardsForTest || rootGame.isLegal(action));

  int batchNumEachThread = calculateBatchNumEachThread(searchN);
  searchN = calculateRealSearchN(searchN);
  if (NNresultBuf.size() < searchN) NNresultBuf.resize(searchN);

  int samplingNumEveryThread = batchNumEachThread * batchSize;

  if (threadNumInGame > 1)
  {
    std::vector<std::mt19937_64> rands;
    for (int i = 0; i < threadNumInGame; i++)
      rands.push_back(std::mt19937_64(rand()));

    std::vector<std::thread> threads;
    for (int i = 0; i < threadNumInGame; ++i) {
      threads.push_back(std::thread(

        [this, i, batchNumEachThread, samplingNumEveryThread, &rands, action]() {
          searchSingleActionThread(
            i,
            NNresultBuf.data() + samplingNumEveryThread * i,
            batchNumEachThread,
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
  else //single thread for debug/speedtest
  {
    searchSingleActionThread(
      0,
      NNresultBuf.data(),
      batchNumEachThread,
      rand,
      action
    );
  }




  for (int i = 0; i < searchN; i++)
  {
    searchResult.addResult(NNresultBuf[i]);
  }

}

void Search::searchSingleActionThread(
  int threadIdx,
  ModelOutputValueV1* resultBuf,
  int batchNum,

  std::mt19937_64& rand,
  Action action
)
{
  Evaluator& eva = evaluators[threadIdx];
  assert(eva.maxBatchsize == batchSize);

  bool isNewGame = action.train == TRA_redistributeCardsForTest;

  for (int batch = 0; batch < batchNum; batch++)
  {
    eva.gameInput.assign(batchSize, rootGame);

    //先走第一步
    for (int i = 0; i < batchSize; i++)
    {
      if(!isNewGame)//ai计算
        eva.gameInput[i].applyTrainingAndNextTurn(rand, action);
      else//重置游戏
        eva.gameInput[i].randomDistributeCards(rand);
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


void SearchResult::initNormDistributionCdfTable()
{
  //正态分布累积分布函数的反函数在0~1上均匀取点
  for (int i = 0; i < NormDistributionSampling; i++)
  {
    double x = (i + 0.5) / NormDistributionSampling;
    normDistributionCdfInv[i] = normalCDFInverse(x);
  }
}

void SearchResult::clear()
{
  isLegal = false;
  num = 0;
  for (int i = 0; i < MAX_SCORE; i++)
    finalScoreDistribution[i] = 0;
  lastCalculate = ModelOutputValueV1::illegalValue;
}

void SearchResult::addResult(ModelOutputValueV1 v)
{
  num += 1;
  for (int i = 0; i < NormDistributionSampling; i++)
  {
    int y = int(v.scoreMean + v.scoreStdev * normDistributionCdfInv[i] + 0.5);
    if (y < 0)y = 0;
    if (y >= MAX_SCORE)y = MAX_SCORE - 1;
    finalScoreDistribution[y] += 1;
  }
}

ModelOutputValueV1 SearchResult::getWeightedMeanScore(double radicalFactor) 
{
  if (!isLegal)
  {
    lastCalculate = ModelOutputValueV1::illegalValue;
    return ModelOutputValueV1::illegalValue;
  }
  double N = 0;//总样本量
  double scoreTotal = 0;//score的和
  double scoreSqrTotal = 0;//score的平方和
  //double winNum = 0;//score>=target的次数期望

  double valueWeightTotal = 0;//sum(n^p*x[n]),x[n] from small to big
  double valueTotal = 0;//sum(n^p)
  double totalNinv = 1.0 / (num * NormDistributionSampling);
  for (int s = 0; s < MAX_SCORE; s++)
  {
    double n = finalScoreDistribution[s]; //当前分数的次数
    double r = (N + 0.5 * n) * totalNinv; //当前分数的排名比例
    N += n;
    scoreTotal += n * s;
    scoreSqrTotal += n * s * s;

    //按排名加权平均
    double w = pow(r, radicalFactor);
    valueWeightTotal += w * n;
    valueTotal += w * n * s;
  }

  ModelOutputValueV1 v;
  v.scoreMean = scoreTotal / N;
  v.scoreStdev = sqrt(scoreSqrTotal * N - scoreTotal * scoreTotal) / N;
  v.value = valueTotal / valueWeightTotal;
  lastCalculate = v;
  return v;
}

int Search::calculateBatchNumEachThread(int searchN) const
{
  int batchEveryThread = (param.samplingNum - 1) / (threadNumInGame * batchSize) + 1;//相当于向上取整
  if (batchEveryThread <= 0)batchEveryThread = 1;
  return batchEveryThread;
}

int Search::calculateRealSearchN(int searchN) const
{
  return calculateBatchNumEachThread(searchN) * threadNumInGame * batchSize;
}
