#include "GameDatabase.h"
#include "UmaData.h"

// 0 空
// 1 草上飞（五星）
// 2 
// 3
// 4
//


const std::string GameDatabase::AllUmaNames[ALL_UMA_NUM] = {
  "空",
  "草上飞(5星)",
  "草上飞(4星)",
  "花炮(5星)",
  "op炮(5星)",
  "小林历奇(4星)",
};


const std::map<int, int> GameDatabase::AllUmaGameIdToSimulatorId =
{
  {0,0},
  {5101101,1},
  {4101101,2},
  {5102402,3},
  {5102401,4},
  {4109801,5},
};
const UmaData GameDatabase::AllUmas[ALL_UMA_NUM] =
{
    //0，空
    {
      5,
      {
        false,false,false,false,false,false, false,false,false,false,false,false, false,false,false,false,false,false, false,false,false,false,false,false, //第一年
        false,false,false,false,false,false, false,false,false,false,false,false, false,false,false,false,false,false, false,false,false,false,false,false, //第二年
        false,false,false,false,false,false, false,false,false,false,false,false, false,false,false,false,false,false, false,false,false,false,false,false, //第三年
        false,false,false,false,false,true
      },
      {
        6,6,6,6,6
      },
      {
        110,110,110,110,110
      },

    },
    //1，草上飞
    {
      5,
      {
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, false,false,false,false,true,false, //第一年
        false,false,false,false,false,false, false,false,false,true,false,false, false,false,false,false,false,false, false,false,false,true,false,true, //第二年
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, true,false,false,false,false,true, //第三年
        false,false,false,false,false,true
      },
      {
        20,0,10,0,0
      },
      {
        118,91,129,96,116
      },
    },
    //2，草上飞（4星
    {
      4,
      {
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, false,false,false,false,true,false, //第一年
        false,false,false,false,false,false, false,false,false,true,false,false, false,false,false,false,false,false, false,false,false,true,false,true, //第二年
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, true,false,false,false,false,true, //第三年
        false,false,false,false,false,true
      },
      {
        20,0,10,0,0
      },
      {
        107,83,117,87,106
      },
    },

    //3，花炮
    {
      5,
      {
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, false,false,false,false,false,false, //第一年
        false,false,false,false,false,false, false,false,false,false,false,false, false,false,false,false,false,false, false,true,false,false,false,true, //第二年
        false,false,false,false,false,true, false,true,false,false,false,true, false,false,false,false,false,false, false,true,false,false,false,true, //第三年
        false,false,false,false,false,true
      },
      {
        10,10,0,0,10
      },
      {
        102,123,100,110,115
      },
    },
    //4，op炮
    {
      5,
      {
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, false,false,false,false,false,false, //第一年
        false,false,false,false,false,false, false,false,false,false,false,false, false,false,false,false,false,false, false,true,false,false,false,true, //第二年
        false,false,false,false,false,true, false,true,false,false,false,true, false,false,false,false,false,false, false,true,false,false,false,true, //第三年
        false,false,false,false,false,true
      },
      {
        0,20,0,10,0
      },
      {
        94,130,86,123,117
      },
    },
    //5,小林檎
    {
      4,
      {
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, false,false,false,false,false,true, //第一年
        false,false,false,false,false,false, true,false,false,false,false,false, false,false,false,false,true,false, false,false,false,true,false,true, //第二年
        false,false,false,true,false,false, false,false,true,false,false,true, false,false,false,false,true,false, true,false,true,false,false,true, //第三年
        false,false,false,false,false,true
      },
      {
        0,0,10,0,20
      },
      {
        104,97,103,96,100
      },
    },
};
