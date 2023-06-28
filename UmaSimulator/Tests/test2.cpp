//≤‚ ‘—µ¡∑ Ù–‘÷µÀ„∑®
#include <iostream>
#include <random>
#include "../Game/Game.h"
using namespace std;


void main_test2()
{
  random_device rd;
  auto rand = mt19937_64(rd);

  Game game;
  int cards[6] = { 1,2,3,6,11,10 };
  int zhongmaBlue[5] = { 18,0,0,0,0 };
  game.newGame(rand, 1, cards, zhongmaBlue);
  game.
}