#include <random>
#include <cassert>
#include <iostream>
#include <sstream>
#include <thread>
#include <atomic>
#include <future>
#include <iostream>
#include <queue>
#include "Search.h"
#include "../GameDatabase/GameConfig.h"
#include "../External/mathFunctions.h"
using namespace std;

const ModelOutputValueV1 ModelOutputValueV1::illegalValue = { 1e-5,0,1e-5 };

const double Search::searchFactorStage[searchStageNum] = { 0.25,0.25,0.5 };
const double Search::searchThreholdStdevStage[searchStageNum] = { 4,4,0 };//4个标准差，比较保守

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

Action Search::intToTwoStageAction(int i)
{
  if (i < 21)
    return Action::intToAction(i);
  else if (i < 21 + 8)
  {
    Action a;
    a.dishType = DISH_sandwich;
    a.train = i - 21;
    return a;
  }
  else if (i < 21 + 8 + 8)
  {
    Action a;
    a.dishType = DISH_curry;
    a.train = i - 21 - 8;
    return a;
  }
  assert(false);
  return Action();
}

Search::Search(Model* model, int batchSize, int threadNumInGame):threadNumInGame(threadNumInGame), batchSize(batchSize)
{
  evaluators.resize(threadNumInGame);
  for (int i = 0; i < threadNumInGame; i++)
    evaluators[i] = Evaluator(model, batchSize);

  allActionResults.resize(Action::MAX_TWOSTAGE_ACTION_TYPE);
  for (int i = 0; i < Action::MAX_TWOSTAGE_ACTION_TYPE; i++)
    allActionResults[i].clear();

  param.searchSingleMax = 0;
}

Search::Search(Model* model, int batchSize, int threadNumInGame, SearchParam param0) :Search(model, batchSize, threadNumInGame)
{
  setParam(param0);
}
void Search::setParam(SearchParam param0)
{
  param = param0;

  //让searchGroupSize是整batch
  param.searchGroupSize = calculateRealSearchN(param.searchGroupSize);
  param.searchSingleMax = calculateRealSearchN(param.searchSingleMax);

  //让param.samplingNum是整batch
  //int batchEveryThread = (param.samplingNum - 1) / (threadNumInGame * batchSize) + 1;//相当于向上取整
  //if (batchEveryThread <= 0)batchEveryThread = 1;
  //int samplingNumEveryThread = batchSize * batchEveryThread;
  //param.samplingNum = threadNumInGame * samplingNumEveryThread;
  //NNresultBuf.resize(param.samplingNum);
}



Action Search::runSearch(const Game& game,
  std::mt19937_64& rand, bool twoStageSearchFirstYear)
{
  assert(param.searchSingleMax > 0 && "Search.param not initialized");

  rootGame = game;
  rootGame.playerPrint = false;
  double radicalFactor = adjustRadicalFactor(param.maxRadicalFactor, rootGame.turn);

  bool needTwoStageSearch =
    twoStageSearchFirstYear &&
    game.turn < 24 &&
    !game.isRacing &&
    game.cook_dish == DISH_none &&
    (game.isDishLegal(DISH_curry) || game.isDishLegal(DISH_sandwich));

  int maxActionType = needTwoStageSearch ? Action::MAX_TWOSTAGE_ACTION_TYPE : Action::MAX_ACTION_TYPE;

  //bool shouldContinueSearch[Action::MAX_ACTION_TYPE];
  for (int actionInt = 0; actionInt < maxActionType; actionInt++)
  {
    Action action = intToTwoStageAction(actionInt);

    allActionResults[actionInt].clear();
    bool islegal = false;
    if (needTwoStageSearch)
    {
      //二阶段搜索不单独做菜
      if (action.train == TRA_none)islegal = false;
      else if (action.dishType == DISH_none)islegal = rootGame.isLegal(action);
      else
        islegal = rootGame.isLegal(Action(action.dishType, TRA_none)) && rootGame.isLegal(Action(DISH_none, action.train));
    }
    else
      islegal = rootGame.isLegal(action);

    allActionResults[actionInt].isLegal = islegal;
    //shouldContinueSearch[actionInt] = allActionResults[actionInt].isLegal;
  }

  assert(param.searchGroupSize == calculateRealSearchN(param.searchGroupSize));//setParam应该处理过了
  int totalSearchN = 0;//到目前一共搜了多少

  //每个action先搜一组
  for (int actionInt = 0; actionInt < maxActionType; actionInt++)
  {
    if (!allActionResults[actionInt].isLegal)continue;
    Action action = intToTwoStageAction(actionInt);
    //cout << action.dishType << action.train << endl;
    searchSingleAction(param.searchGroupSize, rand, allActionResults[actionInt], action);
    totalSearchN += param.searchGroupSize;
  }

  //每次分配searchGroupSize的计算量到searchValue最大的那个action，直到达到searchSingleMax或searchTotalMax终止条件
  while (true)
  {
    if (param.searchGroupSize >= param.searchSingleMax)//前面搜索的一个group已经达到预定目标计算量，无需继续搜索
      break;

    double bestSearchValue = -1e4;
    int bestActionIntToSearch = -1;

    for (int actionInt = 0; actionInt < maxActionType; actionInt++)
    {
      if (!allActionResults[actionInt].isLegal)continue;
      double value = allActionResults[actionInt].getWeightedMeanScore(radicalFactor).value;
      double n = allActionResults[actionInt].num;
      assert(n > 0);
      double tn = double(totalSearchN);
      double policy = 1.0;//对于马娘，常数1就行，懒得在这里调用神经网络
      double searchValue = value + param.searchCpuct * policy * Search::expectedSearchStdev * sqrt(tn) / n;//抄的棋类ai的公式，平均分越高或计算量越少，searchValue越高
      if (searchValue > bestSearchValue)
      {
        bestSearchValue = searchValue;
        bestActionIntToSearch = actionInt;
      }
    }

    assert(bestActionIntToSearch >= 0);

    Action action = intToTwoStageAction(bestActionIntToSearch);
    searchSingleAction(param.searchGroupSize, rand, allActionResults[bestActionIntToSearch], action);
    totalSearchN += param.searchGroupSize;

    if (allActionResults[bestActionIntToSearch].num >= param.searchSingleMax)
      break;
    if (param.searchTotalMax > 0 && totalSearchN >= param.searchTotalMax)
      break;
  }

  //搜索完毕，找最高分的选项

  //把二阶段搜索整合到第一个阶段里
  if (needTwoStageSearch)
    integrateTwoStageResults();

  double bestValue = -1e4;
  int bestActionInt = -1;

  for (int actionInt = 0; actionInt < Action::MAX_ACTION_TYPE; actionInt++)
  {
    if (!allActionResults[actionInt].isLegal)continue;

    ModelOutputValueV1 value = allActionResults[actionInt].getWeightedMeanScore(radicalFactor);
    if (value.value > bestValue)
    {
      bestValue = value.value;
      bestActionInt = actionInt;
    }

  }

  Action bestAction = Action::intToAction(bestActionInt);
  assert(rootGame.isLegal(bestAction));
  return bestAction;
}

