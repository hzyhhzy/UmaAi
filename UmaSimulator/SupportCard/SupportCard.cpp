#include <iostream>
#include "SupportCard.h"
#include "../GameDatabase/GameConstants.h"
#include "../Game/Game.h"

using namespace std;

CardTrainingEffect SupportCard::getCardEffect(const Game& game, int atTrain, int jiBan) const
{
  CardTrainingEffect effect =
  { 
    youQingBasic,
    ganJingBasic,
    xunLianBasic,
    {bonusBasic[0],bonusBasic[1],bonusBasic[2],bonusBasic[3],bonusBasic[4],bonusBasic[5]},
    wizVitalBonusBasic
  };

  bool isShining = true;//是否闪彩
  if (cardType < 5)//速耐力根智卡
  {
    if (jiBan < 80)isShining = false;
    if (cardType != atTrain)isShining = false;
  }
  else if (cardID == SHENTUAN_ID)//神团
  {
    if (!game.venusCardIsQingRe)
      isShining = false;
  }
  else cout << "未知卡";

  if (game.venusIsWisdomActive && game.venusAvailableWisdom == 3)//黄女神
    isShining = true;

  if (!isShining)
  {
    effect.youQing = 0;
  }
  if (!isShining || atTrain != 4)
    effect.vitalBonus = 0;

  //接下来是各种固有
  //1.神团
  if (cardID == 1)
  {
    if (jiBan < 100)
    {
      if (isShining)
        effect.youQing = 20;
      effect.ganJing = 0;
      effect.bonus[5] = 0;
    }
  }
  //2.高峰
  //为了简化，视为初始训练加成是4%，第一年逐渐增大到20%，也就是第n个回合4+n*(2/3)%
  else if (cardID == 2)
  {
    if (game.turn < 24)
      effect.xunLian = 4 + 0.6666667 * game.turn;
  }
  //3.美妙
  else if (cardID == 3)
  {
    //啥都没有
  }
  //4.根乌拉拉
  else if (cardID == 4)
  {
    //啥都没有
  }
  //5.根风神
  else if (cardID == 5)
  {
    //啥都没有
  }
  //6.水司机
  else if (cardID == 6)
  {
    int traininglevel = game.getTrainingLevel(atTrain);
    effect.xunLian = 5 + traininglevel * 5;
    if (effect.xunLian > 25)effect.xunLian = 25;
  }
  //7.根凯斯
  else if (cardID == 7)
  {
    if (jiBan < 80)
    {
      effect.bonus[2] = 0;
    }
  }
  //8.根皇帝
  else if (cardID == 8)
  {
    if (jiBan < 80)
    {
      effect.bonus[0] = 0;
    }
  }
  //9.根善信
  else if (cardID == 9)
  {
    //啥都没有
  }
  //10.速宝穴
  else if (cardID == 10)
  {
    if (jiBan < 100)
    {
      effect.bonus[0] = 0;
    }
  }
  //11.耐海湾
  else if (cardID == 11)
  {
    //啥都没有
  }
  //12.智好歌剧
  else if (cardID == 12)
  {
    if (jiBan < 80)
    {
      effect.bonus[0] = 0;
    }
  }
  //13.根黄金城
  else if (cardID == 13)
  {
    if (jiBan < 100)
    {
      effect.bonus[3] = 0;
    }
  }
  //
  else 
  {
    cout << "未知卡";
  }

  return effect;
}
