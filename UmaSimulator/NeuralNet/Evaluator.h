#pragma once
#include <vector>
#include "NNInput.h"
#include "Model.h"
#include "../Game/Game.h"

//每个线程一个evaluator
//所有线程共用一个model
class Evaluator
{
public:
  Model* model;
  //static lock;//所有的evaluator共用一个lock
  int maxBatchsize;

  std::vector<Game> gameInput;

  std::vector<float> inputBuf;
  std::vector<float> outputBuf;

  std::vector<ModelOutputValueV1> valueResults;
  //std::vector<ModelOutputPolicyV1> policyResults;
  std::vector<Action> actionResults;


  //void evaluate(const Game* games, const float* targetScores, int mode, int gameNum);//计算games中gameNum局游戏的输出。如果没有model，就使用手写逻辑计算policy，但不能计算非结束状态的value
  void evaluateSelf(int mode, float targetScore);//计算gameInput的输出。如果没有model，就使用手写逻辑计算policy，但不能计算非结束状态的value
  
  Evaluator();
  Evaluator(Model* model, int maxBatchsize);

  static Action handWrittenStrategy(const Game& game);
};