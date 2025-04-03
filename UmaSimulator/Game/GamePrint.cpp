#include <iostream>
#include <cassert>
#include <vector>
#include <iomanip>  // for std::setw
#include <algorithm>
#include "../External/termcolor.hpp"
#include "../External/utils.h"
#include "Game.h"
using std::cout;
using std::string;
using std::endl;
using std::to_string;
using std::vector;

std::string Person::getPersonName() const
{
  string s =
    personType == PersonType_unknown ? "未加载" :
    personType == PersonType_scenarioCard ? "[团]老登" :
    personType == PersonType_card ? UTF8_rune_cut(cardParam.cardName, 5) :
    personType == PersonType_npc ? "NPC" :
    personType == PersonType_yayoi ? "理事长" :
    personType == PersonType_reporter ? "记者" :
    "未知";
  return s;
}
static string getColoredColorName(int color)
{
  if (color == L_red)
    return "\033[1;31m红\033[0m";
  else if (color == L_green)
    return "\033[1;32m绿\033[0m";
  else if (color == L_blue)
    return "\033[1;34m蓝\033[0m";
}

std::string ScenarioBuffInfo::buffDescriptions[57] = {
  "（オーラ）得意率+30",
  "（学びの姿勢）hint+80%",
  "（社交術）羁绊+2",
  "（最初の一歩）干劲+15%",
  "（強者の求心力）得意率+60",
  "（たゆまぬ鍛錬）干劲+30%",
  "（時には苛烈に）训练+5%",
  "（共に頂へ）干劲+15%+人头*8%",
  "（高潔な矜持）绝好调时，干劲+50%",
  "（極限の集中）绝好调时，hint+200%",
  "（Dear friend）友情训练后，干劲+1，3回合CD",
  "（慈愛の微笑み）休息后，干劲+1，友情+60%",
  "（心眼）绝好调时，必然发生hint",
  "（英気を養う）休息后，训练+50%。hint量+1",
  "（愛しの子よ、共に栄光へ）绝好调时，友情+15%，超绝好调再+20%",
  "（Off we go）休息后，干劲+200%，体力消耗-100%",
  "（高潔なる魂）绝好调时，干劲+120%",
  "（歴史に名を残す覚悟）绝好调时，得意率+100",
  "（絆が織りなす光）绝好调时，友情+25%",
  "（協力申請）得意率+30",
  "（観察眼）hint+80%",
  "（交渉術）羁绊+2",
  "（リーダーシップ）训练+3%",
  "（気配り上手）得意率+60",
  "（奮励努力）训练+3%，干劲+15%",
  "（手法の改善提案）训练+5%",
  "（切磋琢磨）训练+2+人头*2%",
  "（衰えぬ情熱）训练成功后，训练+7%",
  "（未来を見据えて）友情训练后，hint量+1",
  "（磨励自彊）训练成功后，体力消耗-15%",
  "（君となら、もっと！）友情+22%",
  "（飽くなき挑戦心）训练成功后，训练+25%",
  "（日進月歩）训练+20%",
  "（雲海蒼天）训练成功后，训练+15%，挑战中再+15%",
  "（共に切り開く未来）训练成功后，人头+3，2回合CD",
  "（百折不撓）训练成功后，干劲+120%",
  "（革命の青写真）训练成功后，hint+250%",
  "（集いし理想）训练成功后，友情+25%",
  "（アピール上手）得意率+30",
  "（トレンドチェック）hint+80%",
  "（トーク術）羁绊+2",
  "（アイドルステップ）友情+5%",
  "（個性を伸ばして）得意率+60",
  "（溢れるバイタリティ）友情+3，干劲+15%",
  "（レッスンのコツ）训练+5%",
  "（素敵なハーモニー）友情+3%+彩圈人头*3%",
  "（リズムを合わせて）友情训练后，人头+1，2回合CD",
  "（ヒラメキの連鎖）3人以上训练后，hint数+1",
  "（心繋がるパフォーマンス）3人以上训练后，羁绊+3，友情+10%",
  "（トレーニングの約束）休息后，训练+15%，人头+3",
  "（ユニゾンパフォーマンス）3人以上训练后，人头+1",
  "（一緒に輝きましょう！）友情+22%",
  "（絆が奏でるハーモニー）训练+7%+人头*7%",
  "（溢れる魅力）3人以上训练后，出现率+25",
  "（怪物チャンスマイル♪）友情训练后，干劲+150%",
  "（アピール大成功！）友情训练后，得意率+100",
  "（国民的アイドルウマ娘）5人以上训练后，友情+20%，训练+20%",
};


