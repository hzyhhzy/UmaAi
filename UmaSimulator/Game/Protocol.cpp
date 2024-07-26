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
    auto rand = mt19937_64(114514);
    int newcards[6];
    int newzmbluecount[5];
    for (int i = 0; i < 6; i++) {
        newcards[i] = j["cardId"][i];
        if(i<5){ newzmbluecount[i] = j["zhongMaBlueCount"][i]; }
        
    }
    //int zhongmaBlue[5] = { 18,0,0,0,0 };
    int zhongmaBonus[6] = { 10,10,30,0,10,70 };
    newGame(rand,true,j["umaId"], j["umaStar"],newcards, newzmbluecount,zhongmaBonus);
    
    ptScoreRate = j.contains("ptScoreRate") ? double(j["ptScoreRate"]) : GameConstants::ScorePtRateDefault;
    
    turn = j["turn"];
    gameStage = GameStage_beforeTrain;
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
          personDistribution[i][p] = PSID_noncardYayoi;
        }
        else if (pid == 103) {
          personDistribution[i][p] = PSID_noncardReporter;
        }
        else if (pid >= 1000) {
          personDistribution[i][p] = PSID_npc;
        }
        else if (pid >= 0 && pid < 9)
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

    for (int i = 0; i < 5; i++) {
      cook_material[i] = j["cook_material"][i];
    }
    cook_dish_pt = j["cook_dish_pt"];
    cook_dish_pt_turn_begin = cook_dish_pt;
    cook_dish = j["cook_dish"];
    if (cook_dish != DISH_none)
    {
      int lastDishPtGain = GameConstants::Cook_DishGainPt[cook_dish];
      cook_dish_pt_turn_begin -= lastDishPtGain;
    }
    for (int i = 0; i < 5; i++) {
      cook_farm_level[i] = j["cook_farm_level"][i];
    }

    cook_farm_pt = j["cook_farm_pt"];
    cook_dish_sure_success = j["cook_dish_sure_success"];
    //注意cook_dish_sure_success是下一个料理是否大成功，而不是这个料理是否大成功
    if (cook_dish != DISH_none)
    {
      cook_dish_sure_success = false;
      //如果跨越1500倍数了，或者大于12000，下次必为大成功
      if (cook_dish_pt >= 12000 || cook_dish_pt / 1500 != cook_dish_pt_turn_begin / 1500)
        cook_dish_sure_success = true;
    }
    for (int i = 0; i < 5; i++) {
      cook_win_history[i] = j["cook_win_history"][i];
    }
    
    for (int i = 0; i < 4; i++) {
      cook_harvest_history[i] = j["cook_harvest_history"][i];
      cook_harvest_green_history[i] = j["cook_harvest_green_history"][i];
    }
    //calculate cook_harvest_extra
    if (isXiahesu() || turn >= 72)
    {
      for (int i = 0; i < 5; i++)
        cook_harvest_extra[i] = 0;
    }
    else
    {
      int harvestTurnNum = turn % 4;
      vector<int> harvestBasic = { 0,0,0,0,0 };
      int greenNum = 0;
      for (int i = 0; i < 5; i++)
      {
        harvestBasic[i] = GameConstants::Cook_HarvestBasic[cook_farm_level[i]];
      }
      for (int i = 0; i < harvestTurnNum; i++)
      {
        int matType = cook_harvest_history[i];
        if (matType == -1)
          throw "ERROR: Game::loadGameFromJson cook_harvest_history[i] == -1";

        harvestBasic[matType] += GameConstants::Cook_HarvestExtra[cook_farm_level[matType]];
        if (cook_harvest_green_history[i])
          greenNum += 1;
      }

      double multiplier = 
        greenNum == 0 ? 1.0000001 :
        greenNum == 1 ? 1.1000001 :
        greenNum == 2 ? 1.2000001 :
        greenNum == 3 ? 1.4000001 :
        greenNum == 4 ? 1.6000001 :
        1000;

      for (int i = 0; i < 5; i++)
      {
        int harvestNum = j["cook_harvest_num"][i];
        int extra = int(ceil(harvestNum / multiplier - harvestBasic[i]));
        cook_harvest_extra[i] = extra;
      }
    }

    

    for (int i = 0; i < 8; i++) {
      cook_train_material_type[i] = j["cook_train_material_type"][i];
      cook_train_green[i] = j["cook_train_green"][i];
    }
    cook_main_race_material_type = j.contains("cook_main_race_material_type") ? int(j["cook_main_race_material_type"]) : -1;

    if (friend_type != 0) {
      friend_outgoingUsed = j["friend_outgoingUsed"];
      friend_stage = j["friend_stage"];
    }

    updateDeyilv();
    calculateTrainingValue();
  //for (int k = 1; k < 5; k++) {
   //     cout << trainValue[1][k] << endl;
   // }
    
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

