#include <cassert>
#include <iostream>
#include "Evaluator.h"
#include "../Search/Search.h"


const double statusWeights[5] = { 5.0,5.0,5.0,5.0,5.0 };
const double jibanValue = 12;
const double vitalFactorStart = 5;
const double vitalFactorEnd = 8;

const double reserveStatusFactor = 30;//������ʱ��ÿ�غ�Ԥ�����٣���0�����ӵ��������

const double smallFailValue = -150;
const double bigFailValue = -500;
const double outgoingBonusIfNotFullMotivation = 150;//������ʱ����������
const double nonTrainBonus = 100;//��غϲ�ѵ���»غ�+3��bonus���� ʣ��غ���/TOTAL_TURN
const double raceBonus = 200;//�������棬����������

const double colorBuffEvalRaw[3] = { 100,70,50 };//��ɫbuff��ֵ,���к�buff��turn/TOTAL_TURN
const double colorLevelEvalRaw[3] = { 6,6,4 };//�����Ǵմ��Ŀ�����ɫѵ���ȼ���ֵ
const double colorLevelTargetFactor = 100;//�մ��Ŀ���ϵ��

const double xiangtanEvalBasicStart = 350;//����һ����̸�Ĺ�ֵ˥��(�տ���)
const double xiangtanEvalBasicEnd = 350;//����һ����̸�Ĺ�ֵ˥��(����)
//const double xiangtanExhaustLossMax = 800;//��̸�ľ���û���Ĺ�ֵ�۷�

static void levelGainEvaluation(const Game& g, double* result) { //result[0:3]������������ɫ�Ĺ�ֵ
  bool hasTarget1 = g.turn < 72;//��������һ��uaf
  bool hasTarget2 = g.turn < 60;//������������uaf

  int uafRemainTurn = (g.turn / 12 + 1) * 12 - g.turn - 1;
  if (g.turn < 12)uafRemainTurn += 12;
  int uafRemainTurn2 = uafRemainTurn + 12;
  double targetFactor1 = 3  / (double(uafRemainTurn) + 3);
  double targetFactor2 = 3  / (double(uafRemainTurn2) + 3);


  int targetLv = 10 * (g.turn / 12);
  if (targetLv < 10)targetLv = 10;
  if (targetLv > 50)targetLv = 50;
  int targetLv2 = targetLv + 10;

  auto lvEval = [&](int lv,int color)
    {
      if (lv > 100)lv = 100;
      double v = colorLevelEvalRaw[color] * lv;
      if (hasTarget1 && lv < targetLv)
      {
        v -= colorLevelTargetFactor * targetFactor1 * (targetLv - lv + 8 * targetFactor1);
      }
      if (hasTarget2 && lv < targetLv2)
      {
        v -= colorLevelTargetFactor * targetFactor2 * (targetLv2 - lv);
      }
      return v;
    };

  double gain[3] = { 0,0,0 };
  for (int i = 0; i < 5; i++)
  {
    int color = g.uaf_trainingColor[i];
    int lv = g.uaf_trainingLevel[color][i];
    gain[color] += lvEval(lv + g.uaf_trainLevelGain[i], color) - lvEval(lv, color);
  }

  result[0] = gain[0];
  result[1] = gain[1];
  result[2] = gain[2];
}
static double levelGainEvaluationFriendOutgoing(const Game& g) { //��������յȼ����棨������colorLevelEvalRaw��

  bool hasTarget1 = g.turn < 72;//��������һ��uaf
  bool hasTarget2 = g.turn < 60;//������������uaf

  int uafRemainTurn = (g.turn / 12 + 1) * 12 - g.turn - 1;
  if (g.turn < 12)uafRemainTurn += 12;
  int uafRemainTurn2 = uafRemainTurn + 12;
  double targetFactor1 = 3 * colorLevelTargetFactor / (double(uafRemainTurn) + 3);
  double targetFactor2 = 3 * colorLevelTargetFactor / (double(uafRemainTurn2) + 3);


  int targetLv = 10 * (g.turn / 12);
  if (targetLv < 10)targetLv = 10;
  if (targetLv > 50)targetLv = 50;
  int targetLv2 = targetLv + 10;

  double friendTargetBonus = 0.0;
  for (int color = 0; color < 3; color++)
  {
    for (int i = 0; i < 5; i++)
    {
      int lv = g.uaf_trainingLevel[color][i];
      if (lv == targetLv - 1)
        friendTargetBonus += targetFactor1 * 3;
      if (lv < targetLv)
        friendTargetBonus += targetFactor1;
      if (lv < targetLv2)
        friendTargetBonus += targetFactor2;
    }
  }
  return friendTargetBonus;
}
//һ���ֶκ���������������
inline double statusSoftFunction(double x, double reserve, double reserveInvX2)//reserve�ǿ����Ա����ռ䣨����Ȩ�أ���reserveInvX2��1/(2*reserve)
{
  if (x >= 0)return 0;
  if (x > -reserve)return -x * x * reserveInvX2;
  return x + 0.5 * reserve;
}