std::string ScenarioBuffInfo::getScenarioBuffName(int16_t buffId)
{
  if (buffId < 0)return "(空)";
  int color = buffId / 19;
  int starIdx = buffId % 19;
  int star = starIdx < 4 ? 1 : starIdx < 10 ? 2 : 3;
  string s = to_string(star);
  string buffDescr = ScenarioBuffInfo::buffDescriptions[buffId];
  if (color == L_red)
    s = "\033[1;31m(红" + s + ")" + buffDescr + "\033[0m";
  else if (color == L_green)
    return "\033[1;32m(绿" + s + ")" + buffDescr + "\033[0m";
  else if (color == L_blue)
    return "\033[1;34m(蓝" + s + ")" + buffDescr + "\033[0m";
  return s;
}

bool ScenarioBuffInfo::defaultOrder(int16_t a, int16_t b)
{
  if (a / 19 < b / 19)return true;
  if (a / 19 > b / 19)return false;
  if (getBuffStarStatic(a) > getBuffStarStatic(b))return true;
  if (getBuffStarStatic(a) < getBuffStarStatic(b))return false;
  return a < b;
}

std::string ScenarioBuffInfo::getName() const
{
  return getScenarioBuffName(buffId);
}

std::string ScenarioBuffInfo::getColoredState() const
{
  string name = getName();
  if (isActive)
    name = "\033[1;32m[O]\033[0m" + name;
  else if (coolTime == 0)
  {
    name = "\033[1;31m[X]\033[0m" + name;
  }
  else if (coolTime > 0)
  {
    name = "\033[1;35m[" + to_string(coolTime) + "]\033[0m" + name;
  }

  return name;
}

int16_t ScenarioBuffInfo::getBuffStarStatic(int16_t id)
{
  if (id < 0)return -1;
  int idx = id % 19;
  if (idx < 4)return 1;
  else if (idx < 10)return 2;
  else return 3;
}

std::string Game::getPersonStrColored(int personId, int atTrain) const
{
  if (personId < 0)
    return "Empty";

  if (personId == PS_noncardYayoi)
  {
    //理事长
    int friendship = friendship_noncard_yayoi;
    string s = "理事长";
    if (friendship < 100)
      s = s + ":" + to_string(friendship);
    return "\033[35m" + s + "\033[0m";
  }
  else if (personId == PS_noncardReporter)
  {
    //记者
    int friendship = friendship_noncard_reporter;
    string s = "记者";
    if (friendship < 100)
      s = s + ":" + to_string(friendship);
    return "\033[35m" + s + "\033[0m";
  }
  else//支援卡/红登npc
  {
    string s = "";
    //根据闪彩等给名称加颜色
    if (personId < 6)//card
    {
      const Person& p = persons[personId];
      s = p.getPersonName();
      if (p.personType != PersonType_npc)
      {
        if (p.friendship < 100)
          s = s + ":" + to_string(p.friendship);
      }
      if (p.personType == PersonType_scenarioCard)
        s = "\033[32m" + s + "\033[0m"; // 友人
      else if (p.personType == PersonType_card)
      {
        if (isCardShining(personId, atTrain))
          s = "\033[1;36m" + s + "\033[0m"; //闪彩
        else if (p.friendship < 80)
          s = "\033[33m" + s + "\033[0m"; //需要拉羁绊
      }
      else assert(false);

      //技能启发
      if (p.personType == PersonType_card && p.isHint)
        s = "\033[31m!\033[0m" + s;

      //return s;
    }
    else if (personId >= PS_npc0 && personId <= PS_npc4)
    {
      int tra = personId - PS_npc0;
      s = "[" + Action::trainingName[tra] + "]NPC";
      assert(lg_mainColor == L_red);
      if (isCardShining(personId, atTrain))
        s = "\033[36m" + s + "\033[0m";
    }
    else
    {
      assert(false);
      return "\033[31mUnknown\033[0m";
    }

    if (lg_mainColor == L_red)
    {
      int gauge = lg_red_friendsGauge[personId];
      if (gauge == 20)
        s = s + "\033[1;33mMAX\033[0m";
      else
        s = s + "\033[31m:" + to_string(gauge) + "\033[0m";
    }
    return s;
  }
  
}

