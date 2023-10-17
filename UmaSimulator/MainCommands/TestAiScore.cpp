//测试训练属性值算法
#include <iostream>
#include <random>
#include <sstream>
#include <cassert>
#include <thread>
#include <atomic>
#include "../Game/Game.h"
#include "../NeuralNet/Evaluator.h"
#include "../Search/Search.h"
#include "../External/termcolor.hpp"

#include "../GameDatabase/GameDatabase.h"
#include "../GameDatabase/GameConfig.h"

using namespace std;

const bool handWrittenEvaluationTest = true;
const int threadNum = 8;
const int threadNumInner = 1;
const double targetScore = 0;
//const double radicalFactor = 3;//激进度
const int searchN = handWrittenEvaluationTest ? 1 : 1024;
const bool recordGame = false;


const int totalGames = handWrittenEvaluationTest ? 1000000 : 10000000;
const int gamesEveryThread = totalGames / threadNum;


int umaId = 108401;//谷水，30力加成
int umaStars = 5;
int cards[6] = { 301604,301344,301614,300194,300114,301074 };//友人，高峰，神鹰，乌拉拉，风神，司机
int zhongmaBlue[5] = { 18,0,0,0,0 };
int zhongmaBonus[6] = { 20,0,40,0,20,150 };
bool allowedDebuffs[9] = { false, false, false, false, true, false, false, false, false };//第二年可以不消第几个debuff。第五个是智力，第七个是强心脏


std::atomic<double> totalScore = 0;
std::atomic<double> totalScoreSqr = 0;

std::atomic<int> bestScore = 0;
std::atomic<int> n = 0;
vector<atomic<int>> segmentStats= vector<atomic<int>>(700);//100分一段，700段


void worker()
{
  random_device rd;
  auto rand = mt19937_64(rd());

  Search search(NULL, 16, threadNumInner);

  vector<Game> gameHistory;
  if (recordGame)
    gameHistory.resize(TOTAL_TURN);

  for (int gamenum = 0; gamenum < gamesEveryThread; gamenum++)
  {

    Game game;
    game.newGame(rand, false, umaId, umaStars, cards, zhongmaBlue, zhongmaBonus);
    for (int i = 0; i < 9; i++)
      game.larc_allowedDebuffsFirstLarc[i] = allowedDebuffs[i];

    while(!game.isEnd())
    {
      if (recordGame)
        gameHistory[game.turn] = game;
      Action action;
      if (handWrittenEvaluationTest) {
        action = Evaluator::handWrittenStrategy(game);
      }
      else {
        action = search.runSearch(game, searchN, TOTAL_TURN, targetScore, rand);
      }
      game.applyTrainingAndNextTurn(rand, action);
    }
    //cout << termcolor::red << "育成结束！" << termcolor::reset << endl;
    int score = game.finalScore();
    if (score > 39200)
    {
      if (recordGame)
        for (int i = 0; i < TOTAL_TURN; i++)
          if (!GameConstants::LArcIsRace[i])
            gameHistory[i].print();
      game.printFinalStats();
    }
    n += 1;
    totalScore += score;
    totalScoreSqr += score * score;
    for (int i = 0; i < 700; i++)
    {
      int refScore = i * 100;
      if (score >= refScore)
      {
        segmentStats[i] += 1;
      }
    }

    int bestScoreOld = bestScore;
    while (score > bestScoreOld && !bestScore.compare_exchange_weak(bestScoreOld, score)) {
      // 如果val大于old_max，并且max_val的值还是old_max，那么就将max_val的值更新为val
      // 如果max_val的值已经被其他线程更新，那么就不做任何事情，并且old_max会被设置为max_val的新值
      // 然后我们再次进行比较和交换操作，直到成功为止
    }


    //game.print();
    if (!handWrittenEvaluationTest || n == totalGames)
    {
      if(!handWrittenEvaluationTest)
        game.printFinalStats();
      cout << n << "局，搜索量=" << searchN << "，平均分" << totalScore / n << "，标准差" << sqrt(totalScoreSqr / n - totalScore * totalScore / n / n) << "，最高分" << bestScore << endl;
      cout
        << "UE7概率=" << float(segmentStats[327]) / n << ","
        << "UE8概率=" << float(segmentStats[332]) / n << ","
        << "UE9概率=" << float(segmentStats[338]) / n << ","
        << "UD0概率=" << float(segmentStats[344]) / n << ","
        << "UD1概率=" << float(segmentStats[350]) / n << ","
        << "UD2概率=" << float(segmentStats[356]) / n << ","
        << "UD3概率=" << float(segmentStats[362]) / n << ","
        << "UD4概率=" << float(segmentStats[368]) / n << ","
        << "UD5概率=" << float(segmentStats[375]) / n << ","
        << "UD6概率=" << float(segmentStats[381]) / n << ","
        << "UD7概率=" << float(segmentStats[387]) / n << ","
        << "UD8概率=" << float(segmentStats[394]) / n << ","
        << "UD9概率=" << float(segmentStats[400]) / n << ","
        << "UC0概率=" << float(segmentStats[407]) / n << endl;
    }
  }

}

void main_testAiScore()
{


    // 检查工作目录
    GameDatabase::loadUmas("../db/uma");
    GameDatabase::loadCards("../db/card");
    GameDatabase::loadDBCards("../db/cardDB.json");


  for (int i = 0; i < 200; i++)segmentStats[i] = 0;


  std::vector<std::thread> threads;
  for (int i = 0; i < threadNum; ++i) {
    threads.push_back(std::thread(worker));
  }
  for (auto& thread : threads) {
    thread.join();
  }

  cout << n << "局，搜索量=" << searchN << "，平均分" << totalScore / n << "，标准差" << sqrt(totalScoreSqr / n - totalScore * totalScore / n / n) << "，最高分" << bestScore << endl;

}