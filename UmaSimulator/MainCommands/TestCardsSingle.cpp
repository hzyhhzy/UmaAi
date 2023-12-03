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
#include "../Game/Game.h"
#include "../NeuralNet/Evaluator.h"
#include "../Search/Search.h"
#include "../External/termcolor.hpp"

#include "../GameDatabase/GameDatabase.h"
#include "../GameDatabase/GameConfig.h"

using namespace std;

struct testResult
{
  string cardName;
  int cardId;
  ModelOutputValueV1 result;
};

void main_testCardsSingle()
{
  //string modelPath = "db/model_traced.pt";
  string modelPath = "";

  GameDatabase::loadUmas("../db/umaDB.json");
  //GameDatabase::loadCards("../db/card");
  GameDatabase::loadDBCards("../db/cardDB.json");


  random_device rd;
  auto rand = mt19937_64(rd());

  int toTestCardType = 4;//速耐力根智

  //一速三根一智，测速卡
  //int umaId = 101101;//草上飞，20速10力加成
  //int cards[6] = { 301604,300374,300194,300114,301534,0};//友人，根皇帝，根乌拉拉，根风神，智好歌剧，-   控制变量的五张卡都不要用link
  
  //二速二耐一智，测耐卡
  //int umaId = 102402;//花炮，10速10耐10智加成
  //int cards[6] = { 301604,301524,301474,301614,301654,0 };//友人，智好歌剧，速宝穴，速神鹰，耐谷水，-   控制变量的五张卡都不要用link，但耐卡没神鹰

  //二速二力一智，测力卡
  //int umaId = 102402;//花炮，10速10耐10智加成
  //int cards[6] = { 301604,301524,301474,301074,301564,0 };//友人，智好歌剧，速宝穴，速司机，力奇锐骏，-   控制变量的五张卡都不要用link
   
  //一速三根一智，测根卡
  //int umaId = 101101;//草上飞，20速10力加成
  //int cards[6] = { 301604,301524,301474,300194,300114,0};//友人，智好歌剧，速宝穴，根乌拉拉，根风神，-   控制变量的五张卡都不要用link


  //一速三根一智，测智卡
  int umaId = 101101;//草上飞，20速10力加成
  int cards[6] = { 301604,300374,300194,300114,301474,0};//友人，根皇帝，根乌拉拉，根风神，速宝穴，-   控制变量的五张卡都不要用link
  

  int umaStars = 5;
  int zhongmaBlue[5] = { 18,0,0,0,0 };
  int zhongmaBonus[6] = { 10,10,30,0,10,70 };
  bool allowedDebuffs[9] = { false, false, false, false, true, false, false, false, false };//第二年可以不消第几个debuff。第五个是智力，第七个是强心脏
  

  double radicalFactor = 10;//激进度
  int searchN = 100000;
  int threadNum = 12;
  //int initialStatusBonus = 40;//考虑到手写逻辑比实际ai分低，所以增加初始属性
  //但是手写逻辑不会控属性，还是不加了


  SearchParam searchParam = { searchN,TOTAL_TURN,radicalFactor };

  int batchsize = 1024;
  Model* modelptr = NULL;
  Model model(modelPath, batchsize);
  if (modelPath != "")
  {
    modelptr = &model;
  }

  Model::detect(modelptr);

  Search search(modelptr, batchsize, threadNum, searchParam);

  vector<testResult> allResult;
  for (int cardId = 39999; cardId >= 0; cardId--)
  {
    int cardIdLv50 = cardId * 10 + 4;
    if (!GameDatabase::AllCards.count(cardIdLv50))
      continue;
    auto& card = GameDatabase::AllCards[cardIdLv50];
    if (card.cardType != toTestCardType)
      continue;

    string cardName = card.cardName;

    if (cardId >= 30000)
    {
      cardName = "\033[1;37m" + cardName + "\033[0m";
    }
    else if (cardId >= 20000)
    {
      cardName = "\033[1;33m" + cardName + "SR\033[0m";
    }
    else if (cardId >= 10000)
    {
      cardName = "\033[1;36m" + cardName + "R\033[0m";
    }

    cardName = cardName + "(" + to_string(cardId) + ")";

    //没考虑固有的卡
    if (card.uniqueEffectType == 5 ||
      card.uniqueEffectType == 12 ||
      card.uniqueEffectType == 15
      )
      cardName = "\033[41m\033[30m**\033[0m" + cardName;

    //固有视为全开的卡
    else if (card.uniqueEffectType == 6  
      || cardId == 30155
      || cardId == 30171
      )
      cardName = "\033[43m\033[30m#\033[0m" + cardName;
    else
      cardName = "\033[43m\033[30m\033[0m" + cardName;
    cout << setw(55) << cardName << "：";

    cards[5] = cardIdLv50;
    Game game;
    game.newGame(rand, false, umaId, umaStars, cards, zhongmaBlue, zhongmaBonus);
    //game.addAllStatus(initialStatusBonus);
    Action action = { 8,false,false,false,false };//无条件外出，这样就无视第一回合的人头分布了
    auto value = search.evaluateSingleAction(game, rand, action);

    cout << "胡局分数=\033[1;32m" << int(value.value) << "\033[0m  平均分数=\033[1;32m" << int(value.scoreMean) << "\033[0m" << endl;
    
    testResult tr;
    tr.cardId = cardId;
    tr.cardName = cardName;
    tr.result = value;

    allResult.push_back(tr);
  }

  cout << endl;
  cout << "\033[41m\033[30m**\033[0m是没考虑固有，\033[43m\033[30m#\033[0m是视为固有全开" << endl;
  cout << "按胡局分数从大到小排序：" << endl;
  cout << "-------------------------------------------------------------------------" << endl;
  std::sort(allResult.begin(), allResult.end(), [](const testResult& a, const testResult& b) {
    return a.result.value > b.result.value;
    });

  for (int i = 0; i < allResult.size(); i++)
  {
    auto tr = allResult[i];
    cout << setw(55) << tr.cardName << "："; 
    cout << "胡局分数=\033[1;32m" << int(tr.result.value) << "\033[0m  平均分数=\033[1;32m" << int(tr.result.scoreMean) << "\033[0m" << endl;

  }
  //保存结果，主要是用于selfplay随机选卡
  string resultname = "testcard_" + to_string(toTestCardType) + ".txt";
  ofstream fs(resultname);
  fs << allResult.size() << endl;
  for (int i = 0; i < allResult.size(); i++)
  {
    auto tr = allResult[i];
    fs << tr.cardId << " " << tr.result.value << " " << tr.result.scoreMean << endl;
  }
  fs.close();


  cout << endl;
  cout << "\033[41m\033[30m**\033[0m是没考虑固有，\033[43m\033[30m#\033[0m是视为固有全开" << endl;
  cout << "按平均分数从大到小排序：" << endl;
  cout << "-------------------------------------------------------------------------" << endl;
  std::sort(allResult.begin(), allResult.end(), [](const testResult& a, const testResult& b) {
    return a.result.scoreMean > b.result.scoreMean;
    });

  for (int i = 0; i < allResult.size(); i++)
  {
    auto tr = allResult[i];
    cout << setw(55) << tr.cardName << "：";
    cout << "胡局分数=\033[1;32m" << int(tr.result.value) << "\033[0m  平均分数=\033[1;32m" << int(tr.result.scoreMean) << "\033[0m" << endl;

  }

}