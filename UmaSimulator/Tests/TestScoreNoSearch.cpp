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
  const string modelpath = "./db/model_traced.pt";
  const int threadNum = 16;
  int batchsize = 1024;
#elif USE_BACKEND == BACKEND_NONE
  const string modelpath = "";
  const int threadNum = 8;
  int batchsize = 1;
#elif USE_BACKEND == BACKEND_ONNX
  const string modelpath = "./db/model.onnx";
  const int threadNum = 4;
  int batchsize = 1024;
#else 
  const string modelpath = "./db/model.txt";
  const int threadNum = 16;
  int batchsize = 1024;
#endif

  const double radicalFactor = 5;//������
  TestConfig test;

  // ��鹤��Ŀ¼
  GameDatabase::loadTranslation("./db/text_data.json");
  GameDatabase::loadUmas("./db/umaDB.json");
  GameDatabase::loadDBCards("./db/cardDB.json");

  test = TestConfig::loadFile("./testConfig.json");  
  cout << test.explain() << endl;

  test.totalGames = ((test.totalGames - 1) / (threadNum * batchsize) + 1) * threadNum * batchsize;
  SearchParam searchParam(test.totalGames, radicalFactor);

  cout << "�߳�����" << threadNum << "   batchsize��" << batchsize << endl;
  cout << "�������ļ���" << modelpath << "   ������" << test.totalGames << "���Ѷ�batchsize*�߳���ȡ����" << endl;

  cout << "���ڲ��ԡ���\033[?25l" << endl;
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
  auto value = search.evaluateNewGame(game, rand);
  auto stop = std::chrono::high_resolution_clock::now();

  cout << "ƽ������=\033[1;32m" << int(value.scoreMean) << " \033[0m" << "���ַ���=\033[1;32m" << int(value.value) << "\033[0m " << "��׼��=\033[1;32m" << int(value.scoreStdev) << "\033[0m  " << endl;


  const int nRanks = 44;
  const int ranks[] = { 273, 278, 283, 288, 294,
                  299, 304, 310, 315, 321,
                  327, 332, 338, 344, 350,
                  356, 362, 368, 375, 381,
                  387, 394, 400, 
    407,413,420,427,434,440,447,454,462,469,
    476,483,490,498,505,513,520,528,536,544,552
  };
  const string rankNames[] = { "UF7", "UF8", "UF9", "UE", "UE1",
                         "UE2", "UE3", "UE4", "UE5", "UE6",
                         "UE7", "UE8", "UE9", "UD", "UD1",
                         "UD2", "UD3", "UD4", "UD5", "UD6",
                         "UD7", "UD8", "UD9",
    "UC", "UC1", "UC2", "UC3", "UC4", "UC5", "UC6", "UC7", "UC8", "UC9",
    "UB",
    "UB1",
    "UB2",
    "UB3",
    "UB4",
    "UB5",
    "UB6",
    "UB7",
    "UB8",
    "UB9",
    "UA"
  };

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
    cout << termcolor::bright_cyan << rankNames[j] << "����: " << float(scoreGameCount[ranks[j] * 100]) / c * 100 << "%"
      << termcolor::reset << endl;
  }









  // �������ʱ��
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  double duration_s = 0.000001 * duration.count();
  // �������ʱ��
  std::cout << "��ʱ: " << duration_s << " s, ÿ�� " << test.totalGames / duration_s << " ��" << std::endl;

  system("pause");
}