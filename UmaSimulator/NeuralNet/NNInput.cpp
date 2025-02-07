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
  if (!game.isRacing) {
      for (int tr = 0; tr < 5; tr++)
      {
          for (int i = 0; i < 5; i++)
          {
              if (game.personDistribution[tr][i] == index)
                  buf[7 + tr] = 1.0;
          }
      }
  }

  buf[7 + 5] = cardParam.isLink ? 1.0 : 0.0;

  cardParam.getCardParamNNInputV1(buf + NNINPUT_CHANNELS_PERSON_V1, game);
}






void NNInput_init(float * buf) { //初始化NNInput
  for(int i=0;i<NNINPUT_CHANNELS_V1;++i)
    buf[i] = 0.0;
  return ;
}

void SetValue(float * buf, int &buf_ptr, float value) { //设置值
  buf[buf_ptr] = value;
  buf_ptr++;
  return ;
}


void Game::getNNInputV1(float* buf, const SearchParam& param) const
{
  int buf_ptr = 0; //指向NNInput buf的指针
  NNInput_init(buf); //初始化NNInput
  //设置搜索参数
  buf[buf_ptr] = 1.0 * log(param.maxRadicalFactor + 1.0); //激进因子
  buf_ptr += 6;

  /*** 合法的action选项 ***/
  for(int i=0;i<Action::MAX_ACTION_TYPE;++i)
    SetValue(buf,buf_ptr,isLegal(Action::intToAction(i))?1.0:0.0);

  /*** 参数相关设置 ***/
  //ptScoreRate
  SetValue(buf, buf_ptr, ptScoreRate - 2.0);
  SetValue(buf, buf_ptr, hintPtRate - 5.0);
  SetValue(buf, buf_ptr, (eventStrength - 20) * 0.1);

  /*** 基本状态 ***/
  // 是否为link马
  SetValue(buf,buf_ptr,isLinkUma ? 1.0 : 0.0);
  // 是否为比赛回合
  for (int i = 0; i < TOTAL_TURN; ++i)
    SetValue(buf, buf_ptr, isRacingTurn[i] ? 1.0 : 0.0);
  // 五维属性的成长率
  for (int i = 0; i < 5; ++i)
    SetValue(buf, buf_ptr, 0.1 * float(fiveStatusBonus[i]));
  //第几回合
  assert(turn < TOTAL_TURN);
  buf[buf_ptr + turn] = 1.0;
  buf_ptr += TOTAL_TURN;
  // 游戏阶段
  SetValue(buf,buf_ptr,gameStage);
  // 体力
  SetValue(buf,buf_ptr,vital*0.02);
  // 体力上限
  SetValue(buf,buf_ptr,(maxVital-100.0)*0.10);
  // 干劲
  buf[buf_ptr + motivation - 1] = 1.0;
  buf_ptr += 5;
  // 五维属性
  for(int i=0;i<5;++i) 
    SetValue(buf,buf_ptr,fiveStatus[i] * 0.002);
  // 五维属性上限
  for(int i=0;i<5;++i) 
    SetValue(buf,buf_ptr,(fiveStatusLimit[i] - GameConstants::BasicFiveStatusLimit[i])*0.01);
  // 技能分数
  SetValue(buf,buf_ptr,getSkillScore() * 0.001);
  // 训练等级计数
  for (int i = 0; i < 5; ++i)
    SetValue(buf, buf_ptr, (trainLevelCount[i] % 4) * 0.25);
  // 当前训练等级
  for (int i = 0; i < 5; ++i)
  {
    buf[buf_ptr + (trainLevelCount[i] / 4)] = 1.0;
    buf_ptr += 5;
  }

  /*** Buff状态设置 ***/
  // 失败率改变量
  SetValue(buf, buf_ptr, failureRateBias < 0 ? 1.0 * failureRateBias : 0.0); //是否练习上手
  SetValue(buf, buf_ptr, failureRateBias > 0 ? 1.0 * failureRateBias : 0.0); //是否练习下手
  
  // 是否切者
  SetValue(buf,buf_ptr,isQieZhe?1.0:0.0);
  // 是否爱娇
  SetValue(buf,buf_ptr,isAiJiao?1.0:0.0);
  // 是否积极思考
  SetValue(buf,buf_ptr,isPositiveThinking?1.0:0.0);
  // 是否放松心情
  SetValue(buf,buf_ptr,isRefreshMind?1.0:0.0);

  /*** 种马状态设置 ***/
  // 种马蓝色数量
  for(int i=0;i<5;++i) 
    SetValue(buf,buf_ptr,zhongMaBlueCount[i]*0.1);
  // 种马额外属性加成
  for(int i=0;i<5;++i) 
    SetValue(buf,buf_ptr,zhongMaExtraBonus[i]*0.03);
  // 种马额外pt加成
  SetValue(buf,buf_ptr,zhongMaExtraBonus[5]*0.01);

  /*** 比赛相关 ***/
  // 赛后加成
  SetValue(buf,buf_ptr,saihou*0.03);
  // 是否在比赛
  SetValue(buf,buf_ptr,isRacing?1.0:0.0);


  //buf[buf_ptr + friend_type] = 1.0;
  //buf_ptr += 3;

  /*** 人物分布 ***/
  // 理事长羁绊和位置
  if (!isRacing && friend_type == 0) {
	  SetValue(buf, buf_ptr, friendship_noncard_yayoi * 0.01);
	  SetValue(buf, buf_ptr, friendship_noncard_yayoi >= 40 ? 1.0 : 0.0);
	  SetValue(buf, buf_ptr, friendship_noncard_yayoi >= 60 ? 1.0 : 0.0);
	  SetValue(buf, buf_ptr, friendship_noncard_yayoi >= 80 ? 1.0 : 0.0);
    for (int tr = 0; tr < 5; tr++)
    {
        for (int i = 0; i < 5; i++)
        {
            if (personDistribution[tr][i] == PSID_noncardYayoi)
                buf[buf_ptr + tr] = 1.0;
        }
    }
	  buf_ptr += 5;
  }
  else {
      buf_ptr += 9;
  }
	  
  // 记者羁绊和位置
  if (!isRacing) {
      SetValue(buf, buf_ptr, friendship_noncard_reporter * 0.01);
      SetValue(buf, buf_ptr, friendship_noncard_reporter >= 40 ? 1.0 : 0.0);
      SetValue(buf, buf_ptr, friendship_noncard_reporter >= 60 ? 1.0 : 0.0);
      SetValue(buf, buf_ptr, friendship_noncard_reporter >= 80 ? 1.0 : 0.0);
      for (int tr = 0; tr < 5; tr++)
      {
          for (int i = 0; i < 5; i++)
          {
              if (personDistribution[tr][i] == PSID_noncardReporter)
                  buf[buf_ptr + tr] = 1.0;
          }
      }
      buf_ptr += 5;
  }
  else 
	  buf_ptr += 9;
  
  // 每个训练剩余的空位
  if (!isRacing) {
      for (int tr = 0; tr < 5; tr++)
      {
          int count = 0;
          for (int i = 0; i < 5; i++)
          {
              if (personDistribution[tr][i] == -1)
                  count += 1;
          }
          buf[buf_ptr + count] = 1.0;
          buf_ptr += 6;
      }
  }
  else
	  buf_ptr += 30;
  

  /*** 剧本相关 ***/
  // 菜原料数量
  for (int i = 0; i < 5; ++i)
  {
    int d = cook_material[i];
    SetValue(buf, buf_ptr, d * 0.01); // 菜量范围在[0,999]
    SetValue(buf, buf_ptr, d >= 25 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 40 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 50 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 80 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 150 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 250 ? 1.0 : 0.0);
  }
  // 料理pt
  // 训练等级之类的都是取决于吃菜前的pt，所以输入cook_dish_pt_turn_begin而不是cook_dish_pt
  {
    int d = cook_dish_pt_turn_begin > 12000 ? 12000 : cook_dish_pt;
    SetValue(buf, buf_ptr, d * 0.0002); // 料理pt范围在[0,12000]
    SetValue(buf, buf_ptr, d >= 2000 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 7000 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 12000 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 1500 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 2500 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 5000 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 10000 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 12000 ? 1.0 : 0.001 * (d % 1500));
  }
  // 回合开始前的料理pt
  //SetValue(buf,buf_ptr,*0.0002); // 料理pt范围在[0,50000]
  // 农田等级
  for(int i=0;i<5;++i)
    buf[buf_ptr+i*5+cook_farm_level[i]] = 1.0;
  buf_ptr += 25;
  // 农田升级pt
  SetValue(buf, buf_ptr, cook_farm_pt * 0.002); // 农田升级pt范围在[0,1000]
  SetValue(buf, buf_ptr, cook_farm_pt >= 100 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 180 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 220 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 250 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 360 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 440 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 540 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 660 ? 1.0 : 0.0);
  // 是否大成功
  SetValue(buf,buf_ptr,cook_dish_sure_success?1.0:0.0);
  // 当前菜
  buf[buf_ptr+cook_dish] = 1.0;
  buf_ptr += 14;
  // 试食会历史
  for(int i=0;i<5;++i){
    buf[buf_ptr+cook_win_history[i]] = 1.0;
    buf_ptr += 3;
  }
  // 收获历史
  for(int i=0;i<4;++i){
    int vege_type = cook_harvest_history[i]==-1?5:cook_harvest_history[i];
    buf[buf_ptr+cook_harvest_history[i]] = 1.0;
    buf_ptr += 6; // 0~4表示对应的菜，5表示未选择
  }
  // 是否绿菜
  for(int i=0;i<4;++i)
    SetValue(buf,buf_ptr,cook_harvest_green_history[i]?1.0:0.0);
  // 菜品的额外收获
  for(int i=0;i<5;++i)
    SetValue(buf,buf_ptr,cook_harvest_extra[i]*0.005); // 收获额外范围在[0,160]
  // 各个Action获得菜的种类
  for(int i=0;i<8;++i) {
    if (isLegal(Action(i)))
    {
      buf[buf_ptr + cook_train_material_type[i]] = 1.0;
      buf_ptr += 5;
      SetValue(buf, buf_ptr, cook_train_green[i] ? 1.0 : 0.0);
    }
    else
      buf_ptr += 6;
  }
  // 比赛回合的菜种类
  if(isRacing)
    buf[buf_ptr+cook_main_race_material_type] = 1.0;
  buf_ptr += 5;
  
  /*** 剧本友人卡 ***/
  // 友人卡的状态
  buf[buf_ptr+friend_type] = 1.0;
  buf_ptr += 3;
  // 存在友人卡时，友人卡的相关状态
  if(friend_type != 0) {
    // 友人卡的编号
    //buf[buf_ptr+friend_personId] = 1.0;
    //buf_ptr += 6;
    // 友人卡的状态
    buf[buf_ptr+friend_stage] = 1.0;
    buf_ptr += 3;
    // 友人卡出行的次数
    buf[buf_ptr+friend_outgoingUsed] = 1.0;
    buf_ptr += 6;
    // 友人卡的体力恢复倍率
    SetValue(buf,buf_ptr,friend_vitalBonus - 1.0);
    // 友人卡的事件效果倍率
    SetValue(buf,buf_ptr,friend_statusBonus - 1.0);
  }
  else{
    buf_ptr += 11;
  }

  /*** 训练相关的信息 ***/
  // 训练的数值和体力
  if (!isRacing) {
      for (int i = 0; i < 5; ++i) {
          for (int j = 0; j < 6; ++j) {
              SetValue(buf, buf_ptr, trainValue[i][j] * 0.01);
          }
          SetValue(buf, buf_ptr, trainVitalChange[i] * 0.05);
      }

      // 训练失败率
      for (int i = 0; i < 5; ++i)
        SetValue(buf, buf_ptr, failRate[i] * 0.01);

      // 训练是否闪彩
      for (int i = 0; i < 5; ++i)
        SetValue(buf, buf_ptr, isTrainShining[i] ? 1.0 : 0.0);
  }
  else {
	  buf_ptr += 45;
  }
  // 使用料理的大成功率
  SetValue(buf,buf_ptr,cook_dishpt_success_rate*0.01);
  // 料理pt训练加成
  SetValue(buf,buf_ptr,cook_dishpt_training_bonus*0.01);
  // 料理pt技能点加成
  SetValue(buf,buf_ptr,cook_dishpt_skillpt_bonus*0.01);
  // 料理pt得意率加成
  SetValue(buf,buf_ptr,cook_dishpt_deyilv_bonus*0.05);
  // 各个Action获得的额外菜个数
  for (int i = 0; i < 8; ++i)
  {
    if (isLegal(Action(i)))
      SetValue(buf, buf_ptr, cook_train_material_num_extra[i] * 0.02);
    else
      buf_ptr++;
  }

  // return ;
  //cout <<"NNINPUT_CHANNELS_GAMEGLOBAL_V1 should = "<< buf_ptr << endl;
  assert(buf_ptr == NNINPUT_CHANNELS_GAMEGLOBAL_V1);

  float* cardBuf = buf + NNINPUT_CHANNELS_GAMEGLOBAL_V1;
  //float* headBuf = buf + NNINPUT_CHANNELS_GAMEGLOBAL_V1 + 7 * NNINPUT_CHANNELS_CARD_V1;

  for (int card = 0; card < 6; card++)
  {
    persons[card].getCardNNInputV1(cardBuf + NNINPUT_CHANNELS_CARDPERSON_V1 * card, *this, card);
  }
  
}