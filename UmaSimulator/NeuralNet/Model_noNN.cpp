#include "../config.h"
#if USE_BACKEND == BACKEND_NONE
#include <cassert>
#include "Model.h"
#include "iostream"
using namespace std;

Model::Model(std::string path, int batchsize)
{
  if (path == "")
    return;
  assert(false);
}

void Model::evaluate(float* inputBuf, float* outputBuf, int gameNum)
{
  assert(false);
}

void Model::printBackendInfo()
{
    cout << "AI°æ±¾£ºÊÖÐ´Âß¼­+MonteCarlo" << endl;
}
#endif