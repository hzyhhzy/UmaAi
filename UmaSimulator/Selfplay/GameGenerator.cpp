#include "GameGenerator.h"
#include "../Search/SearchParam.h"
using namespace std;

GameGenerator::GameGenerator(SelfplayParam param, Model* model) :param(param)
{
  evaluator = Evaluator(model, param.batchsize);
  std::random_device rd;
  rand = std::mt19937_64(rd());
  gameBuf.resize(param.batchsize);
  nextGamePointer = param.batchsize;
}

Game GameGenerator::randomOpening()
{
  assert(false);
  return Game();
}

Game GameGenerator::randomizeBeforeOutput(const Game& game)
{
  assert(false);
  return Game();
}

void GameGenerator::newGameBatch()
{
  evaluator.gameInput.resize(param.batchsize);
  for (int i = 0; i < param.batchsize; i++)
  {
    evaluator.gameInput[i] = randomOpening();
  }
  //往后进行一些回合
  //std::normal_distribution<double> norm(0.0, 1.0);
  int maxTurn = TOTAL_TURN - 8;//7个比赛回合，因此最多训练TOTAL_TURN - 7次
  int gameTurn = rand() % maxTurn;
  SearchParam defaultSearchParam = { 1024,TOTAL_TURN,5.0 };//这个参数随意取，只用于生成开局时输入神经网络
  for (int depth = 0; depth < gameTurn; depth++)
  {
    evaluator.evaluateSelf(1, defaultSearchParam);//计算policy


    for (int i = 0; i < param.batchsize; i++)
    {
      assert(!evaluator.gameInput[i].isEnd());//以后的剧本如果难以保证这个，可以删掉这个assert
      evaluator.gameInput[i].applyTrainingAndNextTurn(rand, evaluator.actionResults[i]);
      assert(isVaildGame(evaluator.gameInput[i]));//以后的剧本如果难以保证这个，可以删掉这个assert
    }
  }
  for (int i = 0; i < param.batchsize; i++)
  {
    gameBuf[i] = randomizeBeforeOutput(evaluator.gameInput[i]);
  }
}
bool GameGenerator::isVaildGame(const Game& game)
{
  if (game.isEnd())return false;
  if (game.isRacing)return false;
  return true;
}

Game GameGenerator::get()
{
  while (true)
  {
    if (nextGamePointer >= param.batchsize)
    {
      nextGamePointer = 0;
      newGameBatch();
    }
    if (!isVaildGame(gameBuf[nextGamePointer]))
    {
      nextGamePointer += 1;
      continue;
    }
    else
    {
      nextGamePointer += 1;
      return gameBuf[nextGamePointer - 1];
    }
  }
}
