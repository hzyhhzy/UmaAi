#include <iostream>
#include <cassert>
#include "NNInput.h"
#include "../Search/SearchParam.h"
#include "../Game/Game.h"
using namespace std;


void SupportCard::getNNInputV1(float* buf, const Game& game) const
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
  for (int i = 0; i < 6; i++)
    buf[17 + i] = hintBonus[i] * 0.05;
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
  const int UniqueTypeC = 30;
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
    bufUniqueType[16] = 1.0;
    writeUniqueEffect(uniqueEffectParam[2], 5 * uniqueEffectParam[3]);
  }
  else if (uniqueEffectType == 19)
  {
    bufUniqueType[19] = 1.0;
    writeUniqueEffect(uniqueEffectParam[2], 3 * uniqueEffectParam[3]);
  }
  else if (uniqueEffectType == 20)
  {
    bufUniqueType[20] = 1.0;
    writeUniqueEffect(uniqueEffectParam[2], 3 * uniqueEffectParam[3]);
  }
  else if (uniqueEffectType == 17)
  {
    bufUniqueType[17] = 1.0;
    writeUniqueEffect(8, uniqueEffectParam[3]);
  }
  else if (uniqueEffectType == 18)
  {
    bufUniqueType[18] = 1.0;
  }
  else if (uniqueEffectType == 21)
  {
    int cardTypeCount[7] = { 0,0,0,0,0,0,0 };
    for (int i = 0; i < 6; i++)
    {
      int t = game.cardParam[i].cardType;
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
  else
  {
    assert(false);
  }



}


void Person::getNNInputV1(float* buf, const Game& game, int index) const
{

  for (int ch = 0; ch < NNINPUT_CHANNELS_PERSON_V1; ch++)
    buf[ch] = 0;


  //PersonType不用写，放在固定位置就行
  //charaId不用写
  //cardIdInGame不用写，和卡组参数放在同一个位置就行
  buf[0] = double(friendship) / 100.0;
  buf[1] = friendship >= 80 ? 1.0 : 0.0;
  buf[2] = friendship >= 100 ? 1.0 : 0.0;
  buf[3] = isShining ? 1.0 : 0.0;
  buf[4] = isHint ? 1.0 : 0.0;
  buf[5] = 0.0;//预留cardRecord
  buf[6] = 0.0;//预留cardRecord
  
  //在哪个训练
  for (int tr = 0; tr < 5; tr++)
  {
    for (int i = 0; i < 5; i++)
    {
      if (game.personDistribution[tr][i] == index)
        buf[7 + tr] = 1.0;
    }
  }

  //是否在ss
  for (int i = 0; i < 5; i++)
    if(game.larc_ssPersons[i]==index)
      buf[12] = 1.0;


  if (game.turn >= 2 && index < 15)
  {
    buf[13] = larc_isLinkCard ? 1.0 : 0.0;
    buf[14 + larc_charge] = 1.0;
    buf[17 + larc_statusType] = 1.0;
    assert(larc_specialBuff >= 3 && larc_specialBuff <= 12);
    buf[22 + larc_specialBuff - 3] = 1.0;
    //larc_level完全无用
    buf[32 + (larc_buffLevel % 3)] = 1.0;
    buf[35 + larc_nextThreeBuffs[0]] = 1.0;
    buf[48 + larc_nextThreeBuffs[1]] = 1.0;
    buf[61 + larc_nextThreeBuffs[2]] = 1.0;
  }
  else
  {
    if (personType == 2)
    {
      bool islink = game.cardParam[cardIdInGame].larc_isLink;
      if (islink)
      {
        buf[13] = 1.0;
        int specialBuff = game.cardParam[cardIdInGame].larc_linkSpecialEffect;
        assert(specialBuff >= 3 && specialBuff <= 12);
        buf[22 + specialBuff - 3] = 1.0;

      }
    }
  }

  //total 74

}







void Game::getNNInputV1(float* buf, const SearchParam& param) const
{
  for (int i = 0; i < NNINPUT_CHANNELS_V1; i++)
    buf[i] = 0.0;
  int c = 0;


  buf[c] = 0.0 * log(param.maxDepth + 1.0);
  c++;
  buf[c] = 1.0 * log(param.maxRadicalFactor + 1.0);
  c++;
  buf[c] = 0.0 * (log(param.samplingNum + 1.0) - 7.0);
  c++;

  assert(c == NNINPUT_CHANNELS_SEARCHPARAM_V1);


  //isLegal
  Action action = { 0,false, false, false, false };
  for (int i = 0; i < 10; i++)
  {
    action.train = i;
    if (isLegal(action))
      buf[c] = 1.0;
    c++;
  }

  for (int j = 0; j < 5; j++)
  {
    if (j == 0)
    {
      action.buy50p = true;
      action.buyPt10 = false;
      action.buyVital20 = false;
    }
    else if (j == 1)
    {
      action.buy50p = false;
      action.buyPt10 = true;
      action.buyVital20 = false;
    }
    else if (j == 2)
    {
      action.buy50p = true;
      action.buyPt10 = true;
      action.buyVital20 = false;
    }
    else if (j == 3)
    {
      action.buy50p = true;
      action.buyPt10 = false;
      action.buyVital20 = true;
    }
    else if (j == 4)
    {
      action.buy50p = false;
      action.buyPt10 = false;
      action.buyVital20 = true;
    }

    for (int i = 0; i < 5; i++)
    {
      action.train = i;
      if (isLegal(action))
        buf[c] = 1.0;
      c++;
    }
  }





  for (int i = 0; i < 5; i++)
  {
    buf[c + i] = fiveStatusBonus[i] * 0.05;
  }
  c += 5;

  buf[c] = (eventStrength - 20) * 0.2;
  c++;

  assert(turn < TOTAL_TURN);
  buf[c + turn] = 1.0;
  c += TOTAL_TURN;

  buf[c] = (vital - 50) * 0.02;
  c++;
  buf[c] = (maxVital - 100) * 0.1;
  c++;
  if (isQieZhe)
    buf[c] = 1.0;
  c++;
  if (isAiJiao)
    buf[c] = 1.0;
  c++;

  buf[c] = failureRateBias * 0.5;
  c++;

  for (int i = 0; i < 5; i++)
    buf[c + i] = fiveStatus[i] * 0.003;
  c += 5;
  for (int i = 0; i < 5; i++)
    buf[c + i] = (fiveStatusLimit[i] - GameConstants::BasicFiveStatusLimit[i]) * 0.01;
  c += 5;

  buf[c] = getSkillScore() * 0.0002;
  c++;

  assert(motivation >= 1 && motivation <= 5);
  buf[c + motivation - 1] = 1.0;
  c += 5;

  if (isPositiveThinking)
    buf[c] = 1.0;
  c++;



  for (int i = 0; i < 5; i++)
  {
    assert(trainLevelCount[i] <= 16);
    int a = trainLevelCount[i] / 4;
    int b = trainLevelCount[i] % 4;
    buf[c + a] = 1.0;
    buf[c + b + 5] = 1.0;
    c += 9;
  }

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

  buf[c + normalCardCount] = 1.0;
  c += 7;

  buf[c] = saihou * 0.03;
  c++;

  assert(!isRacing);
  assert(motivationDropCount == 0);//not used

  if (larc_isAbroad)
    buf[c] = 1.0;
  c++;

  buf[c] = larc_supportPtAll / 170000.0;
  c++;

  int larc_trainBonusLevel = (larc_supportPtAll + 85) / 8500;
  if (larc_trainBonusLevel > 40)larc_trainBonusLevel = 40;
  double larc_trainBonus = GameConstants::LArcTrainBonusEvery5Percent[larc_trainBonusLevel];
  buf[c] = larc_trainBonus * 0.05;
  c++;

  int trainBonusLevel20p = larc_trainBonusLevel / 4;
  if (trainBonusLevel20p > 5)trainBonusLevel20p = 5;
  buf[c + trainBonusLevel20p] = 1.0;
  c += 6;

  buf[c] = larc_shixingPt * 0.002;
  c++;

  int shixingPt100 = larc_shixingPt / 100;
  if (shixingPt100 > 10)shixingPt100 = 10;
  buf[c + shixingPt100] = 1.0;
  c += 11;


  for (int i = 0; i < 10; i++)
  {
    int lv = larc_levels[i];
    assert(lv <= 3);
    if (lv > 0)
      buf[c + lv - 1] = 1.0;
    c += 3;
  }

  if (larc_isSSS)
    buf[c] = 1.0;
  c++;

  buf[c] = larc_ssWin * 0.03;
  c++;


  int ssWinSinceLastSSS = larc_ssWinSinceLastSSS;
  if (ssWinSinceLastSSS > 8)ssWinSinceLastSSS = 8;
  buf[c + ssWinSinceLastSSS] = 1.0;
  c += 9;


  for (int i = 0; i < 9; i++)
  {
    if (larc_allowedDebuffsFirstLarc[i])
      buf[c] = 1.0;
    c++;
  }

  assert(larc_zuoyueType >= 0 && larc_zuoyueType <= 2);
  buf[c + larc_zuoyueType] = 1.0;
  c += 3;


  buf[c] = larc_zuoyueVitalBonus - 1.0;
  c++;
  buf[c] = larc_zuoyueStatusBonus - 1.0;
  c++;


  if (larc_zuoyueFirstClick)
    buf[c] = 1.0;
  c++;
  if (larc_zuoyueOutgoingUnlocked)
    buf[c] = 1.0;
  c++;
  if (larc_zuoyueOutgoingRefused)
    buf[c] = 1.0;
  c++;

  assert(larc_zuoyueOutgoingUsed <= 5 && larc_zuoyueOutgoingUsed >= 0);
  buf[c + larc_zuoyueOutgoingUsed] = 1.0;
  c += 6;

  // stageInTurn
  // personDistribution
  assert(larc_ssPersonsCount <= 5 && larc_ssPersonsCount >= 0);
  buf[c + larc_ssPersonsCount] = 1.0;
  c += 6;
  //larc_ssPersons
  //larc_ssPersonsCountLastTurn

  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 6; j++)
    {
      buf[c] = trainValue[i][j] * 0.02;
      c++;
    }
    buf[c] = trainValue[i][6] * 0.05;
    c++;
  }
  for (int i = 0; i < 5; i++)
  {
    buf[c] = failRate[i] * 0.02;
    c++;
  }
  for (int i = 0; i < 5; i++)
  {
    int t = trainShiningNum[i];
    if (t > 2)t = 2; //双彩以上都是直接充满
    buf[c + t] = 1.0;
    c += 3;
  }
  //larc_staticBonus
  for (int i = 0; i < 5; i++)
  {
    buf[c] = larc_shixingPtGainAbroad[i] * 0.01;
    c++;
  }
  //larc_trainBonus
  //larc_ssValue
  //larc_ssFailRate

  c += 9;//reserve

  assert(c == NNINPUT_CHANNELS_GAMEGLOBAL_V1 + NNINPUT_CHANNELS_SEARCHPARAM_V1);

  float* cardBuf = buf + NNINPUT_CHANNELS_GAMEGLOBAL_V1 + NNINPUT_CHANNELS_SEARCHPARAM_V1;
  float* headBuf = buf + NNINPUT_CHANNELS_GAMEGLOBAL_V1 + NNINPUT_CHANNELS_SEARCHPARAM_V1 + 7 * NNINPUT_CHANNELS_CARD_V1;


  //(全局信息)(支援卡1参数)...(支援卡6参数)(佐岳卡参数)(支援卡人头1)...(支援卡人头6)(npc1)...(npc10)(理事长)(记者)(有卡佐岳)(无卡佐岳)
  //如果带了佐岳，则 支援卡6、支援卡人头6 为空
  //如果没带佐岳，则 佐岳卡参数、npc10为空
  if (larc_zuoyueType == 0)
  {
    assert(normalCardCount == 6);
    //支援卡
    for (int i = 0; i < normalCardCount; i++)
    {
      const Person& p = persons[i];
      cardParam[p.cardIdInGame].getNNInputV1(cardBuf + NNINPUT_CHANNELS_CARD_V1 * i, *this);
      p.getNNInputV1(headBuf + NNINPUT_CHANNELS_PERSON_V1 * i, *this, i);
    }
    //npc
    for (int i = 0; i < 9; i++)
    {
      const Person& p = persons[6 + i];
      p.getNNInputV1(headBuf + NNINPUT_CHANNELS_PERSON_V1 * (6 + i), *this, 6 + i);
    }
    //理事长记者
    for (int i = 0; i < 2; i++)
    {
      const Person& p = persons[15 + i];
      p.getNNInputV1(headBuf + NNINPUT_CHANNELS_PERSON_V1 * (16 + i), *this, 15 + i);
    }
    //佐岳
    persons[17].getNNInputV1(headBuf + NNINPUT_CHANNELS_PERSON_V1 * 19, *this, 17);
  }
  else 
  {
    assert(normalCardCount == 5);
    //支援卡
    for (int i = 0; i < normalCardCount; i++)
    {
      const Person& p = persons[i];
      cardParam[p.cardIdInGame].getNNInputV1(cardBuf + NNINPUT_CHANNELS_CARD_V1 * i, *this);
      p.getNNInputV1(headBuf + NNINPUT_CHANNELS_PERSON_V1 * i, *this, i);
    }
    //npc
    for (int i = 0; i < 10; i++)
    {
      const Person& p = persons[5 + i];
      p.getNNInputV1(headBuf + NNINPUT_CHANNELS_PERSON_V1 * (6 + i), *this, 5 + i);
    }
    //理事长记者
    for (int i = 0; i < 2; i++)
    {
      const Person& p = persons[15 + i];
      p.getNNInputV1(headBuf + NNINPUT_CHANNELS_PERSON_V1 * (16 + i), *this, 15 + i);
    }
    //佐岳
    cardParam[persons[17].cardIdInGame].getNNInputV1(cardBuf + NNINPUT_CHANNELS_CARD_V1 * 6, *this);
    persons[17].getNNInputV1(headBuf + NNINPUT_CHANNELS_PERSON_V1 * 18, *this, 17);
  }

}