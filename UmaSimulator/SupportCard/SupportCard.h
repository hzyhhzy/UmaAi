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
	bool isDBCard = false;	// 用来标记是老版本手写的还是新版本自动的
	int cardID = 0;//支援卡id
	int charaId = 0;	// 角色ID
	int cardType = 0;//支援卡类型，0速1耐2力3根4智5团队6友人
	std::string cardName; //卡片名称
	//std::vector<int> cardSkill;	// 技能列表

	bool filled = false;
	double youQingBasic = 0;//友情加成
	double ganJingBasic = 0;//干劲加成
	double xunLianBasic = 0;//训练加成
    double bonusBasic[6] = { 0 };//速耐力根智pt的加成
	int wizVitalBonusBasic = 0;//智力彩圈体力回复量
    int initialBonus[6] = { 0 };//初期速耐力根智pt的提升
	int initialJiBan = 0;//初始羁绊
	double saiHou = 0;//赛后
    int hintBonus[6] = { 0 };//为了简化，把红点的技能等效成多少属性。综合考虑技能有效率（例如高峰90%有效，除了集中力），平均性价比与折扣，种马重复给技能（假设30%）
	double hintProbIncrease = 0;//启发发生率提升
	double deYiLv = 0;//得意率
	double failRateDrop = 0; //失败率降低
	double vitalCostDrop = 0; //体力消费下降
	// 方便调用，根据卡片的等级在游戏初始阶段赋值

	int uniqueEffectType = 0; //支援卡固有类型
	std::vector<int> uniqueEffectParam; //支援卡固有特殊参数

	bool isLink = false;//是否为link卡

	// 根据游戏状态计算支援卡的“固有”
    // 大部分固有都可以在默认参数下计算
	CardTrainingEffect getCardEffect(const Game& game, bool isShining, int atTrain, int jiBan, int effectFactor, int trainingCardNum, int trainingShiningNum) const;

	void getCardParamNNInputV1(float* buf, const Game& game) const;//神经网络输入，size=NNINPUT_CHANNELS_CARD_V1

	void write_to_json(json& j, const std::string cdname, const int id) const;
	void load_from_json(json& j, int x);
};

//支援卡的训练效果
//先把卡分配到对应训练里，然后才计算CardTrainingEffect
// 因为需要互相引用所以都放在一个头文件里
class CardTrainingEffect
{
public:
    bool isFixed = false; // 为True时表示不再对本对象的固有属性进行更新
    double youQing = 0;//友情加成，没闪彩就是0
    double ganJing = 0;//干劲加成
    double xunLian = 0;//训练加成
    double bonus[6] = { 0 };//速耐力根智pt的加成
    int vitalBonus = 0;//体力回复量（主要是智彩圈）
    double failRateDrop = 0; //失败率降低
    double vitalCostDrop = 0; //体力消费下降
	/*
    int initialBonus[6] = { 0 };//初期速耐力根智pt的提升
    int initialJiBan = 0;//初始羁绊
    double saiHou = 0;//赛后
    //int hintBonus[6];//为了简化，把红点的技能等效成多少属性
    //double hintProbIncrease;//启发发生率提升
    double deYiLv = 0;//得意率
	*/

public:
    CardTrainingEffect() {}
    CardTrainingEffect(const SupportCard* sc);

    // 固有词条通用处理函数
    // key 对应 enum class UniqueEffectType
    CardTrainingEffect& apply(int key, int value);

    const std::string explain();
};
