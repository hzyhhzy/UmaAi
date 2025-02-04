#pragma once
#include <vector>
#include <string>
#include "../config.h"
struct SelfplayParam
{
  int threadNum = 16;
  int threadNumInner = 1;
  int maxSampleNum = 1 * 16 * 1;

#if USE_BACKEND == BACKEND_LIBTORCH
  std::string modelPath = "./db/model_traced.pt";
  int batchsize = 1024;
#elif USE_BACKEND == BACKEND_NONE
  std::string modelPath = "";
  int batchsize = 16;
#else
  std::string modelPath = "./db/model.txt";
  int batchsize = 1024;
#endif

  std::string exportDataDir = "./selfplay/0/";
  int sampleNumEachFile = 128;

  int searchN = 1024;
  int searchGroupSize = 1024;
  double searchCpuct = 1.0; 
  double policyDelta = 100.0;//����ÿ���Ͷ��٣�policy���1/e��

  //radicalFactor=radicalFactor_scale * (1/pow(rand(),radicalFactor_pow) - 1)
  double radicalFactor_scale = 2.0;
  double radicalFactor_pow = 1.0;
  double radicalFactor_max = 50;

  //maxDepth_fullProb��������������ֽ�����������log(maxDepth)~��̬�ֲ�(logmean,logstdev)
  double maxDepth_logmean = 2.3;
  double maxDepth_logstdev = 0.7;
  int maxDepth_min = 5;
  double maxDepth_fullProb = 1.0;

  //�俨�������ʽ
  //0 �̶�����̶����̶�����
  //1 ��������̶���ssr���ˣ�������������ѡ����Խǿ�Ŀ�����Խ��
  //2 ��1�Ļ����ϣ�������ӳɣ����Ĳ���Ҳ�Ŷ������������
  int cardRandType = 2;
  
};