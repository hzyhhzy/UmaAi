#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "../config.h"

const int TOTAL_TURN = 78;
const int MAX_INFO_PERSON_NUM = 6;//�е�����Ϣ����ͷ�������˾籾ֻ��֧Ԯ����

class GameConstants
{
public:
  //Reference��https://github.com/mee1080/umasim/blob/main/core/src/commonMain/kotlin/io/github/mee1080/umasim/scenario/mecha/MechaStore.kt
  static const int TrainingBasicValue[5][5][7]; //TrainingBasicValue[��ɫ][�ڼ���ѵ��][LV��][����������pt����]
  static const int FailRateBasic[5][5];//[�ڼ���ѵ��][LV��]��ʧ����= 0.025*(x0-x)^2 + 1.25*(x0-x)
  static const int BasicFiveStatusLimit[5];//��ʼ���ޣ�1200���Ϸ���

  //������Ϸ����
  //static const int NormalRaceFiveStatusBonus;//����������Լӳ�=3�������������⴦����Ҷ�˹�ȣ�
  //static const int NormalRacePtBonus;//�������pt�ӳ�
  static const double EventProb;//ÿ�غ���EventProb�������һ�������Լ�pt +EventStrengthDefault��ģ��֧Ԯ���¼�
  static const int EventStrengthDefault;

  //�籾�����
  static const int FriendCardYayoiSSRId = 30207;//SSR�ﴨ
  static const int FriendCardYayoiRId = 10109;//R�ﴨ
  static const int FriendCardLianghuaSSRId = 30188;//SSR����
  static const int FriendCardLianghuaRId = 10104;//R����
  static const double FriendUnlockOutgoingProbEveryTurnLowFriendship;//ÿ�غϽ�������ĸ��ʣ��С��60
  static const double FriendUnlockOutgoingProbEveryTurnHighFriendship;//ÿ�غϽ�������ĸ��ʣ����ڵ���60
  //static const double FriendEventProb;//�����¼�����//����0.4д���ڶ�Ӧ��������
  

  //�籾���
  static const std::vector<int> Mecha_LinkCharas;// Link��ɫ
  static const double Mecha_GearProb;// �޲�Ȧѵ���г��ֵĸ���
  static const double Mecha_GearProbLinkBonus;// �����׻�ø���Ļ�е���֡������Ӷ�����

  //lv������ʽ���������ݲο���
  //https://github.com/mee1080/umasim/blob/main/core/src/commonMain/kotlin/io/github/mee1080/umasim/scenario/mecha/MechaStore.kt
  //https://github.com/mee1080/umasim/blob/main/data/mecha_memo.md
  static const int Mecha_LvGainBasic[2][3][3][6]; //[�Ƿ����][�޳��֡����֡�����][������1����2][����]
  static const int Mecha_LvGainSubTrainIdx[5][3]; //Lv�����ĸ�������ʲô 


  //����
  static const int FiveStatusFinalScore[1200+800*2+1];//��ͬ���Զ�Ӧ������
  static const double ScorePtRateDefault;//Ϊ�˷��㣬ֱ����Ϊÿ1pt��Ӧ���ٷ֡�
  static const double HintLevelPtRateDefault;//Ϊ�˷��㣬ֱ����Ϊÿһ��hint����pt��
  //static const double ScorePtRateQieZhe;//Ϊ�˷��㣬ֱ����Ϊÿ1pt��Ӧ���ٷ֡�����

  static bool isLinkChara(int id);
  static bool isLinkChara_initialEN(int id);
  static bool isLinkChara_moreGear(int id);
  static bool isLinkChara_initialOverdrive(int id);
  static bool isLinkChara_lvBonus(int id);
  static bool isLinkChara_initialLv(int id);

};