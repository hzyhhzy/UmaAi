#include <iostream>
#include <cassert>
#include "Game.h"
using namespace std;
static bool randBool(mt19937_64& rand, double p)
{
  return rand() % 65536 < p * 65536;
}

//������Game���˳��һ��
void Game::newGame(mt19937_64& rand, bool enablePlayerPrint, int newUmaId, int umaStars, int newCards[6], int newZhongMaBlueCount[5], int newZhongMaExtraBonus[6])
{
  playerPrint = enablePlayerPrint;

  umaId = newUmaId;
  isLinkUma = GameConstants::isLinkChara(umaId);
  for (int i = 0; i < TOTAL_TURN; i++)
    isRacingTurn[i] = GameDatabase::AllUmas[umaId].races[i] == TURN_RACE;
  assert(isRacingTurn[11] == true);//������
  isRacingTurn[TOTAL_TURN - 5] = true;//ura1
  isRacingTurn[TOTAL_TURN - 3] = true;//ura2
  isRacingTurn[TOTAL_TURN - 1] = true;//ura3

  for (int i = 0; i < 5; i++)
    fiveStatusBonus[i] = GameDatabase::AllUmas[umaId].fiveStatusBonus[i];
  eventStrength = GameConstants::EventStrengthDefault;
  turn = 0;
  vital = 100;
  maxVital = 100;
  motivation = 3;

  for (int i = 0; i < 5; i++)
    fiveStatus[i] = GameDatabase::AllUmas[umaId].fiveStatusInitial[i] - 10 * (5 - umaStars); //�������ʼֵ
  for (int i = 0; i < 5; i++)
    fiveStatusLimit[i] = GameConstants::BasicFiveStatusLimit[i]; //ԭʼ��������

  skillPt = 120;
  skillScore = umaStars >= 3 ? 170 * (umaStars - 2) : 120 * (umaStars);//���м���
  ptScoreRate = GameConstants::ScorePtRate;

  failureRateBias = 0;
  isAiJiao = false;
  isPositiveThinking = false;

  for (int i = 0; i < 5; i++)
    zhongMaBlueCount[i] = newZhongMaBlueCount[i];
  for (int i = 0; i < 6; i++)
    zhongMaExtraBonus[i] = newZhongMaExtraBonus[i];

  for (int i = 0; i < 5; i++)
    fiveStatusLimit[i] += int(zhongMaBlueCount[i] * 5.34 * 2); //��������--�������ֵ
  for (int i = 0; i < 5; i++)
    addStatus(i, zhongMaBlueCount[i] * 7); //����

  isRacing = false;

  for (int i = 0; i < MAX_HEAD_NUM; i++)
  {
    persons[i] = Person();
  }

  saihou = 0;
  lianghua_type = 0;
  lianghua_personId = -1;
  lianghua_outgoingUsed = 0;
  lianghua_vitalBonus = 1.0;
  lianghua_statusBonus = 1.0;
  lianghua_guyouEffective = false;
  for (int i = 0; i < 6; i++)
  {
    int cardId = newCards[i];
    persons[i].setCard(cardId);
    saihou += persons[i].cardParam.saiHou;

    if (persons[i].personType == 1)
    {
      lianghua_personId = i;
      bool isSSR = cardId > 300000;
      if (isSSR)
        lianghua_type = 1;
      else
        lianghua_type = 2;
      int friendLevel = cardId % 10;
      if (lianghua_type ==1)
      {
        lianghua_vitalBonus = GameConstants::FriendVitalBonusSSR[friendLevel];
        lianghua_statusBonus = GameConstants::FriendStatusBonusSSR[friendLevel];
      }
      else
      {
        lianghua_vitalBonus = GameConstants::FriendVitalBonusR[friendLevel];
        lianghua_statusBonus = GameConstants::FriendStatusBonusR[friendLevel];
      }
      lianghua_vitalBonus += 1e-10;
      lianghua_statusBonus += 1e-10;//�Ӹ�С����������Ϊ�����������
    }
  }

  for (int i = 0; i < 6; i++)//֧Ԯ����ʼ�ӳ�
  {
    for (int j = 0; j < 5; j++)
      addStatus(j, persons[i].cardParam.initialBonus[j]);
    skillPt += persons[i].cardParam.initialBonus[5];
  }

  persons[6].setNonCard(PersonType_lishizhang);
  persons[7].setNonCard(PersonType_jizhe);
  if (lianghua_type == 0)
    persons[8].setNonCard(PersonType_lianghuaNonCard);
  else
    persons[8].personType = 0;



  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 5; j++)
      uaf_trainingLevel[i][j] = 1;

  for (int k = 0; k < 5; k++)
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 5; j++)
        uaf_winHistory[k][i][j] = false;

  uaf_lastTurnNotTrain = false;
  uaf_xiangtanRemain = 3;

  for (int i = 0; i < 3; i++)
    uaf_buffActivated[i] = 0;

  for (int i = 0; i < 3; i++)
    uaf_buffNum[i] = 0;


  randomDistributeCards(rand); 
}

