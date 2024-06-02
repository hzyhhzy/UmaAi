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
    turn = j["turn"];
    vital = j["vital"];
    maxVital = j["maxVital"];
    motivation = j["motivation"];
    for (int i = 0; i < 5; i++) {
        fiveStatus[i] = j["fiveStatus"][i];
        fiveStatusLimit[i] = j["fiveStatusLimit"][i];
        uaf_trainingColor[i]= j["uaf_trainingColor"][i] - 1;
    }
    for (int i = 0; i < 9; i++) {
      persons[i].friendship = j["persons"][i]["friendship"];
    }
    for (int i = 0; i < 6; i++) {
      persons[i].isHint = j["persons"][i]["isHint"];
    }
    skillPt = j["skillPt"];
    ptScoreRate = j.contains("ptScoreRate") ? double(j["ptScoreRate"]) : GameConstants::ScorePtRate;
    ptScoreRate = j["isQieZhe"] ? ptScoreRate * 1.1 : GameConstants::ScorePtRate;
    //TODO:SkillScore
    failureRateBias= j["failureRateBias"];
    isAiJiao = j["isAiJiao"];
    isPositiveThinking = j["isPositiveThinking"];
    isRacing = j["isRacing"];
    assert(!isRacing);
    isRacingTurn[turn] = false;
    for (int i = 0; i < 5; i++) {
        for (int p = 0; p < 5; p++) {
            if (j["personDistribution"][i][p] == 102) {
                personDistribution[i][p] = 6;
            }
            else if (j["personDistribution"][i][p] == 103) {
                personDistribution[i][p] = 7;
            }
            else if (j["personDistribution"][i][p] == 111) {
                personDistribution[i][p] = 8;
            }
            else {
                personDistribution[i][p] = j["personDistribution"][i][p];
            }
        }
    }

    lockedTrainingId = j["lockedTrainingId"];
    for (int i = 0; i < 3; i++) {
        for (int p = 0; p < 5; p++) {
            uaf_trainingLevel[i][p] = j["uaf_trainingLevel"][i*5+p];
        }
    }

    for (int a = 0; a < 5; a++) {
        for (int b = 0; b < 3; b++) {
            for (int c = 0; c < 5; c++) {
                uaf_winHistory[a][b][c] = j["uaf_winHistory"][a * 15 + b * 5 + c];

            }
        }
    }
    if (lianghua_type != 0) {
        lianghua_outgoingUsed = j["lianghua_outgoingUsed"];
        for (int i = 0; i < 6; i++) {
          if (persons[i].personType == PersonType_lianghuaCard)
            persons[i].friendOrGroupCardStage = j["lianghua_outgoingStage"];
        }
    }
    uaf_lastTurnNotTrain = j["uaf_rankGainIncreased"];
    uaf_xiangtanRemain = j["uaf_xiangtanRemain"];
    for (int i = 0; i < 3; i++) {
        uaf_buffActivated[i] = j["uaf_buffActivated"][i];
        uaf_buffNum[i]= j["uaf_buffNum"][i];
    }
    cardEffectCalculated = false;
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

