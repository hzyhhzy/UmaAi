#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <cassert>
#include <thread>
#include <atomic>
#include <mutex>
#include <cmath>
#include "../Game/Game.h"
#include "../NeuralNet/Evaluator.h"
#include "../Search/Search.h"
#include "../External/termcolor.hpp"

#include "../GameDatabase/GameDatabase.h"
#include "../GameDatabase/GameConfig.h"
#include "../Tests/TestConfig.h"

using namespace std;

namespace TestScoreSearch
{
#if USE_BACKEND == BACKEND_LIBTORCH
  const string modelpath = "../training/example/model_traced.pt";
#elif USE_BACKEND == BACKEND_NONE
  const string modelpath = "";
#else
  const string modelpath = "../training/example/model.txt";
#endif

  const int threadNum = 1;
  const int batchsize = 1;
  const int threadNumInner = 8;
  const double radicalFactor = 3;//激进度
  const int searchDepth = 2 * TOTAL_TURN;
  const int searchN = 2048;
  const bool recordGame = true;
  const bool debugPrint = false;

  int totalGames = 100000;
  int gamesEveryThread = totalGames / threadNum;

  TestConfig test;
  /*
  //int umaId = 108401;//谷水，30力加成
  int umaId = 106501;//太阳神，15速15力加成
  int umaStars = 5;
  //int cards[6] = { 301604,301344,301614,300194,300114,301074 };//友人，高峰，神鹰，乌拉拉，风神，司机
  int cards[6] = { 301604,301724,301614,301304,300114,300374 };//友人，智麦昆，速神鹰，根凯斯，根风神，根皇帝

  int zhongmaBlue[5] = { 18,0,0,0,0 };
  int zhongmaBonus[6] = { 10,10,30,0,10,70 };
  bool allowedDebuffs[9] = { false, false, false, false, false, false, true, false, false };//第二年可以不消第几个debuff。第五个是智力，第七个是强心脏
  */
  std::atomic<double> totalScore = 0;
  std::atomic<double> totalScoreSqr = 0;

  std::atomic<int> bestScore = 0;
  std::atomic<int> n = 0;
  std::mutex printLock;
  vector<atomic<int>> segmentStats = vector<atomic<int>>(700);//100分一段，700段
  std::atomic<int> printThreshold = 2187;

  void worker()
  {
    random_device rd;
    auto rand = mt19937_64(rd());

    Model* modelptr = NULL;
    Model model(modelpath, batchsize);
    if (modelpath != "")
    {
      modelptr = &model;
    }

    SearchParam searchParam(searchN, radicalFactor);
    searchParam.maxDepth = searchDepth;
    //searchParam.searchCpuct = 10.0;
    Search search(modelptr, batchsize, threadNumInner, searchParam);

    vector<Game> gameHistory;

    //if (recordGame)
    //  gameHistory.resize(TOTAL_TURN);

    for (int gamenum = 0; gamenum < gamesEveryThread; gamenum++)
    {
      Game game;
      game.newGame(rand, false, test.umaId, test.umaStars, &test.cards[0], &test.zhongmaBlue[0], &test.zhongmaBonus[0]);


      while (!game.isEnd())
      {
        if (recordGame)
          gameHistory.push_back(game);
        Action action;
        action = search.runSearch(game, rand);
        if (debugPrint)
        {
          game.print();
          search.printSearchResult(true);
          cout<<action.toString()<<endl;
        }
        game.applyAction(rand, action);
      }
      //cout << termcolor::red << "育成结束！" << termcolor::reset << endl;
      int64_t score = game.finalScore();
      if (score >= 50000)
      {
        if (recordGame)
          for (int i = 0; i < gameHistory.size(); i++)
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
      if (score > bestScore + printThreshold)
      {
        if (printThreshold < 100)
        {
          std::lock_guard<std::mutex> lock(printLock);
          game.printFinalStats();
          //cout << printThreshold << endl;
          cout.flush();
        }
        printThreshold = printThreshold / 3;
      }

      while (score > bestScoreOld && !bestScore.compare_exchange_weak(bestScoreOld, score)) {
        // 如果val大于old_max，并且max_val的值还是old_max，那么就将max_val的值更新为val
        // 如果max_val的值已经被其他线程更新，那么就不做任何事情，并且old_max会被设置为max_val的新值
        // 然后我们再次进行比较和交换操作，直到成功为止
      }

      //game.print();
      game.printFinalStats();
      cout << endl << n << "局，搜索量=" << searchN << "，平均分" << totalScore / n << "，标准差" << sqrt(totalScoreSqr / n - totalScore * totalScore / n / n) << "，最高分" << bestScore << endl;
      //for (int i=0; i<400; ++i)
      //    cout << i*100 << ",";
      //cout << endl;
      //for (int i=0; i<400; ++i)
      //    cout << float(segmentStats[i]) / n << ",";
      //cout << endl;
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
        << "UC0概率=" << float(segmentStats[407]) / n << ","
        << "UC1概率=" << float(segmentStats[413]) / n << ","
        << "UC2概率=" << float(segmentStats[420]) / n << ","
        << "UC3概率=" << float(segmentStats[427]) / n << ","
        << "UC4概率=" << float(segmentStats[434]) / n << ","
        << "UC5概率=" << float(segmentStats[440]) / n << ","
        << "UC6概率=" << float(segmentStats[447]) / n << ","
        << "UC7概率=" << float(segmentStats[454]) / n << ","
        << "UC8概率=" << float(segmentStats[462]) / n << ","
        << "UC9概率=" << float(segmentStats[469]) / n << ","
        << "UB0概率=" << float(segmentStats[476]) / n << endl;
    }

  }

}
using namespace TestScoreSearch;
void main_testScoreSearch()
{
  // 检查工作目录
  GameDatabase::loadTranslation("./db/text_data.json");
  GameDatabase::loadUmas("./db/umaDB.json");
  GameDatabase::loadDBCards("./db/cardDB.json");

  test = TestConfig::loadFile("./testConfig.json");  
  cout << test.explain() << endl;
  totalGames = test.totalGames;
  gamesEveryThread = totalGames / threadNum;

  for (int i = 0; i < 700; i++)segmentStats[i] = 0;

  cout << "正在测试……\033[?25l" << endl;

  std::vector<std::thread> threads;
  for (int i = 0; i < threadNum; ++i) {
    threads.push_back(std::thread(worker));
  }
  for (auto& thread : threads) {
    thread.join();
  }

  cout << n << "局，搜索量=" << searchN << "，平均分" << totalScore / n << "，标准差" << sqrt(totalScoreSqr / n - totalScore * totalScore / n / n) << "，最高分" << bestScore << endl;
  system("pause");

}