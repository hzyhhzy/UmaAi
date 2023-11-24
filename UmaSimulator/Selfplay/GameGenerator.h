#pragma once
#include <vector>
#include "SelfplayParam.h"
#include "../Game/Game.h"
#include "../NeuralNet/Evaluator.h"
class GameGenerator
{
  //先随机生成一些开局，然后随机往后进行一些回合数，储存在gameBuf，在发出来之前再加随机性
  SelfplayParam param;
  Evaluator evaluator;
  
  std::mt19937_64 rand;
  std::vector<Game> gameBuf;
  int nextGamePointer;

  Game randomOpening();
  Game randomizeBeforeOutput(const Game& game0);
  void newGameBatch();
  bool isVaildGame(const Game& game);
public:
  GameGenerator(SelfplayParam param, Model* model);
  Game get();
};