void Game::randomDistributeCards(std::mt19937_64& rand)
{
  cardEffectCalculated = false;
  checkLianghuaGuyou();

  //�����غϵ���ͷ���䣬����Ҫ���㣬��Ϊ������������
  if (isRacing)
    return;//�������÷��俨��
  
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 5; j++)
      personDistribution[i][j] = -1;

  int headN[5] = { 0,0,0,0,0 };
  vector<int8_t> buckets[5];
  for (int i = 0; i < 5; i++)
    buckets[i].clear();
  //�ȷ�����/���³�/����
  for (int i = 0; i < MAX_HEAD_NUM; i++)
  {
    Person& p = persons[i];
    if (p.personType == PersonType_jizhe && turn <= 12)//���ߵ�13�غ���
      continue;
    if (p.personType == PersonType_lianghuaCard ||
      p.personType == PersonType_otherFriend ||
      p.personType == PersonType_groupCard ||
      p.personType == PersonType_lishizhang ||
      p.personType == PersonType_jizhe ||
      p.personType == PersonType_lianghuaNonCard)
    {
      int atTrain = p.distribution(rand);
      if (atTrain < 5)
      {
        buckets[atTrain].push_back(i);
      }
    }
  }
  for (int i = 0; i < 5; i++)
  {
    if (buckets[i].size() == 1)
    {
      personDistribution[i][0] = buckets[i][0];
      headN[i] += 1;
    }
    else if (buckets[i].size() > 1)//���ѡһ����ͷ
    {
      personDistribution[i][0] = buckets[i][rand() % buckets[i].size()];
      headN[i] += 1;
    }
    buckets[i].clear();
  }

  //Ȼ������֧ͨԮ��/npc(�����)
  for (int i = 0; i < MAX_HEAD_NUM; i++)
  {
    Person& p = persons[i];
    if (p.personType == PersonType_card ||
      p.personType == PersonType_npc)
    {
      int atTrain = p.distribution(rand);
      if (atTrain < 5)
      {
        buckets[atTrain].push_back(i);
      }
    }
  }
  for (int i = 0; i < 5; i++)
  {
    int maxHead = 5 - headN[i];
    if (buckets[i].size() <= maxHead)
    {
      for (int j = 0; j < buckets[i].size(); j++)
      {
        personDistribution[i][headN[i]] = buckets[i][j];
        headN[i] += 1;
      }
    }
    else//����������5�ˣ����ѡmaxHead��
    {
      for (int j = 0; j < maxHead; j++)
      {
        int idx = rand() % (buckets[i].size() - j);

        int count = 0;
        for (int k = 0; k < buckets[i].size(); k++)
        {
          if (buckets[i][k] != -1)
          {
            if (idx == count)
            {
              personDistribution[i][headN[i]] = buckets[i][k];
              buckets[i][k] = -1;
              headN[i] += 1;
              break;
            }
            else
              count++;
          }
        }
      }
      assert(headN[i] == 5);
    }
  }

  //�Ƿ���hint
  for (int i = 0; i < 6; i++)
    persons[i].isHint = false;

  for (int t = 0; t < 5; t++)
  {
    for (int h = 0; h < 5; h++)
    {
      int pid = personDistribution[t][h];
      if (pid < 0)break;

      if (persons[pid].personType == PersonType_card)
      {
        if (uaf_buffNum[2] > 0)//��buff
          persons[pid].isHint = true;
        else
        {
          double hintProb = 0.06 * (1 + 0.01 * persons[pid].cardParam.hintProbIncrease);
          persons[pid].isHint = randBool(rand, hintProb);
        }
      }
    }
  }

  //�����ɫ
  for (int i = 0; i < 5; i++)
    uaf_trainingColor[i] = rand() % 3;

  calculateTrainingValue();
}

//СдΪһ��������дΪ������6�����������pt��
//k = ��ͷ * ѵ�� * �ɾ� * ����    //֧Ԯ������
//B = ѵ������ֵ + ֧Ԯ���ӳ�  //����ֵ
//C = link����ֵ�ӳɣ�lv12345�ǵ�ǰ���Լ�11223��pt��Ϊ��1
//G = ����ɳ���

//L = k * B * G //�²㣬�Ȳ�ȡ��
//P1 = L + k * C
//P2 = P1 * (1 + ��buff����)����buff����ֻ���ڵ�ǰѵ������������
//P3 = P2 * (1 + link���ӳ� + �����ʤ���ӳ�)
//����T = P3 + ��buff
//�ϲ�U = T - L

