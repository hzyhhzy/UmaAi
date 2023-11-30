#include <iostream>
#include <sstream>
#include <cassert>
#include "vector"
#include "../External/json.hpp"
#include "Protocol.h"
#include "Game.h"
using namespace std;
using json = nlohmann::json;

// 是否把低破卡当做满破处理（会导致一定的预测偏差）
// 为True时会把所有ID的最高位改为满破（马娘5xxx，卡4xxx）
static bool maskUmaId = true;

int mask_umaId(int umaId)
{
    return umaId % 1000000;
}

bool Game::loadGameFromJson(std::string jsonStr)
{
  if (jsonStr == "[test]" || jsonStr == "{\"Result\":1,\"Reason\":null}")
  {
    std::cout << "已成功与URA建立连接，但暂未接收到回合信息，等待游戏开始" << std::endl;
    return false;
  }
  try
  {
    json j = json::parse(jsonStr, nullptr, true, true);

    umaId = j["umaId"];
    if (maskUmaId)
      umaId = mask_umaId(umaId);
    if (!GameDatabase::AllUmas.count(umaId))
      throw string("未知马娘，需要更新ai");
    for (int i = 0; i < 5; i++)
      fiveStatusBonus[i] = GameDatabase::AllUmas[umaId].fiveStatusBonus[i];

    turn = j["turn"];
    if (turn >= TOTAL_TURN && turn < 0)
      throw string("回合数不正确");
    isRacing = GameConstants::LArcIsRace[turn];
    larc_isAbroad = (turn >= 36 && turn <= 42) || (turn >= 60 && turn <= 67);

    vital = j["vital"];
    maxVital = j["maxVital"];
    isQieZhe = j["isQieZhe"];
    isAiJiao = j["isAiJiao"];
    failureRateBias = j["failureRateBias"];
    for (int i = 0; i < 5; i++)
      fiveStatus[i] = j["fiveStatus"][i];
    eventStrength = GameConstants::EventStrengthDefault;
    for (int i = 0; i < 5; i++)
      fiveStatusLimit[i] = j["fiveStatusLimit"][i];

    skillPt = j["skillPt"];
    skillScore = 0;
    motivation = j["motivation"];
    isPositiveThinking = j["isPositiveThinking"];

    for (int i = 0; i < 5; i++)
      trainLevelCount[i] = j["trainLevelCount"][i];

    for (int i = 0; i < 5; i++)
      zhongMaBlueCount[i] = j["zhongMaBlueCount"][i];

    for (int i = 0; i < 6; i++)
      zhongMaExtraBonus[i] = j["zhongMaExtraBonus"][i];

    normalCardCount = j["normalCardCount"];


    saihou = 0;
    for (int i = 0; i < 6; i++)
    {
      int cardId = j["cardId"][i];
      int realCardId = cardId / 10;
      int cardLevel = cardId % 10;

      //如果没找到卡，找一下这个卡的高破在不在数据库中
      if (!GameDatabase::AllCards.count(cardId) || GameDatabase::AllCards[cardId].filled == false)
      {
        while (cardLevel < 5)
        {
          cardLevel += 1;
          int cardId1 = realCardId * 10 + cardLevel;
          if (GameDatabase::AllCards.count(cardId1) && GameDatabase::AllCards[cardId1].filled == true)
            break;
          if (cardLevel == 4)
            throw string("未知支援卡，需要更新ai");
        }
      }


      cardParam[i] = GameDatabase::AllCards[realCardId * 10 + cardLevel];
      cardParam[i].cardID = cardId;
      SupportCard& cardP = cardParam[i];
      saihou += cardP.saiHou;
      int cardType = cardP.cardType;
      if (cardType == 5 || cardType == 6)
      {
        if (realCardId == 30160 || realCardId == 10094)//佐岳卡
        {
          if (realCardId == 30160)
            larc_zuoyueType = 1;
          else
            larc_zuoyueType = 2;

          int zuoyueLevel = cardId % 10;
          if (larc_zuoyueType == 1)
          {
            larc_zuoyueVitalBonus = GameConstants::ZuoyueVitalBonusSSR[zuoyueLevel];
            larc_zuoyueStatusBonus = GameConstants::ZuoyueStatusBonusSSR[zuoyueLevel];
          }
          else
          {
            larc_zuoyueVitalBonus = GameConstants::ZuoyueVitalBonusR[zuoyueLevel];
            larc_zuoyueStatusBonus = GameConstants::ZuoyueStatusBonusR[zuoyueLevel];
          }
          larc_zuoyueVitalBonus += 1e-10;
          larc_zuoyueStatusBonus += 1e-10;//加个小量，避免因为舍入误差而算错

        }
        else
        {
          throw string("不支持带佐岳以外的友人或团队卡");
        }
      }
    }

    for (int i = 0; i < 18; i++) //人头
    {
      auto p = j["persons"][i];
      persons[i].personType = p["personType"];
      if (persons[i].personType == 0)persons[i].personType = 3;
      persons[i].charaId = p["charaId"];
      persons[i].cardIdInGame = p["cardIdInGame"];
      persons[i].friendship = p["friendship"];
      persons[i].isHint = p["isHint"];
      persons[i].cardRecord = p["cardRecord"];
      persons[i].larc_charge = p["larc_charge"];
      persons[i].larc_statusType = p["larc_statusType"];
      persons[i].larc_specialBuff = p["larc_specialBuff"];
      persons[i].larc_level = p["larc_level"];
      persons[i].larc_buffLevel = p["larc_buffLevel"];
      for (int k = 0; k < 3; k++)
        persons[i].larc_nextThreeBuffs[k] = p["larc_nextThreeBuffs"][k];
      if (persons[i].personType == 2)//速耐力根智卡
      {
        SupportCard& cardP = cardParam[persons[i].cardIdInGame];
        persons[i].larc_isLinkCard = cardP.larc_isLink;

        std::vector<int> probs = { 100,100,100,100,100,50 }; //基础概率，速耐力根智鸽
        // 得意率固有需要在这里特判
        double deYiLv = cardP.deYiLv;
        probs[cardP.cardType] += deYiLv;  
        persons[i].distribution = std::discrete_distribution<>(probs.begin(), probs.end());
      }
      else
      {
        persons[i].larc_isLinkCard = false;
        std::vector<int> probs = { 1,1,1,1,1,1 }; //基础概率，速耐力根智鸽
        persons[i].distribution = std::discrete_distribution<>(probs.begin(), probs.end());
      }
    }

    //motivationDropCount = j["motivationDropCount"];
    motivationDropCount = 0;

    larc_supportPtAll = j["larc_supportPtAll"];
    larc_shixingPt = j["larc_shixingPt"];
    for (int i = 0; i < 10; i++)
      larc_levels[i] = j["larc_levels"][i];


    larc_isSSS = j["larc_isSSS"];
    larc_ssWin = j["larc_ssWin"];
    larc_ssWinSinceLastSSS = j["larc_ssWinSinceLastSSS"];
    for (int i = 0; i < 9; i++)
      larc_allowedDebuffsFirstLarc[i] = false;
    larc_allowedDebuffsFirstLarc[4] = true; //todo 用户自定义

    larc_zuoyueFirstClick = j["larc_zuoyueFirstClick"];
    larc_zuoyueOutgoingUnlocked = j["larc_zuoyueOutgoingUnlocked"];
    larc_zuoyueOutgoingRefused = j["larc_zuoyueOutgoingRefused"];
    larc_zuoyueOutgoingUsed = j["larc_zuoyueOutgoingUsed"];

    stageInTurn = 1;


    for (int i = 0; i < 5; i++)
      for (int k = 0; k < 5; k++)
      {
        personDistribution[i][k] = j["personDistribution"][i][k];
      }

    larc_ssPersonsCount = j["larc_ssPersonsCount"];
    for (int i = 0; i < 5; i++)
      larc_ssPersons[i] = j["larc_ssPersons"][i];
    larc_ssPersonsCountLastTurn = larc_ssPersonsCount;

    calculateTrainingValue(); //补全一些没填的信息

    for (int i = 0; i < 5; i++)
      for (int k = 0; k < 7; k++)
      {
        trainValue[i][k] = j["trainValue"][i][k];
      }

    for (int i = 0; i < 5; i++)
      failRate[i] = j["failRate"][i];

    calculateTrainingValue();
  }
  catch (string e)
  {
    cout << "读取游戏信息json出错：" << e << endl;
    //cout << "-- json --" << endl << jsonStr << endl;
    return false;
  }
  catch (std::exception& e)
  {
    cout << "读取游戏信息json出错：未知错误" << endl << e.what() << endl;
    //cout << "-- json --" << endl << jsonStr << endl;
    return false;
  }
  catch (...)
  {
    cout << "读取游戏信息json出错：未知错误"  << endl;
    return false;
  }

  return true;
}

