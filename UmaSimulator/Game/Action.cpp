#include "Action.h"

const Action Action::Action_RedistributeCardsForTest = { -1,false, -1 ,0,0 };

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
Action::Action()
{
  type = 0;
  overdrive = false;
  train = -1;
  mechaHead = 0;
  mechaChest = 0;
}
Action::Action(int id)
{
  type = 0;
  overdrive = false;
  train = -1;
  mechaHead = 0;
  mechaChest = 0;

  if (id < 0 || id >= MAX_ACTION_TYPE)
    throw "Action::Action(): Invalid int";
  Action a;
  if (id < 14)
  {
    type = 1;
    overdrive = id >= 8;
    train = overdrive ? id - 8 : id;
    if (id == 13)train = -1;
    mechaHead = 0;
    mechaChest = 0;
  }
  else if (id < 15 + 6 * 6)
  {
    type = 2;
    overdrive = false;
    train = -1;
    mechaHead = (id - 14) / 6;
    mechaChest = (id - 14) % 6;
  }
  else
    throw "Action::Action(): Invalid int";



}
bool Action::isActionStandard() const
{
  //-1 使用这个标记时，说明要randomDistributeCards，用于测试ai分数，在Search::searchSingleActionThread中使用
  if (type == -1)
  {
    return false;
  }
  //1 普通训练，如果有overdrive则可以决定是否开，也可以先开overdrive而暂时不选训练（仅限第三年下半年点了15级胸，则需要先决定是否开overdrive根据摇人结果而选训练）。选项个数为8+5+1
  else if (type == 1)
  {
    return mechaHead == 0 && mechaChest == 0 && (overdrive ? (train >= -1 && train < 5): (train >= 0 && train < 8));
  }
  //2 升级回合。只考虑整3级，action里只包括头和胸（脚=总-头-胸），选项个数为6*6=36
  else if (type == 2)
  {
    return !overdrive && train == -1 && mechaHead >= 0 && mechaHead <= 5 && mechaChest >= 0 && mechaChest <= 5;
  }
  else
    return false;
}

int Action::toInt() const
{
  if (!isActionStandard())
  {
    throw "Calling Action::toInt() for a non-standard action";
    return -1;
  }
  //1 普通训练，如果有overdrive则可以决定是否开，也可以先开overdrive而暂时不选训练（仅限第三年下半年点了15级胸，则需要先决定是否开overdrive根据摇人结果而选训练）。选项个数为8+5+1
  if (type == 1)
  {
    return overdrive ? (train == -1 ? 5 + 8 : train + 8) : train;
  }
  //2 升级回合。只考虑整3级，action里只包括头和胸（脚=总-头-胸），选项个数为6*6=36
  else if (type == 3)
  {
    return 14 + 6 * mechaHead + mechaChest;
  }
  else
    throw "Calling Action::toInt() for a unknown type action";
}

std::string Action::toString() const
{
  //1 普通训练，如果有overdrive则可以决定是否开，也可以先开overdrive而暂时不选训练（仅限第三年下半年点了15级胸，则需要先决定是否开overdrive根据摇人结果而选训练）。选项个数为8+5+1
  if (type == 1)
  {
    if (overdrive && train == -1)
      return "先开齿轮";
    return overdrive ? "齿轮+" + trainingName[train] : trainingName[train];
  }
  //2 升级回合。只考虑整3级，action里只包括头和胸（脚=总-头-胸），选项个数为6*6=36
  else if (type == 2)
  {
    return "头" + std::to_string(mechaHead) + "级胸" + std::to_string(mechaChest) + "级";
  }
  else
    throw "Action::toString(): Unknown Action";

  return "";
}

Action Action::RedistributeCardsForTest()
{
  Action a = Action();
  a.type = -1;
  return a;
}
