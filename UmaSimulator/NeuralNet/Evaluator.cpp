#include <cassert>
#include <iostream>
#include "Evaluator.h"

void Evaluator::evaluate(const Game* games, const float* targetScores, int mode, int gameNum)
{
  assert(gameNum <= maxBatchsize);
  if (model == NULL)//没神经网络，手写逻辑
  {
    if (mode == 0)//value，必须终局才可计算
    {
      for (int i = 0; i < gameNum; i++)
      {
        const Game& game = games[i];
        assert(game.isEnd() && "无神经网络时，只有游戏结束后才可计算value");
        int score = game.finalScore();
        if (score >= targetScores[i])
          valueResults[i].winRate = 1.0;
        else valueResults[i].winRate = 0.0;
        valueResults[i].scoreMean = score - targetScores[i];
        assert(false);
      }
    }
    else if (mode == 1)//policy，手写逻辑，最优的选择是1，其他的是0
    {
      for (int i = 0; i < gameNum; i++)
      {
        assert(false);
        //policyResults[i] = handWrittenPolicy(games[i]);
      }
    }
  }
  else
  {
    assert(false && "还没写");
  }
}

Evaluator::Evaluator(Model* model, int maxBatchsize):model(model), maxBatchsize(maxBatchsize)
{
  inputBuf.resize(NNINPUT_CHANNELS_V1 * maxBatchsize);
  outputBuf.resize(NNOUTPUT_CHANNELS_V1 * maxBatchsize);
  valueResults.resize(maxBatchsize);
  policyResults.resize(maxBatchsize);
  
}


static double vitalEvaluation(int vital,int maxVital)
{
  if (vital <= 50)
    return 2.0 * vital;
  else if (vital <= 70)
    return 1.5 * (vital - 50) + vitalEvaluation(50, maxVital);
  else if (vital <= maxVital - 10)
    return 1.0 * (vital - 70) + vitalEvaluation(70, maxVital);
  else
    return 0.5 * (vital - (maxVital - 10)) + vitalEvaluation(maxVital - 10, maxVital);
}


static const int finalBonus = 70;//剧本结束时的固定属性收益

const double jibanValue = 2.5;
const double friendValue_nonAbroad = 10;//非海外点友人的分数，不包括额外充电
const double friendValue_abroad = 10;//海外点友人的分数，不包括额外适性pt
const double shixingPtValueSecondYear = 0.3;//第二年远征适性pt价值。第三年视为0
const double ssPriorityValue = 130;//攒够5人时，若训练value大于这个值则攒着ss，否则爆掉。最后一回合的ss单独处理
const double vitalFactor = 1;
const double smallFailValue = -30;
const double bigFailValue = -90;
const double wizFailValue = 5;
const double statusWeights[5] = { 1.0,1.0,1.0,1.0,1.0 };
const double ptWeight = 0.5;
const double restValueFactor = 1.5;//休息估值权重
const float remainStatusFactorEachTurnAbroad = 40;//控属性时给每回合预留多少（远征后）
const float remainStatusFactorEachTurnBeforeAbroad = 20;//控属性时给每回合预留多少（远征前）
const double outgoingBonusIfNotFullMotivation = 30;//掉心情时提高外出分数


