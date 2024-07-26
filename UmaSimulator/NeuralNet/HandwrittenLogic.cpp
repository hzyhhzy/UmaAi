#include <cassert>
#include <iostream>
#include "Evaluator.h"
#include "../Search/Search.h"


const double statusWeights[5] = { 7.0,7.0,7.0,7.0,7.0 };
const double jibanValue = 4;
const double vitalFactorStart = 2;
const double vitalFactorEnd = 5;
const double vitalScaleTraining = 1;

const double reserveStatusFactor = 40;//控属性时给每回合预留多少，从0逐渐增加到这个数字

const double smallFailValue = -150;
const double bigFailValue = -500;
const double outgoingBonusIfNotFullMotivation = 150;//掉心情时提高外出分数
const double raceBonus = 200;//比赛收益，不考虑体力

//const double materialValue[5] = { 0.5,0.2,0.5,0.5,0.3 };//每个料理原料的估值
//const double materialValueScale = 1.0;//料理原料的估值乘以这个系数，方便一起改
const double greenBonusBasicYear1 = 100;//绿色料理的加成，羁绊没满时降低系数，第一年
const double greenBonusBasicYear2 = 100;//绿色料理的加成，第二年
const double greenBonusBasicYear3 = 100;//绿色料理的加成，第三年


//const double xiangtanExhaustLossMax = 800;//相谈耗尽且没达标的估值扣分

//一个分段函数，用来控属性
inline double statusSoftFunction(double x, double reserve, double reserveInvX2)//reserve是控属性保留空间（降低权重），reserveInvX2是1/(2*reserve)
{
  if (x >= 0)return 0;
  if (x > -reserve)return -x * x * reserveInvX2;
  return x + 0.5 * reserve;
}