void Game::printEvents(string s) const
{
#ifdef PRINT_GAME_EVENT
  if (gameSettings.playerPrint) //这个不删是因为模拟器里可能也要进行ai计算
    cout << "\033[32m" + s + "\033[0m" << endl;
#endif
}

static void printStrFixedWidth(string s, int width)
{
  cout << std::left;

  //计算字符串中颜色代码的长度
  int colorCodeLen = 0;
  bool inColorCode = false;
  for (int j = 0; j < s.size(); j++)
  {
    char c = s[j];
    if (c == '\033')
      inColorCode = true;

    if (inColorCode)
    {
      colorCodeLen += 1;
      if (c == 'm')
        inColorCode = false;
    }
  }
  cout << std::setw(width + colorCodeLen) << s;

  cout << std::internal;
}

const int tableWidth = 17;
static void printTableRow(string strs[5])
{
    cout << std::left;
    for (int i = 0; i < 5; i++)
    {
        string s = strs[i];
        // 计算utf8字数与长度的差
        int runeDiff = s.length() - UTF8_rune_count(s);

        //计算字符串中颜色代码的长度
        int colorCodeLen = 0;
        bool inColorCode = false;
        for (int j = 0; j < s.size(); j++)
        {
            char c = s[j];
            if (c == '\033')
                inColorCode = true;

            if (inColorCode)
            {
                colorCodeLen += 1;
                if (c == 'm')
                    inColorCode = false;
            }
        }
        s = "| " + s;
        cout << std::setw(tableWidth + colorCodeLen + runeDiff / 2) << s;
    }
    cout << "|" << endl << std::internal;
}

