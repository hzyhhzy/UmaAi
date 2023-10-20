#pragma once
#include "GameConstants.h"
#include "../SupportCard/CardEffect.h"
#include "../External/json.hpp"
#include "../External/utils.h"
#include <vector>
#include <cstdlib>
#include <string>
#include <iostream>
using json = nlohmann::json;

class Game;

struct SkillList {
	int skillNum;
	std::vector<int> skillIdList; // ��Ƭӵ�еļ��ܱ��
};
struct SupportCard
{
	int cardID;//֧Ԯ��id�����ֿ����бȽϸ��ӵĹ��У�����id�����
	int cardType;//֧Ԯ�����ͣ�0��1��2��3��4��5�Ŷ�6����
	std::string cardName; //��Ƭ����

	bool filled;
	double youQingBasic;//����ӳ�
	double ganJingBasic;//�ɾ��ӳ�
	double xunLianBasic;//ѵ���ӳ�
	double bonusBasic[6];//����������pt�ļӳ�
	int wizVitalBonusBasic;//������Ȧ�����ظ���
	int initialBonus[6];//��������������pt������
	int initialJiBan;//��ʼ�
	double saiHou;//����
	int hintBonus[6];//Ϊ�˼򻯣��Ѻ��ļ��ܵ�Ч�ɶ������ԡ��ۺϿ��Ǽ�����Ч�ʣ�����߷�90%��Ч�����˼���������ƽ���Լ۱����ۿۣ������ظ������ܣ�����30%��
	double hintProbIncrease;//��������������
	double deYiLv;//������
	double failRateDrop; //ʧ���ʽ���
	double vitalCostDrop; //���������½�
	// ������ã����ݿ�Ƭ�ĵȼ�����Ϸ��ʼ�׶θ�ֵ

	bool larc_isLink;//�Ƿ�Ϊlink��
	int larc_linkSpecialEffect;//linkЧ��

	SkillList cardSkill;
	//��Ƭӵ�еļ����б�
	
	//std::string uniqueText;
	CardTrainingEffect getCardEffect(const Game& game, int atTrain, int jiBan, int effectFactor) const;//������Ϸ״̬����֧Ԯ���ġ����С�


	void write_to_json(json& j,const std::string cdname,const int id)
	{
		j["cardId"] = id/10;
		j["cardType"] = cardType;

		j["cardName"] = string_To_UTF8(cdname);

		j["cardValue"][1]["filled"] = false;
		j["cardValue"][2]["filled"] = false;
		j["cardValue"][3]["filled"] = false;

		int x = id % 10;
		j["cardValue"][x]["filled"] = filled;
		if (filled == true) {
			j["cardValue"][x]["filled"] = true;
			j["cardValue"][x]["youQing"] = youQingBasic;
			j["cardValue"][x]["ganJing"] = ganJingBasic;
			j["cardValue"][x]["xunLian"] = xunLianBasic;
			j["cardValue"][x]["bonus"] = bonusBasic;
			j["cardValue"][x]["wizVitalBonus"] = wizVitalBonusBasic;
			j["cardValue"][x]["initialBonus"] = initialBonus;
			j["cardValue"][x]["initialJiBan"] = initialJiBan;
			j["cardValue"][x]["saiHou"] = saiHou;
			j["cardValue"][x]["hintBonus"] = hintBonus;
			j["cardValue"][x]["hintProbIncrease"] = hintProbIncrease;
			j["cardValue"][x]["deYiLv"] = deYiLv;
			j["cardValue"][x]["failRateDrop"] = failRateDrop;
			j["cardValue"][x]["vitalCostDrop"] = vitalCostDrop;
		}

		j["cardSkill"]["skillNum"] = 0;
		j["cardSkill"]["SkillList"] = NULL;

	}

	void load_from_json(json& j,int x) {

		j.at("cardId").get_to(cardID);
		cardID = cardID * 10 + x;
		j.at("cardType").get_to(cardType);
		std::string st;
		j.at("cardName").get_to(st);
		cardName = UTF8_To_string(st);

		j["cardValue"][x].at("filled").get_to(filled);
		if (filled == true) {

			youQingBasic = j["cardValue"][x].value<double>("youQing", 0);
			ganJingBasic = j["cardValue"][x].value<double>("ganJing", 0);
			xunLianBasic = j["cardValue"][x].value<double>("xunLian", 0);
			j["cardValue"][x].at("bonus").get_to(bonusBasic);
			wizVitalBonusBasic = j["cardValue"][x].value("wizVitalBonus", 0);
			j["cardValue"][x].at("initialBonus").get_to(initialBonus);
			initialJiBan = j["cardValue"][x].value("initialJiBan", 0);
			saiHou = j["cardValue"][x].value<double>("saiHou", 0);
			j["cardValue"][x].at("hintBonus").get_to(hintBonus);
			hintProbIncrease = j["cardValue"][x].value<double>("hintProbIncrease", 0);
			deYiLv = j["cardValue"][x].value<double>("deYiLv", 0);
			failRateDrop = j["cardValue"][x].value<double>("failRateDrop", 0);
			vitalCostDrop = j["cardValue"][x].value<double>("vitalCostDrop", 0);
		}

		if (j.contains("larc_isLink"))
			larc_isLink = j["larc_isLink"];
		else
			larc_isLink = false;

		if (j.contains("larc_linkSpecialEffect"))
			larc_linkSpecialEffect = j["larc_linkSpecialEffect"];
		else
			larc_linkSpecialEffect = 0;

		cardSkill.skillNum = j["cardSkill"]["skillNum"];

		cardSkill.skillIdList.resize(cardSkill.skillNum);
		for (int i = 0; i < cardSkill.skillNum; ++i) {
			cardSkill.skillIdList[i] = j["cardSkill"]["skillList"][i];
		}
		/*
		if (j["uniqueEffect"].is_array()) {
			j["uniqueEffect"].at(0).get_to(uniqueText);
			uniqueText = UTF8_To_string(uniqueText);
		}
		*/
		return;
	}
};