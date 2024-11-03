#pragma once
#include <vector>
#include "SearchParam.h"
#include "../Game/Game.h"
#include "../NeuralNet/Evaluator.h"
#include "../NeuralNet/TrainingSample.h"

struct SearchResult
{
  static const int NormDistributionSampling = 128;//统计分数分布时，每个正态分布拆成多少个样本
  static double normDistributionCdfInv[NormDistributionSampling];//正态分布累积分布函数的反函数在0~1上均匀取点
  static void initNormDistributionCdfTable();//初始化

  //Action action;
  bool isLegal;
  int num;
  int32_t finalScoreDistribution[MAX_SCORE];//某个action的最终分数分布预测
  void clear();
  void addResult(ModelOutputValueV1 v); //O(N*NormDistributionSampling)
  ModelOutputValueV1 getWeightedMeanScore(double radicalFactor);//slow, O(MAXSCORE), avoid frequently call

  ModelOutputValueV1 lastCalculate;//上次调用getWeightedMeanScore的计算结果
  bool upToDate;//lastCalculate是否已过时
  double lastRadicalFactor;//上次计算的radicalFactor
};


//一局游戏是一个search
class Search
{
public:
  // 现在希望获得最终分数的分布
  // 使用神经网络进行蒙特卡洛时，如果depth<TOTAL_TURN，没有搜索到终局。这时让神经网络返回预测平均值mean和标准差stdev
  // 对于蒙特卡洛的所有样本，把每个样本视为正态分布，并在正态分布中取NormDistributionSampling个点，加在finalScoreDistribution上
  // 之后对finalScoreDistribution进行处理，计算整体的ModelOutputValueV1中的各项参数
  
  
  //对于每个action先搜searchFactorStage[0]比例的计算量
  //如果某个action的分数相比最高分低了searchThreholdStdevStage个标准差，则排除掉这个选项
  //没被排除的action进行第二轮搜索，计算量是searchFactorStage[1]
  //stage数不能太多，因为每次每个action都要计算getWeightedMeanScore()
  static const int expectedSearchStdev = 2200;
  static const int searchStageNum = 3;
  static const double searchFactorStage[searchStageNum];
  static const double searchThreholdStdevStage[searchStageNum];
  


  Game rootGame;//当前或刚搜索完的是哪个局面
  int threadNumInGame;//一个search里面几个线程
  int batchSize;
  SearchParam param;
  std::vector<Evaluator> evaluators;

  std::vector<SearchResult> allActionResults;//所有可能的选择的打分

  //对于每个可能的情况，每个往后模拟eachSamplingNum局，模拟maxDepth回合后返回神经网络输出（神经网络输出是平均分、标准差、乐观分）
  //关于多线程，暂定的方案如下：
  // 1.Model类的输入是已经整理好的float向量*batchsize，输出也是向量*batchsize。Model类有锁，同时只能计算一个
  // 2.对于evaluateSingleAction，把eachSamplingNum局游戏拆成threadNumInGame组，每组一个线程(Evaluator)。每个线程分成eachSamplingNum/(threadNumInGame*batchsize)小组，每小组batchsize局游戏，依次计算每个小组的分数，都计算完毕之后整合起来
  // 3.如果要跑很多局（例如跑谱），会同时跑threadGame局，总线程数为threadGame*threadNumInGame。若eachSamplingNum较小batchsize较大，可以让threadNumInGame=1
  // 嵌套结构：Search(threadGame个)->Evaluator(threadGame*threadNumInGame个)->Model(1个)

  Search(Model* model, int batchSize, int threadNumInGame);
  Search(Model* model, int batchSize, int threadNumInGame, SearchParam param0);

  void setParam(SearchParam param0);

  Action runSearch(const Game& game,
    std::mt19937_64& rand, bool twoStageSearchFirstYear = true);//对于当前局面，计算每个选项的分数并返回最优选项, twoStageSearchFirstYear是第一年搜索吃菜之后是否再搜索训练

  void printSearchResult(bool showSearchNum);//打印搜索结果

  ModelOutputValueV1 evaluateNewGame(const Game& game,
    std::mt19937_64& rand);//直接从第一回合开始蒙特卡洛，用于测试卡组强度或者ai强度

  //对单个action进行蒙特卡洛，并将结果加到allActionResults里
  void searchSingleAction(
    int searchN,
    std::mt19937_64& rand,
    SearchResult& searchResult,
    Action action);

  //根据搜索结果选出最佳选择，policy也做一定的软化
  //mode=0是根据胜率，=1是根据平均分
  //ModelOutputPolicyV1 extractPolicyFromSearchResults(int mode, float delta = 0);

  //导出上次搜索的数据作为训练样本
  TrainingSample exportTrainingSample(double policyDelta = 100);//policyDelta是policy的软化系数


private:

  std::vector<ModelOutputValueV1> NNresultBuf;

  int calculateBatchNumEachThread(int searchN) const;//每个线程多少batch
  int calculateRealSearchN(int searchN) const;//对线程数*batchsize取整后的计算量
  //计算单个action的数值，单个线程。把每局的结果保存到resultBuf里。先不往allActionResults里加
  void searchSingleActionThread(
    int threadIdx,
    ModelOutputValueV1* resultBuf, 
    int batchNum,

    std::mt19937_64& rand,
    Action action
  );

};