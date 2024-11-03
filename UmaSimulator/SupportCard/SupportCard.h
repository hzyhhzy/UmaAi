#pragma once
#include "../GameDatabase/GameConstants.h"
#include "../External/json.hpp"
#include "../External/utils.h"
#include <vector>
#include <cstdlib>
#include <string>
#include <iostream>
using json = nlohmann::json;

class Game;
class CardTrainingEffect;

struct SupportCard
{
	bool isDBCard = false;	// ����������ϰ汾��д�Ļ����°汾�Զ���
	int cardID = 0;//֧Ԯ��id
	int charaId = 0;	// ��ɫID
	int cardType = 0;//֧Ԯ�����ͣ�0��1��2��3��4��5�Ŷ�6����
	std::string cardName; //��Ƭ����
	//std::vector<int> cardSkill;	// �����б�

	bool filled = false;
	double youQingBasic = 0;//����ӳ�
	double ganJingBasic = 0;//�ɾ��ӳ�
	double xunLianBasic = 0;//ѵ���ӳ�
    double bonusBasic[6] = { 0 };//����������pt�ļӳ�
	int wizVitalBonusBasic = 0;//������Ȧ�����ظ���
    int initialBonus[6] = { 0 };//��������������pt������
	int initialJiBan = 0;//��ʼ�
	double saiHou = 0;//����
  //  int hintBonus[6] = { 0 };//Ϊ�˼򻯣��Ѻ��ļ��ܵ�Ч�ɶ������ԡ��ۺϿ��Ǽ�����Ч�ʣ�����߷�90%��Ч�����˼���������ƽ���Լ۱����ۿۣ������ظ������ܣ�����30%��
  int hintLevel = 0;//���ȼ���0����û���ܣ�1����û�����ۿۣ�n�������ۿ���n+1
	double hintProbIncrease = 0;//�������������
	double deYiLv = 0;//������
	double failRateDrop = 0; //ʧ���ʽ���
	double vitalCostDrop = 0; //���������½�
	// ������ã����ݿ�Ƭ�ĵȼ�����Ϸ��ʼ�׶θ�ֵ

	int uniqueEffectType = 0; //֧Ԯ����������
	std::vector<int> uniqueEffectParam; //֧Ԯ�������������

	bool isLink = false;//�Ƿ�Ϊlink��


	int16_t eventRecoveryAmountUp = 0; //���˿��¼������ӳ�
	int16_t eventEffectUp = 0; //���˿��¼����Լӳ�

	// ������Ϸ״̬����֧Ԯ���ġ����С�
    // �󲿷ֹ��ж�������Ĭ�ϲ����¼���
	CardTrainingEffect getCardEffect(const Game& game, bool isShining, int atTrain, int jiBan, int effectFactor, int trainingCardNum, int trainingShiningNum) const;

	void getCardParamNNInputV1(float* buf, const Game& game) const;//���������룬size=NNINPUT_CHANNELS_CARD_V1

	void write_to_json(json& j, const std::string cdname, const int id) const;
	void load_from_json(json& j, int x);
};

//֧Ԯ����ѵ��Ч��
//�Ȱѿ����䵽��Ӧѵ���Ȼ��ż���CardTrainingEffect
// ��Ϊ��Ҫ�����������Զ�����һ��ͷ�ļ���
class CardTrainingEffect
{
public:
    bool isFixed = false; // ΪTrueʱ��ʾ���ٶԱ�����Ĺ������Խ��и���
    double youQing = 0;//����ӳɣ�û���ʾ���0
    double ganJing = 0;//�ɾ��ӳ�
    double xunLian = 0;//ѵ���ӳ�
    double bonus[6] = { 0 };//����������pt�ļӳ�
    int vitalBonus = 0;//�����ظ�������Ҫ���ǲ�Ȧ��
    double failRateDrop = 0; //ʧ���ʽ���
    double vitalCostDrop = 0; //���������½�
	/*
    int initialBonus[6] = { 0 };//��������������pt������
    int initialJiBan = 0;//��ʼ�
    double saiHou = 0;//����
    //int hintBonus[6];//Ϊ�˼򻯣��Ѻ��ļ��ܵ�Ч�ɶ�������
    //double hintProbIncrease;//�������������
    double deYiLv = 0;//������
	*/

public:
    CardTrainingEffect() {}
    CardTrainingEffect(const SupportCard* sc);

    // ���д���ͨ�ô������
    // key ��Ӧ enum class UniqueEffectType
    CardTrainingEffect& apply(int key, int value);

    const std::string explain();
};