void Game::print() const
{
  
  cout<<"\033[31m-------------------------------------------------------------------------------------------\033[0m"<<endl;
  if (isEnd())
    cout << "游戏已结束，评分 = " << to_string(finalScore()) << endl;
  cout << "当前马娘：" << GameDatabase::AllUmas[umaId].name << endl;
  cout << termcolor::green << "回合数：" << turn + 1 << "/" << TOTAL_TURN << ", 第" << turn / 24 + 1 << "年" << (turn % 24) / 2 + 1 << "月" << (turn % 2 ? "后" : "前") << "半" << termcolor::reset << endl;
  //还有几个回合比赛
  {
    int turnsBeforeRace = -1;
    for (int i = turn; i < TOTAL_TURN; i++)
    {
      if (isRacingTurn[i])
      {
        turnsBeforeRace = i - turn;
        break;
      }
    }
    cout << "距离下一场比赛还有" << termcolor::bright_yellow << turnsBeforeRace << "个回合" << termcolor::reset << endl;

  }
  {
    int totalStatus = fiveStatus[0] + fiveStatus[1] + fiveStatus[2] + fiveStatus[3] + fiveStatus[4];
    cout << termcolor::cyan << "总属性：" << totalStatus << "     pt：" << skillPt << termcolor::reset << endl;
  }
  {
    if (isQieZhe)
      cout << termcolor::bright_yellow << "有切者" << termcolor::reset << endl;
    if (isAiJiao)
      cout << termcolor::bright_yellow << "有爱娇" << termcolor::reset << endl;
    if (isPositiveThinking)
      cout << termcolor::bright_yellow << "有正向思考" << termcolor::reset << endl;
    if (isRefreshMind)
      cout << termcolor::bright_yellow << "有每回合体力+5" << termcolor::reset << endl;
    

    if (failureRateBias < 0)
      cout << termcolor::bright_yellow << "有练习上手" << termcolor::reset << endl;
    if (failureRateBias > 0)
      cout << termcolor::red << "有练习下手" << termcolor::reset << endl;
  }
  
  //友人卡状态
  if (friend_type == 1 || friend_type == 2)
  {
    if (friend_stage == 0)
      cout << termcolor::cyan << "友人卡未点击" << termcolor::reset << endl;
    else if (friend_stage == 1)
      cout << termcolor::cyan << "友人出行未解锁" << termcolor::reset << endl;
    else
    {
      if (friend_qingre)
      {
        cout << termcolor::bright_green << "团卡情热状态已持续 " << friend_qingreTurn << " 回合" << termcolor::reset << endl;
      }
      if (friend_outgoingUsed[4])
        cout << termcolor::cyan << "友人出行已走完" << termcolor::reset << endl;
      else
      {

        cout << termcolor::cyan << "友人出行剩余: ";
        for (int i = 0; i < 5; i++)
        {
          if (friend_outgoingUsed[i])
            cout << termcolor::red << "X " << termcolor::reset;
          else
            cout << termcolor::bright_green << "O " << termcolor::reset;
        }
        cout << endl;

      }
    }
  }

  if (turn < 72)
  {
    int nextGetBuff = (turn / 6 + 1) * 6;
    cout << "距离下次选心得还有 " << nextGetBuff - turn << " 回合" << endl;
  }
  for (int c = 0; c < 3; c++)
  {
    cout << getColoredColorName(c) << "=" << lg_gauge[c] << "/8   ";
  }
  cout << endl;

  {
    string vitalColor;
    if (vital > 70)
      vitalColor = "\033[32m";
    else if (vital > 50)
      vitalColor = "\033[33m";
    else
      vitalColor = "\033[31m";
    cout << "体力：" << vitalColor << "|";
    for (int i = 0; i < vital / 2; i++)
      cout << "#";
    for (int i = vital / 2; i < maxVital / 2; i++)
      cout << "-";
    cout << "|  " << vital << "\033[0m" << "/" << maxVital;


    cout <<"  干劲:" <<
      (motivation == 1 ? "\033[31m绝不调\033[0m" :
        motivation == 2 ? "\033[31m不调\033[0m" :
        motivation == 3 ? "\033[31m普通\033[0m" :
        motivation == 4 ? "\033[33m好调\033[0m" :
        motivation == 5 ? (lg_blue_active?"\033[1;34m超绝好调\033[0m" :"\033[32m绝好调\033[0m" )
        : "未知") << endl;
    cout << endl;
  }

  if (lg_mainColor == L_blue)
  {
    if (lg_blue_active)
    {
      cout << "超绝好调剩余\033[32m" << lg_blue_remainCount << "\033[0m回合，可延长\033[32m" << lg_blue_canExtendCount << "\033[0m次";
      cout << endl;
    }
    else
    {
      cout << "超绝好调充能\033[32m" << lg_blue_currentStepCount << "\033[0m/3";
      cout << endl;
    }
  }

  if (lg_mainColor == L_green)
  {
    if (lg_green_active)
    {
      cout << "挑战领域第\033[32m" << lg_green_continuationZoneCount << "\033[0m回合";
      cout << endl;
    }
    else
    {
      cout << "挑战充能\033[32m" << lg_green_currentStepCount << "\033[0m/4";
      cout << endl;
    }
  }

  for (int i = 0; i < 10; i++)
  {
    printStrFixedWidth(lg_buffs[i].getColoredState(), 80);
    cout << endl;
  }
  

  //string divLine = "|------------------------------------------------------------------------------------|\n";

  string divLineOne = "";
  for (int i = 0; i < tableWidth - 1; i++)
    divLineOne += "-";

  string divLine = "|";
  string divLineWhite = "|";
  for (int i = 0; i < 5; i++)
  {
    bool isShining = trainShiningNum[i] > 0;
    if (isShining)
      divLine += "\033[32m" + divLineOne + "\033[0m";
    else
      divLine += divLineOne;
    divLineWhite += divLineOne;

    if (i != 4)
    {
      divLine += "-";
      divLineWhite += "-";
    }
  }
  divLine += "|\n";
  divLineWhite += "|\n";

  cout << divLine;
  //训练标题，失败率，碎片
  {
    string oneRow[5];//表格中一行要显示的内容
    for (int i = 0; i < 5; i++)
    {
      string s;
      if (i == 0)
        s = "速";
      else if (i == 1)
        s = "耐";
      else if (i == 2)
        s = "力";
      else if (i == 3)
        s = "根";
      else if (i == 4)
        s = "智";

      int fRate = failRate[i];
      if (fRate > 0)
        s = s + "(\033[31m" + to_string(fRate) + "%\033[0m)";
      else if(lg_green_active)
        s = s + "(绿登\033[31m" + to_string(lg_green_endRate[i]) + "%\033[0m)";
      else
        s = s + "(0%)";
      oneRow[i] = s;
    }
    printTableRow(oneRow);
  }
  cout << divLine;
  
  if (stage == ST_train && !isRacing)
  {

    {
      string oneRow[5];//表格中一行要显示的内容
      for (int i = 0; i < 5; i++)
      {
        int gaugeGain = trainShiningNum[i] > 0 ? 3 : 1;
        int gaugeColor = lg_trainingColor[i];
        int currentLv = lg_gauge[gaugeColor];
        int lvAfterTrain = currentLv + gaugeGain;
        if (lvAfterTrain > 8)lvAfterTrain = 8;
        string s = currentLv == 8 ? "MAX" : to_string(currentLv) + "->" + to_string(lvAfterTrain);
        oneRow[i] = getColoredColorName(gaugeColor) + ": " + s;
      }
      printTableRow(oneRow);
    }

    cout << divLine;

    //属性值
    {
      string oneRow[5];//表格中一行要显示的内容
      for (int i = 0; i < 5; i++)
      {
        oneRow[i] = "\033[33m" + to_string(fiveStatus[i]) + "\033[0m" + "/" + to_string(fiveStatusLimit[i]) + "(" + to_string(fiveStatusLimit[i] - fiveStatus[i]) + ")";
      }
      printTableRow(oneRow);
    }
  }

  if (stage != ST_train)
  {
    cout << divLineWhite;
    if (stage == ST_decideEvent)
    {
      cout << termcolor::cyan << "选事件阶段" << termcolor::reset << endl;
      if (decidingEvent == DecidingEvent_three)
      {
        cout << termcolor::green << "正在选择团卡三选一事件" << termcolor::reset << endl;
      }
      else if (decidingEvent == DecidingEvent_outing)
      {
        cout << termcolor::green << "正在选择出行" << termcolor::reset << endl;
      }
      else
        throw("未知的decideEvent");
    }
    else if (stage == ST_chooseBuff)
    {
      cout << termcolor::cyan << "选择心得中：" << termcolor::reset << endl;
      for (int i = 0; i < lg_pickedBuffsNum; i++)
      {
        cout << ScenarioBuffInfo::getScenarioBuffName(lg_pickedBuffs[i]) << endl;
      }
    }
    else if (stage == ST_distribute)
    {
      cout << termcolor::red << "非操作阶段：正在分配人头" << termcolor::reset << endl;
    }
    else if (stage == ST_pickBuff)
    {
      cout << termcolor::red << "非操作回合：正在抽取buff" << termcolor::reset << endl;
    }
    else if (stage == ST_event)
    {
      cout << termcolor::red << "非操作回合：正在处理回合后事件" << termcolor::reset << endl;
    }
    else
    {
      cout << termcolor::red << "未知stage=" << stage << termcolor::reset << endl;
    }

    return;//非训练回合就不显示训练表格了
  }
  else if (stage==ST_train && isRacing)
  {
    cout << divLineWhite;
    cout << "比赛颜色：" << getColoredColorName(lg_trainingColor[T_race]) << endl;
    cout << termcolor::red << "比赛回合" << termcolor::reset << endl;
    return;//比赛回合就不显示训练了
  }

  //体力
  {
    string oneRow[5];//表格中一行要显示的内容
    for (int i = 0; i < 5; i++)
    {
      int newVital = vital;
      newVital += trainVitalChange[i];


      //cout << " The vital dlt of training " << i << " is : " << trainVitalChange[i] << '\n';

      if (newVital > maxVital)
        newVital = maxVital;
      if (newVital < 0)
        newVital = 0;

      string vitalStr = to_string(newVital);
      if (newVital > 70)
        vitalStr = "\033[32m" + vitalStr + "\033[0m";
      else if (newVital > 50)
        vitalStr = "\033[33m" + vitalStr + "\033[0m";
      else
        vitalStr = "\033[31m" + vitalStr + "\033[0m";
      oneRow[i] = "体力：" + vitalStr + "/" + to_string(maxVital);
    }
    printTableRow(oneRow);
  }
  //训练等级
  {
    string oneRow[5];//表格中一行要显示的内容
    for (int i = 0; i < 5; i++)
    {
      oneRow[i] = "等级：" + to_string(getTrainingLevel(i) + 1);
      if (getTrainingLevel(i) < 4)
        oneRow[i] = oneRow[i] + "(+" + to_string(trainLevelCount[i] % 4) + ")";
        
    }
    printTableRow(oneRow);
  }
  
  cout << divLine;
  //单次训练总属性
  {
    string oneRow[5];//表格中一行要显示的内容
    for (int i = 0; i < 5; i++)
    {
      int totalStatus = 0;
      for (int j = 0; j < 5; j++)
        totalStatus += trainValue[i][j];
      oneRow[i] = "属性:\033[32m" + to_string(totalStatus) + "\033[0m,pt:" + to_string(trainValue[i][5]);
    }
    printTableRow(oneRow);
  }
  cout << divLine;

  //人头
  {
    for (int head = 0; head < 5; head++)
    {
      string oneRow[5];//表格中一行要显示的内容
      for (int item = 0; item < 5; item++)
      {
        int personId = personDistribution[item][head];
        if (personId < 0)
          oneRow[item] = "";
        else
          oneRow[item] = getPersonStrColored(personId, item);
      }
      printTableRow(oneRow);
    }
  }

  cout << divLine;
  //此训练加的五属性与pt
  {
    for (int i = 0; i < 7; i++)
    {
      string oneRow[5];//表格中一行要显示的内容
      string s;
      if (i == 0)
        s = "速";
      else if (i == 1)
        s = "耐";
      else if (i == 2)
        s = "力";
      else if (i == 3)
        s = "根";
      else if (i == 4)
        s = "智";
      else if (i == 5)
        s = "pt";
      else if (i == 6)
        s = "体";
      for (int item = 0; item < 5; item++)
      {
        if (s != "体")
          oneRow[item] = s + ": " + to_string(trainValueLower[item][i]) + "+" + to_string(trainValue[item][i] - trainValueLower[item][i]);
        else
          oneRow[item] = s + ": " + to_string(trainVitalChange[item]);
      }
      printTableRow(oneRow);
    }
  }
  cout << divLineWhite;

  //休息外出比赛的菜种
  {
    string oneRow[5];//表格中一行要显示的内容

    for (int t = 5; t < 8; t++)
    {
      string s;
      if (t == 5)
        s = "休息";
      else if (t == 6)
        s = "外出";
      else if (t == 7)
        s = "比赛";
      Action action(ST_train, t);
      if(!isLegal(action))
        s = "\033[31m" + s + ":__\033[0m";
      else
      {
        int color = lg_trainingColor[t];
        s = s + ":";
        s = s + "\033[33m" + getColoredColorName(color) + "\033[0m";
      }
      oneRow[t - 5] = s;
    }
    printTableRow(oneRow);
  }

  cout << divLineWhite;

  cout << "\033[31m-------------------------------------------------------------------------------------------\033[0m" << endl;
  
}

