#pragma once
#include <vector>
#include <string>
struct SelfplayParam
{
  int threadNum = 24;
  int threadNumInner = 1;
  int maxSampleNum = 1000 * 1024 * 16;
  int batchsize = 4;
  //std::string modelPath = "model_traced.pt";
  std::string modelPath = "";
  std::string exportDataDir = "./selfplay/0/";
  int sampleNumEachFile = 1024;

  //log(searchN)~正态分布(searchN_logmean,searchN_logstdev)
  double searchN_logmean = 6.93;//1024
  double searchN_logstdev = 0.0;
  int searchN_min = 128;
  int searchN_max = 65536;

  //radicalFactor=radicalFactor_scale * (1/pow(rand(),radicalFactor_pow) - 1)
  double radicalFactor_scale = 2.0;
  double radicalFactor_pow = 1.0;
  double radicalFactor_max = 50;

  //maxDepth_fullProb的样本搜索到这局结束，其他的log(maxDepth)~正态分布(logmean,logstdev)
  double maxDepth_logmean = 2.3;
  double maxDepth_logstdev = 0.7;
  int maxDepth_min = 5;
  double maxDepth_fullProb = 1.0;

  //配卡的随机方式
  //0 固定卡组固定马固定种马
  //1 随机马，固定带ssr友人，随机种马，随机选卡、越强的卡概率越大，
  //2 在1的基础上，随机马加成，卡的参数也扰动，随机带友人
  int cardRandType = 2;
  
};