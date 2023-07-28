#pragma once
#include "CardEffect.h"
/*
class Game;

struct SupportCard
{
  int cardID;//支援卡id，部分卡具有比较复杂的固有，根据id来辨别
  int cardType;//支援卡类型，0速1耐2力3根4智5友人或团队

  //带basic的都是不算固有的
  double youQingBasic;//友情加成
  double ganJingBasic;//干劲加成
  double xunLianBasic;//训练加成
  double bonusBasic[6];//速耐力根智pt的加成
  int wizVitalBonusBasic;//智力彩圈体力回复量
  int initialBonus[6];//初期速耐力根智pt的提升
  int initialJiBan;//初始羁绊
  double saiHou;//赛后
  int hintBonus[6];//为了简化，把红点的技能等效成多少属性。综合考虑技能有效率（例如高峰90%有效，除了集中力），平均性价比与折扣，种马重复给技能（假设30%）
  double hintProbIncrease;//启发发生率提升
  double deYiLv;//得意率

  int specialCount; // 用来处理特殊的固有


  CardTrainingEffect getCardEffect(const Game& game, int atTrain, int jiBan) const;//根据游戏状态计算支援卡的“固有”
};
*/