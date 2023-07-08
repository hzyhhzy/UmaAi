//≤‚ ‘—µ¡∑ Ù–‘÷µÀ„∑®
#include <iostream>
#include <random>
#include <sstream>
#include "../Game/Game.h"
#include "../External/termcolor.hpp"
using namespace std;


void main_test3()
{

  random_device rd;
  auto rand = mt19937_64(rd());
  int cards[6] = { 1,2,3,4,5,6 };
  int zhongmaBlue[5] = { 18,0,0,0,0 };
  int zhongmaBonus[6] = { 30,0,30,0,200 };
  Game game;
  game.newGame(rand, true, 1, cards, zhongmaBlue, zhongmaBonus);
  game.turn = 40;
  for (int i = 0; i < 6; i++)
    game.cardJiBan[i] = 100;
  game.venusLevelYellow = 5;
  game.venusLevelRed = 5;
  game.venusLevelBlue = 5;
  for (int i = 0; i < 5; i++)
    game.trainLevelCount[i] = 48;
  game.motivation = 5;
  game.venusCardUnlockOutgoing = true;
  game.venusCardIsQingRe = true;
  for (int i = 0; i < 100; i++)
  {
    game.randomDistributeCards(rand);
    game.calculateTrainingValue();
    game.print();
  }
}