static int convertToHalfIfOver1200(int x)
{
  if (x > 1200)
    x = 1200 + (x - 1200) / 2;
  return x;
}
void Game::printFinalStats() const
{
  int fiveStatusScore = 0;
  for (int i = 0; i < 5; i++)
    fiveStatusScore += GameConstants::FiveStatusFinalScore[std::min(fiveStatus[i], fiveStatusLimit[i])];
  int skillScore = int(getSkillScore());

  cout << termcolor::bright_red << "你的得分是：" << termcolor::bright_green << finalScore() << termcolor::reset << endl;
  cout << termcolor::bright_red << "属性分=" << termcolor::bright_green << fiveStatusScore << termcolor::reset << "，" << termcolor::bright_red << "技能分=" << termcolor::bright_green << skillScore << termcolor::reset << endl;
  cout <<
    termcolor::bright_blue << "速=" << termcolor::bright_yellow << convertToHalfIfOver1200(fiveStatus[0]) << termcolor::reset << " " <<
    termcolor::bright_blue << "耐=" << termcolor::bright_yellow << convertToHalfIfOver1200(fiveStatus[1]) << termcolor::reset << " " <<
    termcolor::bright_blue << "力=" << termcolor::bright_yellow << convertToHalfIfOver1200(fiveStatus[2]) << termcolor::reset << " " <<
    termcolor::bright_blue << "根=" << termcolor::bright_yellow << convertToHalfIfOver1200(fiveStatus[3]) << termcolor::reset << " " <<
    termcolor::bright_blue << "智=" << termcolor::bright_yellow << convertToHalfIfOver1200(fiveStatus[4]) << termcolor::reset << " " <<
    termcolor::bright_blue << "pt=" << termcolor::bright_yellow << skillPt << termcolor::reset << " " <<
    endl;
  cout << termcolor::bright_red << "(直接按每pt " << gameSettings.ptScoreRate << "分计算)" << termcolor::reset << endl;
}
