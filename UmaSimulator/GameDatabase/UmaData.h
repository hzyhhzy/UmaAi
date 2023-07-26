#pragma once
#include "GameConstants.h"
#include "../External/json.hpp"
#include "../External/utils.h"
#include <vector>
#include <cstdlib>
using json = nlohmann::json;

// 回合标记位 0-普通，1-生涯比赛，2-建议比赛，4-建议红女神
// 还可以添加其他标记类型
enum TurnFlags { TURN_NORMAL = 0, TURN_RACE = 1, TURN_PREFER_RACE = 2, TURN_RED = 4 };

// 描述自由比赛区间 [start, end]
struct FreeRaceData
{
	int startTurn;	// 开始回合
	int endTurn;	// 结束回合（包含）
	int count;		// 需要的次数

	// 生成friend from_json, to_json供json库使用
	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FreeRaceData, startTurn, endTurn, count)
};

//马娘的参数
struct UmaData
{
	int star;//星数
	int gameId;
	int races[TOTAL_TURN];// 回合标记 enum TurnFlags
	int fiveStatusBonus[5];//属性加成
	int fiveStatusInitial[5];//初始属性
	std::vector<FreeRaceData> freeRaces;	// 自由比赛区间（多个）
	std::string name;

	// 必须inline
	friend void to_json(json& j, const UmaData& me)
	{
		j["gameId"] = me.gameId;
		j["name"] = string_To_UTF8(me.name);
		j["star"] = me.star;
		j["fiveStatusBonus"] = arrayToJson(me.fiveStatusBonus, 5);
		j["fiveStatusInitial"] = arrayToJson(me.fiveStatusInitial, 5);
		j["freeRaces"] = me.freeRaces;
		// dump races
		std::vector<int> races, preferRaces, preferReds;
		for (int i = 0; i < TOTAL_TURN; ++i) {
			if (me.races[i] & TURN_RACE)
				races.push_back(i);
			if (me.races[i] & TURN_PREFER_RACE)
				preferRaces.push_back(i);
			if (me.races[i] & TURN_RED)
				preferReds.push_back(i);
		}
		j["races"] = races;
		j["preferRaces"] = preferRaces;
		j["preferReds"] = preferReds;
	}

	  friend void from_json(const json& j, UmaData& me)
	  {
		  std::string st;
		  j.at("gameId").get_to(me.gameId);
		  j.at("name").get_to(st);
		  me.name = UTF8_To_string(st);
		  j.at("star").get_to(me.star);
		  jsonToArray(j.at("fiveStatusBonus"), me.fiveStatusBonus, 5);
		  jsonToArray(j.at("fiveStatusInitial"), me.fiveStatusInitial, 5);
		  j.at("freeRaces").get_to(me.freeRaces);

		  // 将Races，preferRaces和preferReds压缩进RaceFlags中
		  memset(me.races, 0, sizeof(me.races));
		  if (j["races"][0].is_boolean())
		  {
			  // 老比赛格式
			  for (int i = 0; i < j["races"].size(); ++i)
				  if ((bool)(j["races"][i]))
					  me.races[i] |= TURN_RACE;
		  }
		  else // is int
		  {
			  for (auto turn : j["races"])
				  me.races[turn] |= TURN_RACE;
		  }
		  for (auto turn : j["preferRaces"])
			  me.races[turn] |= TURN_PREFER_RACE;
		  for (auto turn : j["preferReds"])
			  me.races[turn] |= TURN_RED;
	  }
};

