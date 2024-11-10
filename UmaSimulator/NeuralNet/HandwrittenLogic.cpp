#include <cassert>
#include <iostream>
#include "Evaluator.h"
#include "../Search/Search.h"


const double statusWeights[5] = { 6,6,6,6,6 };
const double jibanValue = 12;
const double vitalFactorStart = 3.5;
const double vitalFactorEnd = 7;
const double vitalScaleTraining = 1;

const double reserveStatusFactor = 40;//控属性时给每回合预留多少，从0逐渐增加到这个数字

const double smallFailValue = -150;
const double bigFailValue = -500;
const double outgoingBonusIfNotFullMotivation = 200;//掉心情时提高外出分数
const double raceBonus = 150;//比赛收益，不考虑体力

const double mechaLvBonusStart = 10;
const double mechaLvBonusEnd = 5;
const double mechaLvReserve = 40;
const double overdriveActivateFactor = 1.5;



//const double xiangtanExhaustLossMax = 800;//相谈耗尽且没达标的估值扣分

//一个分段函数，用来控属性
inline double statusSoftFunction(double x, double reserve, double reserveInvX2)//reserve是控属性保留空间（降低权重），reserveInvX2是1/(2*reserve)
{
  if (x >= 0)return 0;
  if (x > -reserve)return -x * x * reserveInvX2;
  return x + 0.5 * reserve;
}

static double mechaLvEvaluation(const Game& g, int train) //mechaLv提升量，靠近上限时衰减
{
  if (train == -1 || train == TRA_outgoing || train == TRA_rest)return 0;

  int turnsBeforeUGE =
    g.turn <= 11 ? 12 :
    g.turn <= 23 ? 23 - g.turn :
    g.turn <= 35 ? 35 - g.turn :
    g.turn <= 47 ? 47 - g.turn :
    g.turn <= 59 ? 59 - g.turn :
    g.turn <= 71 ? 71 - g.turn :
    0;
  double reserve = 1 + mechaLvReserve / 12.0 * turnsBeforeUGE;
  double reserveInvX2 = 1 / (2 * reserve);

  double value = 0;
  for (int item = 0; item < 5; item++)
  {
    int gain = train < 5 ? g.mecha_lvGain[train][item] : 7;
    double remain = g.mecha_rivalLvLimit - g.mecha_rivalLv[item];
    double s0 = statusSoftFunction(-remain, reserve, reserveInvX2);
    double s1 = statusSoftFunction(gain - remain, reserve, reserveInvX2);
    value += (s1 - s0);
  }
  return value;

}

static void statusGainEvaluation(const Game& g, double* result) { //result依次是五种训练的估值
  int remainTurn = TOTAL_TURN - g.turn - 1;//这次训练后还有几个训练回合
  //ura期间的一个回合视为两个回合，因此不需要额外处理
  //if (remainTurn == 2)remainTurn = 1;//ura第二回合
  //else if (remainTurn >= 4)remainTurn -= 2;//ura第一回合

  double reserve = reserveStatusFactor * remainTurn * (1 - double(remainTurn) / (TOTAL_TURN * 2));
  double reserveInvX2 = 1 / (2 * reserve);

  double finalBonus0 = 45;
  finalBonus0 += 30;//ura3和最终事件
  if (remainTurn >= 1)finalBonus0 += 20;//ura2
  if (remainTurn >= 2)finalBonus0 += 20;//ura1

  double remain[5]; //每种属性还有多少空间

  for (int i = 0; i < 5; i++)
  {
    remain[i] = g.fiveStatusLimit[i] - g.fiveStatus[i] - finalBonus0;
  }

  if (g.friend_type == FriendType_lianghua)
  {
    remain[0] -= 25;
    remain[4] -= 25;
  }
  else if (g.friend_type == FriendType_yayoi)
  {
    remain[0] -= 25;
    remain[3] -= 25;
  }


  for (int tra = 0; tra < 5; tra++)
  {
    double res = 0;
    for (int sta = 0; sta < 5; sta++)
    {
      double s0 = statusSoftFunction(-remain[sta], reserve, reserveInvX2);
      double s1 = statusSoftFunction(g.trainValue[tra][sta] - remain[sta], reserve, reserveInvX2);
      res += statusWeights[sta] * (s1 - s0);
    }
    res += g.ptScoreRate * g.trainValue[tra][5];
    result[tra] = res;
  }
}





