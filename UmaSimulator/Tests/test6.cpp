#include <iostream>
#include <iomanip> 
#include <sstream>
#include <fstream>
#include <cassert>
#include <thread>  // for std::this_thread::sleep_for
#include <chrono>  // for std::chrono::seconds
#include "../Game/Game.h"
#include "../Search/Search.h"
using namespace std;


void main_test6()
{
  const int threadNum = 16;
  const int searchN = 8192;
  Search search;
  vector<Evaluator> evaluators;
  for (int i = 0; i < threadNum; i++)
    evaluators.push_back(Evaluator(NULL, 128));
  int lastTurn = -1;
  while (true)
  {
    ifstream fs("./packets/currentGS.json");
    ostringstream tmp;
    tmp << fs.rdbuf();
    fs.close();

    string jsonStr = tmp.str();
    Game game;
    bool suc = game.loadGameFromJson(jsonStr);
    if (!suc)
    {
      cout << "error" << endl;
      continue;
    }
    if (game.turn == lastTurn)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(300));//检查是否有更新
      continue;
    }
    lastTurn = game.turn;
    if (game.venusIsWisdomActive)
      continue;
    game.print();
    cout << endl;
    cout << "计算中..." << endl;
    cout << endl;

    auto printPolicy = [](float p)
    {
      cout << fixed << setprecision(1);
      if (p >= 0.2)cout << "\033[32m";
      else cout << "\033[36m";
      cout << p * 100 << "% ";
      cout << "\033[0m";
    };

    search.runSearch(game, evaluators.data(), searchN, TOTAL_TURN, 27000, threadNum);
    cout << "计算完毕" << endl;
    {
      auto policy = search.extractPolicyFromSearchResults(1);
      if (game.venusAvailableWisdom != 0)
      {
        cout << "使用女神率：";
        printPolicy(policy.useVenusPolicy * 100);
        cout << endl;
      }

      cout << "速耐力根智：";
      for (int i = 0; i < 5; i++)
        printPolicy(policy.trainingPolicy[i]);
      cout << endl;

      cout << "休息，外出，比赛：";
      for (int i = 0; i < 3; i++)
        printPolicy(policy.trainingPolicy[5 + i]);
      cout << endl;

      cout << "红，蓝，黄：";
      for (int i = 0; i < 3; i++)
        printPolicy(policy.threeChoicesEventPolicy[i]);
      cout << endl;

      cout << "五个女神外出以及普通外出：";
      for (int i = 0; i < 6; i++)
        printPolicy(policy.outgoingPolicy[i]);
      cout << endl;
    }

    float maxScore = -10000;
    for (int i = 0; i < 2; i++)
    {
      for (int j = 0; j < 8 + 4 + 6; j++)
      {
        float s = search.allChoicesValue[i][j].avgScoreMinusTarget;
        if (s > maxScore)maxScore = s;
      }
    }

    cout << "期望分数（未考虑技能打折和已买技能）：\033[31m" << maxScore << "\033[0m" << endl;
    cout << "比赛亏损（用于选择比赛回合，以完成粉丝数目标）：\033[31m" << maxScore - std::max(search.allChoicesValue[0][7].avgScoreMinusTarget, search.allChoicesValue[1][7].avgScoreMinusTarget) << "\033[0m" << endl;



    cout << endl;
    cout << "各选项的期望分数：" << endl;
    for (int i = 0; i < 2; i++)
    {
      for (int j = 0; j < 8 + 4 + 6; j++)
      {
        cout
          //<< fixed << setprecision(1) << search.allChoicesValue[i][j].winrate * 100 << "%:" 
          << fixed << setprecision(0) << search.allChoicesValue[i][j].avgScoreMinusTarget << " ";
        if (j == 4 || j == 7 || j == 11)cout << endl;
      }
      cout << endl;
      cout << endl;
    }

  }

}