#include <iostream>
#include <cassert>
#include "NNInput.h"
#include "../Search/SearchParam.h"
#include "../Game/Game.h"
using namespace std;


void SupportCard::getCardParamNNInputV1(float* buf, const Game& game) const
{
  //每张卡的初始属性加成、初始羁绊、赛后加成不需要告诉神经网络，只告诉总赛后

  for (int ch = 0; ch < NNINPUT_CHANNELS_CARD_V1; ch++)
    buf[ch] = 0;

  //0~6 cardtype
  buf[cardType] = 1.0;

  //大致映射到0~1范围
  buf[7] = youQingBasic * 0.04;
  buf[8] = ganJingBasic * 0.02;
  buf[9] = xunLianBasic * 0.05;
  for (int i = 0; i < 6; i++)
    buf[10 + i] = bonusBasic[i] * 0.5;
  buf[16] = wizVitalBonusBasic * 0.2;
  //for (int i = 0; i < 6; i++)
  //  buf[17 + i] = hintBonus[i] * 0.05;
  if(hintLevel > 0)
    buf[17] = hintLevel * 0.4;
  else
    buf[18] = 1.0;
  buf[23] = hintProbIncrease * 0.02;
  buf[24] = deYiLv * 0.02;
  buf[25] = failRateDrop * 0.04;
  buf[26] = vitalCostDrop * 0.05;

  buf[27] = 0.0; //reserve
  buf[28] = 0.0;

  //是否link，link的固定buff，在Person::getNNInputV1中写


  //固有
  assert(isDBCard);
  const int BasicC = 29;
  const int UniqueTypeC = 35;//0~29直接编号，30~34特殊固有
  assert(uniqueEffectType < 30);
  const int UniqueEffectC = 13;
  static_assert(BasicC + UniqueTypeC + UniqueEffectC == NNINPUT_CHANNELS_CARD_V1);
  float* bufUniqueType = buf + BasicC;
  float* bufUniqueValue = bufUniqueType + UniqueTypeC;

  auto writeUniqueEffect = [&](int key, double value)
  {
    //依次是：0~4速耐力根智，5pt，6友情，7干劲，8训练，9失败率，10体力花费减少，11智力彩圈体力
    if (key <= 0)
      return;
    else if (key == 1)
    {
      bufUniqueValue[6] = 0.04 * value;
    }
    else if (key == 2)
    {
      bufUniqueValue[7] = 0.02 * value;
    }
    else if (key == 8)
    {
      bufUniqueValue[8] = 0.05 * value;
    }
    else if (key == 19)
    {
    }
    else if (key == 15)
    {
    }
    else if (key == 27)
    {
      bufUniqueValue[9] = 0.04 * value;
    }
    else if (key == 28)
    {
      bufUniqueValue[10] = 0.05 * value;
    }
    else if (key == 31)
    {
      bufUniqueValue[11] = 0.2 * value;
    }
    else if (key == 3)
    {
      bufUniqueValue[0] = 0.5 * value;
    }
    else if (key == 4)
    {
      bufUniqueValue[1] = 0.5 * value;
    }
    else if (key == 5)
    {
      bufUniqueValue[2] = 0.5 * value;
    }
    else if (key == 6)
    {
      bufUniqueValue[3] = 0.5 * value;
    }
    else if (key == 7)
    {
      bufUniqueValue[4] = 0.5 * value;
    }
    else if (key == 30)
    {
      bufUniqueValue[5] = 0.5 * value;
    }
    else if (key == 41)
    {
      for (int i = 0; i < 5; i++)
        bufUniqueValue[i] = 0.5;
    }
    else
    {
      assert(false && "todo");
      assert(false);
    }
  };

  if (uniqueEffectType == 0)
  {
    bufUniqueType[0] = 1.0;
  }
  else if (uniqueEffectType == 1 || uniqueEffectType == 2)
  {
    if (uniqueEffectParam[1] == 80)
      bufUniqueType[1] = 1.0;
    else if (uniqueEffectParam[1] == 100)
      bufUniqueType[2] = 1.0;
    else
      assert(false);

    writeUniqueEffect(uniqueEffectParam[2], uniqueEffectParam[3]);
    writeUniqueEffect(uniqueEffectParam[4], uniqueEffectParam[5]);
    if (cardID / 10 == 30137)
    {
      writeUniqueEffect(1, 10);
      writeUniqueEffect(2, 15);
    }
  }
  else if (uniqueEffectType == 3)
  {
    bufUniqueType[3] = 1.0;
  }
  else if (uniqueEffectType == 4)
  {
    bufUniqueType[4] = 1.0;
  }
  else if (uniqueEffectType == 5)
  {
    
  }
  else if (uniqueEffectType == 6)
  {
    bufUniqueType[6] = 1.0;
    writeUniqueEffect(1, uniqueEffectParam[1] * uniqueEffectParam[3]);
  }
  else if (uniqueEffectType == 7)
  {
    bufUniqueType[7] = 1.0;
  }
  else if (uniqueEffectType == 8)
  {
    bufUniqueType[8] = 1.0;
  }
  else if (uniqueEffectType == 9)
  {
    bufUniqueType[9] = 1.0;
  }
  else if (uniqueEffectType == 10)
  {
    bufUniqueType[10] = 1.0;
  }
  else if (uniqueEffectType == 11)
  {
    bufUniqueType[11] = 1.0;
  }
  else if (uniqueEffectType == 12)
  {
    bufUniqueType[12] = 1.0;
  }
  else if (uniqueEffectType == 13)
  {
    bufUniqueType[13] = 1.0;
  }
  else if (uniqueEffectType == 14)
  {
    bufUniqueType[14] = 1.0;
  }
  else if (uniqueEffectType == 15)
  {
  }
  else if (uniqueEffectType == 16)
  {
    writeUniqueEffect(uniqueEffectParam[2], uniqueEffectParam[4] * uniqueEffectParam[3]);
    if (uniqueEffectParam[1] == 1 && uniqueEffectParam[4] == 5)//5个速度技能
    {
      bufUniqueType[16] = 1.0;
    }
    else if (uniqueEffectParam[1] == 1 && uniqueEffectParam[4] == 3)//3个速度技能
    {
      bufUniqueType[30] = 1.0;
    }
    else if (uniqueEffectParam[1] == 2 && uniqueEffectParam[4] == 3)//3个加速度技能
    {
      bufUniqueType[31] = 1.0;
    }
    else if (uniqueEffectParam[1] == 3 && uniqueEffectParam[4] == 3)//3个回体技能
    {
      bufUniqueType[32] = 1.0;
    }
    else
    {
      assert(false && "nninput: 未知的购买技能型支援卡固有");
    }
  }
  else if (uniqueEffectType == 17)
  {
    bufUniqueType[17] = 1.0;
    writeUniqueEffect(8, uniqueEffectParam[3]);
  }
  else if (uniqueEffectType == 18)//佐岳
  {
    bufUniqueType[18] = 1.0;
    assert(false && "本ai不支持非剧本友人卡");
  }
  else if (uniqueEffectType == 19)//凉花
  {
    bufUniqueType[19] = 1.0;
    assert(false && "本ai不支持非剧本友人卡");
  }
  else if (uniqueEffectType == 20)//巨匠
  {
    bufUniqueType[20] = 1.0;
    int cardTypeCount[7] = { 0,0,0,0,0,0,0 };
    for (int i = 0; i < 6; i++)
    {
      int t = game.persons[i].cardParam.cardType;
      assert(t <= 6 && t >= 0);
      cardTypeCount[t]++;
    }
    cardTypeCount[5] += cardTypeCount[6];

    for (int i = 0; i < 6; i++)
      if (cardTypeCount[i] > 2)cardTypeCount[i] = 2;
    for (int i = 0; i < 5; i++)
      if (cardTypeCount[i] > 0)
        writeUniqueEffect(i + 3, cardTypeCount[i]);  // 速耐力根智 = 0-4 = CardEffect词条3-7
    if (cardTypeCount[5] > 0)
      writeUniqueEffect(30, cardTypeCount[5]); // pt = 30
  }
  else if (uniqueEffectType == 21)//万籁
  {
    int cardTypeCount[7] = { 0,0,0,0,0,0,0 };
    for (int i = 0; i < 6; i++)
    {
      int t = game.persons[i].cardParam.cardType;
      assert(t <= 6 && t >= 0);
      cardTypeCount[t]++;
    }
    int cardTypes = 0;
    for (int i = 0; i < 7; i++)
      if (cardTypeCount[i] > 0)
        cardTypes++;
    if (cardTypes >= uniqueEffectParam[1])
    {
      bufUniqueType[21] = 1.0;
      writeUniqueEffect(uniqueEffectParam[2], uniqueEffectParam[3]);
    }
  }
  else if (uniqueEffectType == 22)//理事长
  {
    bufUniqueType[22] = 1.0;
  }
  else
  {
    assert(false && "NNInput:未知固有");
  }



}


