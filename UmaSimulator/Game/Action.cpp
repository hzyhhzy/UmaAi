#include "Action.h"
const Action Action::Action_RedistributeCardsForTest = { 0, TRA_redistributeCardsForTest };

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
const std::string Action::dishName[14] =
{
  "无",
  "三明治",
  "咖喱",
  "小速",
  "小耐",
  "小力",
  "小根",
  "小智",
  "大速",
  "大耐",
  "大力",
  "大根",
  "大智",
  "G1Plate"
};
bool Action::isActionStandard() const
{
  if (train >= 0 && train < 8)
  {
    if (dishType == DISH_none)
      return true;
    else
      return false;
  }
  else if (train == TRA_none)
  {
    if (dishType != DISH_none)
      return true;
    else
      return false;
  }
  return false;
}

int Action::toInt() const
{
  if (train >= 0 && train < 8)
  {
    if (dishType == DISH_none)
      return train;
    else
    {
      throw "Action::toInt(): Not standard Action, both dish and training";
      return -1;
    }
  }
  else if (train == TRA_none)
  {
    if (dishType != DISH_none)
    {
      return 8 + dishType - 1;
    }
    else
    {
      throw "Action::toInt(): Not standard Action, no dish and training";
      return -1;
    }
  }
  throw "Action::toInt(): Not standard Action, special training type";
  return -1;
}

Action Action::intToAction(int i)
{
  Action a;
  a.train = TRA_none;
  a.dishType = DISH_none;
  if (i > 0 && i < 8)
    a.train = i;
  else if (i < 21)
    a.dishType = i - 8 + 1;
  else
    throw "Action::intToAction(): Invalid int";
  return a;
}

std::string Action::toString() const
{
  if (train >= 0 && train < 8)
  {
    if (dishType == DISH_none)
      return "不做菜+" + trainingName[train];
    else
      return dishName[dishType] + "+" + trainingName[train];
  }
  else if (train == TRA_none)
  {
    if (dishType != DISH_none)
      return "先做菜:" + dishName[dishType];
    else
      return "什么都不做";
  }
  throw "Action::toString(): Unknown Action";
  return "";
}
