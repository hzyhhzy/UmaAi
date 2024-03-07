#include <cassert>
#include <iostream>
#include "Evaluator.h"
#include "../Search/Search.h"

//void Evaluator::evaluate(const Game* games, const float* targetScores, int mode, int gameNum)
//{
//  assert(false);
//}

void Evaluator::evaluateSelf(int mode, const SearchParam& param)
{
  if (model == NULL)//没神经网络，手写逻辑
  {
    if (mode == 0)//value，必须终局才可计算
    {
      for (int i = 0; i < maxBatchsize; i++)
      {
        const Game& game = gameInput[i];
        assert(game.isEnd() && "无神经网络时，只有游戏结束后才可计算value");
        int score = game.finalScore();

        auto& v = valueResults[i];
        v.scoreMean = score;
        v.scoreStdev = 0; //单个已终局的样本，方差必为0
        v.value = score;
      }
    }
    else if (mode == 1)//policy，手写逻辑，最优的选择是1，其他的是0
    {
      for (int i = 0; i < maxBatchsize; i++)
      {
        actionResults[i] = handWrittenStrategy(gameInput[i]);
      }
    }
  }
  else
  {
    for (int i = 0; i < maxBatchsize; i++)
    {
      const Game& game = gameInput[i];
      if(!game.isEnd())
        game.getNNInputV1(inputBuf.data() + i * NNINPUT_CHANNELS_V1, param);
    }
    model->evaluate(this, inputBuf.data(), outputBuf.data(), maxBatchsize);
    if (mode == 0)//value
    {
      for (int i = 0; i < maxBatchsize; i++)
      {
        const Game& game = gameInput[i];
        if (game.isEnd())
        {
          int score = game.finalScore();

          auto& v = valueResults[i];
          v.scoreMean = score;
          v.scoreStdev = 0; //单个已终局的样本，方差必为0
          v.value = score;
        }
        else
        {
          valueResults[i] = extractValueFromNNOutputBuf(outputBuf.data() + NNOUTPUT_CHANNELS_V1 * i);
        }
      }
    }
    else if (mode == 1)//policy
    {
      for (int i = 0; i < maxBatchsize; i++)
      {
        const Game& game = gameInput[i];
        if (game.isEnd())
        {
        }
        else
        {
          actionResults[i] = extractActionFromNNOutputBuf(outputBuf.data() + NNOUTPUT_CHANNELS_V1 * i, game);
        }
      }
    }
    else assert(false);
  }
}

ModelOutputValueV1 Evaluator::extractValueFromNNOutputBuf(float* buf)
{
  ModelOutputValueV1 v;
  v.scoreMean = 30000 + 200 * buf[NNOUTPUT_CHANNELS_POLICY_V1 + 0];
  v.scoreStdev = 100 * buf[NNOUTPUT_CHANNELS_POLICY_V1 + 1]; 
  v.value = 30000 + 200 * buf[NNOUTPUT_CHANNELS_POLICY_V1 + 2];
  return v;
}

Action Evaluator::extractActionFromNNOutputBuf(float* buf, const Game& game)
{
  Action action;
  assert(false && "todo");
  return action;
}

Evaluator::Evaluator()
{
}

Evaluator::Evaluator(Model* model, int maxBatchsize):model(model), maxBatchsize(maxBatchsize)
{
  gameInput.resize(maxBatchsize);
  inputBuf.resize(NNINPUT_CHANNELS_V1 * maxBatchsize);
  outputBuf.resize(NNOUTPUT_CHANNELS_V1 * maxBatchsize);
  valueResults.resize(maxBatchsize);
  //policyResults.resize(maxBatchsize);
  actionResults.resize(maxBatchsize);

#if USE_BACKEND == BACKEND_CUDA
  inputBufOnesIdx.resize(NNINPUT_MAX_ONES * maxBatchsize);
  inputBufFloatIdx.resize(NNINPUT_MAX_FLOAT * maxBatchsize);
  inputBufFloatValue.resize(NNINPUT_MAX_FLOAT * maxBatchsize);
#endif
}





Action Evaluator::handWrittenStrategy(const Game& game)
{
  assert(false && "todo");
  return Action();
}

