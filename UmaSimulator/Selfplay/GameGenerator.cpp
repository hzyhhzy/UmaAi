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
  //先固定一个卡组，以后再加其他卡和马
  Game game;
  int umaId = 101101;//草上飞
  int umaStars = 5;
  int cards[6] = { 301604,301724,301614,301304,300114,300374 };//友人，智麦昆，速神鹰，根凯斯，根风神，根皇帝
  int zhongmaBlue[5] = { 18,0,0,0,0 };
  int zhongmaBonus[6] = { 10,10,30,0,10,70 };
  
  game.newGame(rand, false, umaId, umaStars, cards, zhongmaBlue, zhongmaBonus);
  std::exponential_distribution<double> expDistr(1.0);

  //给属性加随机
  int r = rand() % 100;
  if (r < 10)//随机扣属性
  {
    double mean = -expDistr(rand) * 20;
    for (int i = 0; i < 5; i++)
      game.addStatus(i, int(expDistr(rand) * mean));
    game.skillPt += int(2 * expDistr(rand) * mean);
  }
  else if (r < 80)//随机加属性，模拟胡局
  {
    double mean = expDistr(rand) * 30;
    for (int i = 0; i < 5; i++)
      game.addStatus(i, int(expDistr(rand) * mean));
    game.skillPt += int(2 * expDistr(rand) * mean);
  }
  if (game.skillPt < 0)game.skillPt = 0;

  //哪些debuff可以不消
  r = rand() % 100;
  if (r < 30)//
  {
    game.larc_allowedDebuffsFirstLarc[4] = true;
  }
  else if (r < 60)//
  {
    game.larc_allowedDebuffsFirstLarc[6] = true;
  }
  else if (r < 90)//
  {
    game.larc_allowedDebuffsFirstLarc[4] = true;
    game.larc_allowedDebuffsFirstLarc[6] = true;
  }


  if (rand() % 8 == 0)
    game.isQieZhe = true;
  if (rand() % 8 == 0)
    game.isAiJiao = true;

  game.motivation = rand() % 5 + 1;

  for (int i = 0; i < 6; i++)
  {
    int cardPerson = i < game.normalCardCount ? i : 17;
    if (rand() % 2)
    {
      int delta = int(expDistr(rand) * 5);
      game.addJiBan(cardPerson, delta);
    }
  }
  for (int i = 0; i < 5; i++)
  {
    if (rand() % 2)
    {
      int delta = int(expDistr(rand) * 1);
      game.addTrainingLevelCount(i, delta);
    }
  }

  if (rand() % 2)
  {
    int delta = int(expDistr(rand) * 4);
    game.maxVital += delta;
  }

  return game;
}

Game GameGenerator::randomizeBeforeOutput(const Game& game0)
{
  Game game = game0;
  std::exponential_distribution<double> expDistr(1.0);

  //给属性加随机
  int type0 = rand() % 100;
  if (type0 < 30)//随机扣属性
  {
    double mean = -expDistr(rand) * 30;
    for (int i = 0; i < 5; i++)
      game.addStatus(i, int(expDistr(rand) * mean));
    game.skillPt += int(2 * expDistr(rand) * mean);
  }
  else if (type0 < 80)//随机加属性，模拟胡局
  {
    double mean = expDistr(rand) * 50;
    for (int i = 0; i < 5; i++)
      game.addStatus(i, int(expDistr(rand) * mean));
    game.skillPt += int(2 * expDistr(rand) * mean);
  }
  if (game.skillPt < 0)game.skillPt = 0;

  if (rand() % 2 == 0)
  {
    int delta= expDistr(rand) * 10;
    delta *= 10;
    if (rand() % 2 == 0)
      delta = -delta;

    game.larc_shixingPt += delta;
    if (game.larc_shixingPt < 0)
      game.larc_shixingPt = 0;
  }

  return game;
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
