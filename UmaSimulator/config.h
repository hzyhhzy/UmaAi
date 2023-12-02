#pragma once

#define UMAAI_MAINAI   //使用ai
//#define UMAAI_TESTSCORE   //测试ai分数
//#define UMAAI_TESTCARDSSINGLE   //测卡，控制五张卡不变只改变一张
//#define UMAAI_SIMULATOR   //养马模拟器
//#define UMAAI_SELFPLAY   //跑数据（用于神经网络训练）
//#define UMAAI_TESTLIBTORCH   //测试c++版torch
//#define UMAAI_MODELBENCHMARK   //测试神经网络速度

#if defined UMAAI_TESTSCORE || defined UMAAI_SIMULATOR 
#define PRINT_GAME_EVENT
#endif

//#define USE_BACKEND_LIBTORCH //使用libtorch计算神经网络


const int MAX_SCORE = 200000;//最大允许的分数，70000在larc剧本肯定很够用了，但不排除selfplay随机出来的开局存在一些极端情况

#if defined USE_BACKEND_LIBTORCH || defined UMAAI_TESTLIBTORCH

const int LIBTORCH_USE_GPU = true;//是否使用GPU

#ifdef _DEBUG 
#pragma comment(lib, "C:/local/libtorch_debug/lib/torch.lib")
#pragma comment(lib, "C:/local/libtorch_debug/lib/c10.lib")
#pragma comment(lib, "C:/local/libtorch_debug/lib/torch_cuda.lib")
#pragma comment(lib, "C:/local/libtorch_debug/lib/torch_cpu.lib")
#pragma comment(lib, "C:/local/libtorch_debug/lib/c10_cuda.lib")
#else
#define TORCH_LIBROOT "C:/local/libtorch/lib/"
#pragma comment(lib, TORCH_LIBROOT "torch.lib")
#pragma comment(lib, TORCH_LIBROOT "c10.lib")
#pragma comment(lib, TORCH_LIBROOT "torch_cuda.lib")
#pragma comment(lib, TORCH_LIBROOT "torch_cpu.lib")
#pragma comment(lib, TORCH_LIBROOT "c10_cuda.lib")
#endif
#endif