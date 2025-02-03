#pragma once

//#define UMAAI_MAINAI   //使用ai
//#define UMAAI_TESTSCORE   //测试ai分数
//#define UMAAI_TESTCARDSSINGLE   //测卡，控制五张卡不变只改变一张
//#define UMAAI_SIMULATOR   //养马模拟器
#define UMAAI_SELFPLAY   //跑数据（用于神经网络训练）
//#define UMAAI_TESTLIBTORCH   //测试c++版torch
//#define UMAAI_MODELBENCHMARK   //测试神经网络速度
//#define UMAAI_TESTSCORESEARCH //测试蒙特卡洛强度
//#define UMAAI_TESTSCORENOSEARCH //测试神经网络/手写逻辑policy强度

#if defined UMAAI_TESTSCORE || defined UMAAI_SIMULATOR 
#define PRINT_GAME_EVENT
#endif

const bool PrintHandwrittenLogicValueForDebug = false;

#define BACKEND_NONE 0//不使用神经网络，使用手写逻辑
#define BACKEND_LIBTORCH 1//使用libtorch(GPU或CPU)计算神经网络
#define BACKEND_CUDA 2//使用cuda(GPU)计算神经网络
#define BACKEND_EIGEN 3//使用Eigen库(CPU)计算神经网络
#define BACKEND_ONNX 4//使用ONNX-DirectML库(GPU)计算神经网络

#define USE_BACKEND BACKEND_NONE

const int MAX_SCORE = 200000;//最大允许的分数

#if USE_BACKEND == BACKEND_LIBTORCH || defined UMAAI_TESTLIBTORCH

const int LIBTORCH_USE_GPU = true;//是否使用GPU

//修改以下两个目录的同时，附加包含目录也需要修改
#define TORCH_LIBROOT "C:/local/libtorch/lib/"
#define TORCH_LIBROOT_DEBUG "C:/local/libtorch_debug/lib/"

#endif


#if USE_BACKEND == BACKEND_CUDA

//修改以下目录的同时，附加包含目录也需要修改
#define CUDA_LIBROOT "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.1/lib/x64/"

#endif