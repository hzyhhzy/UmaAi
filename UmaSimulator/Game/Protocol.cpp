#include <iostream>
#include <sstream>
#include <cassert>
#include "vector"
#include "../External/json.hpp"
#include "Protocol.h"
#include "Game.h"
using namespace std;
using json = nlohmann::json;

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
    auto rand = mt19937_64(114514);
    int newcards[6];
    int newzmbluecount[5];
    for (int i = 0; i < 6; i++) {
        newcards[i] = j["cardId"][i];
        if(i<5){ newzmbluecount[i] = j["zhongMaBlueCount"][i]; }
        
    }
    //int zhongmaBlue[5] = { 18,0,0,0,0 };
    int zhongmaBonus[6] = { 5,5,30,30,5,200 };
    newGame(rand,GameSettings(), j["umaId"], j["umaStar"], newcards, newzmbluecount, zhongmaBonus);
    assert(friend_type == 0 || friend_personId == j["friend_personId"]);
    
    gameSettings.ptScoreRate = j.contains("ptScoreRate") ? double(j["ptScoreRate"]) : GameConstants::ScorePtRateDefault;
    
    turn = j["turn"];
    vital = j["vital"];
    maxVital = j["maxVital"];
    motivation = j["motivation"];
    for (int i = 0; i < 5; i++) {
        fiveStatus[i] = j["fiveStatus"][i];
        fiveStatusLimit[i] = j["fiveStatusLimit"][i];
    }
    
    skillPt = j["skillPt"];
    skillScore = j["skillScore"];
    for (int i = 0; i < 5; i++) {
      trainLevelCount[i] = j["trainLevelCount"][i];
    }

    failureRateBias= j["failureRateBias"];
    isQieZhe = j["isQieZhe"];
    isAiJiao = j["isAiJiao"];
    isPositiveThinking = j["isPositiveThinking"];
    isRefreshMind = j["isRefreshMind"];

    haveCatchedDoll = j.contains("haveCatchedDoll") ? bool(j["haveCatchedDoll"]) : false;

    stage = j["stage"];
    decidingEvent = j["decidingEvent"];
    isRacing = j["isRacing"];
    if (isRacing != isRacingTurn[turn])
    {
      cout << "Warning:实际赛程和预期赛程不一致" << endl;
      isRacingTurn[turn] = isRacing;
    }
    for (int i = 0; i < 6; i++) {
      persons[i].friendship = j["persons"][i]["friendship"];
      persons[i].isHint = j["persons"][i]["isHint"];
    }
    friendship_noncard_yayoi = j["friendship_noncard_yayoi"];
    friendship_noncard_reporter = j["friendship_noncard_reporter"];

    for (int i = 0; i < 5; i++) {
      for (int p = 0; p < 5; p++) {
        int pid = j["personDistribution"][i][p];
        if (pid == 102) {
          personDistribution[i][p] = PS_noncardYayoi;
        }
        else if (pid == 103) {
          personDistribution[i][p] = PS_noncardReporter;
        }
        //else if (pid >= 1000) {
        //  personDistribution[i][p] = PS_npc;
        //}
        else if (pid >= 0 && pid < 16)
        {
          personDistribution[i][p] = pid;
        }
        else if (pid == -1)
        {
          personDistribution[i][p] = -1;
        }
        else
        {
          throw "Game::loadGameFromJson读取到未知的personId:" + to_string(pid);
        }
      }
    }
    


    if (friend_type != 0) {
      for (int i = 0; i < 5; i++) {
        friend_outgoingUsed[i] = j["friend_outgoingUsed"][i];
      }
      friend_stage = j["friend_stage"];
    }
    if (stage == ST_decideEvent && decidingEvent == DecidingEvent_three && friend_stage == FriendStage_notClicked)
    {
      cout << "警告：读取json时，有三选一事件但未记录第一次点击，可能小黑板是半途开启的" << endl;
      friend_stage = FriendStage_beforeUnlockOutgoing;
    }
    friend_qingre = j["friend_qingre"];
    friend_qingreTurn = j["friend_qingreTurn"];

    lg_mainColor = j["lg_mainColor"];
    //if(lg_mainColor!=-1 && lg_mainColor != L_red && lg_mainColor != L_blue)
    //  throw "当前版本暂不支持绿登，请等待新版本";
    for (int i = 0; i < 3; i++) {
      lg_gauge[i] = j["lg_gauge"][i];
    }
    for (int i = 0; i < 8; i++) {
      lg_trainingColor[i] = j["lg_trainingColor"][i];
    }
    for (int i = 0; i < 10; i++) {
      lg_buffs[i].buffId = j["lg_buffs"][i]["buffId"];
      lg_buffs[i].coolTime = j["lg_buffs"][i]["coolTime"];
      lg_buffs[i].isActive = j["lg_buffs"][i]["isActive"];
      if (lg_buffs[i].buffId >= 0)
        lg_haveBuff[lg_buffs[i].buffId] = true;
    }

    lg_pickedBuffsNum = j["lg_pickedBuffsNum"];
    for (int i = 0; i < 9; i++) {
      lg_pickedBuffs[i] = j["lg_pickedBuffs"][i];
    }


    //游戏里的显示是按顺序的
    std::sort(lg_pickedBuffs, lg_pickedBuffs + lg_pickedBuffsNum, ScenarioBuffInfo::defaultOrder);

    lg_blue_active = j["lg_blue_active"];
    lg_blue_remainCount = j["lg_blue_remainCount"];
    lg_blue_currentStepCount = j["lg_blue_currentStepCount"];
    lg_blue_canExtendCount = j["lg_blue_canExtendCount"];
    lg_green_active = j["lg_green_active"];
    lg_green_continuationZoneCount = j["lg_green_continuationZoneCount"];
    lg_green_currentStepCount = j["lg_green_currentStepCount"];

    for (int i = 0; i < 16; i++) {
      lg_red_friendsGauge[i] = j["lg_red_friendsGauge"][i];
      lg_red_friendsLv[i] = j["lg_red_friendsLv"][i];
    }

    calculateScenarioBonus(); 
    calculateTrainingValue();
  //for (int k = 1; k < 5; k++) {
   //     cout << trainValue[1][k] << endl;
   // }
    
  }
  catch (const char* e)
  {
    cout << "\x1b[91m读取游戏信息json出错：" << e << "\x1b[0m" << endl;
    //cout << "-- json --" << endl << jsonStr << endl;
    return false;
  }
  catch (string e)
  {
    cout << "\x1b[91m读取游戏信息json出错：" << e << "\x1b[0m" << endl;
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

