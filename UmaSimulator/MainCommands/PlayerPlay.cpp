#include <iostream>
#include <iomanip> 
#include <random>
#include <sstream>
#include <cassert>
#include <thread>  // for std::this_thread::sleep_for
#include <chrono>  // for std::chrono::seconds
#include "../External/termcolor.hpp"
#include "../Game/Game.h"
#include "../Search/Search.h"
using namespace std;


void main_playerPlay()
{
  GameDatabase::loadUmas("../db/uma");
  GameDatabase::loadCards("../db/card");
  GameDatabase::loadDBCards("../db/cardDB.json");

  const int threadNum = 4;

  cout << termcolor::cyan << "赛马娘凯旋门剧本育成模拟器 v0.1" << termcolor::reset << endl;
  cout << termcolor::cyan << "作者 Sigmoid，QQ: 2658628026" << termcolor::reset << endl;
  cout << termcolor::cyan << "代码开源：" << termcolor::yellow << "https://github.com/hzyhhzy/UmaAi" << termcolor::reset << endl;
  cout << termcolor::bright_cyan << "此模拟器界面类似“小黑板”。为了方便，并没有买技能的功能，把固有技能和各种技能hint都换算成pt，每pt计为" << GameConstants::ScorePtRate << "分（切者" << GameConstants::ScorePtRateQieZhe << "分）" << termcolor::reset << endl;
  cout << endl;

  random_device rd;
  auto rand = mt19937_64(rd());

  int umaId = 101101;//草上飞
  int umaStars = 5;
  int cards[6] = { 301604,301344,300104,300194,300114,301074 };//友人，高峰，美妙，乌拉拉，风神，司机
  int zhongmaBlue[5] = { 18,0,0,0,0 };
  int zhongmaBonus[6] = { 20,0,40,0,20,0 };
  for (int gamenum = 0; gamenum < 100000; gamenum++)
  {
    Search search;
    vector<Evaluator> evaluators;
    for (int i = 0; i < threadNum; i++)
      evaluators.push_back(Evaluator(NULL, 128));
    Game game;
    game.newGame(rand, true, umaId, umaStars, cards, zhongmaBlue, zhongmaBonus);


    cout << termcolor::bright_blue << "------------------------------------------------------------------------------------------------" << termcolor::reset << endl;
    cout << termcolor::green << "你养的马是：" << GameDatabase::AllUmas[umaId].name << termcolor::reset << endl;
    cout << termcolor::green << "你的配卡是：";
    for (int i = 0; i < 6; i++)
      cout << GameDatabase::AllCards[cards[i]].cardName << ",";
    cout << termcolor::reset << endl;
    {
      cout << termcolor::bright_cyan << "按Enter键开始游戏" << termcolor::reset << endl;
      if (gamenum != 0)std::cin.ignore(1000000, '\n');
      std::cin.get();
    }
    cout << endl;

    for (int turn = 0; turn < TOTAL_TURN - 7; turn++)
    {
      //assert(turn == game.turn && "回合数不正确");
      game.randomDistributeCards(rand);
      game.print();
      /*
      //search.runSearch(game, evaluators.data(), 4096, TOTAL_TURN, 27000, threadNum, 0);
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


      {
        auto policy = search.extractPolicyFromSearchResults(1);
        cout << fixed << setprecision(1) << policy.useVenusPolicy * 100 << "% ";
        cout << endl;
        for (int i = 0; i < 8; i++)
          cout << fixed << setprecision(1) << policy.trainingPolicy[i] * 100 << "% ";
        cout << endl;
        for (int i = 0; i < 3; i++)
          cout << fixed << setprecision(1) << policy.threeChoicesEventPolicy[i] * 100 << "% ";
        cout << endl;
        for (int i = 0; i < 6; i++)
          cout << fixed << setprecision(1) << policy.outgoingPolicy[i] * 100 << "% ";
        cout << endl;
      }
      */

      if (game.isRacing)//比赛回合
      {
        assert(false && "比赛回合应该已经在game类里跳过");
      }
      else//常规训练回合
      {
        bool isRemake = false;
        while (true)
        {
          Action action;
          action.train = -1;
          action.buy50p = false;
          action.buyFriend20 = false;
          action.buyPt10 = false;
          action.buyVital20 = false;


          string s;

          cout << termcolor::cyan << "请选择训练：1速，2耐，3力，4根，5智，6SS对战，a友人出行，b普通出行，c休息，d额外比赛，remake重开，cheat作弊" << termcolor::reset << endl;
          cin >> s;

          if (s == "1")
            action.train = 0;
          else if (s == "2")
            action.train = 1;
          else if (s == "3")
            action.train = 2;
          else if (s == "4")
            action.train = 3;
          else if (s == "5")
            action.train = 4;
          else if (s == "6")
            action.train = 5;
          else if (s == "a")
          {
            cout << termcolor::green << "你确定要出行吗？输入y确认，输入n重新选择" << termcolor::reset << endl;
            cin >> s;
            if (s != "y")
              continue;
            action.train = 7;
          }
          else if (s == "b")
          {
            cout << termcolor::green << "你确定要出行吗？输入y确认，输入n重新选择" << termcolor::reset << endl;
            cin >> s;
            if (s != "y")
              continue;
            action.train = 8;
          }
          else if (s == "c")
          {
            cout << termcolor::green << "你确定要休息吗？输入y确认，输入n重新选择" << termcolor::reset << endl;
            cin >> s;
            if (s != "y")
              continue;
            action.train = 6;
          }
          else if (s == "d")
          {
            cout << termcolor::green << "你确定要比赛吗？输入y确认，输入n重新选择" << termcolor::reset << endl;
            cin >> s;
            if (s != "y")
              continue;
            action.train = 9;
          }
          else if (s == "remake")
          {
            cout << termcolor::bright_red << "你确定要重开吗？输入remake确认重开，输入任意其他内容继续游戏" << termcolor::reset << endl;
            cin >> s;
            if (s != "remake")
              continue;
            isRemake = true;
            cout << termcolor::red << "你把" << termcolor::green << GameDatabase::AllUmas[umaId].name << termcolor::red << "做成马肉汉堡了" << termcolor::reset << endl;
            break;
          }
          else if (s == "cheat")//重置卡组分配
          {
            cout << termcolor::bright_cyan << "卡组重新分配！" << termcolor::reset << endl;
            game.randomDistributeCards(rand);
            game.print();
            continue;
          }
          else
          {
            cout << termcolor::red << "输入有误，请重新输入" << termcolor::reset << endl;
            continue;
          }



















          /*

          if (game.venusAvailableWisdom != 0)
          {
            cout << termcolor::cyan << "是否开启女神睿智？y开启，n不开启" << termcolor::reset << endl;
            cin >> s;
            if (s == "y")
            {
              useVenus = true;
            }
            else if (s == "n")
            {
              useVenus = false;
            }
            else
            {
              cout << termcolor::red << "输入有误，请重新输入" << termcolor::reset << endl;
              continue;
            }
          }


          if (chosenTrain == 7 && game.turn <= 12)
          {
            cout << termcolor::red << "前13回合无法比赛" << termcolor::reset << endl;
            continue;
          }




          int chosenOutgoing = 5;
          if (chosenTrain == 6 && game.venusCardUnlockOutgoing)
          {
            cout << termcolor::cyan << "请选择外出：0为普通外出，五个女神外出分别为 1 2 3 4 5" << termcolor::reset << endl;
            cin >> s;
            if (s == "0")
              chosenOutgoing = 5;
            else if (s == "1")
              chosenOutgoing = 0;
            else if (s == "2")
              chosenOutgoing = 1;
            else if (s == "3")
              chosenOutgoing = 2;
            else if (s == "4")
              chosenOutgoing = 3;
            else if (s == "5")
              chosenOutgoing = 4;
            else
            {
              cout << termcolor::red << "输入有误，请重新输入" << termcolor::reset << endl;
              continue;
            }

            if (!game.isOutgoingLegal(chosenOutgoing))
            {
              cout << termcolor::red << "不合法的外出，请重新输入" << termcolor::reset << endl;
              continue;
            }
          }


          assert(game.cardData[0]->cardType == 5 && "神团卡不在第一个位置");


          if (chosenTrain >= 0 && chosenTrain < 5 && game.cardDistribution[chosenTrain][0])//神团卡在选择的训练
          {
            if (chosenSpiritColor == -1)
            {
              cout << termcolor::cyan << "如果出现女神三选一事件，选择什么颜色的碎片？q红，a蓝，z黄" << termcolor::reset << endl;
              cin >> s;
              if (s == "q")
              {
                chosenSpiritColor = 0;
              }
              else if (s == "a")
              {
                chosenSpiritColor = 1;
              }
              else if (s == "z")
              {
                chosenSpiritColor = 2;
              }
              else
              {
                cout << termcolor::red << "输入有误，请重新输入" << termcolor::reset << endl;
                continue;
              }
            }
            else//提前选了碎片了
              cout << termcolor::cyan << "已提前选择碎片颜色" << termcolor::reset << endl;

          }*/

          bool suc = game.applyTraining(rand, action);
          assert(suc);
          break;


        }
        if (isRemake)
          break;
      }
      cout << endl;
      game.checkEventAfterTrain(rand);
      std::this_thread::sleep_for(std::chrono::seconds(1));//等几秒让人看清楚
    }
    cout << termcolor::red << "育成结束！" << termcolor::reset << endl;
    game.printFinalStats();
  }
}