#include <iostream>
#include <iomanip> 
#include <sstream>
#include <fstream>
#include <cassert>
#include <thread>  // for std::this_thread::sleep_for
#include <chrono>  // for std::chrono::seconds

#include "../Game/Game.h"
#include "../GameDatabase/GameConfig.h"
#include "../Search/Search.h"
#include "windows.h"
#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <algorithm>
using namespace std;
using json = nlohmann::json;

wchar_t buf[10240];

#define COMPARE(key) if ( a.key != b.key ) { cout << (#key) << ": 手动 = " << a.key << ", 导出值 = " << b.key << endl; ret = false; }

template <typename T>
bool compare_array(const T* a, const T* b, size_t len, const string& key)
{
	bool ret = true;
	for (int i=0; i<len; ++i)
		if (a[i] != b[i])
		{
			ret = false;
			break;
		}
	if (!ret) {
		cout << key << ": A: [";
		for (int i = 0; i < len; ++i)
			cout << a[i] << ", ";
		cout << "], B: [";
		for (int i = 0; i < len; ++i)
			cout << b[i] << ", ";
		cout << "]" << endl;
	}
	return ret;
		
}

bool compare_card_value(const SupportCard& a, const SupportCard& b)
{
	bool ret = true;
	//	CardValue level[5];	// 各个突破等级的数据
	COMPARE(youQingBasic);//友情加成
	COMPARE(ganJingBasic);//干劲加成
	COMPARE(xunLianBasic);//训练加成
	compare_array(a.bonusBasic, b.bonusBasic, 6, "bonusBasic");
	COMPARE(wizVitalBonusBasic);//智力彩圈体力回复量
	compare_array(a.initialBonus, b.initialBonus, 6, "initialBonus");
	COMPARE(initialJiBan);//初始羁绊
	COMPARE(saiHou);//赛后
  COMPARE(hintLevel);//红点等级
	COMPARE(hintProbIncrease);//启发发生率提升
	COMPARE(deYiLv);//得意率
	COMPARE(failRateDrop); //失败率降低
	COMPARE(vitalCostDrop); //体力消费下降
	return ret;
}

bool compare_card(const SupportCard& a, const SupportCard& b)
{
    bool ret = true;
    COMPARE(cardID);
	COMPARE(cardType);
	COMPARE(cardName);
	for (int i = 0; i < 5; ++i)
	{
		if (a.filled) {
			ret &= compare_card_value(a, b);
		}
	}
//	if (!ret)
//		cout << "(参考固有): " << b.uniqueText << endl;
	//SkillList cardSkill;
	return ret;
}

void test_compare_cards()
{
	for (auto c : GameDatabase::AllCards)
	{
		cout << "- 检查 " << c.second.cardName << endl;
		if (!GameDatabase::DBCards.count(c.first))
		{
			cout << "无此卡片ID：" << c.first << endl;
		}
		auto dbCard = GameDatabase::DBCards[c.first];
		compare_card(c.second, dbCard);
	}
}

void main_test_json()
{
  // 检查工作目录
  GetModuleFileNameW(0, buf, 10240);
  filesystem::path exeDir = filesystem::path(buf).parent_path();
  filesystem::current_path(exeDir);
  std::cout << "当前工作目录：" << filesystem::current_path() << endl;
  cout << "当前程序目录：" << exeDir << endl;

  GameDatabase::loadUmas("db/uma");
  GameDatabase::loadCards("db/card");
  GameDatabase::loadDBCards("db/cardDB.json");
  GameConfig::load("aiConfig.json");
  cout << "载入完成" << endl;

  // todo: 加入对马娘数据的检查
  test_compare_cards();
}