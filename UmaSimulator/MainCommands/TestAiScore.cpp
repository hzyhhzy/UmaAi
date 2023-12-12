//测试训练属性值算法
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <cassert>
#include <thread>
#include <atomic>
#include <mutex>
#include <cmath>
#include <map>
#include "../Game/Game.h"
#include "../NeuralNet/Evaluator.h"
#include "../Search/Search.h"
#include "../External/termcolor.hpp"

#include "../GameDatabase/GameDatabase.h"
#include "../GameDatabase/GameConfig.h"
#include "../Tests/TestConfig.h"
#include "../NeuralNet/Evaluator.h"

using namespace std;

// 仅测试手动分数，不进行NN测试
namespace TestAiScore
{
  const int threadNum = 8;
  const int threadNumInner = 1;
  const int nRanks = 25;
  const int ranks[] = { 273, 278, 283, 288, 294,
                  299, 304, 310, 315, 321,
                  327, 332, 338, 344, 350,
                  356, 362, 368, 375, 381,
                  387, 394, 400, 407, 413 };
  const string rankNames[] = { "UF7", "UF8", "UF9", "UE", "UE1",
                         "UE2", "UE3", "UE4", "UE5", "UE6",
                         "UE7", "UE8", "UE9", "UD", "UD1",
                         "UD2", "UD3", "UD4", "UD5", "UD6",
                         "UD7", "UD8", "UD9", "UC", "UC1" };
  const double radicalFactor = 5;//激进度
  //const bool recordGame = false;
  int totalGames = 100000;
  int batchSize = 256;

  // 手动指定马娘
#if USE_BACKEND != BACKEND_NONE
  const bool handWrittenEvaluationTest = false;
  const int searchN = 2048;
  int gamesEveryThread = (int)ceil((double)totalGames / threadNum / batchSize);
#else
  const bool handWrittenEvaluationTest = true;
  const int searchN = 1;
  int gamesEveryThread = totalGames / threadNum;
#endif
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
  SearchParam searchParam = { searchN,TOTAL_TURN,radicalFactor };
  TestConfig test;

  std::atomic<double> totalScore = 0;
  std::atomic<double> totalScoreSqr = 0;
  std::atomic<int> bestScore = 0;
  std::atomic<int> n = 0;
  std::mutex printLock;
  vector<atomic<int>> segmentStats = vector<atomic<int>>(500);//100分一段，500段
  map<int, GameResult> segmentSample;

  Model* getModel()
  {
    static bool firstCall = true;
    Model* ret = nullptr;
#if USE_BACKEND == BACKEND_LIBTORCH
    //ret = new Model("../training/example/model_traced.pt", batchSize);
    ret = new Model("db/model_traced.pt", batchSize);
#elif USE_BACKEND != BACKEND_NONE
    //ret = new Model("../training/example/model_traced.pt", batchSize);
    ret = new Model("db/model.txt", batchSize);
#endif
    std::unique_lock<std::mutex> lock(printLock, std::defer_lock);
    if (lock.try_lock() && firstCall)
    {
       Model::printBackendInfo();
       firstCall = false;
       lock.unlock();
    }
    return ret;
  }

  GameResult getResult(const Game& game)
  {
    GameResult ret;
    for (int i = 0; i < 5; ++i)
      ret.fiveStatus[i] = (int)game.fiveStatus[i];
    ret.finalScore = game.finalScore();
    ret.fiveStatusScore = 0;
    ret.skillPt = game.skillPt;
    for (int i = 0; i < 5; ++i)
      ret.fiveStatusScore += GameConstants::FiveStatusFinalScore[min(game.fiveStatus[i], game.fiveStatusLimit[i])];
    return ret;
  }

  void printProgress(int value, int maxValue, int width)
  {
    stringstream buf;
    double rate = clamp((double)value / maxValue, 0.0, 1.0);
    int n = rate * width;
    buf << "[" << string(n, '=') << ">" << string(width - n, ' ') << "] " << setprecision((int)(2 + rate)) << rate * 100 << "%   ";

    std::lock_guard<std::mutex> lock(printLock);    // 返回时自动释放cout锁
    cout << buf.str() << "\033[0F" << endl;
    cout.flush();
  }