static double calculateMaxVitalEquvalant(const Game& g)
{
  int t = g.turn;
  if (g.turn >= 76)
    return 0;//最后一回合不需要体力
  if (g.turn > 71)
    return 10;//ura期间可以吃菜
  if (g.turn == 71)
    return 30;//ura前最后一回合，30体力进就行
  int nonRaceTurn = 0;//71回合前，每个训练回合按消耗15体力计算
  for (int i = 71; i > g.turn; i--)
  {
    if (!g.isRacingTurn[i])nonRaceTurn++;
    if (nonRaceTurn >= 6)break;
  }
  int maxVitalEq = 30 + 15 * nonRaceTurn;
  if(maxVitalEq>g.maxVital)
    maxVitalEq = g.maxVital;
  return maxVitalEq;

}

static double vitalEvaluation(int vital, int maxVital)
{
  if (vital <= 50)
    return 2.0 * vital;
  else if (vital <= 70)
    return 1.5 * (vital - 50) + vitalEvaluation(50, maxVital);
  else if (vital <= maxVital)
    return 1.0 * (vital - 70) + vitalEvaluation(70, maxVital);
  else
    return vitalEvaluation(maxVital, maxVital);
}


static Action getMechaUpgradeAdvise(int turn, int en)
{
  Action action;
  action.type = GameStage_beforeMechaUpgrade;
  action.train = -1;
  action.overdrive = false;
  if (turn == 1)
  {
    if (en >= 6)//101
    {
      action.mechaHead = 1;
      action.mechaChest = 0;
    }
    else if (en >= 3)
    {
      action.mechaHead = 0;
      action.mechaChest = 0;
    }
    else
      throw "getMechaUpgradeAdvise : too few mechaEn when upgrade";
  }
  else if (turn == 23)
  {
    if (en >= 12)//022
    {
      action.mechaHead = 2;
      action.mechaChest = 0;
    }
    else if (en >= 9)
    {
      action.mechaHead = 2;
      action.mechaChest = 0;
    }
    else
      throw "getMechaUpgradeAdvise : too few mechaEn when upgrade";
  }
  else if (turn == 35)
  {
    if (en >= 18)//222
    {
      action.mechaHead = 5;
      action.mechaChest = 1;
    }
    else if (en >= 15)
    {
      action.mechaHead = 3;
      action.mechaChest = 2;
    }
    else if (en >= 12)
    {
      action.mechaHead = 2;
      action.mechaChest = 2;
    }
    else
      throw "getMechaUpgradeAdvise : too few mechaEn when upgrade";
  }
  else if (turn == 47)
  {
    if (en >= 24)//530
    {
      action.mechaHead = 5;
      action.mechaChest = 3;
    }
    else if (en >= 21)
    {
      action.mechaHead = 5;
      action.mechaChest = 2;
    }
    else if (en >= 18)
    {
      action.mechaHead = 5;
      action.mechaChest = 1;
    }
    else if (en >= 15)
    {
      action.mechaHead = 5;
      action.mechaChest = 0;
    }
    else
      throw "getMechaUpgradeAdvise : too few mechaEn when upgrade";
  }
  else if (turn == 59)
  {
    if (en >= 30)//055
    {
      action.mechaHead = 0;
      action.mechaChest = 5;
    }
    else if (en >= 27)
    {
      action.mechaHead = 1;
      action.mechaChest = 3;
    }
    else if (en >= 24)
    {
      action.mechaHead = 1;
      action.mechaChest = 2;
    }
    else if (en >= 21)
    {
      action.mechaHead = 0;
      action.mechaChest = 2;
    }
    else if (en >= 18)
    {
      action.mechaHead = 2;
      action.mechaChest = 2;
    }
    else
      throw "getMechaUpgradeAdvise : too few mechaEn when upgrade";
  }
  else if (turn == 71)
  {
    if (en >= 36)//255
    {
      action.mechaHead = 2;
      action.mechaChest = 5;
    }
    else if (en >= 33)
    {
      action.mechaHead = 1;
      action.mechaChest = 5;
    }
    else if (en >= 30)
    {
      action.mechaHead = 0;
      action.mechaChest = 5;
    }
    else if (en >= 27)
    {
      action.mechaHead = 1;
      action.mechaChest = 3;
    }
    else if (en >= 24)
    {
      action.mechaHead = 1;
      action.mechaChest = 2;
    }
    else if (en >= 21)
    {
      action.mechaHead = 0;
      action.mechaChest = 2;
    }
    else if (en >= 18)
    {
      action.mechaHead = 2;
      action.mechaChest = 2;
    }
    else
      throw "getMechaUpgradeAdvise : too few mechaEn when upgrade";
  }
  else
    throw "this turn should not upgrade mecha";
  return action;

}



