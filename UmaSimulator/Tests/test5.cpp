//测试训练属性值算法
#include <iostream>
#include <random>
#include <sstream>
#include <cassert>
#include "../Game/Game.h"
#include "../NeuralNet/Evaluator.h"
#include "../External/termcolor.hpp"
using namespace std;


void main_test5()
{
  random_device rd;
  auto rand = mt19937_64(rd());
  int umaId = 1;
  int cards[6] = { 1,2,3,4,5,6 };
  int zhongmaBlue[5] = { 18,0,0,0,0 };
  int zhongmaBonus[6] = { 30,0,30,0,30,200 };

  double totalScore = 0;
  double totalScoreSqr = 0;
  int bestScore = 0;
  int n = 0;
  for (int gamenum = 0; gamenum < 10000; gamenum++)
  {

    Game game;
    game.newGame(rand, false, umaId, cards, zhongmaBlue, zhongmaBonus);
    while(!game.isEnd())
    {

      bool useVenus = false;
      int chosenSpiritColor = -1;
      int chosenTrain = -1;
      int chosenOutgoing = -1;

      auto policy = Evaluator::handWrittenPolicy(game);

      {
        float bestPolicy = 0.001;
        for (int i = 0; i < 8;i++)
        {
          float p = policy.trainingPolicy[i];
          if (p > bestPolicy)
          {
            chosenTrain = i;
            bestPolicy = p;
          }
        }
      }
      useVenus = policy.useVenusPolicy >= 0.5;
      {
        float bestPolicy = 0.001;
        for (int i = 0; i < 3; i++)
        {
          float p = policy.threeChoicesEventPolicy[i];
          if (p > bestPolicy)
          {
            chosenSpiritColor = i;
            bestPolicy = p;
          }
        }
      }
      {
        float bestPolicy = 0.001;
        for (int i = 0; i < 6; i++)
        {
          float p = policy.outgoingPolicy[i];
          if (p > bestPolicy)
          {
            chosenOutgoing = i;
            bestPolicy = p;
          }
        }
      }


      game.applyTrainingAndNextTurn(rand, chosenTrain, useVenus, chosenSpiritColor, chosenOutgoing);

    }
    //cout << termcolor::red << "育成结束！" << termcolor::reset << endl;
    int score = game.finalScore();
    n += 1;
    totalScore += score;
    totalScoreSqr += score * score;
    if (score > bestScore)bestScore = score;
    //game.print();
    //game.printFinalStats();
  }
  cout << n << "局，平均分" << totalScore / n << "，标准差" << sqrt(totalScoreSqr / n - totalScore * totalScore / n / n) << "，最高分" << bestScore << endl;

}