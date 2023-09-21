#include "Person.h"
#include "../GameDatabase/GameDatabase.h"
using namespace std;
Person::Person()
{
  personType = 0; 
  //cardId = 0;
  charaId = 0;

  cardIdInGame = -1;
  friendship = 0;
  for (int i = 0; i < 5; i++)atTrain[i] = false;
  isShining = false;
  cardRecord = 0;

  larc_isLinkCard = false;
  larc_charge = 0;
  larc_atSS = false;
  larc_statusType = -1;
  larc_specialBuff = 0;
  larc_level = 0;
  for (int i = 0; i < 3; i++)larc_nextThreeBuffs[i] = 0;
  larc_assignedStatusTypeWhenFull = -1;


  std::vector<int> probs = { 1,1,1,1,1,1 }; //速耐力根智鸽
  distribution = std::discrete_distribution<>(probs.begin(), probs.end());
}

void Person::larc_afterSS(std::mt19937_64& rand)
{
  larc_charge = 0;
  larc_atSS = false;
  larc_assignedStatusTypeWhenFull = -1;
  larc_level += 1;
    
  //int levelLoop = larc_level % 3;//buff每三级循环一次，三级内buff必为一个是随机的
  //if(levelLoop)
  // 
  //算了直接随机

  int c = rand() % 6;
  int nextBuff = 0;
  
  //黄金船爱娇，神鹰练习上手，与其他的不一样
  //这里只处理4级以后，前三级不在这里处理
  if (larc_specialBuff == 8)//爱娇
  {
    if (c == 0 || c == 1 || c == 2 || c == 3)//不会重复爱娇，会加属性（有待验证）
      nextBuff = 11;
    else
      nextBuff = 1;
  }
  else if (larc_specialBuff == 9)
  {
    if (c == 0)//会重复练习上手（有待验证）
      nextBuff = 9;
    else if (c == 1)
      nextBuff = 11;
    else if (c == 2 || c == 3)
      nextBuff = 7;
    else
      nextBuff = 1;
  }
  else //比较常规的特殊buff
  {
    if (c == 0 || c == 1)
      nextBuff = larc_specialBuff;
    else if (c == 2 || c == 3)
      nextBuff = 11;
    else
      nextBuff = 1;
  }

  larc_nextThreeBuffs[0] = larc_nextThreeBuffs[1];
  larc_nextThreeBuffs[1] = larc_nextThreeBuffs[2];
  larc_nextThreeBuffs[2] = nextBuff;
}

