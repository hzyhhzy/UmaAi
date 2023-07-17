#include "Model.h"

float ModelOutputValueV1::extract(int i)
{
  return i == 0 ? winrate : avgScoreMinusTarget;
}
