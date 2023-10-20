#pragma once
#include "../GameDatabase/GameConstants.h"
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

	int uniqueEffectType; //支援卡固有触发条件（编号待定）
	int uniqueEffectParam[10]; //支援卡固有效果，速耐力根智pt 友 训 干劲 。。。（编号待定）


	bool larc_isLink;//是否为link卡
	int larc_linkSpecialEffect;//link效果

	//SkillList cardSkill;
	//卡片拥有的技能列表

	//std::string uniqueText;
	CardTrainingEffect getCardEffect(const Game& game, int atTrain, int jiBan, int effectFactor) const;//根据游戏状态计算支援卡的“固有”

	void getNNInputV1(float* buf) const;//神经网络输入，size=NNINPUT_CHANNELS_CARD_V1

	void write_to_json(json& j, const std::string cdname, const int id) const;
	void load_from_json(json& j, int x);
};