void Person::getCardNNInputV1(float* buf, const Game& game, int index) const
{

  assert(personType >= 1 && personType <= 6 && "只有支援卡调用此函数，理事长记者等在全局参数里");

  for (int ch = 0; ch < NNINPUT_CHANNELS_PERSON_V1 + NNINPUT_CHANNELS_CARD_V1; ch++)
    buf[ch] = 0;

  //cardParam在其他地方输入
  //PersonType不用写，uaf剧本只把支援卡输入单独的nninput
  //charaId不用写
  //cardIdInGame不用写，和卡组参数放在对应固定位置就行
  buf[0] = double(friendship) / 100.0;
  buf[1] = friendship >= 80 ? 1.0 : 0.0;
  buf[2] = friendship >= 100 ? 1.0 : 0.0;
  buf[3] = isHint ? 1.0 : 0.0;
  buf[4] = 0.0;//预留
  buf[5] = 0.0;//预留
  buf[6] = 0.0;//预留
  //友人团队卡的状态在全局变量里输入
  
  //在哪个训练
  for (int tr = 0; tr < 5; tr++)
  {
    for (int i = 0; i < 5; i++)
    {
      if (game.personDistribution[tr][i] == index)
        buf[7 + tr] = 1.0;
    }
  }
  cardParam.getCardParamNNInputV1(buf + NNINPUT_CHANNELS_PERSON_V1, game);
}