static void statusGainEvaluation(const Game& g, double* result) { //result����������ѵ���Ĺ�ֵ
  int remainTurn = TOTAL_TURN - g.turn - 2;//���ѵ�����м���ѵ���غ�
  if (remainTurn == 2)remainTurn = 1;//ura�ڶ��غ�
  else if (remainTurn >= 4)remainTurn -= 2;//ura��һ�غ�

  double reserve = reserveStatusFactor * remainTurn * (1 - double(remainTurn) / (TOTAL_TURN * 2));
  double reserveInvX2 = 1 / (2 * reserve);

  double finalBonus0 = g.uaf_haveLose ? 30 : 55;
  finalBonus0 += 20;//ura3�������¼�
  if (remainTurn >= 1)finalBonus0 += 15;//ura2
  if (remainTurn >= 2)finalBonus0 += 15;//ura1

  double remain[5]; //ÿ�����Ի��ж��ٿռ�

  for (int i = 0; i < 5; i++)
  {
    remain[i] = g.fiveStatusLimit[i] - g.fiveStatus[i] - finalBonus0;
  }

  if (g.lianghua_type != 0)
  {
    remain[0] -= 36;
    remain[4] -= 36;
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

static void xiangtanCostFactor(const Game& g, double* result) { //result[0,1,2]������0,1,2����̸�Ĺ�ֵ
  result[0] = 0;
  if (g.uaf_xiangtanRemain == 0)
    return;

  int turnNumBeforeRefresh = 0;
  int nextRefresh = (g.turn / 12 + 1) * 12;
  if (nextRefresh > TOTAL_TURN)nextRefresh = TOTAL_TURN;
  for (int t = g.turn; t < nextRefresh; t++)
  {
    if (!g.isRacingTurn[t])turnNumBeforeRefresh++;
  }

  assert(turnNumBeforeRefresh > 0);
  if (turnNumBeforeRefresh == 1)
  {
    result[1] = 0;
    result[2] = 0;
    return;
  }
  //����Ĳ���ȫ��ƾ�о�Ϲȡ��
  else if (turnNumBeforeRefresh == 2)
  {
    if (g.uaf_xiangtanRemain == 1)
    {
      result[1] = 0.7;
      result[2] = 0;
      return;
    }
    else if (g.uaf_xiangtanRemain == 2)
    {
      result[1] = 0.3;
      result[2] = 1.0;
      return;
    }
    else if (g.uaf_xiangtanRemain == 3)
    {
      result[1] = 0.0;
      result[2] = 0.5;
      return;
    }
  }
  else if (turnNumBeforeRefresh == 3)
  {
    //ûд�ľ���Ĭ��
    if (g.uaf_xiangtanRemain == 2)
    {
      result[1] = 0.6;
      result[2] = 1.5;
      return;
    }
    else if (g.uaf_xiangtanRemain == 3)
    {
      result[1] = 0.3;
      result[2] = 1.0;
      return;
    }
  }

  result[1] = 1.0;
  result[2] = 2.0;
}

static double calculateMaxVitalEquvalant(const Game& g)
{
  int t = g.turn;
  if (t == TOTAL_TURN - 2)//uaf3
    return 0;
  if (t == TOTAL_TURN - 4)//uaf2
    return 50;
  if (t == TOTAL_TURN - 6)//uaf1
    return 65;


  int uafRemainTurn = (t / 12 + 1) * 12 - t - 1;
  if (t < 12)uafRemainTurn += 12;
  bool thisTurnUaf = uafRemainTurn == 0 || (uafRemainTurn == 1 && g.isRacingTurn[t + 1]);
  if (thisTurnUaf)
  {
    int newMaxVital = g.maxVital - 15;
    int uafCount = t / 12;
    if (uafCount == 1 && g.lianghua_type != 0)
      newMaxVital -= 15;
    if (uafCount == 5)
      newMaxVital = 65;
    return newMaxVital;
  }
  return g.maxVital;

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

static double nonTrainEvaluation(const Game& g)//�»غ�+3���Ĺ�ֵ
{
  if (g.isRacingTurn[g.turn + 1])return 0;
  return (1.0 - double(g.turn) / double(TOTAL_TURN)) * nonTrainBonus;
}

Action Evaluator::handWrittenStrategy(const Game& game)
{
  Action bestAction = { -1,0 };
  if (game.isEnd())return bestAction;
  double bestValue = -1e4;
  Game g = game;

  double vitalFactor = vitalFactorStart + (game.turn / double(TOTAL_TURN)) * (vitalFactorEnd - vitalFactorStart);

  int maxVitalEquvalant = calculateMaxVitalEquvalant(game);
  double vitalEvalBeforeTrain = vitalEvaluation(std::min(maxVitalEquvalant, int(game.vital)), game.maxVital);

  double xiangtanCost[3];
  xiangtanCostFactor(game, xiangtanCost);
  double xiangtanEvalBasic = xiangtanEvalBasicStart + (game.turn / double(TOTAL_TURN)) * (xiangtanEvalBasicEnd - xiangtanEvalBasicStart);
  for (int i = 0; i < 3; i++)
    xiangtanCost[i] *= xiangtanEvalBasic;

  double nonTrainE = nonTrainEvaluation(game);

  //���/��Ϣ
  {
    bool isFriendOutgoingAvailable =
      game.lianghua_type != 0 &&
      game.persons[game.lianghua_personId].friendOrGroupCardStage >= 2 &&
      game.lianghua_outgoingUsed < 5 &&
      (!game.isXiahesu());
    Action action = { TRA_rest,0 };
    if (isFriendOutgoingAvailable)action.train = TRA_outgoing;//������������������������Ϣ

    int vitalGain = isFriendOutgoingAvailable ? 50 : game.isXiahesu() ? 40 : 50;
    bool addMotivation = game.motivation < 5 && (isFriendOutgoingAvailable || game.isXiahesu());

    int vitalAfterRest = std::min(maxVitalEquvalant, vitalGain + game.vital);
    double value = vitalFactor * (vitalEvaluation(vitalAfterRest, game.maxVital) - vitalEvalBeforeTrain);
    if (addMotivation)value += outgoingBonusIfNotFullMotivation;

    value += nonTrainE;

    if (PrintHandwrittenLogicValueForDebug)
      std::cout << action.toString() << " " << value << std::endl;
    if (value > bestValue)
    {
      bestValue = value;
      bestAction = action;
    }
  }
  //����
  Action raceAction = { TRA_race,0 };
  if(game.isLegal(raceAction))
  {
    double value = raceBonus;


    int vitalAfterRace = std::min(maxVitalEquvalant, -15 + game.vital);
    value += vitalFactor * (vitalEvaluation(vitalAfterRace, game.maxVital) - vitalEvalBeforeTrain);

    value += nonTrainE;

    if (PrintHandwrittenLogicValueForDebug)
      std::cout << raceAction.toString() << " " << value << std::endl;
    if (value > bestValue)
    {
      bestValue = value;
      bestAction = raceAction;
    }
  }


  //ѵ��
  for (int xt = 0; xt < 10; xt++)
  {
    if (!game.isXiangtanLegal(xt))
      continue;
    g = game;
    if (xt != XT_none)
      g.xiangtanAndRecalculate(xt, true);

    double levelGainE[3], statusGainE[5];
    levelGainEvaluation(g, levelGainE);
    statusGainEvaluation(g, statusGainE);


    for (int tra = 0; tra < 5; tra++)
    {
      Action action = { tra,xt };
      Action actionNoXt = { tra,XT_none };
      if (!g.isLegal(actionNoXt))continue; //��ʱ������һ�η���̸��������䣬���Ǻ�����legal�����б���
      double value = statusGainE[tra]+levelGainE[g.uaf_trainingColor[tra]];


      //����hint���
      int cardHintNum = 0;//����hint���ȡһ�������Դ�ֵ�ʱ��ȡƽ��
      for (int j = 0; j < 5; j++)
      {
        int p = g.personDistribution[tra][j];
        if (p < 0)break;//û��
        if (g.persons[p].isHint)
          cardHintNum += 1;
      }
      double hintProb = 1.0 / cardHintNum;
      double yellowBuffFactor = g.uaf_buffNum[2] > 0 ? 2.0 : 1.0;

      for (int j = 0; j < 5; j++)
      {
        int pi = g.personDistribution[tra][j];
        if (pi < 0)break;//û��
        const Person& p = g.persons[pi];
        if (p.personType == 1)//���˿�
        {
          if (p.friendOrGroupCardStage == 0)
            value += 150;
          else if(p.friendship < 60)
            value += 100;
          else value += 40;
        }
        else if (p.personType == 2)
        {
          if (p.friendship < 80)
          {
            double jibanAdd = 7;
            if (g.isAiJiao)jibanAdd += 2;
            if (p.isHint)
            {
              jibanAdd += 5 * hintProb;
              if (g.isAiJiao)jibanAdd += 2 * hintProb;
            }
            jibanAdd = std::max(7.0, std::min(double(80 - p.friendship), jibanAdd));

            value += jibanAdd * jibanValue;
          }

          if (p.isHint)
          {
            for (int i = 0; i < 5; i++)
              value += p.cardParam.hintBonus[i] * statusWeights[i] * hintProb * yellowBuffFactor;
            value += p.cardParam.hintBonus[5] * g.ptScoreRate * hintProb * yellowBuffFactor;
          }
        }

      }




      int vitalAfterTrain = std::min(maxVitalEquvalant, g.trainVitalChange[tra] + g.vital);
      value += vitalFactor * (vitalEvaluation(vitalAfterTrain, g.maxVital) - vitalEvalBeforeTrain);

      //��ĿǰΪֹ����ѵ���ɹ���value
      double failRate = g.failRate[tra];
      if (failRate > 0)
      {
        double bigFailProb = failRate;
        if (failRate < 20)bigFailProb = 0;
        double failValueAvg = 0.01 * bigFailProb * bigFailValue + (1 - 0.01 * bigFailProb) * smallFailValue;
        
        value = 0.01 * failRate * failValueAvg + (1 - 0.01 * failRate) * value;
      }

      //��������̸����ɫbuff���۵���Ӧ����
      value -= xiangtanCost[Action::XiangtanNumCost[xt]];
      for(int i=0;i<3;i++)
        if (g.uaf_buffNum[i] > 0)
        {
          double buffLoss = colorBuffEvalRaw[i];
          if (i == 0)buffLoss *= (double(g.turn) / TOTAL_TURN);
          value -= buffLoss;
        }

      //��̸�����Ҵ���δ���ѵ���������۷�
      /*
      if (xt != XT_none && (g.uaf_xiangtanRemain == 0))
      {
        bool hasAnyNotReachedTarget = false;


        int targetLv = 10 * (g.turn / 12);
        if (targetLv < 10)targetLv = 10;
        if (targetLv > 50)targetLv = 50;
        for (int color = 0; color < 3; color++)
        {
          for (int i = 0; i < 5; i++)
          {
            int newLv = g.uaf_trainingLevel[color][i];
            if (g.uaf_trainingColor[tra] == color)
              newLv += g.uaf_trainLevelGain[i];
            if (newLv < targetLv)
              hasAnyNotReachedTarget = true;
          }
        }

        if (hasAnyNotReachedTarget)
        {

          int turnNumBeforeRefresh = 0;
          int nextRefresh = (g.turn / 12 + 1) * 12;
          if (nextRefresh > TOTAL_TURN)nextRefresh = TOTAL_TURN;
          for (int t = g.turn; t < nextRefresh; t++)
          {
            if (!g.isRacingTurn[t])turnNumBeforeRefresh++;
          }
          assert(turnNumBeforeRefresh > 0);

          double f = 6.0 / (6.0 + turnNumBeforeRefresh);
          value -= xiangtanExhaustLossMax * f;
        }
      }
      */
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

