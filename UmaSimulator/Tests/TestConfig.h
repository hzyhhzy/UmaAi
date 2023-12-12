#pragma once
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include "../GameDatabase/GameDatabase.h"
#include "../GameDatabase/GameConfig.h"
#include "../External/json.hpp"

using namespace std;
using json = nlohmann::json;

static string AttrNames[] = { "速", "耐", "力", "根", "智", "Pt" };
static string LessonNames[] = { "根", "耐", "力", "速", "智", "Pt", "体力", "友情", "厄运"};

class TestConfig
{
public:
    int umaId = -1;
    int umaStars = 5;
    vector<int> cards; // 6
    vector<int> zhongmaBlue; // 5
    vector<int> zhongmaBonus; // 6
    vector<bool> allowedDebuffs; // 9

    int totalGames = 50000;
    int eventStrength = 20;

public:
    TestConfig() {}
    TestConfig(int umaId, int umaStars, const int *cards, const int* zhongmaBlue, const int* zhongmaBonus, const bool *allowedDebuffs, const int totalGames, const int eventStrength)
        : umaId(umaId), umaStars(umaStars), totalGames(totalGames), eventStrength(eventStrength)
    {
        this->cards = vector<int>(cards, cards + 6);
        this->zhongmaBlue = vector<int>(zhongmaBlue, zhongmaBlue + 5);
        this->zhongmaBonus = vector<int>(zhongmaBonus, zhongmaBonus + 6);
        this->allowedDebuffs = vector<bool>(allowedDebuffs, allowedDebuffs + 9);
    }

    static TestConfig loadFile(const string filename)
    {
        try
        {
            ifstream ifs(filename);
            stringstream ss;
            ss << ifs.rdbuf();
            ifs.close();
            json j = json::parse(ss.str(), nullptr, true, true);
            TestConfig ret = j;
            return ret;
        }
        catch (exception& e)
        {
            cout << "读取测试配置出错: " << endl << e.what() << endl;
        }
        catch (...)
        {
            cout << "读取测试配置出错：未知错误" << endl;
        }
    }

    const string explain()
    {
        stringstream ss;
        int i;
        ss << "\033[1;36m马娘： ☆" << umaStars << GameDatabase::AllUmas[umaId].name << "\033[0m";
        ss << " 加成: ";
        for (i = 0; i < 5; ++i)
            if (GameDatabase::AllUmas[umaId].fiveStatusBonus[i] > 0)
                ss << GameDatabase::AllUmas[umaId].fiveStatusBonus[i] << AttrNames[i] << " ";
        ss << endl << "\033[33m" << "卡组：";
        for (int i : cards)
            ss << GameDatabase::AllCards[i].cardName << "+" << i % 10 << " ";
        ss << endl << "\033[32m种马：";
        for (i = 0; i < 5; ++i)
            if (zhongmaBlue[i] > 0)
                ss << zhongmaBlue[i] << AttrNames[i] << " ";
        ss << " 额外继承属性：";
        for (i = 0; i < 6; ++i)
            if (zhongmaBonus[i] > 0)
                ss << "+" << zhongmaBonus[i] << AttrNames[i] << " ";
        ss << "\033[34m" << endl << "跳过课程：";
        for (i = 0; i < 9; ++i)
            if (allowedDebuffs[i])
                ss << LessonNames[i] << " ";
        ss << ", 局数：" << totalGames << ", 事件强度：" << eventStrength << "属性/Pt"
           << "\033[0m" << endl;
        return ss.str();
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(TestConfig, umaId, umaStars, cards, zhongmaBlue, zhongmaBonus, allowedDebuffs, totalGames, eventStrength);
};

class GameResult
{
public:
    int fiveStatus[5] = { 0, 0, 0, 0, 0 };
    int fiveStatusScore = 0;
    int finalScore = 0; // = fiveStatusScore + skillScore
    int skillPt = 0;

public:
    int skillScore() { return finalScore - fiveStatusScore; }
    const string explain() {
        stringstream ss;
        for (int i = 0; i < 5; ++i) {
            int value = fiveStatus[i];
            if (value > 1200)
                value = 600 + value / 2;    // >1200时减半（去括号）
            ss << AttrNames[i] << "=" << value << " ";
        }
        ss << "Pt=" << skillPt << " ";
        ss << "总分 " << fiveStatusScore << "+" << skillScore() << "=" << finalScore;
        return ss.str();
    }

};

