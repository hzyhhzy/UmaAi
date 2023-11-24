#pragma once

#define UMAAI_MAINAI   //使用ai
//#define UMAAI_TESTSCORE   //测试ai分数
//#define UMAAI_TESTCARDSSINGLE   //测卡，控制五张卡不变只改变一张
//#define UMAAI_SIMULATOR   //养马模拟器
#define UMAAI_SELFPLAY   //跑数据（用于神经网络训练）

#if defined UMAAI_TESTSCORE || defined UMAAI_SIMULATOR 
#define PRINT_GAME_EVENT
#endif

const int MAX_SCORE = 200000;//最大允许的分数，70000在larc剧本肯定很够用了，但不排除selfplay随机出来的开局存在一些极端情况