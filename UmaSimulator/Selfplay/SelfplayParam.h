#pragma once
#include <vector>
#include <string>
struct SelfplayParam
{
  int threadNum = 16;
  int threadNumInner = 1;
  int maxSampleNum = 100 * 1024 * 16;
  int batchsize = 128;
  std::string modelPath = "";
  std::string exportDataDir = "./selfplay/0/";
  int sampleNumEachFile = 1024;

  //log(searchN)~正态分布(searchN_logmean,searchN_logstdev)
  double searchN_logmean = 6.24;//512
  double searchN_logstdev = 1.0;
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
  
};