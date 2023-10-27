#include <iostream>
#include "SupportCard.h"
#include "../NeuralNet/NNInput.h"
#include "../GameDatabase/GameConstants.h"
#include "../Game/Game.h"

using namespace std;

void SupportCard::write_to_json(json& j, const std::string cdname, const int id) const
{
	j["cardId"] = id / 10;
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

void SupportCard::load_from_json(json& j, int x) {

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

	//cardSkill.skillNum = j["cardSkill"]["skillNum"];

	//cardSkill.skillIdList.resize(cardSkill.skillNum);
	//for (int i = 0; i < cardSkill.skillNum; ++i) {
	//	cardSkill.skillIdList[i] = j["cardSkill"]["skillList"][i];
	//}
	/*
	if (j["uniqueEffect"].is_array()) {
		j["uniqueEffect"].at(0).get_to(uniqueText);
		uniqueText = UTF8_To_string(uniqueText);
	}
	*/
	return;
}
