#pragma once
#include <string>
#include <unordered_map>
#include "UmaData.h"
#include "../SupportCard/SupportCard.h"
#include "SkillData.h"

class GameDatabase
{
public:
  static std::unordered_map<int, UmaData> AllUmas;
  static std::unordered_map<int, SupportCard> AllCards;
  static std::unordered_map<int, SupportCard> DBCards;

  static void loadUmas(const std::string& dir);
  static void loadCards(const std::string& dir);
  static void loadDBCards(const std::string& pathname);
  
};