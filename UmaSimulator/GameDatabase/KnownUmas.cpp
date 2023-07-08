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
};


const UmaData GameDatabase::AllUmas[ALL_UMA_NUM] =
{
  //0，空
  {
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
};
