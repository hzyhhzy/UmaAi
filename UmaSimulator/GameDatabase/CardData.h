#pragma once
#include "GameConstants.h"
#include "../SupportCard/CardEffect.h"
#include "../External/json.hpp"
#include "../External/utils.h"
#include <vector>
#include <cstdlib>
#include <iostream>
using json = nlohmann::json;

class Game;

struct CardValue {
	//去除掉了basic字段
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
};
struct SkillList {
	int skillNum;
	std::vector<int> skillIdList; // 卡片拥有的技能编号
};
struct SupportCard
{
	int cardID;//支援卡id，部分卡具有比较复杂的固有，根据id来辨别
	int cardType;//支援卡类型，0速1耐2力3根4智5友人或团队
	std::string cardName; //卡片名称

	CardValue level[5];
	// 各个突破等级的数据

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

	SkillList cardSkill;
	//卡片拥有的技能列表
	
	int effectFactor; // 作为特殊固有处理的参数
	CardTrainingEffect getCardEffect(const Game& game, int atTrain, int jiBan, int effecFactor) const;//根据游戏状态计算支援卡的“固有”

	void cardValueInit(int x) {
		while (x <= 4 && level[x].filled == false)
			x++;
		if (x > 4) {
			std::cout << "未知卡片信息，或卡片内容录入错误: " << cardName << '\n';
			exit(0);
		}

		youQingBasic = level[x].youQingBasic;
		ganJingBasic = level[x].ganJingBasic;
		xunLianBasic = level[x].xunLianBasic;
		for (int i = 0; i < 6; ++i)
			bonusBasic[i] = level[x].bonusBasic[i];
		wizVitalBonusBasic = level[x].wizVitalBonusBasic;
		for (int i = 0; i < 6; ++i)
			initialBonus[i] = level[x].initialBonus[i];
		initialJiBan = level[x].initialJiBan;
		saiHou = level[x].saiHou;
		for (int i = 0; i < 6; ++i)
			hintBonus[i] = level[x].hintBonus[i];
		hintProbIncrease = level[x].hintProbIncrease;
		deYiLv = level[x].deYiLv;
		failRateDrop = level[x].failRateDrop;
		vitalCostDrop = level[x].vitalCostDrop;

		effectFactor = 0;

		return ;

	}

	void write_to_json(json& j,const std::string cdname,const int id)
	{
		j["cardId"] = id;
		j["cardType"] = cardType;

		j["cardName"] = string_To_UTF8(cdname);

		j["cardValue"][0]["filled"] = false;
		j["cardValue"][1]["filled"] = false;
		j["cardValue"][2]["filled"] = false;
		j["cardValue"][3]["filled"] = false;

		for (int x = 0; x < 5; ++x) {
			if (j["cardValue"][x]["filled"] == false) {
				j["cardValue"][x]["filled"] = false;
				continue;
			}
			j["cardValue"][x]["filled"] = true;
			j["cardValue"][x]["youQing"] = level[x].youQingBasic;
			j["cardValue"][x]["ganJing"] = level[x].ganJingBasic;
			j["cardValue"][x]["xunLian"] = level[x].xunLianBasic;
			j["cardValue"][x]["bonus"] = level[x].bonusBasic;
			j["cardValue"][x]["wizVitalBonus"] = level[x].wizVitalBonusBasic;
			j["cardValue"][x]["initialBonus"] = level[x].initialBonus;
			j["cardValue"][x]["initialJiBan"] = level[x].initialJiBan;
			j["cardValue"][x]["saiHou"] = level[x].saiHou;
			j["cardValue"][x]["hintBonus"] = level[x].hintBonus;
			j["cardValue"][x]["hintProbIncrease"] = level[x].hintProbIncrease;
			j["cardValue"][x]["deYiLv"] = level[x].deYiLv;
			j["cardValue"][x]["failRateDrop"] = level[x].failRateDrop;
			j["cardValue"][x]["vitalCostDrop"] = level[x].vitalCostDrop;
		}


		


		j["cardSkill"]["skillNum"] = 0;
		j["cardSkill"]["SkillList"] = NULL;
		
	}

	void load_from_json(json& j) {

		j.at("cardId").get_to(cardID);
		j.at("cardType").get_to(cardType);
		std::string st;
		j.at("cardName").get_to(st);
		cardName = UTF8_To_string(st);


		for (int x = 0; x < 5; ++x) {

			j["cardValue"][x].at("filled").get_to(level[x].filled);
			if (level[x].filled == false) continue;

			j["cardValue"][x].at("youQing").get_to(level[x].youQingBasic);
			j["cardValue"][x].at("ganJing").get_to(level[x].ganJingBasic);
			j["cardValue"][x].at("xunLian").get_to(level[x].xunLianBasic);
			j["cardValue"][x].at("bonus").get_to(level[x].bonusBasic);
			j["cardValue"][x].at("wizVitalBonus").get_to(level[x].wizVitalBonusBasic);
			j["cardValue"][x].at("initialBonus").get_to(level[x].initialBonus);
			j["cardValue"][x].at("initialJiBan").get_to(level[x].initialJiBan);
			j["cardValue"][x].at("saiHou").get_to(level[x].saiHou);
			j["cardValue"][x].at("hintBonus").get_to(level[x].hintBonus);
			j["cardValue"][x].at("hintProbIncrease").get_to(level[x].hintProbIncrease);
			j["cardValue"][x].at("deYiLv").get_to(level[x].deYiLv);
			j["cardValue"][x].at("failRateDrop").get_to(level[x].failRateDrop);
			j["cardValue"][x].at("vitalCostDrop").get_to(level[x].vitalCostDrop);
		}

		cardSkill.skillNum = j["cardSkill"]["skillNum"];

		cardSkill.skillIdList.resize(cardSkill.skillNum);
		for (int i = 0; i < cardSkill.skillNum; ++i) {
			cardSkill.skillIdList[i] = j["cardSkill"]["skillList"][i];
		}

		return;
	}
};