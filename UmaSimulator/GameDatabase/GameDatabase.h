#pragma once
#include <string>
#include "UmaData.h"
#include "../SupportCard/SupportCard.h"


class GameDatabase
{
public:
  static const int ALL_SUPPORTCARD_NUM = 11;
  static const std::string AllSupportCardNames[ALL_SUPPORTCARD_NUM];
  static const SupportCard AllSupportCards[ALL_SUPPORTCARD_NUM];


  static const int ALL_UMA_NUM = 2;
  static const std::string AllUmaNames[ALL_UMA_NUM];
  static const UmaData AllUmas[ALL_UMA_NUM];
};