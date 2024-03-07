#include <fstream>
#include <random>
#include "GameGenerator.h"
#include "../Search/SearchParam.h"
using namespace std;

void GameGenerator::loadCardRankFile()
{
  for (int cardtype = 0; cardtype < 5; cardtype++)
  {
    assert(cardRank[cardtype].size() == 0);
    string path = "./db/cardtest/testcard_" + to_string(cardtype) + ".txt";
    ifstream fs(path);
    int N;
    fs >> N;
    for (int i = 0; i < N; i++)
    {
      int cardid;
      float value, score;
      fs >> cardid >> value >> score;
      cardRank[cardtype].push_back(cardid);
    }
  }
}

std::vector<int> GameGenerator::getRandomCardset(bool mustHaveZuoyue)
{
  vector<int> cardset;
  if (mustHaveZuoyue || rand() % 16 != 0)//带友人
  {
    if (rand() % 2 != 0)
      cardset.push_back(301604);
    else if (mustHaveZuoyue || rand() % 8 != 0)
      cardset.push_back(301600 + rand() % 5);
    else if(rand() % 2 != 0)
      cardset.push_back(100944);
    else
      cardset.push_back(100940 + rand() % 5);
  }

  int cardTypeCount[6] = { 0,0,0,0,0,0 };
  int c1 = rand() % 100;
  if (c1 < 45)//速根智大类与变种
  {
    int c2 = rand() % 100;
    if (c2 < 20)//10031
    {
      cardTypeCount[0] = 1;
      cardTypeCount[3] = 3;
      cardTypeCount[4] = 1;
    }
    else if (c2 < 32)//20021
    {
      cardTypeCount[0] = 2;
      cardTypeCount[3] = 2;
      cardTypeCount[4] = 1;
    }
    else if (c2 < 44)//00041
    {
      cardTypeCount[0] = 0;
      cardTypeCount[3] = 4;
      cardTypeCount[4] = 1;
    }
    else if (c2 < 54)//10022
    {
      cardTypeCount[0] = 1;
      cardTypeCount[3] = 2;
      cardTypeCount[4] = 2;
    }
    else if (c2 < 64)//00032
    {
      cardTypeCount[0] = 0;
      cardTypeCount[3] = 3;
      cardTypeCount[4] = 2;
    }
    else if (c2 < 70)//00131
    {
      cardTypeCount[2] = 1;
      cardTypeCount[3] = 3;
      cardTypeCount[4] = 1;
    }
    else if (c2 < 73)//00221
    {
      cardTypeCount[2] = 2;
      cardTypeCount[3] = 2;
      cardTypeCount[4] = 1;
    }
    else if (c2 < 76)//00122
    {
      cardTypeCount[2] = 1;
      cardTypeCount[3] = 2;
      cardTypeCount[4] = 2;
    }
    else //随机速根智
    {
      const int consideredCardtype[3] = { 0, 3, 4 };
      for (int i = 0; i < 5; i++)
      {
        cardTypeCount[consideredCardtype[rand() % 3]] += 1;
      }
    }
    if (cardset.size() == 0)//没友人，补一个根卡
    {
      cardTypeCount[3] += 1;
    }

  }
  else if (c1 < 70)//速耐大类
  {
    int c2 = rand() % 100;
    if (c2 < 30)//22001
    {
      cardTypeCount[0] = 2;
      cardTypeCount[1] = 2;
      cardTypeCount[4] = 1;
    }
    else if (c2 < 55)//31001
    {
      cardTypeCount[0] = 3;
      cardTypeCount[1] = 1;
      cardTypeCount[4] = 1;
    }
    else if (c2 < 70)//21002
    {
      cardTypeCount[0] = 2;
      cardTypeCount[1] = 1;
      cardTypeCount[4] = 2;
    }
    else if (c2 < 80)//32000
    {
      cardTypeCount[0] = 3;
      cardTypeCount[1] = 2;
      cardTypeCount[4] = 0;
    }
    else //随机速耐智
    {
      const int consideredCardtype[3] = { 0, 1, 4 };
      for (int i = 0; i < 5; i++)
      {
        cardTypeCount[consideredCardtype[rand() % 3]] += 1;
      }
    }
    if (cardset.size() == 0)//没友人，补一个耐卡
    {
      cardTypeCount[1] += 1;
    }
  }
  else //随机配卡
  {
    for (int i = 0; i < 6 - cardset.size(); i++)
    {
      cardTypeCount[rand() % 5] += 1;
    }
  }

  double cardRankFactor = rand() % 2 == 0 ? 2.0 : rand() % 2 ? 5.0 : 10.0;//一半是以top2的卡为主，四分之一是top5，四分之一是top10

  std::uniform_real_distribution<double> uniDistr(0.0, 1.0);
  for (int ct = 0; ct < 5; ct++)
  {
    for (int i = 0; i < cardTypeCount[ct]; i++)
    {
      int cardId = -1;
      while (true)
      {
        int t = int(cardRankFactor * (1.0 / (uniDistr(rand) + 1e-8) - 1.0));// p(N) ~ 1/N^2
        if (t >= cardRank[ct].size() || t < 0)
          continue;


        break;
      }
      cardset.push_back(cardId);
    }
  }
  assert(cardset.size() == 6);
  for (int i = 0; i < 6; i++)
  {
    if (rand() % 16 == 0)//非满破卡
    {
      cardset[i] = (cardset[i] / 10) * 10 + rand() % 5;
    }
  }
  return cardset;
}

void GameGenerator::randomizeUmaCardParam(Game& game)
{
  std::exponential_distribution<double> expDistr(1.0);
  std::normal_distribution<double> normDistr(0.0, 1.0);
  std::uniform_real_distribution<double> uniDistr(0.0, 1.0);

  //随机马娘属性加成
  if (rand() % 4 == 0)
  {
    for (int i = 0; i < 5; i++)
    {
      int x = game.fiveStatusBonus[i] + int(normDistr(rand) * 5.0 + 0.5);
      if (x < 0)x = 0;
      if (x > 30)x = 30;
      game.fiveStatusBonus[i] = x;
    }
  }

  //随机卡组参数

  for (int i = 0; i < 6; i++)
  {
    if (rand() % 8 == 0 && game.persons[i].cardParam.cardType < 5)
    {
      auto& p = game.persons[i].cardParam;
      if (rand() % 4 == 0)
        p.bonusBasic[rand() % 6] += 1;


      if (rand() % 2 == 0)
      {
        p.xunLianBasic += 1.0 + int(normDistr(rand) * 5.0 + 0.5);
        if (p.xunLianBasic < 0)p.xunLianBasic = 0;
      }

      if (rand() % 2 == 0)
      {
        p.youQingBasic += 2.0 + int(normDistr(rand) * 10.0 + 0.5);
        if (p.youQingBasic < 0)p.youQingBasic = 0;
      }

      if (rand() % 2 == 0)
      {
        p.ganJingBasic += 4.0 + int(normDistr(rand) * 20.0 + 0.5);
        if (p.ganJingBasic < 0)p.ganJingBasic = 0;
      }

      if (rand() % 2 == 0)
      {
        p.deYiLv += 4.0 + int(normDistr(rand) * 20.0 + 0.5);
        if (p.deYiLv < 0)p.deYiLv = 0;
      }

    }
  }

}
