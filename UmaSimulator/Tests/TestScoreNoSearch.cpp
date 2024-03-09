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

void main_testScoreNoSearch()
{
#if USE_BACKEND == BACKEND_LIBTORCH
  const string modelpath = "../training/example/model_traced.pt";
#elif USE_BACKEND == BACKEND_NONE
  const string modelpath = "";
#else
  const string modelpath = "../training/example/model.txt";
#endif

  const int threadNum = 8;
  int batchsize = 16;
  const double radicalFactor = 5;//激进度
  TestConfig test;

  // 检查工作目录
  GameDatabase::loadTranslation("./db/text_data.json");
  GameDatabase::loadUmas("./db/umaDB.json");
  GameDatabase::loadDBCards("./db/cardDB.json");

  test = TestConfig::loadFile("./testConfig.json");  
  cout << test.explain() << endl;

  test.totalGames = ((test.totalGames - 1) / (threadNum * batchsize) + 1) * threadNum * batchsize;
  SearchParam searchParam = { test.totalGames,TOTAL_TURN,radicalFactor };

  cout << "线程数：" << threadNum << "   batchsize：" << batchsize << endl;
  cout << "神经网络文件：" << modelpath << "   局数：" << test.totalGames << "（已对batchsize*线程数取整）" << endl;

  cout << "正在测试……\033[?25l" << endl;
  random_device rd;
  auto rand = mt19937_64(rd());

  Model* modelptr = NULL;
  Model model(modelpath, batchsize);
  if (modelpath != "")
  {
    modelptr = &model;
  }

  Search search(modelptr, batchsize, threadNum, searchParam);

  Game game;
  game.newGame(rand, false, test.umaId, test.umaStars, &test.cards[0], &test.zhongmaBlue[0], &test.zhongmaBonus[0]);
  
  auto start = std::chrono::high_resolution_clock::now();
  auto value = search.evaluateNewGame(game, test.totalGames, radicalFactor, rand);
  auto stop = std::chrono::high_resolution_clock::now();

  cout << "平均分数=\033[1;32m" << int(value.scoreMean) << " \033[0m" << "胡局分数=\033[1;32m" << int(value.value) << "\033[0m " << "标准差=\033[1;32m" << int(value.scoreStdev) << "\033[0m  " << endl;


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

  vector<int> scoreGameCount(MAX_SCORE);
  int c = 0;
  for (int s = MAX_SCORE - 1; s >= 0; s--)
  {
    c += search.allActionResults[0].finalScoreDistribution[s];
    scoreGameCount[s] = c;
  }
  //assert(c == test.totalGames * Search::NormDistributionSampling);

  for (int j = 0; j < nRanks; ++j)
  {  
    cout << termcolor::bright_cyan << rankNames[j] << "概率: " << float(scoreGameCount[ranks[j] * 100]) / c * 100 << "%"
      << termcolor::reset << endl;
  }









  // 计算持续时间
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  double duration_s = 0.000001 * duration.count();
  // 输出持续时间
  std::cout << "用时: " << duration_s << " s, 每秒 " << test.totalGames / duration_s << " 局" << std::endl;

  system("pause");
}