void Game::getNNInputV1(float* buf, const SearchParam& param) const
{
  throw "not implemented";
  /*
  for (int i = 0; i < NNINPUT_CHANNELS_V1; i++)
    buf[i] = 0.0;
  int c = 0;

  //search param
  {

    buf[c] = 1.0 * log(param.maxRadicalFactor + 1.0);
    c++;

    //其他的我觉得都没必要输入
    
    //buf[c] = 0.0 * log(param.maxDepth + 1.0);
    //c++;
    //buf[c] = 0.0 * (log(param.samplingNum + 1.0) - 7.0);
    //c++;
    
    c += 5;//reserve

  }


;
  //isLegal
  for (int i = 0; i < Action::MAX_ACTION_TYPE; i++)
  {
    buf[c + i] = isLegal(Action::intToAction(i)) ? 1.0 : 0.0;
  }
  c += Action::MAX_ACTION_TYPE;

  buf[c] = isLinkUma ? 1.0 : 0.0;
  c++;

  //第几回合
  assert(turn < TOTAL_TURN);
  buf[c + turn] = 1.0;
  c += TOTAL_TURN;

  //赛程
  for (int i = 0; i < TOTAL_TURN; i++)
    buf[c + i] = isRacingTurn[i] ? 1.0 : 0.0;
  c += TOTAL_TURN;

  for (int i = 0; i < 5; i++)
  {
    buf[c + i] = fiveStatusBonus[i] * 0.05;
  }
  c += 5;

  buf[c] = (eventStrength - 20) * 0.2;
  c++;


  buf[c] = (vital - 50) * 0.02;
  c++;
  buf[c] = (maxVital - 100) * 0.1;
  c++;

  assert(motivation >= 1 && motivation <= 5);
  buf[c + motivation - 1] = 1.0;
  c += 5;

  for (int i = 0; i < 5; i++)
    buf[c + i] = fiveStatus[i] * 0.003;
  c += 5;
  for (int i = 0; i < 5; i++)
    buf[c + i] = (fiveStatusLimit[i] - GameConstants::BasicFiveStatusLimit[i]) * 0.01;
  c += 5;

  buf[c] = getSkillScore() * 0.0002;
  c++;

  buf[c] = ptScoreRate - 2.0;
  c++;

  //练习上手
  if (failureRateBias < 0)
    buf[c] = failureRateBias * 0.5;
  c++;

  //练习下手
  if (failureRateBias > 0)
    buf[c] = failureRateBias * 0.5;
  c++;

  if (isAiJiao)
    buf[c] = 1.0;
  c++;

  if (isPositiveThinking)
    buf[c] = 1.0;
  c++;


  for (int i = 0; i < 5; i++)
  {
    buf[c + i] = zhongMaBlueCount[i] * 0.1;
  }
  c += 5;

  for (int i = 0; i < 5; i++)
  {
    buf[c + i] = zhongMaExtraBonus[i] * 0.03;
  }
  c += 5;
  buf[c] = zhongMaExtraBonus[5] * 0.01;
  c++;


  buf[c] = saihou * 0.03;
  c++;

  assert(!isRacing);


  //理事长和记者和无卡友人
  //3*9=21 channels
  for (int p = 0; p < 3; p++)
  {
    if (p == 2 && lianghua_type != 0)
      break;
    buf[c + p * 7] = persons[p + 6].friendship * 0.01;
    buf[c + p * 7 + 1] = persons[p + 6].friendship >= 40 ? 1.0 : 0.0;
    buf[c + p * 7 + 2] = persons[p + 6].friendship >= 60 ? 1.0 : 0.0;
    buf[c + p * 7 + 3] = persons[p + 6].friendship >= 80 ? 1.0 : 0.0;

    //在哪个训练
    for (int tr = 0; tr < 5; tr++)
    {
      for (int i = 0; i < 5; i++)
      {
        if (personDistribution[tr][i] == p + 6)
          buf[c + p * 7 + 4 + tr] = 1.0;
      }
    }
  }
  c += 3 * 9;


  for (int i = 0; i < 5; i++)
  {
    buf[c + i * 3 + uaf_trainingColor[i]] = 1.0;
  }
  c += 3 * 5;

  //训练等级
  int targetLv = 10 * (turn / 12);
  if (targetLv < 10)targetLv = 10;
  if (targetLv > 50)targetLv = 0;//ura阶段
  for (int color = 0; color < 3; color++)
  {
    for (int t = 0; t < 5; t++)
    {
      int lv = uaf_trainingLevel[color][t];
      buf[c] = lv * 0.02;
      c++;

      int lv10 = lv / 10;
      assert(lv10 >= 0 && lv10 <= 10);
      if (lv10 > 5)lv10 = 5;
      buf[c + lv10] = 1.0;
      c += 6;

      buf[c] = lv >= targetLv ? 1.0 : 0.0;
      c++;
    }
  }


  buf[c] = uaf_lastTurnNotTrain ? 1.0 : 0.0;
  c++;

  assert(uaf_xiangtanRemain >= 0 && uaf_xiangtanRemain <= 3);
  buf[c + uaf_xiangtanRemain] = 1.0;
  c += 4;

  for (int i = 0; i < 3; i++)
  {
    int n = uaf_buffNum[i];
    if (n > 5)n = 5;
    assert(n >= 0);
    buf[c + n] = 1.0;
    c += 6;
  }

  if (lianghua_type != 0)
  {
    assert(lianghua_type <= 2);
    buf[c + lianghua_type - 1] = 1.0;
    c += 2;
    int lianghua_stage = persons[lianghua_personId].friendOrGroupCardStage;
    assert(lianghua_stage <= 2);
    buf[c + lianghua_stage] = 1.0;
    c += 3;
    buf[c + lianghua_outgoingUsed] = 1.0;
    c += 6;
    buf[c] = 2.0 * (lianghua_vitalBonus - 1);
    c++;
    buf[c] = 4.0 * (lianghua_statusBonus - 1);
    c++;
    buf[c] = lianghua_guyouEffective ? 1.0 : 0.0;
    c++;
  }
  else
    c += 14;


  for (int color = 0; color < 3; color++)
  {
    int lvt = uaf_trainLevelColorTotal[color];
    int lv50 = lvt / 50;
    int rem = 50 * (lv50 + 1) - lvt;
    buf[c] = 0.004 * lvt;
    c++;
    buf[c] = 0.02 * rem;
    c++;
    assert(lv50 <= 10 && lv50 >= 0);
    buf[c + lv50] = 1.0;
    c += 11;
  }

  int uafTimes = uaf_competitionFinishedNum();
  for (int color = 0; color < 3; color++)
  {
    int loseTimes = uafTimes * 5 - uaf_colorWinCount[color];
    assert(loseTimes >= 0);
    if (loseTimes > 6)loseTimes = 6;
    buf[c + loseTimes] = 1.0;
    c += 7;
  }

  buf[c] = uaf_trainingBonus * 0.02;
  c++;




  //训练数值
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 6; j++)
    {
      buf[c] = trainValue[i][j] * 0.02;
      c++;
    }
    buf[c] = trainVitalChange[i] * 0.05;
    c++;
  }
  for (int i = 0; i < 5; i++)
  {
    buf[c] = failRate[i] * 0.02;
    c++;
  }
  for (int i = 0; i < 5; i++)
  {
    buf[c] = uaf_trainLevelGain[i] * 0.2;
    c++;
  }
  for (int i = 0; i < 5; i++)
  {
    buf[c] = trainShiningNum[i] > 0 ? 1.0 : 0.0;
    c++;
  }


  //检查是否有一次小于12win
  bool haveBigLose = false;
  for (int t = 0; t < uafTimes; t++)
  {
    int num = 0;
    for (int color = 0; color < 3; color++)
      for (int i = 0; i < 5; i++)
        if (!uaf_winHistory[t][color][i])
          num++;
    if (num > 3)
      haveBigLose = true;
  }

  if (uaf_haveLose)
    buf[c] = 1.0;
  c++;
  if (haveBigLose)
    buf[c] = 1.0;
  c++;

  for (int color = 0; color < 3; color++)
    if (uaf_haveLoseColor[color])
      buf[c + color] = 1.0;
  c += 3;

  for (int i = 0; i < 5; i++)
  {
    buf[c] = (trainValueCardMultiplier[i] - 1) * 1.0;
    c++;
  }

  c += 10;//reserve
  assert(c == NNINPUT_CHANNELS_GAMEGLOBAL_V1);

  float* cardBuf = buf + NNINPUT_CHANNELS_GAMEGLOBAL_V1;
  //float* headBuf = buf + NNINPUT_CHANNELS_GAMEGLOBAL_V1 + 7 * NNINPUT_CHANNELS_CARD_V1;

  for (int card = 0; card < 6; card++)
  {
    persons[card].getCardNNInputV1(cardBuf + NNINPUT_CHANNELS_CARDPERSON_V1 * card, *this, card);
  }
  */
}