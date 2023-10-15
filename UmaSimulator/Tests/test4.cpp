//测试不同人头数的概率
#include <iostream>
#include <random>
#include <sstream>
#include <cassert>
#include "../Game/Game.h"
#include "../External/termcolor.hpp"
using namespace std;


void main_test4()
{

  random_device rd;
  auto rand = mt19937_64(rd());
  int umaId = 1;
  int cards[6] = { 1,2,3,4,5,6 };
  int zhongmaBlue[5] = { 18,0,0,0,0 };
  int zhongmaBonus[6] = { 30,0,30,0,0,200 };

  double totalScore = 0;
  double totalScoreSqr = 0;
  int bestScore = 0;
  int n = 0;
  int stat[7] = { 0,0,0,0,0,0,0 };
  Game game;
  game.newGame(rand, false, umaId, 5, cards, zhongmaBlue, zhongmaBonus);
  game.turn = 50;
  for (int gamenum = 0; gamenum < 10000000; gamenum++)
  {
    game.randomDistributeCards(rand);

    int maxHead = 0;
    for (int item = 0; item < 5; item++)
    {
      int h = 0;
      for (int j = 0; j < 5; j++)
      {
        if (game.personDistribution[item][j]!=-1)
          h++;
      }
      if (h > maxHead)maxHead = h;
    }
    //game.print();
    //cout << maxHead << endl;
    stat[maxHead]++;

  }
  for (int j = 0; j < 7; j++)
  {
    cout << stat[j] << endl;
  }

}