#pragma once
#include <vector>
#include "../Game/Game.h"
#include "../NeuralNet/Evaluator.h"
//一局游戏是一个search
class Search
{
public:
  int threadNumInGame;//一个search里面几个线程
  int batchSize;
  std::vector<Evaluator> evaluators;
  //Game rootGame;//当前要分析的局面


  //10对应action.train
  //非远征(turn<=35 || 43<=turn<=59)，allChoicesValue[0]有效，allChoicesValue[1~3]无效，allChoicesValue[0][action.train]
  //第一次远征尼尔赏前(36<=turn<=39)，allChoicesValue[0~3]有效，allChoicesValue[buy50 + 2*buyPt10][action.train]
  //第一次远征尼尔赏后(turn==41)，allChoicesValue[0~1]有效，allChoicesValue[buy50][action.train]，从这里开始买20%友情和10pt全自动
  //第二次远征(turn>=60)，allChoicesValue[0~3]有效，allChoicesValue[buy50 + 2*buyVital20][action.train]
  static int buyBuffChoiceNum(int turn);//有几种可能的买buff的方式
  static Action buyBuffAction(int idx, int turn);//四种买buff的方式
  ModelOutputValueV1 allChoicesValue[4][10];

  std::vector<ModelOutputValueV1> resultBuf;


  //对于每个可能的情况（最多40种），每个往后模拟eachSamplingNum局，模拟maxDepth回合后返回神经网络输出（神经网络输出是达到targetScore的概率）
  //关于多线程，暂定的方案如下：
  // 1.Model类的输入是已经整理好的float向量*batchsize，输出也是向量*batchsize。Model类有锁，同时只能计算一个
  // 2.对于evaluateSingleAction，把eachSamplingNum局游戏拆成threadNumInGame组，每组一个线程(Evaluator)。每个线程分成eachSamplingNum/(threadNumInGame*batchsize)小组，每小组batchsize局游戏，依次计算每个小组的分数，都计算完毕之后整合起来
  // 3.如果要跑很多局（例如跑谱），会同时跑threadGame局，总线程数为threadGame*threadNumInGame。若eachSamplingNum较小batchsize较大，可以让threadNumInGame=1
  // 嵌套结构：Search(threadGame个)->Evaluator(threadGame*threadNumInGame个)->Model(1个)
  
  Search(Model* model, int batchSize, int threadNumInGame);



  Action runSearch(const Game& game, 
    int samplingNum, int maxDepth, int targetScore,
    std::mt19937_64& rand);

  //计算单个action的数值
  ModelOutputValueV1 evaluateSingleAction(
    const Game& game,
    int samplingNum, int maxDepth, int targetScore,
    std::mt19937_64& rand,
    Action action);

  //计算单个action的数值，单个线程
  void evaluateSingleActionThread(
    int threadIdx,
    ModelOutputValueV1* resultBuf, //把每局的结果保存到这里
    const Game& game,
    int samplingNum, int maxDepth, int targetScore,

    std::mt19937_64& rand,
    Action action
  );

  //根据搜索结果选出最佳选择，policy也做一定的软化
  //mode=0是根据胜率，=1是根据平均分
  ModelOutputPolicyV1 extractPolicyFromSearchResults(int mode, float delta = 0);

};