void Search::printSearchResult(bool showSearchNum)
{
  for (int actionInt = 0; actionInt < Action::MAX_ACTION_TYPE; actionInt++)
  {
    Action action = Action::intToAction(actionInt);
    SearchResult& res = allActionResults[actionInt];
    if (!res.isLegal)continue;
    ModelOutputValueV1 value = res.getWeightedMeanScore(param.maxRadicalFactor);
    cout << action.toString() << ":" << int(value.value);
    if (showSearchNum)
      cout << ", searchNum=" << res.num ;
    cout << endl;
  }
}

ModelOutputValueV1 Search::evaluateNewGame(const Game& game, std::mt19937_64& rand)
{
  rootGame = game;
  //param.maxDepth = TOTAL_TURN;
  //param.maxRadicalFactor = radicalFactor;
  //param.samplingNum = searchN;
  double radicalFactor = adjustRadicalFactor(param.maxRadicalFactor, game.turn);
  allActionResults[0].clear();
  allActionResults[0].isLegal = true;
  searchSingleAction(param.searchSingleMax, rand, allActionResults[0], Action::Action_RedistributeCardsForTest);
  return allActionResults[0].getWeightedMeanScore(adjustRadicalFactor(radicalFactor,game.turn));
}


void Search::searchSingleAction(
  int searchN,
  std::mt19937_64& rand,
  SearchResult& searchResult,
  Action action)
{
  //先检查action是否合法
  assert(action.train == TRA_redistributeCardsForTest || rootGame.isLegal(action)
    || (rootGame.isLegal(Action(action.dishType, TRA_none)) && rootGame.isLegal(Action(DISH_none, action.train))));

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
  bool isTwoStageAction = (action.dishType != DISH_none && action.train != TRA_none);
  bool isNewGame = action.train == TRA_redistributeCardsForTest;

  for (int batch = 0; batch < batchNum; batch++)
  {
      eva.gameInput.assign(batchSize, rootGame);

      //先走第一步
      for (int i = 0; i < batchSize; i++)
      {
          if (isNewGame)//重置游戏
              eva.gameInput[i].randomDistributeCards(rand);
          else//常规
              eva.gameInput[i].applyAction(rand, action);
      }
      int maxdepth = isNewGame ? param.maxDepth + 1 : param.maxDepth;
      for (int depth = 0; depth < param.maxDepth; depth++)
      {
          eva.evaluateSelf(1, param);//计算policy
          //bool distributeCards = (depth != maxDepth - 1);//最后一层就不分配卡组了，直接调用神经网络估值


          bool allFinished = true;
          for (int i = 0; i < batchSize; i++)
          {
              if (!eva.gameInput[i].isEnd())
                  eva.gameInput[i].applyAction(rand, eva.actionResults[i]);
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

void Search::integrateTwoStageResults()
{
  for (int dish = 1; dish <= 2; dish++)
  {
    if (!rootGame.isDishLegal(dish))
      continue;
    double bestValue = -1e6;
    int bestActionInt = -1;
    for (int tra = 0; tra < 8; tra++)
    {
      int actionInt = 21 - 8 + dish * 8 + tra;
      auto& res = allActionResults[actionInt];

      if (res.isLegal)
      {
        double value = res.getWeightedMeanScore(res.lastRadicalFactor).value;
        //cout << dish << tra << " " << value << endl;
        if (value > bestValue)
        {
          bestValue = value;
          bestActionInt = actionInt;
        }
      }
    }
    allActionResults[Action(dish, TRA_none).toInt()] = allActionResults[bestActionInt];
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
  upToDate = true;
  lastCalculate = ModelOutputValueV1::illegalValue;
}

void SearchResult::addResult(ModelOutputValueV1 v)
{
  upToDate = false;
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
  if (upToDate && lastRadicalFactor == radicalFactor)
    return lastCalculate;
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
  upToDate = true;
  lastRadicalFactor = radicalFactor;
  lastCalculate = v;
  return v;
}

int Search::calculateBatchNumEachThread(int searchN) const
{
  int batchEveryThread = (searchN - 1) / (threadNumInGame * batchSize) + 1;//相当于向上取整
  if (batchEveryThread <= 0)batchEveryThread = 1;
  return batchEveryThread;
}

int Search::calculateRealSearchN(int searchN) const
{
  return calculateBatchNumEachThread(searchN) * threadNumInGame * batchSize;
}
