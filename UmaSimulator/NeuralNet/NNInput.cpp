#include <iostream>
#include <cassert>
#include"NNInput.h"
#include "../Game/Game.h"
using namespace std;

void Game::getNNInputV1(float* buf, float targetScore, int mode) const
{
  for (int i = 0; i < NNINPUT_CHANNELS_V1; i++)buf[i] = 0.0;
  if (isEnd())return;
  //stageInTurn=0是还没分配卡组，让神经网络估计一下达标概率（value）。stageInTurn=1是分配了卡组等待玩家选择，让神经网络估计每个选项是最优解的概率（policy）
  assert((stageInTurn == 0 && mode == 0) || (stageInTurn == 1 && mode == 1));
  
  //0~77 回合数
  assert(turn >= 0 && turn < TOTAL_TURN);
  buf[turn] = 1.0;

  //78 体力
  buf[78] = (vital-50.0) / 30.0;

  //79 体力上限
  buf[79] = (maxVital-100.0) / 10.0;

  //80 切者
  buf[80] = isQieZhe;

  //81 爱娇
  buf[81] = isAiJiao;

  //82~83 练习上手和练习下手
  if (failureRateBias > 0)
    buf[82] = failureRateBias / 2.0;
  else if (failureRateBias < 0)
    buf[83] = - failureRateBias / 2.0;

  //84~88 五维属性
  for (int i = 0; i < 5; i++)
    buf[84 + i] = fiveStatus[i] / 1000.0;

  //89~93 五维属性上限
  for (int i = 0; i < 5; i++)
    buf[89 + i] = (fiveStatusLimit[i]-GameConstants::BasicFiveStatusLimit[i]) / 200.0;

  //94 技能点
  // 直接换算到目标分数里了，所以不需要输入了。
  //buf[94] = skillPt / 2000.0;

  //95~99 干劲
  buf[95 - 1 + motivation] = 1.0;

  //v1版本，cardId不作为输入

  //100~105 羁绊
  for (int i = 0; i < 6; i++)
    buf[100 + i] = cardJiBan[i] / 100.0;

  //106~111 羁绊是否不低于80
  for (int i = 0; i < 6; i++)
    buf[106 + i] = cardJiBan[i] >= 80;

  //112~116 训练等级计数
  for (int i = 0; i < 5; i++)
    buf[112 + i] = trainLevelCount[i] / 48.0;

  //117~146 训练等级（5x6）
  for (int i = 0; i < 5; i++)
    buf[117 + i * 6 + getTrainingLevel(i)] = 1.0;

  //147~151 种马蓝因子
  for (int i = 0; i < 5; i++)
    buf[147 + i] = zhongMaBlueCount[i] / 9.0;

  //152~157 种马单次继承收益
  for (int i = 0; i < 5; i++)
    buf[152 + i] = zhongMaExtraBonus[i] / 30.0;
  buf[157] = zhongMaExtraBonus[5] / 200.0;

  //158 是否为比赛
  if (isRacing)
    buf[158] = 1.0;

  //159~176 女神等级
  buf[159 + venusLevelRed] = 1.0;
  buf[165 + venusLevelBlue] = 1.0;
  buf[171 + venusLevelYellow] = 1.0;

  //177~302 已有碎片
  //碎片：每个碎片用6通道表示属性，3通道表示颜色
  //一共(8+4+2)*9=126通道

  //177~248 底层碎片
  for (int i = 0; i < 8; i++)
  {
    int s = venusSpiritsBottom[i];
    if (s == 0)continue;
    int channel0 = 177 + i * 9;
    int type = s % 8 - 1;
    assert(type >= 0 && type < 6);
    int color = s / 8;
    assert(color >= 0 && color < 3);

    buf[channel0 + type] = 1.0;
    buf[channel0 + 6 + color] = 1.0;
  }

  //249~302 上层碎片
  for (int i = 0; i < 6; i++)
  {
    int s = venusSpiritsUpper[i];
    if (s == 0)continue;
    int channel0 = 249 + i * 9;
    int type = s % 8 - 1;
    assert(type >= 0 && type < 6);
    int color = s / 8;
    assert(color >= 0 && color < 3);

    buf[channel0 + type] = 1.0;
    buf[channel0 + 6 + color] = 1.0;
  }

  //303~305 可用的女神睿智
  if (venusAvailableWisdom != 0)
    buf[303 - 1 + venusAvailableWisdom] = 1.0;

  //306 女神睿智是否开启
  buf[306] = venusIsWisdomActive;

  //307 是否点击过神团
  buf[307] = venusCardFirstClick;

  //308 神团是否解锁外出
  buf[308] = venusCardUnlockOutgoing;

  //309 神团是否情热
  buf[309] = venusCardIsQingRe;

  //310~320 神团情热了几个回合
  buf[310 + venusCardQingReContinuousTurns] = 1.0;

  //321~325 女神外出用了哪几个
  for (int i = 0; i < 5; i++)
    buf[321 + i] = venusCardOutgoingUsed[i];

  //326~330 stageInTurn(暂时无用)
  assert(stageInTurn >= 0 && stageInTurn < 5);
  buf[326 + stageInTurn] = 1.0;

  if (mode == 1)
  {
    //331~370 支援卡分布
    for (int i = 0; i < 5; i++)
      for (int j = 0; j < 8; j++)
        buf[331 + 8 * i + j] = cardDistribution[i][j];

    //371~376 支援卡红点
    for (int i = 0; i < 6; i++)
      buf[371 + i] = cardHint[i];
  }

  //377~456 训练碎片
  //每个碎片用6通道表示属性，3通道表示颜色，1通道表示是否双倍
  //一共8*10=80通道
  for (int i = 0; i < 8; i++)
  {
    int s = venusSpiritsUpper[i];
    if (s == 0)continue;
    bool doubled = s >= 32;
    s = s % 32;
    int channel0 = 377 + i * 10;
    int type = s % 8 - 1;
    assert(type >= 0 && type < 6);
    int color = s / 8;
    assert(color >= 0 && color < 3);

    buf[channel0 + type] = 1.0;
    buf[channel0 + 6 + color] = 1.0;
    buf[channel0 + 9] = doubled;
  }

  //457~462 碎片加成
  for (int i = 0; i < 6; i++)
    buf[457 + i] = spiritBonus[i] / 10.0;

  if (mode == 1)
  {
    //463~492 训练属性
    for (int i = 0; i < 5; i++)
      for (int j = 0; j < 6; j++)
        buf[463 + 6 * i + j] = trainValue[i][j] / 50.0;

    //493~497 训练体力
    for (int i = 0; i < 5; i++)
      buf[493 + i] = trainValue[i][6] / 20.0;

    //498~502 训练失败率
    for (int i = 0; i < 5; i++)
      buf[498 + i] = failRate[i] / 30.0;

    //503~507 训练失败率>0
    for (int i = 0; i < 5; i++)
      buf[503 + i] = failRate[i] > 0;

    //508~512 训练失败率>=20
    for (int i = 0; i < 5; i++)
      buf[508 + i] = failRate[i] >= 20;
  }

  //513~599 马娘id
  static_assert(GameDatabase::ALL_UMA_NUM <= 599 - 513, "超过v1版神经网络支持马娘数上限");
  assert(umaId >= 0 && umaId + 513 <= 599);
  buf[513 + umaId] = 1.0;

  //600~899 支援卡id，若i号位的支援卡为j，则buf[600+j*6+i]=1
  assert(GameDatabase::ALL_SUPPORTCARD_NUM <= 50, "超过v1版神经网络支持支援卡数上限");
  for (int i = 0; i < 6; i++)
    buf[600 + cardId[i] * 6 + i] = 1.0;
  
  //900 目标分数
  float remainScoreExceptPt = targetScore - getSkillScore();
  buf[900] = (remainScoreExceptPt - 25000) / 2000.0;

  //901~902 模式
  assert(901 + mode < NNINPUT_CHANNELS_V1);
  buf[901 + mode] = 1.0;














}