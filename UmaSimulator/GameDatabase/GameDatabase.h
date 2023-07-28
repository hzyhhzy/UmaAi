#pragma once
#include <string>
#include <unordered_map>
#include "UmaData.h"
#include "CardData.h"
#include "SkillData.h"

class GameDatabase
{
public:
  static std::unordered_map<int, UmaData> AllUmas;
  static std::unordered_map<int, SupportCard> AllCards;

  static void loadUmas(const std::string& dir);
  static void loadCards(const std::string& dir);
};