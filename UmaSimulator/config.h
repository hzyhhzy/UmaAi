#pragma once

#define UMAAI_MAINAI   //ʹ��ai
//#define UMAAI_TESTSCORE   //����ai����
//#define UMAAI_TESTCARDSSINGLE   //�⿨���������ſ�����ֻ�ı�һ��
//#define UMAAI_SIMULATOR   //����ģ����
//#define UMAAI_SELFPLAY   //�����ݣ�����������ѵ����
//#define UMAAI_TESTLIBTORCH   //����c++��torch
//#define UMAAI_MODELBENCHMARK   //�����������ٶ�
//#define UMAAI_TESTSCORESEARCH //�������ؿ���ǿ��
//#define UMAAI_TESTSCORENOSEARCH //����������/��д�߼�policyǿ��

#if defined UMAAI_TESTSCORE || defined UMAAI_SIMULATOR 
#define PRINT_GAME_EVENT
#endif

const bool PrintHandwrittenLogicValueForDebug = false;

#define BACKEND_NONE 0//��ʹ��������
#define BACKEND_LIBTORCH 1//ʹ��libtorch(GPU��CPU)����������
#define BACKEND_CUDA 2//ʹ��cuda(GPU)����������
#define BACKEND_EIGEN 3//ʹ��Eigen��(CPU)����������

#define USE_BACKEND BACKEND_NONE

const int MAX_SCORE = 200000;//�������ķ�����70000��larc�籾�϶��ܹ����ˣ������ų�selfplay��������Ŀ��ִ���һЩ�������

#if USE_BACKEND == BACKEND_LIBTORCH || defined UMAAI_TESTLIBTORCH

const int LIBTORCH_USE_GPU = true;//�Ƿ�ʹ��GPU

//�޸���������Ŀ¼��ͬʱ�����Ӱ���Ŀ¼Ҳ��Ҫ�޸�
#define TORCH_LIBROOT "C:/local/libtorch/lib/"
#define TORCH_LIBROOT_DEBUG "C:/local/libtorch_debug/lib/"

#endif


#if USE_BACKEND == BACKEND_CUDA

//�޸�����Ŀ¼��ͬʱ�����Ӱ���Ŀ¼Ҳ��Ҫ�޸�
#define CUDA_LIBROOT "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.1/lib/x64/"

#endif