void Game::calculateTrainingValue()
{


  //�籾ѵ���ӳ�
  uaf_trainingBonus = 0;
  int competitionFinishedNum = uaf_competitionFinishedNum();
  uaf_haveLose = false;
  uaf_haveLoseColor[0] = false;
  uaf_haveLoseColor[1] = false;
  uaf_haveLoseColor[2] = false;
  for (int color = 0; color < 3; color++)
  {
    int levelTotal = 0;
    int winCount = 0;
    for (int i = 0; i < 5; i++)
      levelTotal += uaf_trainingLevel[color][i];

    for (int i = 0; i < competitionFinishedNum; i++)
      for (int j = 0; j < 5; j++)
      {
        if(uaf_winHistory[i][color][j])
          winCount += 1;
        else
        {
          uaf_haveLose = true;
          uaf_haveLoseColor[color] = true;
        }
      }

    uaf_trainLevelColorTotal[color] = levelTotal;
    uaf_colorWinCount[color] = winCount;
    uaf_trainingBonus += GameConstants::UAF_WinNumTrainingBonus[winCount];
  }

  //ѵ���ȼ��������뿨���Ĺ�ϵ
  static const int cardNumLevelBonus[6] = { 0,1,2,2,3,3 };

  //�����ʵĿ���persons�ı�ż�����
  bool isCardShining_record[6] = { false,false,false,false,false,false };
  int cardNum_record[5];

  //����ȼ�������
  for (int tra = 0; tra < 5; tra++)
  {
    int cardNum = 0;
    int shiningNum = 0;
    int linkNum = 0;
    for (int h = 0; h < 5; h++)
    {
      int pIdx = personDistribution[tra][h];
      if (pIdx < 0)break;
      const Person& p = persons[pIdx];
      if (pIdx >= 6)continue;//����֧Ԯ��
      assert(p.personType == PersonType_card || p.personType == PersonType_lianghuaCard || p.personType == PersonType_otherFriend || p.personType == PersonType_groupCard);
      
      cardNum += 1;
      if (isCardShining(pIdx, tra))
      {
        shiningNum += 1;
        isCardShining_record[pIdx] = true;
      }
      if (p.cardParam.isLink)
      {
        linkNum += 1;
      }

    }
    cardNum_record[tra] = cardNum;
    trainShiningNum[tra] = shiningNum;

    int levelGain = 3 + cardNumLevelBonus[cardNum];
    if (shiningNum > 0)
      levelGain *= 2;
    levelGain += linkNum;
    if (uaf_lastTurnNotTrain)
      levelGain += 3;
    uaf_trainLevelGain[tra] = levelGain;
  }


  //linkͳ����ӳ�
  int trainingColorNum[3];//������ɫ��ѵ������
  int levelGainColorTotal[3];//������ɫ��ѵ���ȼ���������
  int linkBasicValueBonus[3][6];//������ɫ�Ļ���ֵ������

  for (int color = 0; color < 3; color++)
  {
    trainingColorNum[color] = 0;
    levelGainColorTotal[color] = 0;
    for (int i = 0; i < 6; i++)
      linkBasicValueBonus[color][i] = 0;

    for (int t = 0; t < 5; t++)
    {
      if (uaf_trainingColor[t] != color)
        continue;
      trainingColorNum[color] += 1;
      levelGainColorTotal[color] += uaf_trainLevelGain[t];
      int splevel = uaf_trainingLevel[color][t];
      int gain = splevel < 30 ? 1 : splevel < 50 ? 2 : 3;
      linkBasicValueBonus[color][t] += gain;
    }
    linkBasicValueBonus[color][5] = trainingColorNum[color] - 1;
    if (trainingColorNum[color] <= 1)
    {
      for (int t = 0; t < 6; t++)
        linkBasicValueBonus[color][t] = 0;//һ��link��û�гԲ����ӳ�
    }
  }

  //�������ѵ���ĸ�����ֵ
  for (int t = 0; t < 5; t++)
  {
    int basicValueLower[6] = { 0,0,0,0,0,0 };//ѵ�����²����ֵ��=ԭ����ֵ+֧Ԯ���ӳ�
    double valueLowerDouble[6] = { 0,0,0,0,0,0 };//ѵ�����²�ֵ���Ȳ�ȡ��
    double valueDouble[6] = { 0,0,0,0,0,0 };//ѵ�����������Ȳ�ȡ��
    //int basicValue[6];//ѵ���Ļ���ֵ��= basicValueLower + linkBasicValueBonus
    int cardNum = 0;//���ſ������³����߲���
    int totalXunlian = 0;//ѵ��1+ѵ��2+...
    int totalGanjing = 0;//�ɾ�1+�ɾ�2+...
    double totalYouqingMultiplier = 1.0;//(1+����1)*(1+����2)*...
    int vitalCostBasic;//�������Ļ�������=ReLU(������������+link������������-�ǲ��������ļ���)
    double vitalCostMultiplier = 1.0;//(1-�������ļ�����1)*(1-�������ļ�����2)*...
    double failRateMultiplier = 1.0;//(1-ʧ�����½���1)*(1-ʧ�����½���2)*...

    int color = uaf_trainingColor[t]; 
    int colorNum = trainingColorNum[color];
    int splevel = uaf_trainingLevel[color][t];
    int tlevel = getTrainingLevel(t);
    for (int i = 0; i < 6; i++)
      basicValueLower[i] = GameConstants::TrainingBasicValue[color][t][tlevel][i];
    vitalCostBasic = -GameConstants::TrainingBasicValue[color][t][tlevel][6];
    vitalCostBasic += colorNum - 1;//linkԽ����������Խ��

    for (int a = 0; a < 5; a++)
    {
      int pid = personDistribution[t][a];
      if (pid < 0)break;//û��
      if (pid >= 6)continue;//���ǿ�
      cardNum += 1;
      const Person& p = persons[pid];
      bool isThisCardShining = isCardShining_record[pid];//���ſ���û��
      bool isThisTrainingShining = trainShiningNum[t];//���ѵ����û��
      CardTrainingEffect eff = cardEffectCalculated ? cardEffects[pid] : 
        p.cardParam.getCardEffect(*this, isThisCardShining, t, p.friendship, p.cardRecord, cardNum_record[t], trainShiningNum[t]);
      if (!cardEffectCalculated)cardEffects[pid] = eff;
        
      for (int i = 0; i < 6; i++)//����ֵbonus
      {
        if (basicValueLower[i] > 0)
          basicValueLower[i] += eff.bonus[i];
      }
      if (isCardShining_record[pid])//���ʣ�����ӳɺ��ǲʻظ�
      {
        totalYouqingMultiplier *= (1 + 0.01 * eff.youQing);
        if (t == TRA_wiz)
          vitalCostBasic -= eff.vitalBonus;
      }
      totalXunlian += eff.xunLian;
      totalGanjing += eff.ganJing;
      vitalCostMultiplier *= (1 - 0.01 * eff.vitalCostDrop);
      failRateMultiplier *= (1 - 0.01 * eff.failRateDrop);

    }

    //������ʧ����

    if (vitalCostBasic < 0)//uafѵ���������������ʹ�Ǻܶ��ǲ�
      vitalCostBasic = 0;
    int vitalChangeInt = -int(vitalCostBasic * vitalCostMultiplier);
    if (vitalChangeInt > maxVital - vital)vitalChangeInt = maxVital - vital;
    if (vitalChangeInt < -vital)vitalChangeInt = -vital;
    trainVitalChange[t] = vitalChangeInt;
    failRate[t] = calculateFailureRate(t, failRateMultiplier);


    //��ͷ * ѵ�� * �ɾ� * ����    //֧Ԯ������
    double cardMultiplier = (1 + 0.05 * cardNum) * (1 + 0.01 * totalXunlian) * (1 + 0.1 * (motivation - 3) * (1 + 0.01 * totalGanjing)) * totalYouqingMultiplier;
    trainValueCardMultiplier[t] = cardMultiplier;

    //�²���Կ�ʼ����
    for (int i = 0; i < 6; i++)
    {
      bool isRelated = basicValueLower[i] != 0;
      double bvl = basicValueLower[i];
      double umaBonus = i < 5 ? 1 + 0.01 * fiveStatusBonus[i] : 1;
      valueLowerDouble[i] = bvl * cardMultiplier * umaBonus;
      valueDouble[i] = valueLowerDouble[i];
      //����linkѵ��
      if (i != t)
      {
        valueDouble[i] += linkBasicValueBonus[color][i] * cardMultiplier * (isRelated ? umaBonus : 1);
      }


      trainValueLower[t][i] = std::min(int(valueLowerDouble[i]),100);
      
    }
    
    //������link���ӳ�
    int linkNumTrainingRate = isXiahesu() ? GameConstants::UAF_LinkNumBonusXiahesu[colorNum] : GameConstants::UAF_LinkNumBonus[colorNum];
    valueDouble[t] *= 1.0 + 0.01 * linkNumTrainingRate;

    //�籾�ӳ�
    for (int i = 0; i < 6; i++)
    {
      valueDouble[i] *= 1.0 + 0.01 * uaf_trainingBonus;
    }

    //��buff����ֻ���ڵ�ǰѵ������������
    if (uaf_buffNum[1] > 0)
    {
      double redBuffMultiplier = 1 + 0.01 * GameConstants::UAF_RedBuffBonus[colorNum];
      valueDouble[t] *= redBuffMultiplier;
    }

    //��buff
    if (uaf_buffNum[0] > 0)
    {
      for (int i = 0; i < 5; i++)
      {
        int levelGain = uaf_trainLevelGain[i];
        //int maxLevelGain = 100 - uaf_trainingLevel[uaf_trainingColor[i]][i];
        //if (levelGain > maxLevelGain)levelGain = maxLevelGain;
        valueDouble[i] += int(levelGain / 2) + 1;
      }
      valueDouble[5] += 20;
    }

    //�ϲ�=����-�²�

    for (int i = 0; i < 6; i++)
    {
      //upper��������ȥ����100���޵��²�
      trainValueLower[t][i] = calculateRealStatusGain(i, trainValueLower[t][i]);
      int upper = valueDouble[i] - trainValueLower[t][i];
      if (upper > 100)upper = 100;
      int tot = upper + trainValueLower[t][i];
      trainValue[t][i] = calculateRealStatusGain(i, tot);
    }


  }
  cardEffectCalculated = true;
}
int Game::calculateRealStatusGain(int idx, int value) const//����1200����Ϊ2�ı�����ʵ����������ֵ
{
  if (idx == 5)return value;
  int newValue = fiveStatus[idx] + value;
  if (newValue <= 1200)return value;
  if (value == 1)return 2;
  return (newValue / 2) * 2 - fiveStatus[idx];
}
void Game::addStatus(int idx, int value)
{
  int t = fiveStatus[idx] + value;
  
  if (t > fiveStatusLimit[idx])
    t = fiveStatusLimit[idx];
  if (t < 1)
    t = 1;
  if (idx < 5 && t > 1200)
    t = (t / 2) * 2;
  fiveStatus[idx] = t;
}
void Game::addVital(int value)
{
  vital += value;
  if (vital > maxVital)
    vital = maxVital;
  if (vital < 0)
    vital = 0;
}
void Game::addMotivation(int value)
{
  if (value < 0)
  {
    if (isPositiveThinking)
      isPositiveThinking = false;
    else
    {
      motivation += value;
      if (motivation < 1)
        motivation = 1;
    }
  }
  else
  {
    motivation += value;
    if (motivation > 5)
      motivation = 5;
  }
}
void Game::addJiBan(int idx, int value)
{
  auto& p = persons[idx];
  if (p.personType == 1 || p.personType == 2)
  {
    if (isAiJiao)value += 2;
  }
  else if (p.personType == 4 || p.personType == 5 || p.personType == 6)
  {
    //����
  }
  else //npc
    value = 0;
  p.friendship += value;
  if (p.friendship > 100)p.friendship = 100;
}
void Game::addAllStatus(int value)
{
  for (int i = 0; i < 5; i++)addStatus(i, value);
}
int Game::calculateFailureRate(int trainType, double failRateMultiply) const
{
  //������ϵ�ѵ��ʧ���ʣ����κ��� A*(x0-x)^2+B*(x0-x)
  //���Ӧ����2%����
  static const double A = 0.025;
  static const double B = 1.25;
  double x0 = 0.1 * GameConstants::FailRateBasic;
  
  double f = 0;
  double dif = x0 - vital;
  if (dif > 0)
  {
    f = A * dif * dif + B * dif;
  }
  if (f < 0)f = 0;
  if (f > 99)f = 99;//����ϰ���֣�ʧ�������99%
  f *= failRateMultiply;//֧Ԯ����ѵ��ʧ�����½�����
  int fr = round(f);
  fr += failureRateBias;
  if (fr < 0)fr = 0;
  if (fr > 100)fr = 100;
  return fr;
}
int Game::uaf_competitionFinishedNum() const
{
  if (turn < 24)
    return 0;
  else if (turn < 36)
    return 1;
  else if (turn < 48)
    return 2;
  else if (turn < 60)
    return 3;
  else if (turn < 72)
    return 4;
  return 5;
}
bool Game::isXiangtanLegal(int x) const
{
  if (x == XT_none)return true;
  if (Action::XiangtanNumCost[x] > uaf_xiangtanRemain)return false;//��̸��������
  bool haveColor[3] = { false,false,false };
  for (int i = 0; i < 5; i++)
    haveColor[uaf_trainingColor[i]] = true;
  if (x <= 6)//һ����̸
  {
    if (!haveColor[Action::XiangtanFromColor[x]])
      return false;
    else return true;
  }
  else if (x == XT_b)
    return haveColor[1] && haveColor[2];
  else if (x == XT_r)
    return haveColor[0] && haveColor[2];
  else if (x == XT_y)
    return haveColor[0] && haveColor[1];
  else
    assert(false);
  return false;
}
void Game::xiangtanAndRecalculate(int x, bool forHandwrittenLogic)
{
  if (x == 0)return;
  int targetC = Action::XiangtanToColor[x];
  int sourceC = Action::XiangtanFromColor[x];
  for (int i = 0; i < 5; i++)
  {
    if (sourceC == -1 || sourceC == uaf_trainingColor[i])
      uaf_trainingColor[i] = targetC;
  }
  uaf_xiangtanRemain -= Action::XiangtanNumCost[x];
  assert(uaf_xiangtanRemain >= 0);
  if (!forHandwrittenLogic)
    cardEffectCalculated = false;//������������¼��㣬������˾��֮��Ĺ��п��ܻ�ı�
  calculateTrainingValue();
}
void Game::runRace(int basicFiveStatusBonus, int basicPtBonus)
{
  double raceMultiply = 1 + 0.01 * saihou;
  int fiveStatusBonus = floor(raceMultiply * basicFiveStatusBonus);
  int ptBonus = floor(raceMultiply * basicPtBonus);
  addAllStatus(fiveStatusBonus);
  skillPt += basicPtBonus;
}

