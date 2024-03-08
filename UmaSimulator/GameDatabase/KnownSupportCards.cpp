#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "GameDatabase.h"
#include "../Game/Game.h"

using json = nlohmann::json;
using namespace std;

unordered_map<int, SupportCard> GameDatabase::AllCards;
unordered_map<int, SupportCard> GameDatabase::DBCards;

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

                    SupportCard jdata[5];

                    for (int x = 0; x < 5; ++x) {
                        jdata[x].load_from_json(j, x);
                    }

                    cout << "载入支援卡 #" << jdata[4].cardName << " --- " << jdata[4].cardID << endl;
                    if (GameDatabase::AllCards.count(jdata[4].cardID) > 0)
                        cout << "错误：重复支援卡 #" << jdata[4].cardName << " --- " << jdata[4].cardID << endl;
                    else {
                        for (int x = 0; x < 5; ++x) 
                            GameDatabase::AllCards[jdata[x].cardID] = jdata[x];
                    }
                        
                }
                catch (exception& e)
                {
                    cout << "支援卡信息JSON出错: " << entry.path() << endl << e.what() << endl;
                }
            }
        }
        cout << "共载入 " << GameDatabase::AllCards.size() << " 支援卡" << endl;
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

void GameDatabase::loadDBCards(const string& pathname)
{
    try
    {
        ifstream ifs(pathname);
        stringstream ss;
        ss << ifs.rdbuf();
        ifs.close();
        json j = json::parse(ss.str(), nullptr, true, true);

        for (auto & it : j.items()) 
        {
           // cout << it.key() << endl;
            for (int x = 0; x < 5; ++x) {
                SupportCard jdata;
                jdata.load_from_json(it.value(),x);
                jdata.isDBCard = true;
                if (GameDatabase::AllCards.count(jdata.cardID) > 0) // 如果已经被loadCards载入（有手动数据）
                    GameDatabase::DBCards[jdata.cardID] = jdata;    // 仍然把数据暂存在DBCards里（现在没使用这个表），用于验算
                else
                    GameDatabase::AllCards[jdata.cardID] = jdata;   // 没有手动数据则用自动数据
            }
        }
        cout << "共载入 " << GameDatabase::AllCards.size()/5 << " 支援卡" << endl;
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

CardTrainingEffect SupportCard::getCardEffect(const Game& game, bool isShining, int atTrain, int jiBan, int effectFactor, int trainingCardNum, int trainingShiningNum) const
{
    CardTrainingEffect effect(this);

    //printf("This card is :%d", cardID);
    

    if (isDBCard || effectFactor==-1)  // 暂时使用effectFactor=-1表示验算模式
    {
        // 新的固有处理代码
        // 由于不能维护isFixed状态，每次都要重新计算
        double rate;
        int count, totalJiBan;
        double expectedVital;
        auto args = uniqueEffectParam;
        int type = uniqueEffectType;

        if (effectFactor == -1)
        {
            // 让手写卡也使用新代码/新数据计算，用来比对计算结果
            // 重新用新卡数据初始化自己
            effect = CardTrainingEffect(&GameDatabase::DBCards[cardID]);
            type = GameDatabase::DBCards[cardID].uniqueEffectType;
            args = GameDatabase::DBCards[cardID].uniqueEffectParam;
        }


        //已知的问题
        // 1.智高峰等（根据xx技能个数），按回合数近似
        // 2.根双涡轮等（友情训练次数），视为全开
        // 3.智太阳神等（某条件提升得意率），视为全开
        // 4.速成田路（粉丝数），按回合数估计
        // 5.智波旁、智小栗帽等（初始状态相关），暂时没写
        // 6.旧智中山（随机让失败率变成0），暂时没写
        // 不确定还有没有其他的
        switch (type)
        {
            case 0:
                break;
            case 1:   // 羁绊>=args[1]时生效
            case 2:
                if (jiBan >= args[1])
                {
                    if (args[2] > 0)
                        effect.apply(args[2], args[3]);
                    if (args[4] > 0)
                        effect.apply(args[4], args[5]);
                    if (cardID / 10 == 30137)    // 神团额外词条特判
                    {
                        effect.apply(1, 10).apply(2, 15);
                    }
                }
                break;
            case 3:   // 进王：羁绊+非擅长训练
                if (jiBan >= args[1] && cardType != atTrain)
                    effect.xunLian += 20;
                break;
            case 4:   // 成田路。没有粉丝数，用回合数估算
              rate = game.turn <= 33 ? 0 :
                game.turn <= 40 ? 0.2 :
                game.turn <= 42 ? 0.25 :
                game.turn <= 58 ? 0.7 :
                1.0;
                effect.xunLian += rate * (double)args[2];
                break;
            case 5:   // 根据编成支援卡类型的初始属性加成(对应属性+10，友人/团队卡全属性+2), 暂不处理
                break;
            case 6:   // 天狼星，需要用到effectFactor
                effect.apply(1, max(0, min(args[1], args[1] - effectFactor)) * args[3]);
                break;
            case 7:   // 青竹，等
                // 需要计算训练后的体力，暂时以-20估算
                expectedVital = game.vital + game.trainVitalChange[atTrain];
                rate = clamp(expectedVital / game.maxVital, 0.3, 1.0);
                // (0.3, 1) --> (1, 0)
                effect.apply(1, args[5] + args[2] * (1 - rate) / 0.7);
                break;
            case 8:   // 彩珠
                effect.xunLian += 5 + 3 * clamp((game.maxVital - 100) / 4, 0, 5);
                break;
            case 9:   // 地堡，需要计算总羁绊
                totalJiBan = 0;
                for (int i = 0; i < 6; ++i)
                    totalJiBan += game.persons[i].friendship;
                rate = double(totalJiBan) / 600;
                effect.xunLian += rate * 20;
                break;
            case 10:   // 根神鹰，需要计算同时训练的卡数量
                effect.xunLian += args[2] * min(5, trainingCardNum);
                break;
            case 11:   // 水司机，需要当前训练等级
                effect.xunLian += args[2] * min(5, 1 + game.getTrainingLevel(atTrain));
                break;
            case 12:   // 智中山
                break;
            case 13:   // B95，麻酱
                if (trainingShiningNum >= 1)
                    effect.apply(args[1], args[2]);
                break;
            case 14:   // 耐善信, 暂时按训练前体力算
                rate = clamp((double)game.vital / 100, 0.0, 1.0);    // >100也是满训练
                effect.xunLian += 5 + 15 * rate;
                break;
            case 15:   // 智帽，暂时不计
                break;
            case 16:   // x个xxx类型技能，获得xxx加成（例如智高峰）
                if (args[1] == 1)//速度技能
                {
                  count = 1 + game.turn / 6;
                }
                else if (args[1] == 2)//加速度技能
                {
                  count = 0.7 + game.turn / 12.0;
                }
                else if (args[1] == 3)//回体技能
                {
                  count = 0.4 + game.turn / 15.0;
                }
                else
                {
                  count = 0;
                  assert(false && "未知的购买技能型支援卡固有");
                }
                if (count > args[4])
                  count = args[4];
                effect.apply(args[2], args[3] * count);
                break;
            case 17:   // 根大和
                count = 0;
                for (int i = 0; i < 5; ++i)
                    count += min(5, 1+game.getTrainingLevel(i));
                effect.xunLian += (count / 25.0) * args[3];
                break;
            case 18:   // 佐岳
                break;
            case 19:    // 凉花
                break;
            case 20:    // 巨匠
              if (jiBan >= 80)
              {
                int cardTypeCount[7] = { 0,0,0,0,0,0,0 };
                for (int i = 0; i < 6; i++)
                {
                  int t = game.persons[i].cardParam.cardType;
                  assert(t <= 6 && t >= 0);
                  cardTypeCount[t]++;
                }
                cardTypeCount[5] += cardTypeCount[6];

                for (int i = 0; i < 6; i++)
                  if (cardTypeCount[i] > 2)cardTypeCount[i] = 2;
                for (int i = 0; i < 5; i++)
                  if (cardTypeCount[i] > 0)
                    effect.apply(i + 3, cardTypeCount[i]);  // 速耐力根智 = 0-4 = CardEffect词条3-7
                if (cardTypeCount[5] > 0)
                  effect.apply(30, cardTypeCount[5]); // pt = 30
              }
            break;
            case 21:   // 耐万籁，编入4种支援卡时+10训练
              {
                int cardTypeCount[7] = { 0,0,0,0,0,0,0 };
                for (int i = 0; i < 6; i++)
                {
                  int t = game.persons[i].cardParam.cardType;
                  assert(t <= 6 && t >= 0);
                  cardTypeCount[t]++;
                }
                int cardTypes = 0;
                for (int i = 0; i < 7; i++)
                  if (cardTypeCount[i] > 0)
                    cardTypes++;
                if (cardTypes >= args[1])
                  effect.apply(args[2], args[3]);
              }
              break;
            default:   // type == 0
                if (uniqueEffectType != 0) {
                    cout << "未知固有 #" << uniqueEffectType << endl;
                }
                break;
        }   // switch
    }
   else
   {
     assert(false);
   }
    
    return effect;
}
