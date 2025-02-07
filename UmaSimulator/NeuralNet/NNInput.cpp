#include <iostream>
#include <cassert>
#include "NNInput.h"
#include "../Search/SearchParam.h"
#include "../Game/Game.h"
using namespace std;


void SupportCard::getCardParamNNInputV1(float* buf, const Game& game) const
{
  //ÿ�ſ��ĳ�ʼ���Լӳɡ���ʼ�����ӳɲ���Ҫ���������磬ֻ����������

  for (int ch = 0; ch < NNINPUT_CHANNELS_CARD_V1; ch++)
    buf[ch] = 0;

  //0~6 cardtype
  buf[cardType] = 1.0;

  //����ӳ�䵽0~1��Χ
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

  //�Ƿ�link��link�Ĺ̶�buff����Person::getNNInputV1��д


  //����
  assert(isDBCard);
  const int BasicC = 29;
  const int UniqueTypeC = 35;//0~29ֱ�ӱ�ţ�30~34�������
  assert(uniqueEffectType < 30);
  const int UniqueEffectC = 13;
  static_assert(BasicC + UniqueTypeC + UniqueEffectC == NNINPUT_CHANNELS_CARD_V1);
  float* bufUniqueType = buf + BasicC;
  float* bufUniqueValue = bufUniqueType + UniqueTypeC;

  auto writeUniqueEffect = [&](int key, double value)
  {
    //�����ǣ�0~4���������ǣ�5pt��6���飬7�ɾ���8ѵ����9ʧ���ʣ�10�������Ѽ��٣�11������Ȧ����
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
    if (uniqueEffectParam[1] == 1 && uniqueEffectParam[4] == 5)//5���ٶȼ���
    {
      bufUniqueType[16] = 1.0;
    }
    else if (uniqueEffectParam[1] == 1 && uniqueEffectParam[4] == 3)//3���ٶȼ���
    {
      bufUniqueType[30] = 1.0;
    }
    else if (uniqueEffectParam[1] == 2 && uniqueEffectParam[4] == 3)//3�����ٶȼ���
    {
      bufUniqueType[31] = 1.0;
    }
    else if (uniqueEffectParam[1] == 3 && uniqueEffectParam[4] == 3)//3�����弼��
    {
      bufUniqueType[32] = 1.0;
    }
    else
    {
      assert(false && "nninput: δ֪�Ĺ�������֧Ԯ������");
    }
  }
  else if (uniqueEffectType == 17)
  {
    bufUniqueType[17] = 1.0;
    writeUniqueEffect(8, uniqueEffectParam[3]);
  }
  else if (uniqueEffectType == 18)//����
  {
    bufUniqueType[18] = 1.0;
    assert(false && "��ai��֧�ַǾ籾���˿�");
  }
  else if (uniqueEffectType == 19)//����
  {
    bufUniqueType[19] = 1.0;
    assert(false && "��ai��֧�ַǾ籾���˿�");
  }
  else if (uniqueEffectType == 20)//�޽�
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
        writeUniqueEffect(i + 3, cardTypeCount[i]);  // ���������� = 0-4 = CardEffect����3-7
    if (cardTypeCount[5] > 0)
      writeUniqueEffect(30, cardTypeCount[5]); // pt = 30
  }
  else if (uniqueEffectType == 21)//����
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
  else if (uniqueEffectType == 22)//���³�
  {
    bufUniqueType[22] = 1.0;
  }
  else
  {
    assert(false && "NNInput:δ֪����");
  }



}


