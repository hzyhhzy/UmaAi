#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "GameDatabase.h"
#include "../Game/Game.h"


using json = nlohmann::json;
using namespace std;

unordered_map<int, SupportCard> GameDatabase::AllCards;

void GameDatabase::loadCards(const string& dir)
{
    try
    {
        for (auto entry : filesystem::directory_iterator(dir + "/"))
        {
            //cout << entry.path() << endl;
            if (entry.path().extension() == ".json")
            {
                try
                {
                    ifstream ifs(entry.path());
                    stringstream ss;
                    ss << ifs.rdbuf();
                    ifs.close();
                    json j = json::parse(ss.str(), nullptr, true, true);

                    SupportCard jdata;
                    jdata.load_from_json(j);
                    cout << "载入支援卡 #" << jdata.cardName<<" --- "<<jdata.cardID << endl;
                    if (AllCards.count(jdata.cardID) > 0)
                        cout << "错误：重复支援卡 #" << jdata.cardName << " --- " << jdata.cardID << endl;
                    else
                        AllCards[jdata.cardID] = jdata;
                }
                catch (exception& e)
                {
                    cout << "支援卡信息JSON出错: " << entry.path() << endl << e.what() << endl;
                }
            }
        }
        cout << "共载入 " << AllCards.size() << " 个支援卡数据" << endl;
    }
    catch (exception& e)
    {
        cout << "读取支援卡信息出错: " << endl << e.what() << endl;
    }
    catch (...)
    {
        cout << "读取支援卡信息出错：未知错误" << endl;
    }
}

CardTrainingEffect SupportCard::getCardEffect(const Game& game, int atTrain, int jiBan,int effectFactor) const
{
    CardTrainingEffect effect =
    {
      youQing,
      ganJing,
      xunLian,
      {bonus[0],bonus[1],bonus[2],bonus[3],bonus[4],bonus[5]},
      wizVitalBonus
    };

    bool isShining = true;//是否闪彩
    if (cardType < 5)//速耐力根智卡
    {
        if (jiBan < 80)isShining = false;
        if (cardType != atTrain)isShining = false;
    }
    else if (GameDatabase::AllCards[cardID].cardType == 5)//神团
    {
        if (!game.venusCardIsQingRe)
            isShining = false;
    }
    else std::cout << "未知卡";

    if (game.venusIsWisdomActive && game.venusAvailableWisdom == 3)//黄女神
        isShining = true;

    if (!isShining)
    {
        effect.youQing = 0;
    }
    if (!isShining || atTrain != 4)
        effect.vitalBonus = 0;

    //接下来是各种固有
    //1.神团
    if (cardID == 1)
    {
        if (jiBan < 100)
        {
            if (isShining)
                effect.youQing = 20;
            effect.ganJing = 0;
            effect.bonus[5] = 0;
        }
    }
    //2.高峰
    //为了简化，视为初始训练加成是4%，第一年逐渐增大到20%，也就是第n个回合4+n*(2/3)%
    else if (cardID == 2)
    {
        if (game.turn < 24)
            effect.xunLian = 4 + 0.6666667 * game.turn;
    }
    //3.美妙
    else if (cardID == 3)
    {
        //啥都没有
    }
    //4.根乌拉拉
    else if (cardID == 4)
    {
        //啥都没有
    }
    //5.根风神
    else if (cardID == 5)
    {
        //啥都没有
    }
    //6.水司机
    else if (cardID == 6)
    {
        int traininglevel = game.getTrainingLevel(atTrain);
        effect.xunLian = 5 + traininglevel * 5;
        if (effect.xunLian > 25)effect.xunLian = 25;
    }
    //7.根凯斯
    else if (cardID == 7)
    {
        if (jiBan < 80)
        {
            effect.bonus[2] = 0;
        }
    }
    //8.根皇帝
    else if (cardID == 8)
    {
        if (jiBan < 80)
        {
            effect.bonus[0] = 0;
        }
    }
    //9.根善信
    else if (cardID == 9)
    {
        //啥都没有
    }
    //10.速宝穴
    else if (cardID == 10)
    {
        if (jiBan < 100)
        {
            effect.bonus[0] = 0;
        }
    }
    //11.耐海湾
    else if (cardID == 11)
    {
        //啥都没有
    }
    //12.智好歌剧
    else if (cardID == 12)
    {
        if (jiBan < 80)
        {
            effect.bonus[0] = 0;
        }
    }
    //13.根黄金城
    else if (cardID == 13)
    {
        if (jiBan < 100)
        {
            effect.bonus[3] = 0;
        }
    }
    //14.智波旁
    else if (cardID == 14)
    {
        //啥都没有
    }
    //15.耐狄杜斯
    else if (cardID == 15)
    {
        int totalJiBan = 0;
        for (int i = 0; i < 6; i++)
            totalJiBan += game.cardJiBan[i];
        effect.xunLian = totalJiBan / 30;
    }
    //16.智小栗帽
    else if (cardID == 16)
    {
        //啥都没有
    }
    //17根大脚
    else if (cardID == 17) {
        //啥都没有
    }
    //速北黑
    else if (cardID == 18) {
        //啥都没有
    }
    //福来
    else if (cardID == 19) {
        //啥都没有
    }
    //速子
    else if (cardID == 20) {
        if (jiBan < 100)
        {
            effect.youQing = 22;
        }
    }
    //21耐光钻
    else if (cardID == 21)
    {
        //啥都没有
    }
    //22，耐桂冠
    else if (cardID == 22)
    {
      if (game.turn < 24)
        effect.bonus[1] = 1;
      else if (game.turn < 48)
        effect.bonus[1] = 2;
      else
        effect.bonus[1] = 3;
    }
    //23力白仁
    else if (cardID == 23)
    {
      int traininglevel = game.getTrainingLevel(atTrain);
      effect.xunLian = 5 + traininglevel * 5;
      if (effect.xunLian > 25)effect.xunLian = 25;
    }
    //24力重炮
    else if (cardID == 24)
    {
        if (jiBan < 100)
        {
            effect.xunLian = 0;
        }
    }
    //25力内恰
    else if (cardID == 25)
    {
      if (jiBan < 100)
      {
        effect.bonus[2] = 0;
      }
    }
    //28根涡轮
    else if (cardID == 28)
    {
      //以后再想办法
    }
    //29根进王
    else if (cardID == 29)
    {
      if (jiBan < 80 || atTrain == 3)
        effect.xunLian = 0;
    }
    //30根青竹
    else if (cardID == 30)
    {
      if (effect.youQing > 0)
      {
        float extraBonus = 5 + (100 - game.vital) / 7.0;
        if (extraBonus > 15)extraBonus = 15;
        if (extraBonus < 5)extraBonus = 5; // std::cout << effect.youQing << " ";

        effect.youQing = 120 * (1 + 0.01 * extraBonus) - 100;

      }

    }
    //也问
    else if (cardID == 45)
    {
        if (jiBan < 80)
        {

            effect.bonus[5] = 0;
        }
    }
    //耐特
    else if (cardID == 46)
    {
        if (isShining)
        {
            effect.ganJing = 60;
        }
    }
    //根特
    else if (cardID == 47)
    {
        //null
    }
    //速尔丹
    else if (cardID == 48)
    {
        if (jiBan < 80)
        {
            effect.bonus[2] = 0;
        }
    }
    //智内恰
    else if (cardID == 49)
    {
        //啥都没有
    }

    else
    {
      //  std::cout << "未知卡";
    }

    return effect;
}
