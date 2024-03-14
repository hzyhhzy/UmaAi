#include "SearchParam.h"
#include "../GameDatabase/GameConstants.h"

SearchParam::SearchParam() :
  searchSingleMax(-1),
  searchTotalMax(0),
  searchGroupSize(128),
  searchCpuct(1.0),
  maxDepth(TOTAL_TURN),
  maxRadicalFactor(0.0)
{
}

SearchParam::SearchParam(int searchSingleMax, double maxRadicalFactor) :
  searchSingleMax(searchSingleMax),
  searchTotalMax(0),
  searchGroupSize(128),
  searchCpuct(1.0),
  maxDepth(TOTAL_TURN),
  maxRadicalFactor(maxRadicalFactor)
{
}

SearchParam::SearchParam(int searchSingleMax, int searchTotalMax, int searchGroupSize, double searchCpuct, int maxDepth, double maxRadicalFactor) :
  searchSingleMax(searchSingleMax),
  searchTotalMax(searchTotalMax),
  searchGroupSize(searchGroupSize),
  searchCpuct(searchCpuct),
  maxDepth(maxDepth),
  maxRadicalFactor(maxRadicalFactor)
{
}