void Person::getCardNNInputV1(float* buf, const Game& game, int index) const
{

  assert(personType >= 1 && personType <= 6 && "ֻ��֧Ԯ�����ô˺��������³����ߵ���ȫ�ֲ�����");

  for (int ch = 0; ch < NNINPUT_CHANNELS_PERSON_V1 + NNINPUT_CHANNELS_CARD_V1; ch++)
    buf[ch] = 0;

  //cardParam�������ط�����
  //PersonType����д��uaf�籾ֻ��֧Ԯ�����뵥����nninput
  //charaId����д
  //cardIdInGame����д���Ϳ���������ڶ�Ӧ�̶�λ�þ���
  buf[0] = double(friendship) / 100.0;
  buf[1] = friendship >= 80 ? 1.0 : 0.0;
  buf[2] = friendship >= 100 ? 1.0 : 0.0;
  buf[3] = isHint ? 1.0 : 0.0;
  buf[4] = 0.0;//Ԥ��
  buf[5] = 0.0;//Ԥ��
  buf[6] = 0.0;//Ԥ��
  //�����Ŷӿ���״̬��ȫ�ֱ���������
  
  //���ĸ�ѵ��
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






void NNInput_init(float * buf) { //��ʼ��NNInput
  for(int i=0;i<NNINPUT_CHANNELS_V1;++i)
    buf[i] = 0.0;
  return ;
}

void SetValue(float * buf, int &buf_ptr, float value) { //����ֵ
  buf[buf_ptr] = value;
  buf_ptr++;
  return ;
}


void Game::getNNInputV1(float* buf, const SearchParam& param) const
{
  int buf_ptr = 0; //ָ��NNInput buf��ָ��
  NNInput_init(buf); //��ʼ��NNInput
  //������������
  buf[buf_ptr] = 1.0 * log(param.maxRadicalFactor + 1.0); //��������
  buf_ptr += 6;

  /*** �Ϸ���actionѡ�� ***/
  for(int i=0;i<Action::MAX_ACTION_TYPE;++i)
    SetValue(buf,buf_ptr,isLegal(Action::intToAction(i))?1.0:0.0);

  /*** ����������� ***/
  //ptScoreRate
  SetValue(buf, buf_ptr, ptScoreRate - 2.0);
  SetValue(buf, buf_ptr, hintPtRate - 5.0);
  SetValue(buf, buf_ptr, (eventStrength - 20) * 0.1);

  /*** ����״̬ ***/
  // �Ƿ�Ϊlink��
  SetValue(buf,buf_ptr,isLinkUma ? 1.0 : 0.0);
  // �Ƿ�Ϊ�����غ�
  for (int i = 0; i < TOTAL_TURN; ++i)
    SetValue(buf, buf_ptr, isRacingTurn[i] ? 1.0 : 0.0);
  // ��ά���Եĳɳ���
  for (int i = 0; i < 5; ++i)
    SetValue(buf, buf_ptr, 0.1 * float(fiveStatusBonus[i]));
  //�ڼ��غ�
  assert(turn < TOTAL_TURN);
  buf[buf_ptr + turn] = 1.0;
  buf_ptr += TOTAL_TURN;
  // ��Ϸ�׶�
  SetValue(buf,buf_ptr,gameStage);
  // ����
  SetValue(buf,buf_ptr,vital*0.02);
  // ��������
  SetValue(buf,buf_ptr,(maxVital-100.0)*0.10);
  // �ɾ�
  buf[buf_ptr + motivation - 1] = 1.0;
  buf_ptr += 5;
  // ��ά����
  for(int i=0;i<5;++i) 
    SetValue(buf,buf_ptr,fiveStatus[i] * 0.002);
  // ��ά��������
  for(int i=0;i<5;++i) 
    SetValue(buf,buf_ptr,(fiveStatusLimit[i] - GameConstants::BasicFiveStatusLimit[i])*0.01);
  // ���ܷ���
  SetValue(buf,buf_ptr,getSkillScore() * 0.001);
  // ѵ���ȼ�����
  for (int i = 0; i < 5; ++i)
    SetValue(buf, buf_ptr, (trainLevelCount[i] % 4) * 0.25);
  // ��ǰѵ���ȼ�
  for (int i = 0; i < 5; ++i)
  {
    buf[buf_ptr + (trainLevelCount[i] / 4)] = 1.0;
    buf_ptr += 5;
  }

  /*** Buff״̬���� ***/
  // ʧ���ʸı���
  SetValue(buf, buf_ptr, failureRateBias < 0 ? 1.0 * failureRateBias : 0.0); //�Ƿ���ϰ����
  SetValue(buf, buf_ptr, failureRateBias > 0 ? 1.0 * failureRateBias : 0.0); //�Ƿ���ϰ����
  
  // �Ƿ�����
  SetValue(buf,buf_ptr,isQieZhe?1.0:0.0);
  // �Ƿ񰮽�
  SetValue(buf,buf_ptr,isAiJiao?1.0:0.0);
  // �Ƿ����˼��
  SetValue(buf,buf_ptr,isPositiveThinking?1.0:0.0);
  // �Ƿ��������
  SetValue(buf,buf_ptr,isRefreshMind?1.0:0.0);

  /*** ����״̬���� ***/
  // ������ɫ����
  for(int i=0;i<5;++i) 
    SetValue(buf,buf_ptr,zhongMaBlueCount[i]*0.1);
  // ����������Լӳ�
  for(int i=0;i<5;++i) 
    SetValue(buf,buf_ptr,zhongMaExtraBonus[i]*0.03);
  // �������pt�ӳ�
  SetValue(buf,buf_ptr,zhongMaExtraBonus[5]*0.01);

  /*** ������� ***/
  // ����ӳ�
  SetValue(buf,buf_ptr,saihou*0.03);
  // �Ƿ��ڱ���
  SetValue(buf,buf_ptr,isRacing?1.0:0.0);


  //buf[buf_ptr + friend_type] = 1.0;
  //buf_ptr += 3;

  /*** ����ֲ� ***/
  // ���³���λ��
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
	  
  // ������λ��
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
  
  // ÿ��ѵ��ʣ��Ŀ�λ
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
  

  /*** �籾��� ***/
  // ��ԭ������
  for (int i = 0; i < 5; ++i)
  {
    int d = cook_material[i];
    SetValue(buf, buf_ptr, d * 0.01); // ������Χ��[0,999]
    SetValue(buf, buf_ptr, d >= 25 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 40 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 50 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 80 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 150 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 250 ? 1.0 : 0.0);
  }
  // ����pt
  // ѵ���ȼ�֮��Ķ���ȡ���ڳԲ�ǰ��pt����������cook_dish_pt_turn_begin������cook_dish_pt
  {
    int d = cook_dish_pt_turn_begin > 12000 ? 12000 : cook_dish_pt;
    SetValue(buf, buf_ptr, d * 0.0002); // ����pt��Χ��[0,12000]
    SetValue(buf, buf_ptr, d >= 2000 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 7000 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 12000 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 1500 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 2500 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 5000 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 10000 ? 1.0 : 0.0);
    SetValue(buf, buf_ptr, d >= 12000 ? 1.0 : 0.001 * (d % 1500));
  }
  // �غϿ�ʼǰ������pt
  //SetValue(buf,buf_ptr,*0.0002); // ����pt��Χ��[0,50000]
  // ũ��ȼ�
  for(int i=0;i<5;++i)
    buf[buf_ptr+i*5+cook_farm_level[i]] = 1.0;
  buf_ptr += 25;
  // ũ������pt
  SetValue(buf, buf_ptr, cook_farm_pt * 0.002); // ũ������pt��Χ��[0,1000]
  SetValue(buf, buf_ptr, cook_farm_pt >= 100 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 180 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 220 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 250 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 360 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 440 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 540 ? 1.0 : 0.0);
  SetValue(buf, buf_ptr, cook_farm_pt >= 660 ? 1.0 : 0.0);
  // �Ƿ��ɹ�
  SetValue(buf,buf_ptr,cook_dish_sure_success?1.0:0.0);
  // ��ǰ��
  buf[buf_ptr+cook_dish] = 1.0;
  buf_ptr += 14;
  // ��ʳ����ʷ
  for(int i=0;i<5;++i){
    buf[buf_ptr+cook_win_history[i]] = 1.0;
    buf_ptr += 3;
  }
  // �ջ���ʷ
  for(int i=0;i<4;++i){
    int vege_type = cook_harvest_history[i]==-1?5:cook_harvest_history[i];
    buf[buf_ptr+cook_harvest_history[i]] = 1.0;
    buf_ptr += 6; // 0~4��ʾ��Ӧ�Ĳˣ�5��ʾδѡ��
  }
  // �Ƿ��̲�
  for(int i=0;i<4;++i)
    SetValue(buf,buf_ptr,cook_harvest_green_history[i]?1.0:0.0);
  // ��Ʒ�Ķ����ջ�
  for(int i=0;i<5;++i)
    SetValue(buf,buf_ptr,cook_harvest_extra[i]*0.005); // �ջ���ⷶΧ��[0,160]
  // ����Action��ò˵�����
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
  // �����غϵĲ�����
  if(isRacing)
    buf[buf_ptr+cook_main_race_material_type] = 1.0;
  buf_ptr += 5;
  
  /*** �籾���˿� ***/
  // ���˿���״̬
  buf[buf_ptr+friend_type] = 1.0;
  buf_ptr += 3;
  // �������˿�ʱ�����˿������״̬
  if(friend_type != 0) {
    // ���˿��ı��
    //buf[buf_ptr+friend_personId] = 1.0;
    //buf_ptr += 6;
    // ���˿���״̬
    buf[buf_ptr+friend_stage] = 1.0;
    buf_ptr += 3;
    // ���˿����еĴ���
    buf[buf_ptr+friend_outgoingUsed] = 1.0;
    buf_ptr += 6;
    // ���˿��������ָ�����
    SetValue(buf,buf_ptr,friend_vitalBonus - 1.0);
    // ���˿����¼�Ч������
    SetValue(buf,buf_ptr,friend_statusBonus - 1.0);
  }
  else{
    buf_ptr += 11;
  }

  /*** ѵ����ص���Ϣ ***/
  // ѵ������ֵ������
  if (!isRacing) {
      for (int i = 0; i < 5; ++i) {
          for (int j = 0; j < 6; ++j) {
              SetValue(buf, buf_ptr, trainValue[i][j] * 0.01);
          }
          SetValue(buf, buf_ptr, trainVitalChange[i] * 0.05);
      }

      // ѵ��ʧ����
      for (int i = 0; i < 5; ++i)
        SetValue(buf, buf_ptr, failRate[i] * 0.01);

      // ѵ���Ƿ�����
      for (int i = 0; i < 5; ++i)
        SetValue(buf, buf_ptr, isTrainShining[i] ? 1.0 : 0.0);
  }
  else {
	  buf_ptr += 45;
  }
  // ʹ������Ĵ�ɹ���
  SetValue(buf,buf_ptr,cook_dishpt_success_rate*0.01);
  // ����ptѵ���ӳ�
  SetValue(buf,buf_ptr,cook_dishpt_training_bonus*0.01);
  // ����pt���ܵ�ӳ�
  SetValue(buf,buf_ptr,cook_dishpt_skillpt_bonus*0.01);
  // ����pt�����ʼӳ�
  SetValue(buf,buf_ptr,cook_dishpt_deyilv_bonus*0.05);
  // ����Action��õĶ���˸���
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