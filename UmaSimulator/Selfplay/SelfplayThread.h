#pragma once
#include <vector>
#include <string>
#include "GameGenerator.h"
#include "../NeuralNet/TrainingSample.h"
#include "../Search/Search.h"
class SelfplayThread
{
  SelfplayParam param;
  GameGenerator gameGenerator;
  Search search;
  std::mt19937_64 rand;
  std::vector<TrainingSample> sampleData;

  std::vector<float> nnInputBuf;
  std::vector<float> nnOutputBuf;
  TrainingSample generateSingleSample();
  void writeDataToFile();
public:
  SelfplayThread(SelfplayParam param, Model* model);
  void run();
};