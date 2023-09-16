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
	std::vector<int> skillIdList; // 卡片拥有的技能编号
};
struct SupportCard
{
	int cardID;//支援卡id，部分卡具有比较复杂的固有，根据id来辨别
	int cardType;//支援卡类型，0速1耐2力3根4智5团队6友人
	std::string cardName; //卡片名称

	bool filled;
	double youQingBasic;//友情加成
	double ganJingBasic;//干劲加成
	double xunLianBasic;//训练加成
	double bonusBasic[6];//速耐力根智pt的加成
	int wizVitalBonusBasic;//智力彩圈体力回复量
	int initialBonus[6];//初期速耐力根智pt的提升
	int initialJiBan;//初始羁绊
	double saiHou;//赛后
	int hintBonus[6];//为了简化，把红点的技能等效成多少属性。综合考虑技能有效率（例如高峰90%有效，除了集中力），平均性价比与折扣，种马重复给技能（假设30%）
	double hintProbIncrease;//启发发生率提升
	double deYiLv;//得意率
	double failRateDrop; //失败率降低
	double vitalCostDrop; //体力消费下降
	// 方便调用，根据卡片的等级在游戏初始阶段赋值

	bool larc_isLink;//是否为link卡
	int larc_linkSpecialEffect;//link效果

	SkillList cardSkill;
	//卡片拥有的技能列表
	
	//std::string uniqueText;
	CardTrainingEffect getCardEffect(const Game& game, int atTrain, int jiBan, int effecFactor) const;//根据游戏状态计算支援卡的“固有”


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