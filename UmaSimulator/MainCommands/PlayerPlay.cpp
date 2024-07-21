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
  GameDatabase::loadUmas("./db/umaDB.json");
  //GameDatabase::loadCards("../db/card");
  GameDatabase::loadDBCards("./db/cardDB.json");

  const int threadNum = 8;
  const int searchN = 8192;
  const double radicalFactor = 5;
  SearchParam param(searchN, radicalFactor);


  cout << termcolor::cyan << "赛马娘种菜杯剧本育成模拟器 v0.1" << termcolor::reset << endl;
  cout << termcolor::cyan << "作者 Sigmoid，QQ: 2658628026" << termcolor::reset << endl;
  cout << termcolor::cyan << "代码开源：" << termcolor::yellow << "https://github.com/hzyhhzy/UmaAi" << termcolor::reset << endl;
  cout << termcolor::bright_cyan << "此模拟器界面类似“小黑板”。为了方便，并没有买技能的功能，把固有技能和各种技能hint都换算成pt，每pt计为" << GameConstants::ScorePtRateDefault << "分（切者" << GameConstants::ScorePtRateDefault * 1.1 << "分）" << termcolor::reset << endl;
  cout << termcolor::bright_cyan << "所有农田的升级均自动进行" << termcolor::reset << endl;
  cout << endl;

  random_device rd;
  auto rand = mt19937_64(rd());

  int umaId = 101101;//草上飞
  int umaStars = 5;
  int cards[6] = { 302074,302064,302084,301874,300194,301724 };//友人，速强击，力西野花，根巨匠，根乌拉拉，智麦昆
  int zhongmaBlue[5] = { 18,0,0,0,0 };
  int zhongmaBonus[6] = { 5,15,25,5,5,150 };

  int batchsize = 512;


#if USE_BACKEND == BACKEND_LIBTORCH
  const string modelpath = "../training/example/model_traced.pt";
#elif USE_BACKEND == BACKEND_NONE
  const string modelpath = "";
#else
  const string modelpath = "../training/example/model.txt";
#endif

  Model* modelptr = NULL;
  Model model(modelpath, batchsize);
  if (modelpath != "")
  {
    modelptr = &model;
  }


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


      /*
      if (game.turn < TOTAL_TURN - 1){

        //  std::cout << "????? -- turn: " << game.turn << "  tot_turn-1 : "<< TOTAL_TURN - 1 << "-----------------\n";
        //assert(true && "todo");
        
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
      
      }*/
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

     // auto tdata = search.exportTrainingSample();

      Action action;
      action.train = -1;
      string dishKeys[14] = { "0","a1","a2","b1","b2","b3","b4","b5","c1","c2","c3","c4","c5","d" };//13种菜对应的键盘输入

      string s;
      if (game.isRacing)
      {
        cout << termcolor::green << "比赛回合：" << termcolor::reset <<
          termcolor::cyan << "0" << termcolor::reset << ":不吃菜并比赛 " <<
          termcolor::cyan << "remake" << termcolor::reset << ":重开 " <<
          termcolor::cyan << "cheat" << termcolor::reset << ":重置比赛菜种 " <<
          endl;
        
      }
      else
      {
        cout << termcolor::green << "请选择训练：" << termcolor::reset <<
          termcolor::cyan << "1" << termcolor::reset << ":速 " <<
          termcolor::cyan << "2" << termcolor::reset << ":耐 " <<
          termcolor::cyan << "3" << termcolor::reset << ":力 " <<
          termcolor::cyan << "4" << termcolor::reset << ":根 " <<
          termcolor::cyan << "5" << termcolor::reset << ":智 " <<
          termcolor::cyan << "6" << termcolor::reset << ":休息 " <<
          termcolor::cyan << "7" << termcolor::reset << ":外出(优先友人) " <<
          termcolor::cyan << "8" << termcolor::reset << ":比赛 " <<
          termcolor::cyan << "remake" << termcolor::reset << ":重开 " <<
          termcolor::cyan << "cheat" << termcolor::reset << ":重置人头分布 " <<
          endl;
      }

      //显示可以吃的菜
      if (game.cook_dish == DISH_none)
      {
        if (game.isRacing)
          cout << termcolor::green << "吃菜并比赛：" << termcolor::reset;
        else
          cout << termcolor::green << "吃菜：" << termcolor::reset;
        int legalDishNum = 0;
        for (int i = 1; i < 14; i++)
        {
          if (!game.isDishLegal(i))
            continue;
          legalDishNum += 1;
          cout << termcolor::cyan << dishKeys[i] << termcolor::reset << ":" << Action::dishName[i] << " ";
        }
        if (legalDishNum == 0)
          cout << termcolor::red << "没有可以吃的菜" << termcolor::reset;
        cout << endl;
      }

      cin >> s;

      //s是不是吃菜
      bool isDish = false;
      for (int i = 1; i < 14; i++)
      {
        if (s == dishKeys[i])
        {
          action.dishType = i;
          action.train = TRA_none;
          isDish = true;
        }
      }
      if (!isDish)
      {
        action.dishType = 0;
        if (game.isRacing && s == "0")
          action.train = TRA_race;
        else if (s == "1")
          action.train = TRA_speed;
        else if (s == "2")
          action.train = TRA_stamina;
        else if (s == "3")
          action.train = TRA_power;
        else if (s == "4")
          action.train = TRA_guts;
        else if (s == "5")
          action.train = TRA_wiz;
        else if (s == "6")
        {
          cout << termcolor::green << "你确定要休息吗？输入y确认，输入n重新选择" << termcolor::reset << endl;
          cin >> s;
          if (s != "y")
            continue;
          action.train = TRA_rest;
        }
        else if (s == "7")
        {
          cout << termcolor::green << "你确定要外出吗？输入y确认，输入n重新选择" << termcolor::reset << endl;
          cin >> s;
          if (s != "y")
            continue;
          action.train = TRA_outgoing;
        }
        else if (s == "8")
        {
          cout << termcolor::green << "你确定要比赛吗？输入y确认，输入n重新选择" << termcolor::reset << endl;
          cin >> s;
          if (s != "y")
            continue;
          action.train = TRA_race;
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
        else
        {
          cout << termcolor::red << "输入有误，请重新输入" << termcolor::reset << endl;
          continue;
        }
      }
      if (!game.isLegal(action))
      {
        cout << termcolor::red << "这个操作不合法，请重新输入" << termcolor::reset << endl;
        continue;
      }
      game.applyAction(rand, action);
      cout << endl;
      if (game.isEnd())break;
    }
    
    cout << termcolor::red << "育成结束！" << termcolor::reset << endl;
    game.printFinalStats();
  }
}