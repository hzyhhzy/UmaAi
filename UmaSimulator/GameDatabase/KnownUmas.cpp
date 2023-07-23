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
  "特别周"
  "泳装特别周"
  "总大将特别周"
  "草上飞",
  "花炮",
  "op炮",
  "小林历奇",
  "水麦昆",
  "无声铃鹿",
  "op帝王",
  "火鸡帝王",
  "爱丽数码",
  "僵尸数码",
  "罗布罗伊"
};


const std::map<int, int> GameDatabase::AllUmaGameIdToSimulatorId =
{
  {0,0},
  {100101,1},
  {100102,2},
  {100103,3},
  {101101,4},
  {102402,5},
  {102401,6},
  {109801,7},
  {101303,8},
  {100201,9},
  {100301,10},
  {100302,11},
  {101901,12},
  {101902,13},
  {104701,14}
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
    //1，特别周
    {
      5,
      {
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, false,false,false,false,false,false, //第一年
        false,false,true,false,false,false, false,false,false,true,false,false, false,false,false,false,false,false, false,true,false,false,false,false, //第二年
        false,false,false,false,false,false, false,true,false,false,false,false, false,false,false,false,false,false, false,false,false,true,false,true, //第三年
        false,false,false,false,false,true
      },
      {
        0,20,0,0,10
      },
      {
        102,108,120,110,111
      },
    },
    //2，泳装特别周

    {
      5,
      {
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, false,false,false,false,false,false, //第一年
        false,false,true,false,false,false, false,false,false,true,false,false, false,false,false,false,false,false, false,true,false,false,false,false, //第二年
        false,false,false,false,false,false, false,true,false,false,false,false, false,false,false,false,false,false, false,false,false,true,false,true, //第三年
        false,false,false,false,false,true
      },
      {
        0,10,10,10,0
      },
      {
        94,110,125,119,102
      },
    },

    //3，总大将特别周
    {
      5,
      {
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, false,false,false,false,false,false, //第一年
        false,false,true,false,false,false, false,false,false,true,false,false, false,false,false,false,false,false, false,true,false,false,false,false, //第二年
        false,false,false,false,false,false, false,true,false,false,false,false, false,false,false,false,false,false, false,false,false,true,false,true, //第三年
        false,false,false,false,false,true
      },
      {
        10,10,0,0,10
      },
      {
        117,100,115,100,118
      },
    },
    //4，草上飞
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

    //5，花炮
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
    //6，op炮
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
    //7,小林檎
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
    //8,水麦昆
    {
      3,
      {
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, false,false,false,false,false,false, //第一年
        false,false,false,false,false,false, false,false,false,false,false,false, false,false,false,false,false,true, false,true,false,false,false,false, //第二年
        false,false,false,false,false,false, false,true,false,false,false,true, false,false,false,false,false,false, false,true,false,false,false,false, //第三年
        false,false,false,false,false,true
        },
      {
        8,8,0,0,14
      },
      {
        82,85,82,113,88
      },
    },
    //9，无声铃鹿
    {
      5,
      {
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, false,false,false,false,false,false, //第一年
        false,false,false,false,true,false, false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, //第二年
        false,false,false,false,true,false, false,false,false,false,false,true, false,false,false,false,false,false, true,true,false,false,false,false, //第三年
        false,false,false,false,false,true
      },
      {
        20,0,0,10,0
      },
      {
        124,102,94,122,108
      },

    },

    //10，原皮帝王
    {
      5,
      {
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, false,false,false,false,false,false, //第一年
        false,true,false,false,false,false, true,false,false,true,false,false, false,false,false,false,false,false, false,true,false,false,false,false, //第二年
        false,false,false,false,false,false, true,false,false,false,false,false, false,false,false,false,false,false, false,false,false,true,false,true, //第三年
        false,false,false,false,false,true
      },
      {
        20,10,0,0,0
      },
      {
        109,109,102,112,118
      },

    },

    //11，火鸡帝王
    {
      5,
      {
        false,false,false,false,false,false, false,false,false,false,false,true, false,false,false,false,false,false, false,false,false,false,false,false, //第一年
        false,true,false,false,false,false, true,false,false,true,false,false, false,false,false,false,false,false, false,true,false,false,false,false, //第二年
        false,false,false,false,false,false, true,false,false,false,false,false, false,false,false,false,false,false, false,false,false,true,false,true, //第三年
        false,false,false,false,false,true
      },
      {
        10,10,0,10,0
      },
      {
        109,125,98,106,112
      },

    },
    //12-13,数码
    {
      5,
      {
        false,false,false,false,false,false, false,false,false,false,false,true,
        false,false,false,false,false,false, false,false,false,false,false,false, //第一年
        false,false,false,true,false,false, false,false,true,false,false,false,
        true,false,false,false,false,false, false,false,false,true,false,false, //第二年
        false,false,false,false,false,false, false,false,false,false,false,false,
        false,false,false,false,false,false, true,false,false,false,false,false, //第三年
        false,false,false,false,false,true
      },
      {
        8,8,7,0,7
      },
      {
        104,111,101,122,112
      },
    },
    {
      5,
      {
        false,false,false,false,false,false, false,false,false,false,false,true,
        false,false,false,false,false,false, false,false,false,false,false,false, //第一年
        false,false,false,true,false,false, false,false,true,false,false,false,
        true,false,false,false,false,false, false,false,false,true,false,false, //第二年
        false,false,false,false,false,false, false,false,false,false,false,false,
        false,false,false,false,false,false, true,false,false,false,false,false, //第三年
        false,false,false,false,false,true
      },
      {
        7,7,8,8,0
      },
      {
        104,111,120,129,86
      },
    },
    //14,罗布罗伊
    {
      5,
      {
        false,false,false,false,false,false, false,false,false,false,false,true,
        false,false,false,false,false,false, false,false,false,false,false,false, //第一年
        false,false,false,false,false,false, false,true,false,true,false,false,
        false,false,false,false,false,false, false,true,false,false,false,true, //第二年
        false,false,false,false,false,false, false,true,false,false,false,true,
        false,false,false,false,false,false, false,true,false,true,false,true, //第三年
        false,false,false,false,false,true
      },
      {
        0,10,0,0,20
      },
      {
        108,118,111,91,122
      },
};
