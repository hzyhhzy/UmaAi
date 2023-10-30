#pragma once

#define UMAAI_MAINAI   //使用ai
//#define UMAAI_TESTSCORE   //测试ai分数
//#define UMAAI_SIMULATOR   //养马模拟器

#if defined UMAAI_TESTSCORE || defined UMAAI_SIMULATOR 
#define PRINT_GAME_EVENT
#endif

const int MAX_SCORE = 70000;//最大允许的分数，70000在larc剧本肯定很够用了