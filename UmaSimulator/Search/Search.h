#pragma once
#include <vector>
#include "SearchParam.h"
#include "../Game/Game.h"
#include "../NeuralNet/Evaluator.h"
#include "../NeuralNet/TrainingSample.h"
//一局游戏是一个search
class Search
{
  // 现在希望获得最终分数的分布
  // 使用神经网络进行蒙特卡洛时，如果depth<TOTAL_TURN，没有搜索到终局。这时让神经网络返回预测平均值mean和标准差stdev
  // 对于蒙特卡洛的所有样本，把每个样本视为正态分布，并在正态分布中取NormDistributionSampling个点，加在finalScoreDistribution上
  // 之后对finalScoreDistribution进行处理，计算整体的ModelOutputValueV1中的各项参数

  
  static const int NormDistributionSampling = 128;//统计分数分布时，每个正态分布拆成多少个样本

public:
  Game gameLastSearch;//上次搜索的是哪个局面
  int threadNumInGame;//一个search里面几个线程
  int batchSize;
  SearchParam param;
  //Game rootGame;//当前要分析的局面


  //10对应action.train
  //非远征(turn<=35 || 43<=turn<=59)，allChoicesValue[0]有效，allChoicesValue[1~3]无效，allChoicesValue[0][action.train]
  //第一次远征尼尔赏前(36<=turn<=39)，allChoicesValue[0~3]有效，allChoicesValue[buy50 + 2*buyPt10][action.train]
  //第一次远征尼尔赏后(turn==41)，allChoicesValue[0~1]有效，allChoicesValue[buy50][action.train]，从这里开始买20%友情和10pt全自动
  //第二次远征(turn>=60)，allChoicesValue[0~3]有效，allChoicesValue[buy50 + 2*buyVital20][action.train]
  static int buyBuffChoiceNum(int turn);//有几种可能的买buff的方式
  static Action buyBuffAction(int idx, int turn);//四种买buff的方式
  ModelOutputValueV1 allChoicesValue[4][10];//所有可能的选择的打分



  //对于每个可能的情况（最多40种），每个往后模拟eachSamplingNum局，模拟maxDepth回合后返回神经网络输出（神经网络输出是达到targetScore的概率）
  //关于多线程，暂定的方案如下：
  // 1.Model类的输入是已经整理好的float向量*batchsize，输出也是向量*batchsize。Model类有锁，同时只能计算一个
  // 2.对于evaluateSingleAction，把eachSamplingNum局游戏拆成threadNumInGame组，每组一个线程(Evaluator)。每个线程分成eachSamplingNum/(threadNumInGame*batchsize)小组，每小组batchsize局游戏，依次计算每个小组的分数，都计算完毕之后整合起来
  // 3.如果要跑很多局（例如跑谱），会同时跑threadGame局，总线程数为threadGame*threadNumInGame。若eachSamplingNum较小batchsize较大，可以让threadNumInGame=1
  // 嵌套结构：Search(threadGame个)->Evaluator(threadGame*threadNumInGame个)->Model(1个)
  
  Search(Model* model, int batchSize, int threadNumInGame, SearchParam param0);



  Action runSearch(const Game& game, 
    std::mt19937_64& rand);

  //计算单个action的数值
  ModelOutputValueV1 evaluateSingleAction(
    const Game& game,
    std::mt19937_64& rand,
    Action action);


  //根据搜索结果选出最佳选择，policy也做一定的软化
  //mode=0是根据胜率，=1是根据平均分
  //ModelOutputPolicyV1 extractPolicyFromSearchResults(int mode, float delta = 0);

  //导出上次搜索的数据作为训练样本
  TrainingSample exportTrainingSample(float policyDelta = 50);//policyDelta是policy的软化系数
  
private:
  std::vector<Evaluator> evaluators;
  std::vector<ModelOutputValueV1> NNresultBuf;

  int32_t finalScoreDistribution[MAX_SCORE];//某个action的最终分数分布预测
  double normDistributionCdfInv[NormDistributionSampling];//正态分布累积分布函数的反函数在0~1上均匀取点



  //计算单个action的数值，单个线程
  void evaluateSingleActionThread(
    int threadIdx,
    ModelOutputValueV1* resultBuf, //把每局的结果保存到这里
    const Game& game,
    int samplingNum,

    std::mt19937_64& rand,
    Action action
  );

  void addNormDistribution(double mean, double stdev);//在finalScoreDistribution中加上平均值为mean，标准差为stdev的正态分布，总权重为NormDistributionSampling

};