void Game::addStatusFriend(int idx, int value)
{
  value = int(value * lianghua_statusBonus);
  if (idx == 5)skillPt += value;
  else addStatus(idx, value);
}

void Game::addVitalFriend(int value)
{
  value = int(value * lianghua_vitalBonus);
  addVital(value);
}


void Game::handleFriendOutgoing(std::mt19937_64& rand)
{
  assert(lianghua_type!=0 && persons[lianghua_personId].friendOrGroupCardStage>=2 && lianghua_outgoingUsed < 5);
  int pid = lianghua_personId;
  if (lianghua_outgoingUsed == 0)
  {
    addVitalFriend(35);
    addMotivation(1);
    addStatusFriend(0, 15);
    addJiBan(pid, 5);
  }
  else if (lianghua_outgoingUsed == 1)
  {
    addVitalFriend(30);
    addMotivation(1);
    addStatusFriend(0, 10);
    addStatusFriend(4, 10);
    addJiBan(pid, 5);
  }
  else if (lianghua_outgoingUsed == 2)
  {
    addVitalFriend(50);
    addMotivation(1);
    addJiBan(pid, 5);
  }
  else if (lianghua_outgoingUsed == 3)
  {
    addVitalFriend(30);
    addMotivation(1);
    addStatusFriend(0, 25);
    addJiBan(pid, 5);
  }
  else if (lianghua_outgoingUsed == 4)
  {
    //�д�ɹ��ͳɹ�
    if (rand() % 4 != 0)//���Թ��ƣ�75%��ɹ�
    {
      addVitalFriend(40);
      addMotivation(1);
      addStatusFriend(0, 30);
      addJiBan(pid, 5);
      skillPt += 72;//���ܵȼ�
    }
    else
    {
      addVitalFriend(35);
      addStatusFriend(0, 15);
      addJiBan(pid, 5);
      skillPt += 40;//���ܵȼ�
    }
  }
  else assert(false && "δ֪�ĳ���");

  //ȫ��ȼ�+1
  for(int i=0;i<3;i++)
    for (int j = 0; j < 5; j++)
    {
      uaf_trainingLevel[i][j] += 1;
      if (uaf_trainingLevel[i][j] > 100)
        uaf_trainingLevel[i][j] = 100;
    }

  lianghua_outgoingUsed += 1;
}
void Game::handleFriendUnlock(std::mt19937_64& rand)
{
  printEvents("�������������");
  maxVital += 4;
  if (maxVital > 120)maxVital = 120;
  addVitalFriend(20);
  addMotivation(1);
  addJiBan(lianghua_personId, 5);
  persons[lianghua_personId].friendOrGroupCardStage = 2;
}
void Game::handleFriendClickEvent(std::mt19937_64& rand, int atTrain)
{
  assert(persons[lianghua_personId].personType == PersonType_lianghuaCard);
  if (persons[lianghua_personId].friendOrGroupCardStage == 0)
  {
    printEvents("��һ�ε�����"); 
    persons[lianghua_personId].friendOrGroupCardStage = 1;
    maxVital += 4;
    if (maxVital > 120)maxVital = 120;
    addStatusFriend(0, 5);
    addStatusFriend(4, 5);
    addJiBan(lianghua_personId, 10);
    addMotivation(1);
  }
  else
  {
    if (rand() % 5 < 3)return;//40%���ʳ��¼���60%���ʲ���
    addJiBan(lianghua_personId, 5);
    addVitalFriend(7);
    if (rand() % 10 == 0)
    {
      if (motivation != 5)
        printEvents("�����¼�����+1");
      addMotivation(1);//10%���ʼ�����
    }

  }

}
void Game::handleFriendFixedEvent()
{
  if (lianghua_type == 0)return;//û���˿�
  if (persons[lianghua_personId].friendOrGroupCardStage < 2)return;//����û������û�¼�
  if (turn == 23)
  {
    maxVital += 4;
    if (maxVital > 120)maxVital = 120;
    addVitalFriend(10);
    addMotivation(1);
    addStatusFriend(0, 10);
    addJiBan(lianghua_personId, 5);
    skillPt += 40;//����ĩ�ţ����ǵ������ȫ��ȫ�飬�����н�����������hint����Ч��
  }
  else if (turn == 77)
  {
    if (lianghua_outgoingUsed >= 5)//�������
    {
      addStatusFriend(0, 30);
      addStatusFriend(4, 30);
      addStatusFriend(5, 45);
    }
    else
    {
      addStatusFriend(0, 25);
      addStatusFriend(4, 25);
      addStatusFriend(5, 35);
    }

  }
  else
  {
    assert(false && "�����غ�û�����˹̶��¼�");
  }
}
void Game::checkLianghuaGuyou()
{
  if (lianghua_type == 1)
  {
    if ((!lianghua_guyouEffective) && persons[lianghua_personId].friendship >= 60)
    {
      lianghua_guyouEffective = true;
      for (int card = 0; card < 6; card++)
      {
        Person& p = persons[card];
        auto probs = p.distribution.probabilities();
        probs[5] /= 2;
        p.distribution = std::discrete_distribution<>(probs.begin(), probs.end());
      }
    }
  }
}
bool Game::applyTraining(std::mt19937_64& rand, Action action)
{
  uaf_lastTurnNotTrain = false;
  if (isRacing)
  {
    assert(false && "���о籾��������checkEventAfterTrain()�ﴦ������applyTraining");
    return false;//���о籾��������checkEventAfterTrain()�ﴦ���൱�ڱ����غ�ֱ���������������������
  }
  if (action.train == TRA_rest)//��Ϣ
  {
    if (isXiahesu())
    {
      addVital(40);
      addMotivation(1);
    }
    else
    {
      int r = rand() % 100;
      if (r < 25)
        addVital(70);
      else if (r < 82)
        addVital(50);
      else
        addVital(30);
    }
    uaf_lastTurnNotTrain = true;
  }
  else if (action.train == TRA_race)//����
  {
    if (turn <= 12 || turn >= 72)
    {
      printEvents("Cannot race now."); 
      return false;
    }
    addAllStatus(1);//������
    runRace(2, 40);//���ԵĽ���

    //����̶�15
    addVital(-15);
    if (rand() % 10 == 0)
      addMotivation(1);
    uaf_lastTurnNotTrain = true;
  }
  else if (action.train == TRA_outgoing)//���
  {
    if (isXiahesu())
    {
      printEvents("�ĺ���ֻ����Ϣ���������");
      return false;
    }

    //��������
    if (lianghua_type != 0 &&  //����������
      persons[lianghua_personId].friendOrGroupCardStage >= 2 &&  //�ѽ������
      lianghua_outgoingUsed < 5  //���û����
      )
    {
      handleFriendOutgoing(rand);
    }
    else //��ͨ����
    {
      //���ò�����ˣ���50%��2���飬50%��1����10����
      if (rand() % 2)
        addMotivation(2);
      else
      {
        addMotivation(1);
        addVital(10);
      }
    }
    uaf_lastTurnNotTrain = true;
  }
  else if (action.train <= 4 && action.train >= 0)//����ѵ��
  {
    if (action.xiangtanType != XT_none)
      xiangtanAndRecalculate(action.xiangtanType, false);

    if (rand() % 100 < failRate[action.train])//ѵ��ʧ��
    {
      if (failRate[action.train] >= 20 && (rand() % 100 < failRate[action.train]))//ѵ����ʧ�ܣ�������Ϲ�µ�
      {
        printEvents("ѵ����ʧ�ܣ�");
        addStatus(action.train, -10);
        if (fiveStatus[action.train] > 1200)
          addStatus(action.train, -10);//��Ϸ��1200���Ͽ����Բ��۰룬�ڴ�ģ�������Ӧ1200���Ϸ���
        //�����2��10�������ĳ�ȫ����-4���������
        for (int i = 0; i < 5; i++)
        {
          addStatus(i, -4);
          if (fiveStatus[i] > 1200)
            addStatus(i, -4);//��Ϸ��1200���Ͽ����Բ��۰룬�ڴ�ģ�������Ӧ1200���Ϸ���
        }
        addMotivation(-3);
        addVital(10);
      }
      else//Сʧ��
      {
        printEvents("ѵ��Сʧ�ܣ�");
        addStatus(action.train, -5);
        if (fiveStatus[action.train] > 1200)
          addStatus(action.train, -5);//��Ϸ��1200���Ͽ����Բ��۰룬�ڴ�ģ�������Ӧ1200���Ϸ���
        addMotivation(-1);
      }
    }
    else
    {
      //�ȼ���ѵ��ֵ
      for (int i = 0; i < 5; i++)
        addStatus(i, trainValue[action.train][i]);
      skillPt += trainValue[action.train][5];
      addVital(trainVitalChange[action.train]);

      vector<int> hintCards;//���ļ����������̾����
      bool clickFriend = false;//���ѵ����û������
      for (int i = 0; i < 5; i++)
      {
        int p = personDistribution[action.train][i];
        if (p < 0)break;//û��
        int personType = persons[p].personType;

        if (personType == PersonType_lianghuaCard)//���˿�
        {
          addJiBan(p, 4);
          clickFriend = true;
        }
        else if (personType== PersonType_card)//��ͨ��
        {
          addJiBan(p, 7);
          if(persons[p].isHint)
            hintCards.push_back(p);
        }
        else if (personType == PersonType_npc)//npc
        {
          assert(false);
        }
        else if (personType == PersonType_lishizhang)//���³�
        {
          int jiban = persons[p].friendship;
          int g = jiban < 40 ? 2 : jiban < 60 ? 3 : jiban < 80 ? 4 : 5;
          skillPt += g;
          addJiBan(p, 7);
        }
        else if (personType == PersonType_jizhe)//����
        {
          int jiban = persons[p].friendship;
          int g = jiban < 40 ? 2 : jiban < 60 ? 3 : jiban < 80 ? 4 : 5;
          addStatus(action.train, g);
          addJiBan(p, 9);
        }
        else if (personType == PersonType_lianghuaNonCard)//�޿�����
        {
          int jiban = persons[p].friendship;
          int g = jiban < 40 ? 2 : jiban < 60 ? 3 : jiban < 80 ? 4 : 5;
          skillPt += g;
          addJiBan(p, 7);
        }
        else
        {
          //��������/�ſ��ݲ�֧��
          assert(false);
        }
      }

      if (hintCards.size() > 0)
      {
        int hintCard = hintCards[rand() % hintCards.size()];//���һ�ſ���hint

        addJiBan(hintCard, 5);
        auto& hintBonus = persons[hintCard].cardParam.hintBonus;
        for (int i = 0; i < 5; i++)
          addStatus(i, hintBonus[i]);
        skillPt += hintBonus[5];
        //��buff��˫��
        if (uaf_buffNum[2] > 0)
        {
          for (int i = 0; i < 5; i++)
            addStatus(i, hintBonus[i]);
          skillPt += hintBonus[5];
        }
      }

      if (clickFriend)
        handleFriendClickEvent(rand, action.train);
      

      //ѵ���ȼ�����
      int thisColor = uaf_trainingColor[action.train];
      for (int i = 0; i < 5; i++)
      {
        if (uaf_trainingColor[i] == thisColor)
        {
          uaf_trainingLevel[thisColor][i] += uaf_trainLevelGain[i];
          if (uaf_trainingLevel[thisColor][i] > 100)uaf_trainingLevel[thisColor][i] = 100;
        }
      }

    }

    //buff����-1
    for (int color = 0; color < 3; color++)
    {
      if (uaf_buffNum[color] > 0)uaf_buffNum[color] -= 1;
    }
  }
  else
  {
    printEvents("δ֪��ѵ����Ŀ");
    return false;
  }
  return true;
}


