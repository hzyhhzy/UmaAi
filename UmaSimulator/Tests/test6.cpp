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
  const int threadNum = 12;
  const int searchN = 6144;
  Search search;
  vector<Evaluator> evaluators;
  for (int i = 0; i < threadNum; i++)
    evaluators.push_back(Evaluator(NULL, 128));
  int lastTurn = -1;
  int scoreFirstTurn = 0;   // 第一回合分数
  int scoreLastTurn = 0;   // 上一回合分数
  while (true)
  {
    ifstream fs("./packets/currentGS.json");
    if (!fs.good())
    {
      cout << "等待游戏开始" << endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(3000));//延迟几秒，避免刷屏
      continue;
    }
    ostringstream tmp;
    tmp << fs.rdbuf();
    fs.close();

    string jsonStr = tmp.str();
    Game game;
    bool suc = game.loadGameFromJson(jsonStr);
    if (!suc)
    {
      cout << "出现错误" << endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(3000));//延迟几秒，避免刷屏
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

    if (game.turn == 0)//第一回合
    {
      scoreFirstTurn = 0;
      scoreLastTurn = 0;
    }
    game.print();
    cout << endl;
    cout << "计算中..." << endl;
    cout << endl;

    auto printPolicy = [](float p)
    {
      cout << fixed << setprecision(1);
      if (p >= 0.3)cout << "\033[33m";
      //else if (p >= 0.1)cout << "\033[32m";
      else cout << "\033[36m";
      cout << p * 100 << "% ";
      cout << "\033[0m";
    };

    search.runSearch(game, evaluators.data(), searchN, TOTAL_TURN, 0, threadNum);
    cout << "计算完毕" << endl;
    cout << ">>" << endl;
    {
      auto policy = search.extractPolicyFromSearchResults(1);
      if (game.venusAvailableWisdom != 0)
      {
        cout << "使用女神率：";
        printPolicy(policy.useVenusPolicy);
        cout << endl;
      }
      if (!game.isRacing)
      {
        if (game.venusAvailableWisdom != 0)
        {
          cout << "在" << (policy.useVenusPolicy > 0.5 ? "" : "\033[31m不\033[0m") << "使用女神的前提下：";
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
    }

    float maxScore = -1e6;
    for (int i = 0; i < 2; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        float s = search.allChoicesValue[i][j].avgScoreMinusTarget;
        if (s > maxScore)maxScore = s;
      }
    }
    if(game.turn==0)
    {
      scoreFirstTurn = maxScore;
    }
    else
    {
      cout<<"\033[31m以下两个指标没有考虑技能，买技能后下降正常\033[0m" << endl;
      cout << "此局运气：\033[33m" << maxScore - scoreFirstTurn << "\033[0m" << endl;
      cout << "此回合运气：\033[33m" << maxScore - scoreLastTurn << "\033[0m" << endl; 
      cout << "比赛亏损（用于选择比赛回合，以完成粉丝数目标）：\033[33m" << maxScore - std::max(search.allChoicesValue[0][7].avgScoreMinusTarget, search.allChoicesValue[1][7].avgScoreMinusTarget) << "\033[0m" << endl;
      cout << "<<" << endl;
    }
    scoreLastTurn = maxScore;
    /*
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
    */

  }

}