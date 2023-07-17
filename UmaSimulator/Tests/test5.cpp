//测试训练属性值算法
#include <iostream>
#include <random>
#include <sstream>
#include <cassert>
#include "../Game/Game.h"
#include "../NeuralNet/Evaluator.h"
#include "../Search/Search.h"
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

  Search search;
  Evaluator evaluator(NULL, 16);
  int searchN = 2048;

  for (int gamenum = 0; gamenum < 100000; gamenum++)
  {

    Game game;
    game.newGame(rand, false, umaId, cards, zhongmaBlue, zhongmaBonus);
    while(!game.isEnd())
    {
      //auto policy = Evaluator::handWrittenPolicy(game);
      search.runSearch(game, evaluator, searchN, TOTAL_TURN, 27000);
      auto policy = search.extractPolicyFromSearchResults(1);
      Search::runOneTurnUsingPolicy(rand, game, policy, true);
    }
    //cout << termcolor::red << "育成结束！" << termcolor::reset << endl;
    int score = game.finalScore();
    n += 1;
    totalScore += score;
    totalScoreSqr += score * score;
    if (score > bestScore)bestScore = score;
    //game.print();
    game.printFinalStats();

    cout << n << "局，搜索量=" << searchN << "，平均分" << totalScore / n << "，标准差" << sqrt(totalScoreSqr / n - totalScore * totalScore / n / n) << "，最高分" << bestScore << endl;

  }
}