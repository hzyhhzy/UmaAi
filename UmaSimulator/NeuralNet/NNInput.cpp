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
      bufUniqueType[19] = 1.0;
    }
    else if (uniqueEffectParam[1] == 2 && uniqueEffectParam[4] == 3)//3个加速度技能
    {
      bufUniqueType[22] = 1.0;
    }
    else if (uniqueEffectParam[1] == 3 && uniqueEffectParam[4] == 3)//3个回体技能
    {
      bufUniqueType[20] = 1.0;
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
  else if (uniqueEffectType == 18)
  {
    bufUniqueType[18] = 1.0;
  }
  else if (uniqueEffectType == 21)
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
  else
  {
    assert(false && "todo");
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

  assert(false && "todo");
  assert(personType >= 1 && personType <= 6);
  buf[74 + personType] = 1.0;
  //total 81

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
  assert(false && "todo");
  //buf[c] = 0.0 * (log(param.samplingNum + 1.0) - 7.0);
  //c++;

  assert(c == NNINPUT_CHANNELS_SEARCHPARAM_V1);


  assert(false && "todo");
  //isLegal
  assert(false && "todo");





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
  //if (isQieZhe)
  //  buf[c] = 1.0;
  c++;
  if (isAiJiao)
    buf[c] = 1.0;
  c++;

  //练习上手
  if (failureRateBias < 0)
    buf[c] = failureRateBias * 0.5;
  //练习下手放在输入通道的最后了
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



  assert(false && "todo  trainlevel");

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

  //练习下手
  if (failureRateBias > 0)
    buf[c] = failureRateBias * 0.5;
  //练习下手放在输入通道的最后了
  c++;

  c += 8;//reserve

  assert(c == NNINPUT_CHANNELS_GAMEGLOBAL_V1 + NNINPUT_CHANNELS_SEARCHPARAM_V1);

  float* cardBuf = buf + NNINPUT_CHANNELS_GAMEGLOBAL_V1 + NNINPUT_CHANNELS_SEARCHPARAM_V1;
  float* headBuf = buf + NNINPUT_CHANNELS_GAMEGLOBAL_V1 + NNINPUT_CHANNELS_SEARCHPARAM_V1 + 7 * NNINPUT_CHANNELS_CARD_V1;



}