bool Game::isLegal(Action action) const
{
  if (isRacing)
  {
    assert(false && "���о籾��������checkEventAfterTrain()�ﴦ������applyTraining");
    return false;//���о籾��������checkEventAfterTrain()�ﴦ���൱�ڱ����غ�ֱ���������������������
  }
  if (action.xiangtanType != XT_none && (!(action.train >= 0 && action.train <= 4)))
    return false;//��̸����ѵ��
  if (action.train == TRA_rest)
  {
    return true;
  }
  else if (action.train == TRA_outgoing)
  {
    if (isXiahesu())
    {
      return false;//�ҽ��ĺ��޵������Ϊ��Ϣ
    }
    return true;
  }
  else if (action.train == TRA_race)
  {
    if (turn <= 12 || turn >= 72)
    {
      return false;
    }
    return true;
  }
  else if (action.train >= 0 && action.train <= 4)
  {
    if (!isXiangtanLegal(action.xiangtanType))
      return false;
    //�����̸�Ƿ�������
    //�󲿷�����Ѿ���isXiangtanLegal���ų���ֻʣ��һ�����û�ų���ֻ��̸һ��a��ɫb�������ѵ����c
    if (action.xiangtanType >= 1 && action.xiangtanType <= 6)
    {
      int c = uaf_trainingColor[action.train];
      if (c != Action::XiangtanFromColor[action.xiangtanType] && c != Action::XiangtanToColor[action.xiangtanType])
        return false;
    }
    return true;
  }
  else
  {
    assert(false && "δ֪��ѵ����Ŀ");
    return false;
  }
  return false;
}