  void monteCarloWorker()
  {
    random_device rd;
    auto rand = mt19937_64(rd());
    Model* modelptr = getModel();
    Search search(nullptr, batchSize, threadNumInner, searchParam);
    printProgress(0, totalGames, 70);
    /*
    vector<Game> gameHistory;
    if (recordGame)
      gameHistory.resize(TOTAL_TURN);
    */
    for (int gamenum = 0; gamenum < gamesEveryThread; gamenum++)
    {
      Game game;
      game.newGame(rand, false, test.umaId, test.umaStars, &test.cards[0], &test.zhongmaBlue[0], &test.zhongmaBonus[0]);
      game.eventStrength = test.eventStrength;
      for (int i = 0; i < 9; i++)
        game.larc_allowedDebuffsFirstLarc[i] = test.allowedDebuffs[i];

      while (!game.isEnd())
      {
       // if (recordGame)
       //   gameHistory[game.turn] = game;
        Action action;
        if (handWrittenEvaluationTest) {
          action = Evaluator::handWrittenStrategy(game);
        }
        else {
          action = search.runSearch(game, rand);
        }
        game.applyTrainingAndNextTurn(rand, action);
      }
      //cout << termcolor::red << "育成结束！" << termcolor::reset << endl;
      GameResult result = getResult(game);
      int score = result.finalScore;
      /*
      if (score > 42000)
      {
        if (recordGame)
          for (int i = 0; i < TOTAL_TURN; i++)
            if (!GameConstants::LArcIsRace[i])
              gameHistory[i].print();
        game.printFinalStats();
      }
      */
      n += 1;
      printProgress(n, totalGames, 70);
      totalScore += score;
      totalScoreSqr += score * score;
      for (int i = 0; i < 700; i++)
      {
        int refScore = i * 100;
        if (score >= refScore)
        {
          segmentStats[i] += 1;
        }
        if (score >= refScore && score < refScore + 100 && segmentSample.count(refScore) == 0)
          segmentSample[i] = result;    // 每隔100分记录一局属性
      }
      int bestScoreOld = bestScore;
      while (score > bestScoreOld && !bestScore.compare_exchange_weak(bestScoreOld, score)) {
          // 如果val大于old_max，并且max_val的值还是old_max，那么就将max_val的值更新为val
          // 如果max_val的值已经被其他线程更新，那么就不做任何事情，并且old_max会被设置为max_val的新值
          // 然后我们再次进行比较和交换操作，直到成功为止
      }

    }

  } // worker()

  void NNWorker()
  {
    random_device rd;
    auto rand = mt19937_64(rd());
    Model* modelptr = getModel();
    Search search(modelptr, batchSize, threadNumInner, searchParam);
    printProgress(0, totalGames, 70);

    for (int gamenum = 0; gamenum < gamesEveryThread; gamenum++)
    {
        Game game;
        game.newGame(rand, false, test.umaId, test.umaStars, &test.cards[0], &test.zhongmaBlue[0], &test.zhongmaBonus[0]);
        game.eventStrength = test.eventStrength;
        for (int i = 0; i < 9; i++)
            game.larc_allowedDebuffsFirstLarc[i] = test.allowedDebuffs[i];
   
        Action action = { 8,false,false,false,false };//无条件外出，这样就无视第一回合的人头分布了
        auto value = search.evaluateSingleAction(game, rand, action);
        for (Evaluator ev : search.evaluators)
            for (Game g : ev.gameInput)
            {
                GameResult result = getResult(g);
                int score = result.finalScore;
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
                    if (score >= refScore && score < refScore + 100 && segmentSample.count(refScore) == 0)
                        segmentSample[i] = result;    // 每隔100分记录一局属性
                }
                int bestScoreOld = bestScore;
                while (score > bestScoreOld && !bestScore.compare_exchange_weak(bestScoreOld, score));
            }
        printProgress(n, totalGames, 70);
    }
    if (modelptr)
        delete modelptr;    // sanity
  } // worker()
}

using namespace TestAiScore;

void main_testAiScore()
{
  // 检查工作目录
  
  GameDatabase::loadTranslation("../db/text_data.json");
  GameDatabase::loadUmas("../db/umaDB.json");
  GameDatabase::loadDBCards("../db/cardDB.json");
  test = TestConfig::loadFile("../ConfigTemplate/testConfig.json");  
  
  // 独立测卡工具直接使用当前目录
  /*
  GameDatabase::loadTranslation("db/text_data.json");
  GameDatabase::loadUmas("db/umaDB.json");
  GameDatabase::loadDBCards("db/cardDB.json");
  test = TestConfig::loadFile("testConfig.json");  
  */
  cout << test.explain() << endl;
  cout << "正在测试……\033[?25l" << endl;
  for (int i = 0; i < 200; i++)segmentStats[i] = 0;
  std::vector<std::thread> threads;
  totalGames = test.totalGames;

#if USE_BACKEND != BACKEND_NONE
  gamesEveryThread = (int)ceil((double)totalGames / threadNum / batchSize);
  for (int i = 0; i < threadNum; ++i) {
      threads.push_back(std::thread(NNWorker));
  }
#else
  gamesEveryThread = totalGames / threadNum;
  for (int i = 0; i < threadNum; ++i) {
      threads.push_back(std::thread(monteCarloWorker));
  }
#endif

  for (auto& thread : threads)
      thread.join();

  cout << endl << n << "局，搜索量=" << searchN << "，平均分" << totalScore / n << "，标准差" << sqrt(totalScoreSqr / n - totalScore * totalScore / n / n) << "，最高分" << bestScore << endl;

  for (int j = 0; j < nRanks; ++j)
      if (ranks[j] * 100 + 800 > totalScore / n && segmentStats[ranks[j]] > 0)
      {
          int k = 0;
          while (k < 6 && segmentSample.count(ranks[j] + k) == 0) k++;
          cout << "--------" << endl;
          cout << termcolor::bright_cyan << rankNames[j] << "概率: " << float(segmentStats[ranks[j]]) / n * 100 << "%"
              << termcolor::reset << " | 参考属性: " << segmentSample[ranks[j] + k].explain() << endl;
      }
  system("pause");
}
