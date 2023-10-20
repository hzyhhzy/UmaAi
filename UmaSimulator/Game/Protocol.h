#pragma once
#include <iostream>
#include <sstream>
#include <cassert>
#include "../External/json.hpp"
#include "Game.h"
#include "vector"
#include "map"
#include "unordered_map"
using namespace std;
using json = nlohmann::json;
using namespace nlohmann::literals;

class VenusDataSet
{
    public:
        int venusLevelYellow;
        int venusLevelRed;
        int venusLevelBlue;
        vector<int> venusSpiritsBottom;
        vector<int> venusSpiritsUpper;
        int venusAvailableWisdom;
        bool venusIsWisdomActive;
        bool venusCardFirstClick;
        bool venusCardUnlockOutgoing;
        bool venusCardIsQingRe;
        int venusCardQingReContinuousTurns;
        vector<bool> venusCardOutgoingUsed;
        vector<int> spiritDistribution;
        vector<int> spiritBonus;

    public:
        // 使用该宏时，Json中不存在的Key可以用默认值替换，但Key=Null时会爆异常。
        // 因此生成JSON时应避免产生Null值，而是直接省略该键
        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VenusDataSet, venusLevelYellow, venusLevelRed, venusLevelBlue,
            venusSpiritsBottom, venusSpiritsUpper, venusAvailableWisdom, venusIsWisdomActive,
            venusCardFirstClick, venusCardUnlockOutgoing, venusCardIsQingRe,
            venusCardQingReContinuousTurns, venusCardOutgoingUsed, spiritDistribution, spiritBonus);
};

class LArcRivalData
{
    public:
        int id;
        int commandId;
        int atTrain;
        int boost;
        int chargeNum;
        int lv;
        int specialEffect;
        vector<int> nextThreeEffects;
        bool hasAiJiao;
        bool hasTrain;
        bool isCard;
        bool isLink;
    public:
        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LArcRivalData, id, commandId, atTrain, boost, chargeNum,
            lv, specialEffect, nextThreeEffects, hasAiJiao, hasTrain, isCard, isLink);
};

class LArcDataSet
{
    public:
        int totalApproval;
        double approvalRate;
        vector<int> lessonLevels;
        int shixingPt;
        vector<LArcRivalData> rivals;
        int ssApprovalRate;
        vector<int> ssStatus;
        vector<int> ssBonusStatus;
        bool isSpecialMatch;
        int16_t ssCount;
        int16_t currentSSCount;
        vector<int16_t> currentSSRivals;
        int16_t contNonSSS;
        vector<int> shiningCount;
        vector<bool> friendAppear;
        vector<int> chargedNum;
        vector<int> chargedFullNum;
        int turn;
        bool isAbroad;
        bool isBegin;

    public:
        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LArcDataSet, totalApproval, approvalRate, lessonLevels, shixingPt, rivals,
            ssApprovalRate, ssStatus, ssBonusStatus, isSpecialMatch, ssCount, 
            currentSSCount, currentSSRivals, contNonSSS, shiningCount, friendAppear,
            chargedNum, chargedFullNum, turn, isAbroad, isBegin);
};

class URAResponse
{
    public:
        int umaId;
        int16_t turn;   // 从0开始，到77结束
        int16_t vital;
        int16_t maxVital;
        bool isQieZhe;
        bool isAiJiao;
        int16_t failureRateBias;
        vector<int16_t> fiveStatus;
        vector<int16_t> fiveStatusLimit;
        int16_t skillPt;
        int16_t motivation;
        vector<int> cardId;
        vector<int16_t> cardJiBan;
        vector<int16_t> trainLevelCount;
        vector<int16_t> zhongMaBlueCount;
        vector<int16_t> zhongMaExtraBonus;
        bool isRacing;

        int fans;
        bool friendCardFirstClick;
        bool friendCardUnlockOutgoing;
        int friendCardOutgoingStage;
        int stageInTurn;
        int motDropCount;
        vector<vector<bool>> cardDistribution;
        vector<bool> cardHint;

        VenusDataSet venusData;
        LArcDataSet larcData;
        vector<vector<int16_t>> trainValue;
        vector<int16_t> failRate;

    public:
        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(URAResponse, umaId, turn, vital, maxVital,
            isQieZhe, isAiJiao, failureRateBias, fiveStatus, fiveStatusLimit, skillPt,
            motivation, cardId, cardJiBan, trainLevelCount, zhongMaBlueCount,
            zhongMaExtraBonus, isRacing, fans, friendCardFirstClick, friendCardUnlockOutgoing,
            friendCardOutgoingStage, stageInTurn, motDropCount, cardDistribution, cardHint,
            venusData, larcData, trainValue, failRate);
};