float Game::getSkillScore() const
{
  return ptScoreRate * skillPt + skillScore;
}

int Game::finalScore() const
{
  int total = 0;
  for (int i = 0; i < 5; i++)
    total += GameConstants::FiveStatusFinalScore[min(fiveStatus[i],fiveStatusLimit[i])];
  
  total += getSkillScore();
  //return uaf_haveLose ? 10000 : 20000;
  return total;
}

bool Game::isEnd() const
{
  return turn >= TOTAL_TURN;
}

int Game::getTrainingLevel(int item) const
{
  if (isXiahesu())return 4;

  int splevel = uaf_trainingLevel[uaf_trainingColor[item]][item];
  return convertTrainingLevel(splevel);
}


void Game::checkEventAfterTrain(std::mt19937_64& rand)
{

  checkFixedEvents(rand);

  checkRandomEvents(rand);


  //�غ���+1
  turn++;
  if (turn < TOTAL_TURN)
  {
    isRacing = isRacingTurn[turn];
    if (isRacing)
      checkEventAfterTrain(rand);//��������غ�
  }
  else
  {
    printEvents("���ɽ���!");
    printEvents("��ĵ÷��ǣ�" + to_string(finalScore()));
  }

}
void Game::uaf_checkNewBuffAfterLevelGain()
{
  for (int color = 0; color < 3; color++)
  {
    int leveltotal = 0;
    for (int i = 0; i < 5; i++)
      leveltotal += uaf_trainingLevel[color][i];

    int buffNumTotal = leveltotal / 50;
    int buffNumUsed = uaf_buffActivated[color];
    if (buffNumTotal > buffNumUsed)
    {
      uaf_buffNum[color] += 2 * (buffNumTotal - buffNumUsed);
      uaf_buffActivated[color] = buffNumTotal;
    }
  }
}
void Game::uaf_runCompetition(int n)//��n��uaf���
{
  uaf_xiangtanRemain = 3;
  int winCount = 0;
  int levelRequired = 10 + n * 10;
  for (int color = 0; color < 3; color++)
  {
    for (int i = 0; i < 5; i++)
    {
      if (uaf_trainingLevel[color][i] >= levelRequired)
      {
        uaf_winHistory[n][color][i] = true;
        winCount++;
      }
    }

  }

  if (winCount < 12)// <12win ͻ��/����
  {
    //������3��5��10��15��20
    int statusGain = n * 5;
    if (statusGain == 0)statusGain = 3;
    if (isLinkUma)statusGain += 3;
    addAllStatus(statusGain);
    skillPt += 30 + 10 * n;
  }
  else // >=12win �ۺ���ʤ����ʤ
  {
    addJiBan(6, 5);//���³��
    int statusGain = 5 + n * 5;
    if (isLinkUma)statusGain += 3;
    addAllStatus(statusGain);
    skillPt += 40 + 10 * n;
  }
  if (winCount == 15) //ȫʤ
  {
    addMotivation(1);
    addVital(15);
  }
}
void Game::checkFixedEvents(std::mt19937_64& rand)
{
  //������̶ֹ��¼�
  uaf_checkNewBuffAfterLevelGain();
  if (isRacing)//���ı���
  {
    runRace(3, 45);
    addJiBan(6, 4);
    uaf_lastTurnNotTrain = true;
  }

  if (turn == 11)//��̸ˢ��
  {
    assert(isRacing);
    uaf_xiangtanRemain = 3;
  }
  else if (turn == 23)//��һ�����
  {
    uaf_runCompetition(0);
    //����¼���������ѡ������������ѡ����
    {
      int vitalSpace = maxVital - vital;//�������������
      handleFriendFixedEvent();
      if (vitalSpace >= 20)
        addVital(20);
      else
        addAllStatus(5);
    }
    printEvents("uaf���1����");
  }
  else if (turn == 29)//�ڶ���̳�
  {

    for (int i = 0; i < 5; i++)
      addStatus(i, zhongMaBlueCount[i] * 6); //�����ӵ���ֵ

    double factor = double(rand() % 65536) / 65536 * 2;//�籾�������0~2��
    for (int i = 0; i < 5; i++)
      addStatus(i, int(factor*zhongMaExtraBonus[i])); //�籾����
    skillPt += int((0.5 + 0.5 * factor) * zhongMaExtraBonus[5]);//���߰��㼼�ܵĵ�Чpt

    for (int i = 0; i < 5; i++)
      fiveStatusLimit[i] += zhongMaBlueCount[i] * 2; //��������--�������ֵ��18�����μ̳й��Ӵ�Լ36���ޣ�ÿ��ÿ��������+1���ޣ�1200�۰��ٳ�2

    for (int i = 0; i < 5; i++)
      fiveStatusLimit[i] += rand() % 8; //��������--�����μ̳��������

    printEvents("�ڶ���̳�");
  }
  else if (turn == 35)//uaf2
  {
    uaf_runCompetition(1);
    printEvents("uaf���2����");
  }

  else if (turn == 47)//�ڶ������
  {
    uaf_runCompetition(2);
    //����¼���������ѡ������������ѡ����
    {
      int vitalSpace = maxVital - vital;//�������������
      if (vitalSpace >= 30)
        addVital(30);
      else
        addAllStatus(8);
    }
    printEvents("uaf���3����");
  }
  else if (turn == 48)//�齱
  {
    int rd = rand() % 100;
    if (rd < 16)//��Ȫ��һ�Ƚ�
    {
      addVital(30);
      addAllStatus(10);
      addMotivation(2);

      printEvents("�齱�����������Ȫ/һ�Ƚ�");
    }
    else if (rd < 16 + 27)//���Ƚ�
    {
      addVital(20);
      addAllStatus(5);
      addMotivation(1);
      printEvents("�齱��������˶��Ƚ�");
    }
    else if (rd < 16 + 27 + 46)//���Ƚ�
    {
      addVital(20);
      printEvents("�齱������������Ƚ�");
    }
    else//��ֽ
    {
      addMotivation(-1);
      printEvents("�齱��������˲�ֽ");
    }
  }
  else if (turn == 49)
  {
    skillScore += 170;
    printEvents("���еȼ�+1");
  }
  else if (turn == 53)//������̳�
  {
    for (int i = 0; i < 5; i++)
      addStatus(i, zhongMaBlueCount[i] * 6); //�����ӵ���ֵ

    double factor = double(rand() % 65536) / 65536 * 2;//�籾�������0~2��
    for (int i = 0; i < 5; i++)
      addStatus(i, int(factor * zhongMaExtraBonus[i])); //�籾����
    skillPt += int((0.5 + 0.5 * factor) * zhongMaExtraBonus[5]);//���߰��㼼�ܵĵ�Чpt

    for (int i = 0; i < 5; i++)
      fiveStatusLimit[i] += zhongMaBlueCount[i] * 2; //��������--�������ֵ��18�����μ̳й��Ӵ�Լ36���ޣ�ÿ��ÿ��������+1���ޣ�1200�۰��ٳ�2

    for (int i = 0; i < 5; i++)
      fiveStatusLimit[i] += rand() % 8; //��������--�����μ̳��������

    printEvents("������̳�");

    if (persons[6].friendship >= 60)
    {
      skillScore += 170;//���м��ܵȼ�+1
      addMotivation(1);
    }
    else
    {
      addVital(-5);
      skillPt += 25;
    }
  }
  else if (turn == 59)//uaf4
  {
    uaf_runCompetition(3);
    printEvents("uaf���4����");
    }
  else if (turn == 65)//����+1�������ľ���hint+1
  {
    addMotivation(1);
    skillPt += 5;//���hintһ�㲻��
  }
  else if (turn == 70)
  {
    skillScore += 170;//���м��ܵȼ�+1
  }
  else if (turn == 71)//uaf5
  {
    uaf_runCompetition(4);
    printEvents("uaf���5����");
  }
  else if (turn == 73)//ura1
  {
    assert(isRacing);
    runRace(10, 40);
    printEvents("ura1����");
  }
  else if (turn == 75)//ura2
  {
    assert(isRacing);
    runRace(10, 60);
    printEvents("ura2����");
  }
  else if (turn == 77)//ura3����Ϸ����
  {
    assert(isRacing);
    runRace(10, 80);

    //����
    if (persons[7].friendship >= 100)
    {
      addAllStatus(5);
      skillPt += 20;
    }
    else if (persons[7].friendship >= 80)
    {
      addAllStatus(3);
      skillPt += 10;
    }
    else
    {
      skillPt += 5;
    }

    bool allWin = true;//ÿһ����15win
    bool allTotallyWin = true;//ÿһ����12win����

    for (int c = 0; c < 5; c++)
    {
      int thisWinNum = 0;
      for (int color = 0; color < 3; color++)
      {
        for (int j = 0; j < 5; j++)
        {
          if (uaf_winHistory[c][color][j])
            thisWinNum++;
        }
      }
      if (thisWinNum < 15)allWin = false;
      if (thisWinNum < 12)allTotallyWin = false;
    }
    if (allWin)
    {
      skillPt += 40;//ȫ��ȫ�������
      addAllStatus(55);
      skillPt += 140;
      //cout << 1;
    }
    else if (allTotallyWin)
    {
      skillPt += 40;//ȫ��ȫ�������
      addAllStatus(30);
      skillPt += 90;
      //cout << 2;
    }
    else
    {
      addAllStatus(20);
      skillPt += 70;
      //cout << 3;
    }


    int totalLevel = 0;
    for (int color = 0; color < 3; color++)
    {
      for (int j = 0; j < 5; j++)
      {
        totalLevel += uaf_trainingLevel[color][j];
      }
    }
    if (totalLevel >= 1200)
    {
      skillPt += 60;//���۽���λ
    }
    else
    {
      skillPt += 20;//һ�۽���λ
    }


    //���˿��¼�
    handleFriendFixedEvent();

    addAllStatus(5);
    skillPt += 30;

    printEvents("ura3��������Ϸ����");
  }
}

