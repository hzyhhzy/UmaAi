#pragma once
#include "GameConstants.h"
//马娘的参数
struct UmaData
{
  int star;//星数
  bool races[TOTAL_TURN];//哪些回合有比赛
  int fiveStatusBonus[5];//属性加成
  int fiveStatusInitial[5];//初始属性
};
