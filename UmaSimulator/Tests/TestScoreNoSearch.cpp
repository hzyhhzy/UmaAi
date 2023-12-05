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
  const string modelpath = "../training/example/model_traced.pt";
  //const string modelpath = "";
  const int threadNum = 8;
  const double radicalFactor = 5;//激进度
  TestConfig test;

  // 检查工作目录
  GameDatabase::loadTranslation("../db/text_data.json");
  GameDatabase::loadUmas("../db/umaDB.json");
  GameDatabase::loadDBCards("../db/cardDB.json");

  test = TestConfig::loadFile("../ConfigTemplate/testConfig.json");  
  cout << test.explain() << endl;

  SearchParam searchParam = { test.totalGames,TOTAL_TURN,radicalFactor };

  cout << "神经网络文件：" << modelpath << "   局数：" << test.totalGames << endl;

  cout << "正在测试……\033[?25l" << endl;
  random_device rd;
  auto rand = mt19937_64(rd());

  int batchsize = 1024;
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
  auto value = search.evaluateSingleAction(game, rand, action);

  cout << "平均分数=\033[1;32m" << int(value.scoreMean) << " \033[0m" << "胡局分数=\033[1;32m" << int(value.value) << "\033[0m " << "标准差=\033[1;32m" << int(value.scoreStdev) << "\033[0m  " << endl;

  system("pause");
}