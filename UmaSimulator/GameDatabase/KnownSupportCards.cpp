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
        cout << "共载入 " << GameDatabase::AllCards.size() << " 个支援卡数据" << endl;
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

        SupportCard jdata;
        for (auto & it : j.items()) 
        {
            for (int x = 0; x < 5; ++x) {
                SupportCard jdata;
                jdata.load_from_json(it.value(),x);
                if (GameDatabase::AllCards.count(jdata.cardID) > 0) continue;
                jdata.isDBCard = true;

                GameDatabase::AllCards[jdata.cardID] = jdata;
            }
        }
        cout << "共载入 " << GameDatabase::AllCards.size() << " 支援卡元数据" << endl;
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

CardTrainingEffect SupportCard::getCardEffect(const Game& game, int atTrain, int jiBan, int effectFactor) const
{
    CardTrainingEffect effect(this);
    bool isShining = true;//是否闪彩
    if (cardType < 5)//速耐力根智卡
    {
        if (jiBan < 80)isShining = false;
        if (cardType != atTrain)isShining = false;
    }
    else if (cardType == 5)//神团
    {
    //    if (!game.venusCardIsQingRe)
            isShining = false;
    }
    else if (cardType == 6)//友人
      isShining = false;
    else std::cout << "未知卡";

    //if (game.venusIsWisdomActive && game.venusAvailableWisdom == 3)//黄女神
    //    isShining = true;

    if (!isShining)
    {
        effect.youQing = 0;
    }
    if (!isShining || atTrain != 4)
        effect.vitalBonus = 0;

    if (isDBCard)
    {
        // 新的固有处理代码
        // 由于不能维护isFixed状态，每次都要重新计算
        double rate;
        int count, totalJiBan;
        double expectedVital;
        bool apply = false; // 默认不执行最后的applyUniqueEffect
        auto args = uniqueEffectParam;
        unordered_map<int, int> effectValues;
        switch (uniqueEffectType)
        {
            case 101:   // 羁绊>=args[1]时生效
                if (jiBan >= args[1])
                    apply = true;
                break;
            case 102:   // 进王：羁绊+非擅长训练
                if (jiBan >= args[1] && cardType != atTrain)
                    apply = true;
                break;
            case 103:   // DD：宇宙卡组，假设固定生效
                apply = true;
                break;
            case 104:   // 成田路。没有粉丝数，用回合数估算
                effectValues[(int)UniqueEffectType::TrainingEffectUp] = clamp((double)game.turn*2*args[2] / TOTAL_TURN, 0.0, (double)args[2]);
                apply = true;
                break;
            case 105:   // 根据编成支援卡类型的初始属性加成(对应属性+10，友人/团队卡全属性+2), 暂不处理
                break;
            case 106:   // 天狼星，需要用到effectFactor
                effectValues[(int)UniqueEffectType::SpecialTagEffectUp] = min(args[1], effectFactor) * args[3];
                apply = true;
            break;
            case 107:   // 青竹，等
                // 需要计算训练后的体力，暂时以-20估算
                expectedVital = game.vital - 20;
                rate = clamp(expectedVital / game.maxVital, 0.3, 1.0);
                // (0.3, 1) --> (1, 0)
                effectValues[(int)UniqueEffectType::SpecialTagEffectUp] = args[5] + args[2] * (1 - rate) / 0.7;
                apply = true;
                break;
            case 108:   // 彩珠
                effectValues[(int)UniqueEffectType::TrainingEffectUp] = 5 + 3 * clamp((game.maxVital - 100) / 4, 0, 5);
                apply = true;
                break;
            case 109:   // 地堡，需要计算总羁绊
                totalJiBan = 0;
                for (int i = 0; i < 6; ++i)
                    totalJiBan += game.persons[i].friendship;
                rate = totalJiBan / 600;
                effectValues[(int)UniqueEffectType::TrainingEffectUp] = rate * 20;
                apply = true;
                break;
            case 110:   // 根神鹰，需要计算同时训练的卡数量
                /*
                count = 0;
                for (int i = 0; i < 6; ++i)
                    if (game.cardDistribution[i, atTrain])
                        ++count;
                effect["TrainingEffectUp"] = args[2] * Math.Min(5, count);
                apply = true;*/
                break;
            case 111:   // 水司机，需要当前训练等级
                /*
                effect["TrainingEffectUp"] = args[2] * Math.Min(5, game.trainLevelCount[atTrain] / 12 + 1);
                apply = true;
                */
                break;
            case 112:   // 智中山
                break;
            case 113:   // B95，麻酱
                // todo: 需要判断是否为彩圈训练
                apply = true; break;
            case 114:   // 耐善信, 暂时按训练前体力算
                rate = clamp((double)game.vital / 100, 0.0, 1.0);    // >100也是满训练
                effectValues[(int)UniqueEffectType::TrainingEffectUp] = 5 + 15 * rate;
                apply = true;
                break;
            case 115:   // 智帽，暂时不计
                break;
            case 116:   // 高峰，速善信，等：统一按初始1层，年底半满，第二年四月学满计算
                count = (game.turn >= 30 ? args[4] :
                    (game.turn >= 24 ? (args[4] + 1) / 2 : 1));
                effectValues[args[2]] = args[3] * count;
                apply = true;
                break;
            case 117:   // 根大和，需要训练等级
                /*
                count = 0;
                for (int i = 0; i < 5; ++i)
                    count += Math.Min(5, game.trainLevelCount[i] / 12 + 1);
                rate = Math.Clamp((double)count / 25, 0, 1);
                effect["TrainingEffectUp"] = rate * args[3];
                apply = true;*/
                break;
            case 118:   // 佐岳
                break;
            default:   // type == 0
                if (uniqueEffectType != 0) {
                    cout << "未知固有 #" << uniqueEffectType << endl;
                }
                apply = true;
                break;
        }   // switch

        if (apply)
        {
            // uniqueEffectValues的固定面板值，和effectValues里动态添加的面板值都要生效
            for (auto it : uniqueEffectValues)
                effect.applyUniqueEffectLine(it.first, it.second);
            for (auto it : effectValues)
                effect.applyUniqueEffectLine(it.first, it.second);
        }
    }
    else
    {
        // 老版本固有代码
        int cardSpecialEffectId = cardID / 10;

        //接下来是各种固有
        //1.神团
        if (cardSpecialEffectId == 30137)
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
        else if (cardSpecialEffectId == 30134)
        {
            if (game.turn < 24)
                effect.xunLian = 4 + 0.6666667 * game.turn;
        }
        //3.美妙
        else if (cardSpecialEffectId == 30010)
        {
            //啥都没有
        }
        //4.根乌拉拉
        else if (cardSpecialEffectId == 30019)
        {
            //啥都没有
        }
        //5.根风神
        else if (cardSpecialEffectId == 30011)
        {
            //啥都没有
        }
        //6.水司机
        else if (cardSpecialEffectId == 30107)
        {

        int traininglevel = game.getTrainingLevel(atTrain);
        effect.xunLian = 5 + traininglevel * 5;
        if (effect.xunLian > 25)effect.xunLian = 25;
    }
    //7.根凯斯
    else if (cardSpecialEffectId == 30130)
    {
        if (jiBan < 80)
        {
            effect.bonus[2] = 0;
        }
    }
    //8.根皇帝
    else if (cardSpecialEffectId == 30037)
    {
        if (jiBan < 80)
        {
            effect.bonus[0] = 0;
        }
    }
    //9.根善信
    else if (cardSpecialEffectId == 30027)
    {
        //啥都没有
    }
    //10.速宝穴
    else if (cardSpecialEffectId == 30147)
    {
        if (jiBan < 100)
        {
            effect.bonus[0] = 0;
        }
    }
    //11.耐海湾
    else if (cardSpecialEffectId == 30016)
    {
        //啥都没有
    }
    //12.智好歌剧
    else if (cardSpecialEffectId == 30152)
    {
        if (jiBan < 80)
        {
            effect.bonus[0] = 0;
        }
    }
    //13.根黄金城
    else if (cardSpecialEffectId == 30153)
    {
        if (jiBan < 100)
        {
            effect.bonus[3] = 0;
        }
    }
    //14.智波旁
    else if (cardSpecialEffectId == 30141)
    {
        //啥都没有
    }
    //15.耐狄杜斯
    else if (cardSpecialEffectId == 30099)
    {
        int totalJiBan = 0;
        for (int i = 0; i < game.normalCardCount; i++)
            totalJiBan += game.persons[i].friendship;
        if(game.larc_zuoyueType!=0)
          totalJiBan += game.persons[17].friendship;
        effect.xunLian = totalJiBan / 30;
    }
    //速子
    else if (cardSpecialEffectId == 30101) {
        if (jiBan < 100)
        {
          if (effect.youQing > 0)
            effect.youQing = 20;
        }
    }
    //22，耐桂冠
    else if (cardSpecialEffectId == 30142)
    {
      if (game.turn < 24)
        effect.bonus[1] = 1;
      else if (game.turn < 48)
        effect.bonus[1] = 2;
      else
        effect.bonus[1] = 3;
    }
    //23力白仁
    else if (cardSpecialEffectId == 30123)
    {
      int traininglevel = game.getTrainingLevel(atTrain);
      effect.xunLian = 5 + traininglevel * 5;
      if (effect.xunLian > 25)effect.xunLian = 25;
    }
    //24力重炮
    else if (cardSpecialEffectId == 30151)
    {
        if (jiBan < 100)
        {
            effect.xunLian = 0;
        }
    }
    //25力内恰
    else if (cardSpecialEffectId == 30138)
    {
      if (jiBan < 100)
      {
        effect.bonus[2] = 0;
      }
    }
    //28根涡轮
    else if (cardSpecialEffectId == 30112)
    {
      //以后再想办法
    }
    //29根进王
    else if (cardSpecialEffectId == 30083)
    {
      if (jiBan < 80 || atTrain == 3)
        effect.xunLian = 0;
    }
    //30根青竹
    else if (cardSpecialEffectId == 30094)
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
        else if (cardSpecialEffectId == 30126)
        {
            if (jiBan < 80)
            {

                effect.bonus[5] = 0;
            }
        }
        //耐特
        else if (cardSpecialEffectId == 30127)
        {
            if (isShining)
            {
                effect.ganJing = 60;
            }
        }
        //根特
        else if (cardSpecialEffectId == 47)
        {
            //null
        }
        //速尔丹
        else if (cardSpecialEffectId == 30119)
        {
            if (jiBan < 80)
            {
                effect.bonus[2] = 0;
            }
        }
        // 皇团
        else if (cardSpecialEffectId == 30067) {

            if (jiBan < 80)
                effect.bonus[5] = 0;

    }
    // 红宝
    else if (cardSpecialEffectId == 30114) {
        if (jiBan < 80)
            effect.bonus[2] = 0;
    }
    // 福来
    else if (cardSpecialEffectId == 30078) {
        effect.failRateDrop += 10;
    }
    // 绿帽
    else if (cardSpecialEffectId == 30021) {
        effect.failRateDrop += 7;
        effect.vitalCostDrop += 4;
    }
    // 力奶奶
    else if (cardSpecialEffectId == 30156) {
      if (jiBan < 80)
      {
        effect.bonus[2] = 0;
      }
    }
    // 力飞驹
    else if (cardSpecialEffectId == 30132) {
      int guyouLevel = (game.maxVital - 100) / 4;
      if (guyouLevel > 5)guyouLevel = 5;
      effect.xunLian = 5 + 3 * guyouLevel;
    }
    //速神鹰
    else if (cardSpecialEffectId == 30161)
    {
      if (jiBan < 100)
      {
        for (int i = 0; i < 5; i++)
          effect.bonus[i] -= 1;
      }
    }
    //速黄金船
    else if (cardSpecialEffectId == 30168)
    {
      if (jiBan < 80)
      {
        effect.bonus[0] -= 1;
      }
    }
    else if (cardSpecialEffectId == 30154)
    {
      if (game.turn < 24)
        effect.bonus[0] = 1;
      else if (game.turn < 48)
        effect.bonus[0] = 2;
      else
        effect.bonus[0] = 3;
    }
    else if (cardSpecialEffectId == 30165)
    {
      if (jiBan >= 80)
      {
        effect.bonus[3] += 2;
      }
    }
    else if (cardSpecialEffectId == 30139)
    {
      if (jiBan >= 100)
      {
        effect.bonus[1] += 3;
      }
    }
    else if (cardSpecialEffectId == 30164)
    {
      if (jiBan >= 80)
      {
        effect.bonus[5] += 2;
      }
    }
    else if (cardSpecialEffectId == 30166)
    {
      if (game.turn < 12)
        effect.xunLian += 5.0 + (10.0 / 12) * game.turn;
      else
        effect.xunLian += 15;
    }
    else if (cardSpecialEffectId == 30158)
    {
      if (jiBan >= 100)
      {
        effect.youQing += (100 + effect.youQing) * 0.2;
      }
    }
    else if (cardSpecialEffectId == 30148)
    {
      int t = 0;
      for (int i = 0; i < 5; i++)
        t += game.getTrainingLevel(atTrain);
      int y = 5 + t;
      if (y > 20)y = 20;
      effect.xunLian += y;
    }
    else if (cardSpecialEffectId == 30163)
    {
      if (jiBan >= 80)
      {
        effect.xunLian += 10;
      }
    }
    // 智茶座
    else if (cardSpecialEffectId == 30157) {
      if (jiBan >= 100)
      {
        effect.bonus[4] = 2;
        effect.bonus[5] = 1;
      }
    }
    //[智]真弓快车(id:30149)的固有是闪彩的训练60干劲加成，但是在把五个人头检查一遍之前并不知道闪没闪彩，因此检查完五个人头之后还需要额外对这张卡的参数进行处理
    //后续处理写在Game类里了，虽然很丑陋，但没想到什么好办法
    else if (cardSpecialEffectId == 30149)
    {
      //参与友情训练时，干劲加成60
      //固有常开比高峰低150多分
      //实际比固有常开低100多分
      effect.ganJing += 60;
    }
    else
    {
      //  std::cout << "未知卡";
    }
    if (!isShining)
      effect.youQing = 0;
    return effect;
}
