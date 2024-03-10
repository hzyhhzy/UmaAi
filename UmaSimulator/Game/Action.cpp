#include "Action.h"

const int Action::XiangtanFromColor[10] = { -1,0,0,1,1,2,2,-1,-1,-1 };
const int Action::XiangtanToColor[10] = { -1,1,2,0,2,0,1,0,1,2 };
const int Action::XiangtanNumCost[10] = { 0,1,1,1,1,1,1,2,2,2 };
const Action Action::Action_RedistributeCardsForTest = { TRA_redistributeCardsForTest,0 };

const std::string Action::trainingName[8] =
{
  "速",
  "耐",
  "力",
  "根",
  "智",
  "休息",
  "外出",
  "比赛"
};
const std::string Action::xiangtanName[10] =
{
  "无",
  "蓝->红",
  "蓝->黄",
  "红->蓝",
  "红->黄",
  "黄->蓝",
  "黄->红",
  "全蓝",
  "全红",
  "全黄",
};

int Action::toInt() const
{
  if (train < 5)return xiangtanType * 5 + train;
  if (train == TRA_rest)return 50;
  if (train == TRA_outgoing)return 51;
  if (train == TRA_race)return 52;
  return -1;
}

Action Action::intToAction(int i)
{
  Action a = { 0,0 };
  if (i == 50)a.train = TRA_rest;
  else if (i == 51)a.train = TRA_outgoing;
  else if (i == 52)a.train = TRA_race;
  else
  {
    a.train = i % 5;
    a.xiangtanType = i / 5;
  }
  return a;
}

std::string Action::toString() const
{
  return trainingName[train] + " 相谈:" + xiangtanName[xiangtanType];
}
