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
#include "../NeuralNet/TrainingSample.h"
#include "../Selfplay/GameGenerator.h"
using namespace std;


void main_playerPlay()
{
  GameDatabase::loadUmas("../db/umaDB.json");
  //GameDatabase::loadCards("../db/card");
  GameDatabase::loadDBCards("../db/cardDB.json");

  const int threadNum = 8;
  const int searchN = 8192;
  const double radicalFactor = 5;
  SearchParam param = { searchN,TOTAL_TURN,radicalFactor };


  cout << termcolor::cyan << "赛马娘凯旋门剧本育成模拟器 v0.1" << termcolor::reset << endl;
  cout << termcolor::cyan << "作者 Sigmoid，QQ: 2658628026" << termcolor::reset << endl;
  cout << termcolor::cyan << "代码开源：" << termcolor::yellow << "https://github.com/hzyhhzy/UmaAi" << termcolor::reset << endl;
  cout << termcolor::bright_cyan << "此模拟器界面类似“小黑板”。为了方便，并没有买技能的功能，把固有技能和各种技能hint都换算成pt，每pt计为" << GameConstants::ScorePtRate << "分（切者" << GameConstants::ScorePtRateQieZhe << "分）" << termcolor::reset << endl;
  cout << termcolor::bright_cyan << "所有Lv2的升级（消除debuff）均自动进行，Lv3需要玩家手动购买" << termcolor::reset << endl;
  cout << termcolor::bright_cyan << "第二年的凯旋门允许不消智力debuff，如果pt不够消除其他debuff则模拟器按输凯旋门计算" << termcolor::reset << endl;
  cout << endl;

  random_device rd;
  auto rand = mt19937_64(rd());

  int umaId = 103001;//米浴
  int umaStars = 5;
  int cards[6] = { 301604,301344,301614,300194,300114,301074 };//友人，高峰，神鹰，乌拉拉，风神，司机
  int zhongmaBlue[5] = { 18,0,0,0,0 };
  int zhongmaBonus[6] = { 20,0,40,0,20,150 };

  int batchsize = 512;
  //Model model("../training/example/model_traced.pt", batchsize);
  //Model* modelptr = &model;
  Model* modelptr = NULL;
  //GameGenerator gameGenerator(SelfplayParam(), NULL);
  for (int gamenum = 0; gamenum < 100000; gamenum++)
  {
    Search search(modelptr, batchsize, threadNum, param);
    Game game;
    game.newGame(rand, true, umaId, umaStars, cards, zhongmaBlue, zhongmaBonus);
    //game = gameGenerator.get();
    //for (int i = 0; i < 36; i++)
    //{
    //  Action act = { 4,false, false, false, false };
    //  if (game.larc_ssPersonsCount >= 5)
    //    act.train = 5;
    //  game.applyTrainingAndNextTurn(rand, act);
    //}
    game.larc_allowedDebuffsFirstLarc[4] = true;//允许不消除智力debuff


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

    while(game.turn < TOTAL_TURN)
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));//等几秒让人看清楚
      //assert(turn == game.turn && "回合数不正确");
      game.print();

      if (game.turn < TOTAL_TURN - 1)
      {
        Action handWrittenStrategy = Evaluator::handWrittenStrategy(game);
        string strategyText[10] =
        {
          "速",
          "耐",
          "力",
          "根",
          "智",
          "SS",
          "休息",
          "友人外出",
          "普通外出",
          "比赛"
        };
        cout << "手写逻辑：" << strategyText[handWrittenStrategy.train];
        if (game.larc_isAbroad)
        {
          cout << "   ";
          if (!handWrittenStrategy.buy50p)
            cout << "不";
          cout << "购买+50%";
        }
        cout << endl;

        game.playerPrint = false;
        search.runSearch(game, rand);

        game.playerPrint = true;
        for (int i = 0; i < Search::buyBuffChoiceNum(game.turn); i++)
        {
          if (Search::buyBuffChoiceNum(game.turn) > 1 && i == 0)
            cout << "不买:              ";
          if (i == 1)
            cout << "买+50%:            ";
          if (i == 2 && game.turn < 50)
            cout << "买pt+10:           ";
          if (i == 2 && game.turn >= 50)
            cout << "买体力-20%:        ";
          if (i == 3 && game.turn < 50)
            cout << "买+50%与pt+10:     ";
          if (i == 3 && game.turn >= 50)
            cout << "买+50%与体力-20%:  ";
          cout << "速耐力根智: ";
          for (int j = 0; j < 10; j++)
          {
            double score = search.allChoicesValue[i][j].value;
            if (score > -20000)
              cout
              //<< fixed << setprecision(1) << search.allChoicesValue[i][j].winrate * 100 << "%:" 
              << fixed << setprecision(0) << score << " ";
            else
              cout << "-- ";
            if (j == 4)cout << " | SS:";
            if (j == 5)cout << " | 休息:";
            if (j == 6)cout << " 友人外出:";
            if (j == 7)cout << " 普通外出:";
            if (j == 8)cout << " 比赛:";
          }
          cout << endl;
        }
      }
      /*

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

      auto tdata = search.exportTrainingSample();

      if (game.isRacing)//比赛回合
      {
        assert(false && "比赛回合应该已经在game类里跳过");
      }
      else//常规训练回合
      {
        Action action;
        action.train = -1;
        action.buy50p = false;
        action.buyFriend20 = false;
        action.buyPt10 = false;
        action.buyVital20 = false;


        string s;

        cout << termcolor::cyan << "请选择训练：1速，2耐，3力，4根，5智，S键SS对战，a友人出行，b普通出行，c休息，d额外比赛，remake重开，cheat作弊" << termcolor::reset << endl;
        if (game.larc_isAbroad)
        {
          cout << termcolor::cyan << "输入b1购买速+50%，b2购买耐+50%，b3购买力+50%，b4购买根+50%，b5购买智+50%，b6购买pt+10，b7购买体力消费-20%，b8购买友情+20%" << termcolor::reset << endl;
        }
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
        else if (s == "s")
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
          cout << termcolor::red << "你把" << termcolor::green << GameDatabase::AllUmas[umaId].name << termcolor::red << "做成马肉汉堡了" << termcolor::reset << endl;
          break;
        }
        else if (s == "cheat")//重置卡组分配
        {
          cout << termcolor::bright_cyan << "卡组重新分配！" << termcolor::reset << endl;
          game.randomDistributeCards(rand);
          //game.print();
          continue;
        }
        else if (s.size() == 2 && s[0] == 'b' && (s[1] >= '1' && s[1] <= '8'))
        {
          int buy_idx = s[1] == '1' ? 3:
            s[1] == '2' ? 1:
            s[1] == '3' ? 2:
            s[1] == '4' ? 0:
            s[1] == '5' ? 4:
            s[1] == '6' ? 5:
            s[1] == '7' ? 6:
            s[1] == '8' ? 7:
            -1;
          bool suc = game.tryBuyUpgrade(buy_idx, 3);
          if (!suc)
          {
            cout << termcolor::red << "购买失败！" << termcolor::reset << endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));//等几秒让人看清楚
          }
          else
          {
            cout << termcolor::cyan << "购买" << buy_idx + 1 << "号升级Lv3成功" << termcolor::reset << endl;
            //game.print();
          }
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
        cout << endl;
        game.checkEventAfterTrain(rand);
        if(!game.isEnd())
          game.randomDistributeCards(rand);
      }
    }
    cout << termcolor::red << "育成结束！" << termcolor::reset << endl;
    game.printFinalStats();
  }
}