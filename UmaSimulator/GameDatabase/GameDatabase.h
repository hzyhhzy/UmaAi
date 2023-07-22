#pragma once
#include <string>
#include <map>
#include "UmaData.h"
#include "../SupportCard/SupportCard.h"


class GameDatabase
{
public:
  static const int ALL_SUPPORTCARD_NUM = 50;
  static const std::string AllSupportCardNames[ALL_SUPPORTCARD_NUM];
  static const std::map<int, int> AllSupportCardGameIdToSimulatorId;
  static SupportCard AllSupportCards[ALL_SUPPORTCARD_NUM];


  static const int ALL_UMA_NUM = 16;
  static const std::string AllUmaNames[ALL_UMA_NUM];
  static const std::map<int, int> AllUmaGameIdToSimulatorId;
  static const UmaData AllUmas[ALL_UMA_NUM];
};