Action Evaluator::handWrittenStrategy(const Game& game)
{
  Action action;
  action.train = -1;
  action.buy50p = false;
  action.buyFriend20 = false;
  action.buyPt10 = false;
  action.buyVital20 = false;
  if(game.isEnd())
    return action;



  if (game.isRacing)//比赛回合
  {
    assert(false);
    return action;
  }
  double basicChoiceValues[9];//9个基础项目（训练，休息，外出）
  for (int i = 0; i < 9; i++)
    basicChoiceValues[i] = -100000;



  int vitalAfterRest = std::min(int(game.maxVital), int(50 + game.vital));
  if (game.turn == 65)vitalAfterRest = game.vital;//最后一回合
  else if (game.turn == 63)vitalAfterRest = std::min(50, vitalAfterRest);//倒数第二回合
  else if (game.turn == 62)vitalAfterRest = std::min(65, vitalAfterRest);//倒数第三回合
  else if (game.turn == 61)vitalAfterRest = std::min(80, vitalAfterRest);//倒数第四回合
  else if (game.turn == 60)vitalAfterRest = std::min(95, vitalAfterRest);//倒数第五回合
  double restValue = restValueFactor * (vitalEvaluation(vitalAfterRest, game.maxVital) - vitalEvaluation(game.vital, game.maxVital));
  
  bool friendOutgoingAvailable = game.larc_zuoyueOutgoingUnlocked && game.larc_zuoyueOutgoingUsed < 5 && !game.larc_isAbroad;
  
  if (friendOutgoingAvailable)
  {
    if (game.motivation < 5)restValue += outgoingBonusIfNotFullMotivation;
    basicChoiceValues[7] = restValue;
  }
  else
  {
    if (game.motivation < 5 && game.larc_isAbroad && game.turn != 65)restValue += outgoingBonusIfNotFullMotivation;
    if (game.turn == 65)restValue = 0;
    basicChoiceValues[6] = restValue;
  }

  //计算ss的价值
  if (game.turn == 58)//最后一回合，直接看ss本身的价值
  {
    if (game.larc_isSSS)
      basicChoiceValues[5] = 200;
    else
      basicChoiceValues[5] = 25 * game.larc_ssPersonsCount;
  }
  else
  {
    if (game.larc_ssPersonsCount >= 5)
      basicChoiceValues[5] = ssPriorityValue;
    else
      basicChoiceValues[5] = 0;
  }

  for (int item = 0; item < 5; item++)
  {
    double value = 0;



    //充电量
    if (game.turn >= 2 && !game.larc_isAbroad)
    {
      double expectChargeNum = 0;
      double chargeValue = game.turn < 20 ? 12.0 :
        game.turn < 40 ? 10.0 :
        game.turn < 58 ? 5.0 :
        0.0;


      bool haveZuoyue = false;
      int chargeN = game.trainShiningNum[item] + 1;
      int totalCharge = 0;
      int totalChargeFull = 0;
      for (int j = 0; j < 5; j++)
      {
        int p = game.personDistribution[item][j];
        if (p < 0)break;//没人
        int personType = game.persons[p].personType;

        if (personType == 1)//佐岳卡
        {
          haveZuoyue = true;
        }
        else if (personType == 2 || personType == 3)//普通卡,npc
        {
          totalCharge += std::min(chargeN, 3 - game.persons[p].larc_charge);
          if (game.persons[p].larc_charge < 3 && game.persons[p].larc_charge + chargeN >= 3)
            totalChargeFull += 1;
        }
      }
      expectChargeNum = totalCharge;
      if (haveZuoyue)
      {
        value += friendValue_nonAbroad;
        expectChargeNum += 2;
        if (game.larc_zuoyueType == 1 && game.persons[17].friendship < 60)
          expectChargeNum += 2;
          
      }
      value += chargeValue * expectChargeNum;

    }
    else if (game.larc_isAbroad && game.turn < 50)//第二年远征
    {
      value += shixingPtValueSecondYear * game.larc_shixingPtGainAbroad[item];

      for (int j = 0; j < 5; j++)
      {
        int p = game.personDistribution[item][j];
        if (p < 0)break;//没人
        int personType = game.persons[p].personType;

        if (personType == 1)//佐岳卡
        {
          value += shixingPtValueSecondYear * 20;
          value += friendValue_abroad;
          break;
        }
      }
    }






    //处理hint和羁绊
    int cardHintNum = 0;//所有hint随机取一个，所以打分的时候取平均
    for (int j = 0; j < 5; j++)
    {
      int p = game.personDistribution[item][j];
      if (p < 0)break;//没人
      if (game.persons[p].isHint)
        cardHintNum += 1;
    }

    for (int j = 0; j < 5; j++)
    {
      int pi = game.personDistribution[item][j];
      if (pi < 0)break;//没人
      const Person& p = game.persons[pi];
      if (p.personType != 2)continue;//充电和友人已经在前面考虑了

      if (p.friendship < 80)
      {
        float jibanAdd = 7;
        if (game.isAiJiao)jibanAdd += 2;
        if (p.isHint)
        {
          jibanAdd += 5 / cardHintNum;
          if (game.isAiJiao)jibanAdd += 2 / cardHintNum;
        }
        jibanAdd = std::min(float(80 - p.friendship), jibanAdd);

        value += jibanAdd * jibanValue;
      }
      
      if (p.isHint)
      {
        for (int i = 0; i < 5; i++)
          value += game.cardParam[p.cardIdInGame].hintBonus[i] * statusWeights[i] / cardHintNum;
        value += game.cardParam[p.cardIdInGame].hintBonus[5] * ptWeight / cardHintNum;
      }

    }
      
    for (int i = 0; i < 5; i++)
    {
      //不仅要考虑属性加多少，还要考虑是否溢出
      float gain = game.trainValue[item][i];
      float remain = game.fiveStatusLimit[i] - game.fiveStatus[i] - finalBonus;
      if (gain > remain)gain = remain;
      float turnReserve = game.turn >= 60 ?
        remainStatusFactorEachTurnAbroad * (TOTAL_TURN - game.turn - 2) :
        remainStatusFactorEachTurnBeforeAbroad * (TOTAL_TURN - game.turn - 2);//距离上限不到turnReserve时降低权重
        


      float remainAfterTrain = remain - gain;

      if (remainAfterTrain < turnReserve)//从remain-turnReserve到remain逐渐降低权重
      {
        if (remain < turnReserve)//训练前就进入了turnReserve区间
        {
          gain = 0.5 * (remain * remain - remainAfterTrain * remainAfterTrain) / turnReserve;
        }
        else
        {
          gain = (remain - turnReserve) + 0.5 * turnReserve - 0.5 * remainAfterTrain * remainAfterTrain / turnReserve;
        }
      }

          
      value += gain * statusWeights[i];
    }
    double ptWeightThisGame = ptWeight;
    if (game.isQieZhe) 
      ptWeightThisGame *= GameConstants::ScorePtRateQieZhe / GameConstants::ScorePtRate;
    value += game.trainValue[item][5] * ptWeightThisGame;
    //value += vitalValue * game.trainValue[item][6];

    int vitalAfterTrain = std::min(int(game.maxVital), game.trainValue[item][6] + game.vital);
    value += vitalFactor * (vitalEvaluation(vitalAfterTrain, game.maxVital) - vitalEvaluation(game.vital, game.maxVital));
        

    double failRate = game.failRate[item];
    if (failRate > 0)
    {
      double failValueAvg = wizFailValue;
      if (item != 5)
      {
        double bigFailProb = failRate;
        if (failRate < 20)bigFailProb = 0;
        failValueAvg = 0.01 * bigFailProb * bigFailValue + (1 - 0.01 * bigFailProb) * smallFailValue;
      }
      value = 0.01 * failRate * failValueAvg + (1 - 0.01 * failRate) * value;
    }
    
    basicChoiceValues[item] = value;

  }

  //找到最好的那个训练
  double bestValue = -1e4;
  for (int i = 0; i < 9; i++)
  {
    if (basicChoiceValues[i] > bestValue)
    {
      action.train = i;
      bestValue = basicChoiceValues[i];
    }
  }
  
  if (game.larc_isAbroad && action.train < 5)//要考虑购买适性升级
  {
    //只考虑购买+50%，体力消耗减少不买，10pt和20%友情出了凯旋门自动买
    int cost50p = 100000;
    if (game.larc_levels[GameConstants::UpgradeId50pEachTrain[action.train]] == 2)
      cost50p = 200;
    else if (game.larc_levels[GameConstants::UpgradeId50pEachTrain[action.train]] == 1)
      cost50p = 300;

    if (game.turn < 50)// 第二年
    {
      int buy50pCount = 0;
      for (int i = 0; i < 5; i++)
      {
        if (game.larc_levels[i] == 3)
          buy50pCount += 1;
      }
      //速耐力根买一个+50%，智不买
      if (buy50pCount == 0 &&
        action.train < 4 &&
        bestValue>100 &&
        game.larc_shixingPt >= cost50p)
      {
        action.buy50p = true;
      }
    }
    else //第三年
    {
      //无脑买
      if (
        action.train < 5 &&
        game.larc_shixingPt >= cost50p)
      {
        action.buy50p = true;
      }

    }
  }
  return action;
}