void Game::checkRandomEvents(std::mt19937_64& rand)
{
  if (turn>=72)
    return;//ura�ڼ䲻�ᷢ����������¼�

  //���˻᲻���������
  if (lianghua_type != 0)
  {
    Person& p = persons[lianghua_personId];
    assert(p.personType == PersonType_lianghuaCard);
    if (p.friendOrGroupCardStage==1)
    {
      double unlockOutgoingProb = p.friendship >= 60 ?
        GameConstants::FriendUnlockOutgoingProbEveryTurnHighFriendship :
        GameConstants::FriendUnlockOutgoingProbEveryTurnLowFriendship;
      if (randBool(rand, unlockOutgoingProb))//����
      {
        handleFriendUnlock(rand);
      }
    }
  }

  //ģ���������¼�

  //֧Ԯ�������¼��������һ������5�
  if (randBool(rand, GameConstants::EventProb))
  {
    int card = rand() % 6;
    addJiBan(card, 5);
    //addAllStatus(4);
    addStatus(rand() % 5, eventStrength);
    skillPt += eventStrength;
    printEvents("ģ��֧Ԯ������¼���" + persons[card].cardParam.cardName + " ���+5��pt���������+" + to_string(eventStrength));

    //֧Ԯ��һ����ǰ�����¼�������
    if (randBool(rand, 0.2 * (1.0 - turn * 1.0 / TOTAL_TURN)))
    {
      addMotivation(1);
      printEvents("ģ��֧Ԯ������¼�������+1");
    }
    if (randBool(rand, 0.3))
    {
      addVital(10);
      printEvents("ģ��֧Ԯ������¼�������+10");
    }
    else if (randBool(rand, 0.03))
    {
      addVital(-10);
      printEvents("ģ��֧Ԯ������¼�������-10");
    }
  }

  //ģ����������¼�
  if (randBool(rand, 0.1))
  {
    addAllStatus(3);
    printEvents("ģ����������¼���ȫ����+3");
  }

  //������
  if (randBool(rand, 0.10))
  {
    addVital(5);
    printEvents("ģ������¼�������+5");
  }

  //��30�������Է��¼���,���ҳԷ������Ըĳ�10����
  if (randBool(rand, 0.02))
  {
    addVital(30);
    printEvents("ģ������¼�������+10");
  }

  //������
  if (randBool(rand, 0.02))
  {
    addMotivation(1);
    printEvents("ģ������¼�������+1");
  }

  //������
  if (randBool(rand, 0.04))
  {
    addMotivation(-1);
    printEvents("ģ������¼���\033[0m\033[33m����-1\033[0m\033[32m");
  }

}

void Game::applyTrainingAndNextTurn(std::mt19937_64& rand, Action action)
{
  assert(turn < TOTAL_TURN && "Game::applyTrainingAndNextTurn��Ϸ�ѽ���");
  assert(!isRacing && "�����غ϶���checkEventAfterTrain��������");
  bool suc = applyTraining(rand, action);
  assert(suc && "Game::applyTrainingAndNextTurnѡ���˲��Ϸ���ѵ��");

  checkEventAfterTrain(rand);
  if (isEnd()) return;

  assert(!isRacing && "�����غ϶���checkEventAfterTrain��������");

  // ���Ҫ֧������Ϸ�иı�����ʣ���Ҫ��������µ����ʵ�ֵ
  randomDistributeCards(rand);
}

bool Game::isCardShining(int personIdx, int trainIdx) const
{
  const Person& p = persons[personIdx];
  if(p.personType==PersonType_card)
  { 
    return p.friendship >= 80 && trainIdx == p.cardParam.cardType;
  }
  else if (p.personType == PersonType_groupCard)
  {
    return p.friendOrGroupCardStage == 3;
  }
  return false;
}