static void statusGainEvaluation(const Game& g, double* result) { //result依次是五种训练的估值
  int remainTurn = TOTAL_TURN - g.turn - 1;//这次训练后还有几个训练回合
  //ura期间的一个回合视为两个回合，因此不需要额外处理
  //if (remainTurn == 2)remainTurn = 1;//ura第二回合
  //else if (remainTurn >= 4)remainTurn -= 2;//ura第一回合

  double reserve = reserveStatusFactor * remainTurn * (1 - double(remainTurn) / (TOTAL_TURN * 2));
  double reserveInvX2 = 1 / (2 * reserve);

  double finalBonus0 = 60;
  finalBonus0 += 30;//ura3和最终事件
  if (remainTurn >= 1)finalBonus0 += 20;//ura2
  if (remainTurn >= 2)finalBonus0 += 20;//ura1

  double remain[5]; //每种属性还有多少空间

  for (int i = 0; i < 5; i++)
  {
    remain[i] = g.fiveStatusLimit[i] - g.fiveStatus[i] - finalBonus0;
  }

  if (g.friend_type != 0)
  {
    remain[0] -= 25;
    remain[4] -= 25;
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


Action Evaluator::handWrittenStrategy(const Game& game)
{
  Action bestAction;
  bestAction.dishType = DISH_none;
  bestAction.train = TRA_none;

  if (game.isEnd())return bestAction;
  //比赛
  if (game.isRacing)
  {
    if (game.turn < 72)//常规比赛不吃菜
    {
      bestAction.train = TRA_race;
      return bestAction;
    }
    if (game.turn == TOTAL_TURN - 1)//ura最后一回合，能吃什么就吃什么
    {
      //从后往前挨个试
      for (int i = DISH_g1plate; i >= 1; i--)
      {
        bestAction.dishType = i;
        if (game.isLegal(bestAction))return bestAction;
      }
      //什么都吃不了
      bestAction.dishType = DISH_none;
      bestAction.train = TRA_race;
      return bestAction;
    }


    //ura1和ura2，算一下能不能吃g1plate
    int g1plateCost = game.cook_win_history[4] == 2 ? 80 : 100;

    bool haveG1Plate = true;
    for (int matType = 0; matType < 5; matType++)
    {
      int matGain = 1.5001 * GameConstants::Cook_HarvestBasic[game.cook_farm_level[matType]];
      if (matType == game.cook_main_race_material_type)
        matGain += 1.5001 * GameConstants::Cook_HarvestExtra[game.cook_farm_level[matType]];
      if (game.cook_material[matType] < g1plateCost || game.cook_material[matType] + matGain < 2 * g1plateCost)
      {
        haveG1Plate = false;
        break;
      }
    }
    if (haveG1Plate)
    {
      bestAction.dishType = DISH_g1plate;
      return bestAction;
    }
    else
    {
      //吃了下回合训练没法吃了
      bestAction.train = TRA_race;
      return bestAction;
    }
  }

  //ura期间如果能吃G1Plate就直接吃，否则不吃
  if (game.turn >= 72 && game.cook_dish==DISH_none)
  {
    if (game.isDishLegal(DISH_g1plate))
    {
      bestAction.dishType = DISH_g1plate;
      return bestAction;
    }
    //否则是吃过了或者吃不了，寻找最优训练
  }


  //常规训练回合
  
  //如果吃了2级3级菜，直接选择对应训练
  if (game.cook_dish != DISH_none)
  {
    int dishLevel = GameConstants::Cook_DishLevel[game.cook_dish];
    if (dishLevel == 2 || dishLevel == 3)
    {
      int tra = GameConstants::Cook_DishMainTraining[game.cook_dish];
      bestAction.dishType = DISH_none;
      bestAction.train = tra;
      return bestAction;
    }
  }




  double bestValue = -1e4;


  double vitalFactor = vitalFactorStart + (game.turn / double(TOTAL_TURN)) * (vitalFactorEnd - vitalFactorStart);

  int maxVitalEquvalant = calculateMaxVitalEquvalant(game);
  double vitalEvalBeforeTrain = vitalEvaluation(std::min(maxVitalEquvalant, int(game.vital)), game.maxVital);

  double greenBonus =
    game.turn < 24 ? greenBonusBasicYear1 :
    game.turn < 48 ? greenBonusBasicYear2 :
    greenBonusBasicYear3;
  //羁绊没满时降低绿色料理加成
  for (int i = 0; i < 6; i++)
    if (game.persons[i].friendship < 80)
      greenBonus *= 0.85;

  //外出/休息
  {
    bool isFriendOutgoingAvailable =
      game.friend_type != 0 &&
      game.friend_stage >= 2 &&
      game.friend_outgoingUsed < 5 &&
      (!game.isXiahesu());
    Action action;
    action.dishType = DISH_none;
    action.train = TRA_rest;
    if (isFriendOutgoingAvailable || game.isXiahesu())action.train = TRA_outgoing;//有友人外出优先外出，否则休息

    int vitalGain = isFriendOutgoingAvailable ? 50 : game.isXiahesu() ? 40 : 50;
    bool addMotivation = game.motivation < 5 && action.train == TRA_outgoing;

    int vitalAfterRest = std::min(maxVitalEquvalant, vitalGain + game.vital);
    double value = vitalFactor * (vitalEvaluation(vitalAfterRest, game.maxVital) - vitalEvalBeforeTrain);
    if (addMotivation)value += outgoingBonusIfNotFullMotivation;

    bool isGreen = game.cook_train_green[action.train];
    if (isFriendOutgoingAvailable && action.train == TRA_outgoing)
      isGreen = true;
    if (isGreen)value += greenBonus;

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
  raceAction.dishType = DISH_none;
  raceAction.train = TRA_race;
  if(game.isLegal(raceAction))
  {
    double value = raceBonus;


    int vitalAfterRace = std::min(maxVitalEquvalant, -15 + game.vital);
    value += vitalFactor * (vitalEvaluation(vitalAfterRace, game.maxVital) - vitalEvalBeforeTrain);

    if (game.cook_train_green[TRA_race])
      value += greenBonus;

    if (PrintHandwrittenLogicValueForDebug)
      std::cout << raceAction.toString() << " " << value << std::endl;
    if (value > bestValue)
    {
      bestValue = value;
      bestAction = raceAction;
    }
  }


  //训练

  //先找到最好的训练，然后计算要不要吃菜
  {
    double statusGainE[5];
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
        if (p.personType == PersonType_scenarioCard)//友人卡
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
            if (game.friend_type == 1)
              jibanAdd += 1;
            if (haveFriend && game.friend_type == 1)
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

      int bestDish = 0;
      double bestDishValue = 0;
      //考虑吃各种菜
      //第一年：自己相应训练只有一种菜
      //第二年：只考虑lv2菜
      //第三年：lv2和lv3都考虑
      //ura：大概率已经吃过g1plate了，这里不考虑吃菜
      if (game.cook_dish == DISH_none)
      {
        if (game.turn < 24)//第一年
        {
          if (game.cook_dish_pt < 2500)//吃到2500pt
          {
            if (game.isDishLegal(DISH_curry))
            {
              if (tra == 0 || tra == 1 || tra == 3)
                bestDish = DISH_curry;
            }
            else if (game.isDishLegal(DISH_sandwich))
            {
              if (tra == 0 || tra == 2 || tra == 4)
                bestDish = DISH_sandwich;
            }
          }
        }
        else if (game.turn < 48)//第二年
        {
          //能吃lv2就赶快吃，否则不吃
          int dish = DISH_speed1 + tra;
          assert(GameConstants::Cook_DishMainTraining[dish] == tra);
          if (game.isDishLegal(dish))
          {
            bestDish = dish;
          }
        }
        else if (game.turn < 72)//第三年
        {
          //前半年能吃lv3就赶快吃，否则不吃
          int dish = DISH_speed2 + tra;
          assert(GameConstants::Cook_DishMainTraining[dish] == tra);
          if (game.isDishLegal(dish))
          {
            int matReserve =
              game.turn < 60 ? (tra == TRA_speed ? 40 : 0) :
              game.turn < 68 ? (tra == TRA_speed ? 80 : 80) :
              (tra == TRA_speed ? 120 : 40);
            int matRemain = game.cook_material[tra] - 250;
            if (matRemain >= matReserve)
            {
              bestDish = dish;
            }
          }
          
        } 
      }

      //理论上，估值需要乘上菜的训练加成然后减去菜的开销，但过于复杂，懒得考虑了
      int vitalAfterTrain = std::min(maxVitalEquvalant, game.trainVitalChange[tra] + game.vital);
      value += vitalScaleTraining * vitalFactor * (vitalEvaluation(vitalAfterTrain, game.maxVital) - vitalEvalBeforeTrain);

      //到目前为止都是训练成功的value
      //计算吃菜之后的体力，重新计算失败率
      double failRate = game.failRate[tra];
      if (bestDish != 0 && failRate > 0)
      {
        int dishLevel = GameConstants::Cook_DishLevel[bestDish];
        int dishMainTraining = GameConstants::Cook_DishMainTraining[bestDish];
        int vitalGainDish = dishLevel==1?0:
          dishLevel == 2 ? (game.cook_farm_level[dishMainTraining] >= 3 ? 5 : 0):
          dishLevel == 3 ? (game.cook_farm_level[dishMainTraining] >= 3 ? 20 : 15): //假设一半大成功加体力
          25;
        failRate -= 1.7 * vitalGainDish;//粗糙估计
        if (failRate < 0)failRate = 0;
      }


      if (failRate > 0)
      {
        double bigFailProb = failRate;
        if (failRate < 20)bigFailProb = 0;
        double failValueAvg = 0.01 * bigFailProb * bigFailValue + (1 - 0.01 * bigFailProb) * smallFailValue;
        
        value = 0.01 * failRate * failValueAvg + (1 - 0.01 * failRate) * value;
      }

      Action action;
      if (bestDish != DISH_none)//先吃菜，下一次再训练
      {
        action.dishType = bestDish;
        action.train = TRA_none;
      }
      else
      {
        action.dishType = DISH_none;
        action.train = tra;
      }

      if (value > bestValue)
      {
        bestValue = value;
        bestAction = action;
      }
      if (PrintHandwrittenLogicValueForDebug)
        std::cout << action.toString() << " " << value << std::endl;
    }


  }
  return bestAction;
}

