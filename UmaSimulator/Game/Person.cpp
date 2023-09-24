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
  //for (int i = 0; i < 5; i++)atTrain[i] = false;
  isShining = false;
  isHint = false;
  cardRecord = 0;

  larc_isLinkCard = false;
  larc_charge = 0;
  larc_atSS = false;
  larc_statusType = -1;
  larc_specialBuff = 0;
  larc_level = 0;
  for (int i = 0; i < 3; i++)larc_nextThreeBuffs[i] = 0;
  //larc_assignedStatusTypeWhenFull = -1;


  std::vector<int> probs = { 1,1,1,1,1,1 }; //速耐力根智鸽
  distribution = std::discrete_distribution<>(probs.begin(), probs.end());
}

void Person::initAtTurn3(std::mt19937_64& rand, int specialBuff, int statusType)
{
  larc_specialBuff = specialBuff;
  larc_statusType = statusType;
  larc_level = 1;


  //黄金船爱娇，神鹰练习上手，与其他的不一样
  if (larc_specialBuff == 8)//爱娇
  {
    larc_nextThreeBuffs[0] = 8;//1级必为爱娇
    if (rand() % 2)
    {
      larc_nextThreeBuffs[1] = 11;
      larc_nextThreeBuffs[2] = 1;
    }
    else
    {
      larc_nextThreeBuffs[1] = 1;
      larc_nextThreeBuffs[2] = 11 ;
    }
  }
  else //三个buff随机分配位置
  {
    static const int shuffle3_list[6][3] =
    {
      {0,1,2},
      {0,2,1},
      {1,0,2},
      {1,2,0},
      {2,0,1},
      {2,1,0}
    };

    int8_t threeBuffs[3] = { 1,11,larc_specialBuff };//前三级必为这三个
    if (larc_specialBuff == 9)//神鹰的练习上手+适性pt
      threeBuffs[1] = 7;

    //三个buff随机分配位置
    int order = rand() % 6;
    for (int i = 0; i < 3; i++)
      larc_nextThreeBuffs[i] = threeBuffs[shuffle3_list[order][i]];
  }
}

void Person::larc_afterSS(std::mt19937_64& rand)
{
  larc_charge = 0;
  larc_atSS = false;
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

