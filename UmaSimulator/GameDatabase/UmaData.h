#pragma once
#include "GameConstants.h"
#include "../External/json.hpp"
#include <vector>
using json = nlohmann::json;

template <typename T>
json arrayToJson(T* arr, int len)
{
	json j;
	for (int i = 0; i < len; ++i)
		j += arr[i];
	return j;
}

template <typename T>
int jsonToArray(const json& j, T* buf, int bufSize)
{
	int count = 0;
	if (!j.is_array() || bufSize <= 0)
		throw "Must be array";

	for (auto it : j) {
		buf[count++] = it;
		if (count >= bufSize) break;
	}
	return count;
}

std::string string_To_UTF8(const std::string& str);
std::string UTF8_To_string(const std::string& str);

//马娘的参数
struct UmaData
{
  int star;//星数
  bool races[TOTAL_TURN];//哪些回合有比赛
  int fiveStatusBonus[5];//属性加成
  int fiveStatusInitial[5];//初始属性
};

// 描述自由比赛区间 [start, end]
struct FreeRaceData
{
	int startTurn;	// 开始回合
	int endTurn;	// 结束回合（包含）
	int count;		// 需要的次数

	// 生成friend from_json, to_json供json库使用
	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FreeRaceData, startTurn, endTurn, count)
};

struct BonusData
{
	int triggerTurn;	// 结算回合，从0开始算的
	std::vector<int> statusBonus;	// 属性，pt奖励（技能hint转为pt奖励）
	std::vector<int> raceTurns;	// 需要比赛的回合编号（0..TOTAL_TURN)
	std::string description; // 奖励描述

	// 必须inline
	friend void to_json(json& j, const BonusData& me)
	{
		j["triggerTurn"] = me.triggerTurn;
		j["statusBonus"] = me.statusBonus;
		j["raceTurns"] = me.raceTurns;
		j["description"] = string_To_UTF8(me.description);
	}

	friend void from_json(const json& j, BonusData& me)
	{
		std::string st;
		j.at("triggerTurn").get_to(me.triggerTurn);
		j.at("statusBonus").get_to(me.statusBonus);
		j.at("raceTurns").get_to(me.raceTurns);
		j.at("description").get_to(st);
		me.description = UTF8_To_string(st);
	}
};

struct JsonUmaData : public UmaData
{
	std::string name;
	int gameId;
	std::vector<FreeRaceData> freeRaces;	// 自由比赛区间（多个）
	std::vector<int> preferRaces;	// 倾向比赛的回合编号
	std::vector<int> preferReds;	// 倾向红女神的回合编号
	std::vector<BonusData> bonusData;	// 隐藏回合奖励列表

	// 必须inline
	friend void to_json(json& j, const JsonUmaData& me)
	{
		j["gameId"] = me.gameId;
		j["name"] = string_To_UTF8(me.name);
		j["star"] = me.star;
		j["races"] = arrayToJson(me.races, TOTAL_TURN);
		j["fiveStatusBonus"] = arrayToJson(me.fiveStatusBonus, 5);
		j["fiveStatusInitial"] = arrayToJson(me.fiveStatusInitial, 5);
		j["freeRaces"] = me.freeRaces;
		j["preferRaces"] = me.preferRaces;
		j["preferReds"] = me.preferReds;
		j["bonusData"] = me.bonusData;
	}

	friend void from_json(const json& j, JsonUmaData& me)
	{
		std::string st;
		j.at("gameId").get_to(me.gameId);
		j.at("name").get_to(st);
		me.name = UTF8_To_string(st);
		j.at("star").get_to(me.star);
		j.at("races").get_to(me.races);
		jsonToArray(j.at("fiveStatusBonus"), me.fiveStatusBonus, 5);
		jsonToArray(j.at("fiveStatusInitial"), me.fiveStatusInitial, 5);
		j.at("freeRaces").get_to(me.freeRaces);
		j.at("preferRaces").get_to(me.preferRaces);
		j.at("preferReds").get_to(me.preferReds);
		j.at("bonusData").get_to(me.bonusData);
	}
};
