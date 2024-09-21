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

std::vector<int> GameGenerator::getRandomCardset()
{
  vector<int> cardset;
  if (rand() % 16 != 0)//带友人
  {
    if (rand() % 2 != 0)
      cardset.push_back(301884);
    else if (rand() % 8 != 0)
      cardset.push_back(301880 + rand() % 5);
    else if(rand() % 2 != 0)
      cardset.push_back(101044);
    else
      cardset.push_back(101040 + rand() % 5);
  }

  int cardTypeCount[6] = { 0,0,0,0,0,0 };
  
  for (int i = 0; i < 6 - cardset.size(); i++)
  {
    cardTypeCount[rand() % 5] += 1;
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

        cardId = cardRank[ct][t] * 10 + 4;

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

  //随机马娘属性加成，随机link
  if (rand() % 4 == 0)
  {
    game.isLinkUma = rand() % 8 == 0;
    for (int i = 0; i < 5; i++)
    {
      int x = game.fiveStatusBonus[i] + int(normDistr(rand) * 5.0 + 0.5);
      if (x < 0)x = 0;
      if (x > 30)x = 30;
      game.fiveStatusBonus[i] = x;
    }
  }

  //随机赛程
  if (rand() % 2 == 0)
  {
    for (int t = 12; t < 72; t++)
    {
      game.isRacingTurn[t] = rand() % 8 == 0;
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
