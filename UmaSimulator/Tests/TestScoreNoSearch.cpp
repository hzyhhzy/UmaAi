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
  int batchsize = 2048;
  const double radicalFactor = 5;//激进度


  TestConfig test;




  // 检查工作目录
  GameDatabase::loadTranslation("../db/text_data.json");
  GameDatabase::loadUmas("../db/umaDB.json");
  GameDatabase::loadDBCards("../db/cardDB.json");

  test = TestConfig::loadFile("../ConfigTemplate/testConfig.json");  
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
  for (int i = 0; i < 9; i++)
    game.larc_allowedDebuffsFirstLarc[i] = test.allowedDebuffs[i];
  Action action = { 8,false,false,false,false };//无条件外出，这样就无视第一回合的人头分布了

  auto start = std::chrono::high_resolution_clock::now();
  auto value = search.evaluateSingleAction(game, rand, action);
  auto stop = std::chrono::high_resolution_clock::now();

  cout << "平均分数=\033[1;32m" << int(value.scoreMean) << " \033[0m" << "胡局分数=\033[1;32m" << int(value.value) << "\033[0m " << "标准差=\033[1;32m" << int(value.scoreStdev) << "\033[0m  " << endl;

  // 计算持续时间
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  double duration_s = 0.000001 * duration.count();
  // 输出持续时间
  std::cout << "用时: " << duration_s << " s, 每秒 " << test.totalGames / duration_s << " 局" << std::endl;

  system("pause");

}