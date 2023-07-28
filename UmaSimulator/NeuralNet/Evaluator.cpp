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
          valueResults[i].winrate = 1.0;
        else valueResults[i].winrate = 0.0;
        valueResults[i].avgScoreMinusTarget = score - targetScores[i];
      }
    }
    else if (mode == 1)//policy，手写逻辑，最优的选择是1，其他的是0
    {
      for (int i = 0; i < gameNum; i++)
      {
        policyResults[i] = handWrittenPolicy(games[i]);
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

static const int fixedBonusAfterTurn[TOTAL_TURN] = //在这个回合之后的各种事件（swbc等）的固定属性收益
{
  166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,166,
  150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,150,//wbc前
  115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,//swbc前，假设一半概率卡红
  74,74,74,74,74,64//最后6回合
};

static const int nearestBigRaceTurn[TOTAL_TURN] = //距离最近的大比赛还有多少回合
{
  23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
  23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
  23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
  5,4,3,2,1,0
};
static const int nearestBigRace[TOTAL_TURN] = //最近的大比赛是哪个
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,
};
//大比赛捏红系数
const float reserveRedFactorGUR = 1;
const float reserveRedFactorWBC = 3;
const float reserveRedFactorSWBC = 6;
const float reserveRedFactorGM = 6; 
static const int reserveRedFactor[TOTAL_TURN] =
{
  reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,reserveRedFactorGUR,
  reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,reserveRedFactorWBC,
  reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,reserveRedFactorSWBC,
  reserveRedFactorGM,reserveRedFactorGM,reserveRedFactorGM,reserveRedFactorGM,reserveRedFactorGM,reserveRedFactorGM
};



const double jibanValue = 2;
const double venusValue_first = 40;
const double venusValue_beforeOutgoing = 10;
const double venusValue_afterOutgoing = 20;
const double venusValue_activated = 6;
const double spiritValue = 25;
const double vitalFactor = 0.7;
const double smallFailValue = -30;
const double bigFailValue = -90;
const double wizFailValue = 5;
const double statusWeights[5] = { 1.0,1.0,1.0,1.0,1.0 };
const double ptWeight = 0.5;
const double restValueFactor = 1.2;//休息估值权重
const float remainStatusFactorEachTurn = 16;//控属性时给每回合预留多少
const double outgoingBonusIfNotFullMotivation = 30;//掉心情时提高女神外出分数


ModelOutputPolicyV1 Evaluator::handWrittenPolicy(const Game& game0)
{
  ModelOutputPolicyV1 policy=
  {
    {0,0,0,0,0,0,0,0},
    0,
    {0,0,0,0,0,0},
    {0,0,0}
  };
  if(game0.isEnd())
    return policy;

  int chosenTrain = 0;
  bool useVenus = false;
  int chosenOutgoing = 0;
  int chosenSpiritColor = 0;



  Game game = game0;//创建一个副本，因为有可能要开女神


  float reserveRedBigRaceBonus = 0;//快到大比赛了，把红攒着
  if (game.venusAvailableWisdom == 1)
  {

  }

  if (game.isRacing)//比赛回合
  {
    if (game.venusAvailableWisdom != 1)
      useVenus = false;//不是红肯定不开
    else
    {
      int nearestBigRaceTurnNum = nearestBigRaceTurn[game.turn];
      if (nearestBigRaceTurnNum == 0)
        useVenus = true;
      else
      {
        int nearestBigRaceId=nearestBigRace[game.turn];
        if((nearestBigRaceId==0&&nearestBigRaceTurnNum<2)
          || (nearestBigRaceId == 1 && nearestBigRaceTurnNum < 3)
          || (nearestBigRaceId == 2 && nearestBigRaceTurnNum < 5)
          || (nearestBigRaceId == 3 && nearestBigRaceTurnNum < 5)
          )
          useVenus = false;
        else
          useVenus = true;
      }
    }
  }
  else//常规训练回合
  {
    if (game.venusAvailableWisdom != 0)
    {
      if (game.venusAvailableWisdom != 1)
      {
        useVenus = true;//不是红，除非休息肯定要开，如果休息，后面的代码会把useVenus设成false
      }
      else
      {
        //快到大比赛了捏红
        int nearestBigRaceTurnNum = nearestBigRaceTurn[game.turn];
        if (nearestBigRaceTurnNum == 0)
          useVenus = true;
        else if (game.vital<50 || game.motivation<5)
          useVenus = true;
        else
        {
          int nearestBigRaceId = nearestBigRace[game.turn];
          if ((nearestBigRaceId == 0 && nearestBigRaceTurnNum < 2)
            || (nearestBigRaceId == 1 && nearestBigRaceTurnNum < 3)
            || (nearestBigRaceId == 2 && nearestBigRaceTurnNum < 5)
            || (nearestBigRaceId == 3 && nearestBigRaceTurnNum < 5)
            )
            useVenus = false;
          else
            useVenus = true;
        }

      }
    }
    
    if(useVenus)
      game.activateVenusWisdom();


    int vitalAfterRest = std::min(game.maxVital, 50 + game.vital);
    double restValue = restValueFactor * (vitalEvaluation(vitalAfterRest, game.maxVital) - vitalEvaluation(game.vital, game.maxVital));
    //std::cout << restValue << " "<<game.vital<<std::endl;
    if (game.venusSpiritsBottom[7] == 0)restValue += spiritValue;

    double bestValue = -10000;
    int bestTrain = -1;
    for (int item = 0; item < 5; item++)
    {

      double expectSpiritNum = int(game.spiritDistribution[item] / 32) + 1;
      double value = 0;
      assert(game.cardData[0]->cardType == 5 && "神团卡不在第一个位置");

      int cardHintNum = 0;//所有hint随机取一个，所以打分的时候取平均
      for (int head = 0; head < 6; head++)
      {
        if (game.cardDistribution[item][head] && game.cardHint[head])
          cardHintNum++;
      }
      if (game.venusIsWisdomActive && game.venusAvailableWisdom == 2)//开蓝所有hint生效
        cardHintNum = 1;


      for (int head = 0; head < 6; head++)
      {
        if (!game.cardDistribution[item][head])
          continue;
        if (head == 0)
        {
          if (!game.venusCardFirstClick)
            value += venusValue_first;
          else if (!game.venusCardUnlockOutgoing)
          {
            expectSpiritNum += 0.5;
            value += venusValue_beforeOutgoing;
          }
          else if (!game.venusCardIsQingRe)
          {
            expectSpiritNum += 0.5;
            value += venusValue_afterOutgoing;
          }
          else
          {
            expectSpiritNum += 1;
            value += venusValue_activated;
          }


          //选等级最低的那种颜色的碎片
          if (game.venusLevelRed <= game.venusLevelBlue && game.venusLevelRed <= game.venusLevelYellow)
            chosenSpiritColor = 0;
          else if (game.venusLevelBlue <= game.venusLevelYellow && game.venusLevelBlue < game.venusLevelRed)
            chosenSpiritColor = 1;
          else
            chosenSpiritColor = 2;
          //掉心情了必须选红
          if (game.motivation < 5)
            chosenSpiritColor = 0;

        }
        else
        {
          if (game.cardJiBan[head] < 80)
          {
            float jibanAdd = 7;
            if (game.isAiJiao)jibanAdd += 2;
            if (game.cardHint[head])
            {
              jibanAdd += 5 / cardHintNum;
              if (game.isAiJiao)jibanAdd += 2 / cardHintNum;
            }
            jibanAdd = std::min(float(80 - game.cardJiBan[head]), jibanAdd);

            value += jibanAdd * jibanValue;
          }
        }
        if (game.cardHint[head])
        {
          for (int i = 0; i < 5; i++)
            value += game.cardData[head]->hintBonus[i] * statusWeights[i] / cardHintNum;
          value += game.cardData[head]->hintBonus[5] * ptWeight / cardHintNum;
        }

      }
      //开蓝
      if (game.venusAvailableWisdom == 2 && game.venusIsWisdomActive)
      {
        auto blueBonus = game.calculateBlueVenusBonus(item);
        for (int i = 0; i < 5; i++)
          value += blueBonus[i] * statusWeights[i];
        value += blueBonus[5] * ptWeight;
      }


      if (game.venusSpiritsBottom[7] > 0)
        expectSpiritNum = 0;
      else if (game.venusSpiritsBottom[6] > 0)
        expectSpiritNum = std::min(1.0, expectSpiritNum);
      else if (game.venusSpiritsBottom[5] > 0)
        expectSpiritNum = std::min(2.0, expectSpiritNum);
      value += spiritValue * expectSpiritNum;

      for (int i = 0; i < 5; i++)
      {
        //不仅要考虑属性加多少，还要考虑是否溢出
        float gain = game.trainValue[item][i];
          float remain = game.fiveStatusLimit[i] - game.fiveStatus[i] - fixedBonusAfterTurn[game.turn];
          if (gain > remain)gain = remain;
          float turnReserve = remainStatusFactorEachTurn * (TOTAL_TURN - game.turn);//距离上限不到turnReserve时降低权重

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
      double ptWeightThisGame = ptWeight / 1.8 * (game.isQieZhe ? GameConstants::ScorePtRateQieZhe : GameConstants::ScorePtRate);
      value += game.trainValue[item][5] * ptWeightThisGame;
      //value += vitalValue * game.trainValue[item][6];

      int vitalAfterTrain = std::min(game.maxVital, game.trainValue[item][6] + game.vital);
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


      if (value > bestValue)
      {
        bestValue = value;
        bestTrain = item;
      }
    }
    chosenTrain = bestTrain;

    if (game.motivation < 5)//掉心情了，该交外出了
    {
      if (game.venusCardUnlockOutgoing && (!game.venusCardOutgoingUsed[4]))//有女神外出
        restValue += outgoingBonusIfNotFullMotivation;
    }

    if (restValue>bestValue)//有女神外出就外出，没有就休息
    {
      useVenus = false;//无论是什么女神，都不值得用
      if (game.venusCardUnlockOutgoing && !game.venusCardOutgoingUsed[4] && !game.isXiaHeSu())
      {
        chosenTrain = 6;
        chosenOutgoing =
          (!game.venusCardOutgoingUsed[2]) ? 2 :
          (!game.venusCardOutgoingUsed[0]) ? 0 :
          (!game.venusCardOutgoingUsed[1]) ? 1 :
          (!game.venusCardOutgoingUsed[3]) ? 3 :
          4;
      }
      else
        chosenTrain = 5;
    }
    
  }
  policy.trainingPolicy[chosenTrain] = 1.0;
  if (useVenus)policy.useVenusPolicy = 1.0;
  policy.threeChoicesEventPolicy[chosenSpiritColor] = 1.0;
  policy.outgoingPolicy[chosenOutgoing] = 1.0;
  return policy;
}
