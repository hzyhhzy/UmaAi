#pragma once
#include <cstdint>
#include <random>
#include <string>
#include "../SupportCard/SupportCard.h"
struct Game;


enum PersonTypeEnum :int8_t
{
  PersonType_unknown = 0,
  PersonType_scenarioCard,
  PersonType_card,
  PersonType_npc,
  PersonType_yayoi,
  PersonType_reporter,
  PersonType_otherFriend,
  PersonType_groupCard
};

enum FriendStageEnum :int8_t
{
  FriendStage_notClicked = 0,
  FriendStage_beforeUnlockOutgoing,
  FriendStage_afterUnlockOutgoing,
  FriendStage_refusedOutgoing
};

struct Person //�κ�һ�����ܳ�����ѵ�������ͷ
{
  //bool isCard;//�Ƿ�Ϊ֧Ԯ��������Ϊ���³����߻��߲�����������   ��personType�����
  SupportCard cardParam;//֧Ԯ������
  int8_t personType;//0����δ֪��1����籾����֧Ԯ����R��SSR���У���2������֧ͨԮ�� //���������ﱭai������person�����ˣ�3����npc(uaf�籾û��)��4���³���5���ߣ�6��������������7�������˿���8�����Ŷӿ�����
  int16_t charaId;//��ͷ��Ӧ������id������д����һ��0��ֻ���ڻ�����֣�

  int8_t friendship;//�
  //bool atTrain[5];//�Ƿ������ѵ���������ͨ�Ŀ�ֻ��one-hot����ȫ�գ�����ssr��������������true
  bool isHint;//�Ƿ���hint�����˿�����npc��Ϊfalse
  int8_t cardRecord;//��¼һЩ��������ʱ����ı�Ĳ�������������ֵĹ���
  //int8_t friendOrGroupCardStage;//ֻ�����˿��Ŷӿ���Ч��0��δ�����1���ѵ����δ�������У�2���ѽ������е�û���ȣ�3������״̬
  //int8_t groupCardShiningContinuousTurns;//�Ŷӿ������˼����غ��ˣ��»غϽ������ȵĸ�������йأ����ݿ����ڴ�ʦ����ai���ҵ���


  
  std::discrete_distribution<> distribution;//distribution(rand)���Ը��ݵ���������0~5���������������ſ����������������Ǹ롣ssr������������

  Person();//δ���ص���ͷ
  void setCard(int cardId);//�Ѵ���ͷ����Ϊĳ��֧Ԯ����ֻ���Ǹտ��ֵ�״̬���������Ϸ��;����Ҫ�ֶ��޸���
  void setExtraDeyilvBonus(int deyilvBonus);//����ĵ����ʼӳ�
  //void setNonCard(int pType);//�Ѵ���ͷ����Ϊ��֧Ԯ����ͷ�����³����ߵȣ���ֻ���Ǹտ��ֵ�״̬���������Ϸ��;����Ҫ�ֶ��޸���
  

  void getCardNNInputV1(float* buf, const Game& game, int index) const;//����������������������֧Ԯ��������Game����֧Ԯ���������ڶ�Ӧλ��
  std::string getPersonName() const;//�����������
  //std::string getPersonStrColored(const Game& game, int personId, int atTrain) const;//���������������ϳɴ���ɫ���ַ�������С�ڰ�������ʾ
};