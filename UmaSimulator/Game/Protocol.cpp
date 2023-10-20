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

int mask_scId(int scId)
{
    return scId % 100000;
}

bool Game::loadGameFromJson(std::string jsonStr)
{
  
  try
  {
    json j = json::parse(jsonStr, nullptr, true, true);
    URAResponse resp = j.get<URAResponse>();

    // 转换到Game对象中
    umaId = mask_umaId(resp.umaId); // 暂时还是mask一下
    turn = resp.turn;   // 从0开始
    if (turn >= TOTAL_TURN && turn < 0)
        throw string("回合数不正确");
    vital = resp.vital;
    maxVital = resp.maxVital;
    isQieZhe = resp.isQieZhe;
    isAiJiao = resp.isAiJiao;
    failureRateBias = resp.failureRateBias;
    copy(resp.fiveStatus.begin(), resp.fiveStatus.end(), fiveStatus);
    copy(resp.fiveStatusLimit.begin(), resp.fiveStatusLimit.end(), fiveStatusLimit);
    skillPt = resp.skillPt;
    motivation = resp.motivation;
    normalCardCount = 5;    // 默认值
    saihou = 0; // 默认值
    // todo: 马娘，支援卡
    //copy(resp.cardJiBan.begin(), resp.cardJiBan.end(), cardJiBan);
    // 小黑板的训练等级要除以3
    for (int i = 0; i < 5; ++i)
        trainLevelCount[i] = resp.trainLevelCount[i] / 3;
    copy(resp.zhongMaBlueCount.begin(), resp.zhongMaBlueCount.end(), zhongMaBlueCount);
    copy(resp.zhongMaExtraBonus.begin(), resp.zhongMaExtraBonus.end(), zhongMaExtraBonus);
    isRacing = resp.isRacing;
    stageInTurn = resp.stageInTurn;
    motivationDropCount = resp.motDropCount;
    // 凯旋门相关
    larc_isAbroad = resp.larcData.isAbroad;
    larc_shixingPt = resp.larcData.shixingPt;
    larc_supportPtAll = resp.larcData.totalApproval;
    larc_isSSS = resp.larcData.isSpecialMatch;
    larc_ssWin = resp.larcData.ssCount;
    larc_ssWinSinceLastSSS = resp.larcData.contNonSSS * 5;
    larc_ssPersonsCount = resp.larcData.currentSSCount;
    larc_zuoyueFirstClick = resp.friendCardFirstClick;
    larc_zuoyueOutgoingRefused = resp.friendCardUnlockOutgoing;
    larc_zuoyueOutgoingUsed = resp.friendCardOutgoingStage;

    // larc rivals/Persons 信息
    if (!resp.larcData.isBegin) {
        copy(resp.larcData.lessonLevels.begin(), resp.larcData.lessonLevels.end(), larc_levels); 
        memset(larc_ssPersons, 0, sizeof(int16_t) * 5);
        copy(resp.larcData.currentSSRivals.begin(), resp.larcData.currentSSRivals.end(), larc_ssPersons);

        for (int i = 0; i < resp.larcData.rivals.size(); ++i)
        {
            auto rival = &resp.larcData.rivals[i];
            if (rival->nextThreeEffects[0] > 0) // 排除自己
            {
                // 按顺序覆盖以前的Person[0..14]信息
                persons[i].personType = rival->isCard ? 2 : 3; 
                persons[i].larc_isLinkCard = rival->isLink;
                // if 是支援卡
                // cardIdInGame, isShining, friendship, isHint, cardRecord
                persons[i].charaId = rival->id;
                persons[i].larc_charge = rival->boost;
                persons[i].larc_statusType = rival->commandId;  // 已经转成了01234
                persons[i].larc_specialBuff = rival->specialEffect;
                persons[i].larc_level = rival->lv;
                copy(rival->nextThreeEffects.begin(), rival->nextThreeEffects.end(), persons[i].larc_nextThreeBuffs);
            }
        }
    }
    /*
    venusLevelYellow = j["venusLevelYellow"];
    venusLevelRed = j["venusLevelRed"];
    venusLevelBlue = j["venusLevelBlue"];

    for (int i = 0; i < 8; i++)
      venusSpiritsBottom[i] = j["venusSpiritsBottom"][i];

    for (int i = 0; i < 6; i++)
      venusSpiritsUpper[i] = j["venusSpiritsUpper"][i];

    venusAvailableWisdom = j["venusAvailableWisdom"];
    venusIsWisdomActive = j["venusIsWisdomActive"];
    venusCardFirstClick = j["venusCardFirstClick"];
    venusCardUnlockOutgoing = j["venusCardUnlockOutgoing"];
    venusCardIsQingRe = j["venusCardIsQingRe"];
    venusCardQingReContinuousTurns = j["venusCardQingReContinuousTurns"];

    for (int i = 0; i < 5; i++)
      venusCardOutgoingUsed[i] = j["venusCardOutgoingUsed"][i];

    // std::cout << "VenusCard load finished\n";

    stageInTurn = j["stageInTurn"];
    for (int i = 0; i < 5; i++)
      for (int k = 0; k < 8; k++)
      {
        cardDistribution[i][k] = j["cardDistribution"][i][k];
      }

    for (int i = 0; i < 6; i++)
      cardHint[i] = j["cardHint"][i];

    for (int i = 0; i < 8; i++)
      spiritDistribution[i] = j["spiritDistribution"][i];


    // 5号是友人或团队
    if ( GameDatabase::AllCards[cardId[0]].cardType != 5)//1号位不是神团，交换卡组位置，把神团换到1号位
    {
      int s = -1;//神团原位置
      for (int i = 1; i < 6; i++)
      {
        if (GameDatabase::AllCards[cardId[i]].cardType == 5)
        {
          s = i;
          break;
        }
      }
      if (s == -1)
        throw string("没带神团");

      std::swap(cardId[s], cardId[0]);
      std::swap(cardData[s], cardData[0]);
      std::swap(cardJiBan[s], cardJiBan[0]);

      for (int i = 0; i < 5; i++)
        std::swap(cardDistribution[i][s], cardDistribution[i][0]);

      std::swap(cardHint[s], cardHint[0]);
    }

    // std::cout << "Swap load finished\n";

    initRandomGenerators();
    calculateVenusSpiritsBonus();

    for (int i = 0; i < 5; i++)
      for (int k = 0; k < 7; k++)
      {
        trainValue[i][k] = j["trainValue"][i][k];
      }

    for (int i = 0; i < 5; i++)
      failRate[i] = j["failRate"][i];
    
    */
    memset(trainValue, 0, sizeof(int16_t) * 5 * 7);
    memset(larc_ssValue, 0, sizeof(int16_t) * 5);
    calculateTrainingValue();
  }
  catch (string e)
  {
    cout << "读取游戏信息json出错：" << e << endl << "-- json --" << endl << jsonStr << endl;
    return false;
  }
  catch (std::exception& e)
  {
      cout << "读取游戏信息json出错：未知错误" << endl << e.what()
          << endl << "-- json --" << endl << jsonStr << endl;
    return false;
  }
  catch (...)
  {
    cout << "读取游戏信息json出错：未知错误"  << endl;
    return false;
  }

  return true;
}