Action Evaluator::handWrittenStrategy(const Game& game)
{
  Action bestAction;
  if (game.isEnd())return bestAction;
  if (game.gameStage == GameStage_beforeMechaUpgrade)
    return getMechaUpgradeAdvise(game.turn, game.mecha_EN);

  bestAction.type = GameStage_beforeTrain;

  //比赛
  if (game.isRacing)
  {
    bestAction.train = TRA_race;
    return bestAction;
  }
  





  double bestValue = -1e4;


  double vitalFactor = vitalFactorStart + (game.turn / double(TOTAL_TURN)) * (vitalFactorEnd - vitalFactorStart);

  int maxVitalEquvalant = calculateMaxVitalEquvalant(game);
  double vitalEvalBeforeTrain = vitalEvaluation(std::min(maxVitalEquvalant, int(game.vital)), game.maxVital);

  double mechaLvFactor = mechaLvBonusStart + (game.turn / double(TOTAL_TURN)) * (mechaLvBonusEnd - mechaLvBonusStart);

  //外出/休息
  {
    bool isFriendOutgoingAvailable =
      game.friend_type != 0 &&
      game.friend_stage >= 2 &&
      game.friend_outgoingUsed < 5 &&
      (!game.isXiahesu());
    Action action;
    action.type = GameStage_beforeTrain;
    action.train = TRA_rest;
    if (isFriendOutgoingAvailable || game.isXiahesu())action.train = TRA_outgoing;//有友人外出优先外出，否则休息

    int vitalGain = isFriendOutgoingAvailable ? 50 : game.isXiahesu() ? 40 : 50;
    bool addMotivation = game.motivation < 5 && action.train == TRA_outgoing;

    int vitalAfterRest = std::min(maxVitalEquvalant, vitalGain + game.vital);
    double value = vitalFactor * (vitalEvaluation(vitalAfterRest, game.maxVital) - vitalEvalBeforeTrain);
    if (addMotivation)value += outgoingBonusIfNotFullMotivation;

    if (PrintHandwrittenLogicValueForDebug)
      std::cout << action.toString() << " " << value << std::endl;
    if (value > bestValue)
    {
      bestValue = value;
      bestAction = action;
    }
  }
  //比赛
  Action raceAction;
  raceAction.train = TRA_race;
  if(game.isLegal(raceAction))
  {
    double value = raceBonus;


    int vitalAfterRace = std::min(maxVitalEquvalant, -15 + game.vital);
    value += vitalFactor * (vitalEvaluation(vitalAfterRace, game.maxVital) - vitalEvalBeforeTrain);

    double mechaLvValue = mechaLvEvaluation(game, TRA_race);
    value += mechaLvValue;

    if (PrintHandwrittenLogicValueForDebug)
      std::cout << raceAction.toString() << " " << value << std::endl;
    if (value > bestValue)
    {
      bestValue = value;
      bestAction = raceAction;
    }
  }


  //训练

  double statusGainE[5];
  //先找到最好的训练，然后计算要不要吃菜
  {
    statusGainEvaluation(game, statusGainE);


    for (int tra = 0; tra < 5; tra++)
    {
      double value = statusGainE[tra];


      //处理hint和羁绊
      int cardHintNum = 0;//所有hint随机取一个，所以打分的时候取平均
      for (int j = 0; j < 5; j++)
      {
        int p = game.personDistribution[tra][j];
        if (p < 0)break;//没人
        if (p >= 6)continue;//不是卡
        if (game.persons[p].isHint)
          cardHintNum += 1;
      }
      double hintProb = 1.0 / cardHintNum;
      bool haveFriend = false;
      for (int j = 0; j < 5; j++)
      {
        int pi = game.personDistribution[tra][j];
        if (pi < 0)break;//没人
        if (pi >= 6)continue;//不是卡
        const Person& p = game.persons[pi];
        if (p.personType == PersonType_friendCard)//友人卡
        {
          haveFriend = true;
          if (game.friend_stage == FriendStage_notClicked)
            value += 150;
          else if(p.friendship < 60)
            value += 100;
          else value += 40;
        }
        else if (p.personType == PersonType_card)
        {
          if (p.friendship < 80)
          {
            double jibanAdd = 7;
            if (game.friend_type == FriendType_yayoi)
              jibanAdd += 1;
            if (haveFriend && game.friend_type == FriendType_yayoi)
              jibanAdd += 2;
            if (game.isAiJiao)jibanAdd += 2;
            if (p.isHint)
            {
              jibanAdd += 5 * hintProb;
              if (game.isAiJiao)jibanAdd += 2 * hintProb;
            }
            jibanAdd = std::min(double(80 - p.friendship), jibanAdd);

            value += jibanAdd * jibanValue;
          }

          if (p.isHint)
          {
            double hintBonus = p.cardParam.hintLevel==0?
              (1.6 * (statusWeights[0] + statusWeights[1] + statusWeights[2] + statusWeights[3] + statusWeights[4])) :
              game.hintPtRate*game.ptScoreRate*p.cardParam.hintLevel;
            value += hintBonus * hintProb;
          }
        }

      }

      double mechaLvValue = mechaLvEvaluation(game, tra);
      value += mechaLvValue;


      //理论上，估值需要乘上菜的训练加成然后减去菜的开销，但过于复杂，懒得考虑了
      int vitalAfterTrain = std::min(maxVitalEquvalant, game.trainVitalChange[tra] + game.vital);
      value += vitalScaleTraining * vitalFactor * (vitalEvaluation(vitalAfterTrain, game.maxVital) - vitalEvalBeforeTrain);

      //到目前为止都是训练成功的value
      //计算吃菜之后的体力，重新计算失败率
      double failRate = game.failRate[tra];
      bool overdriveAvailable = !game.mecha_overdrive_enabled && game.mecha_overdrive_energy >= 3;
      if (overdriveAvailable && game.mecha_upgradeTotal[2] >= 12 && failRate > 0)
      {
        int vitalGain = 15;
        failRate -= 1.7 * vitalGain;//粗糙估计
        if (failRate < 0)failRate = 0;
      }


      if (failRate > 0)
      {
        double bigFailProb = failRate;
        if (failRate < 20)bigFailProb = 0;
        double failValueAvg = 0.01 * bigFailProb * bigFailValue + (1 - 0.01 * bigFailProb) * smallFailValue;
        
        value = 0.01 * failRate * failValueAvg + (1 - 0.01 * failRate) * value;
      }



      if (value > bestValue)
      {
        bestValue = value;


        Action action;
        action.type = GameStage_beforeTrain;
        action.overdrive = false;
        action.train = tra;

        //是否overdrive
        if (overdriveAvailable)
        {
          if (game.mecha_overdrive_energy >= 6 ||
            (game.mecha_overdrive_energy >= 5 && game.turn < 71 && game.isRacingTurn[game.turn + 1]))
            action.overdrive = true;
          if(game.turn >= 69)
            action.overdrive = true;
          double overdriveBound =
            game.turn <= 11 ? 0 :
            game.turn <= 23 ? 50 :
            game.turn <= 35 ? 300 :
            game.turn <= 47 ? 350 :
            game.turn <= 59 ? 400 :
            game.turn <= 71 ? 400 :
            0;
          overdriveBound *= overdriveActivateFactor;
          if (statusGainE[tra] >= overdriveBound)
            action.overdrive = true;
        }

        if (action.overdrive)
        {
          if (game.mecha_upgradeTotal[1] >= 15)
            action.train = TRA_none;
        }

        bestAction = action;
      }
      //if (PrintHandwrittenLogicValueForDebug)
      //  std::cout << action.toString() << " " << value << std::endl;
    }


  }
  return bestAction;
}

