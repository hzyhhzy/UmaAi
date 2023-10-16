#include "Model.h"

float ModelOutputValueV1::extract(int i)
{
  assert(false);
  return i == 0 ? winRate : scoreMean;
}
