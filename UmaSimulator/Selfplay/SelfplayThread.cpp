#include "SelfplayThread.h"


SelfplayThread::SelfplayThread(SelfplayParam param, Model* model) :param(param), gameGenerator(param, model), search(model, param.batchsize, param.threadNumInner)
{
  std::random_device rd;
  rand = std::mt19937_64(rd());
  sampleData.resize(param.sampleNumEachFile);
}

void SelfplayThread::run()
{
  int fileNum = 1 + ((param.maxSampleNum - 1) / param.threadNum);
  for (int f = 0; f < fileNum; f++)
  {
    for (int g = 0; g < param.sampleNumEachFile; g++)
      sampleData[g] = generateSingleSample();
    writeDataToFile();
  }
}

TrainingSample SelfplayThread::generateSingleSample()
{
  assert(false);
  return TrainingSample();
}

void SelfplayThread::writeDataToFile()
{
  assert(false);
}
