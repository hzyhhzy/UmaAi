#include <iostream>
#include <cassert>
#include <vector>
#include <iomanip>  // for std::setw
#include "../External/termcolor.hpp"
#include "Game.h"
using std::cout;
using std::string;
using std::endl;
using std::to_string;
using std::vector;

std::string Person::getPersonStrColored(const Game& game) const
{
  string s =
    personType == 0 ? "未加载" :
    personType == 1 ? "[友]佐岳" :
    personType == 2 ? game.cardParam[cardIdInGame].cardName.substr(0, 8) :
    personType == 3 ? "NPC" :
    personType == 4 ? "理事长" :
    personType == 5 ? "记者" :
    personType == 6 ? "佐岳" :
    "未知";
  if (personType == 1 || personType == 2 || personType == 4 || personType == 5 || personType == 6)
  {
    if (friendship < 100)
      s = s + ":" + to_string(friendship);
  }

  //根据闪彩等给名称加颜色
  if (personType == 1)
    s = "\033[32m" + s + "\033[0m"; // 友人
  else if (personType == 2)
  {
    if(isShining)
      s = "\033[1;36m" + s + "\033[0m"; //闪彩
    else if(friendship<80)
      s = "\033[33m" + s + "\033[0m"; //需要拉羁绊
  }
  else if (personType == 3)
  {
    if (!game.larc_isAbroad && larc_charge < 3)
      s = "\033[36m" + s + "\033[0m"; //可充电
    else
      s = "\033[34m" + s + "\033[0m"; //不可充电
  }
  else if (personType == 4 || personType == 5 || personType == 6)
  {
    s = "\033[35m" + s + "\033[0m"; //理事长记者等
  }
  else
    s = "\033[31m" + s + "\033[0m"; //bug

  //技能启发
  if (personType == 2 && isHint)
    s = "\033[31m!\033[0m" + s;

  //充电状态与ss buff
  if ((personType == 2 || personType == 3) && !(game.turn < 2 || game.larc_isAbroad))
  {
    if (personType == 3)//npc把自己的特殊buff写在名字里
      s = s + GameConstants::LArcSSBuffNames[larc_specialBuff];
    s = s + ";";
    if (larc_charge < 3)
      s = s + "\033[1;31m" + to_string(larc_charge) + "\033[0m";
    for (int i = 0; i < 1; i++)
      s = s + GameConstants::LArcSSBuffNames[larc_nextThreeBuffs[i]];
  }
  return s;
}

void Game::printEvents(string s) const
{
  if (playerPrint)
    cout << "\033[32m" + s + "\033[0m" << endl;
}

static void printTableRow(string strs[5])
{
  const int width = 17;
  cout << std::left;
  for (int i = 0; i < 5; i++)
  {
    string s = strs[i];

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
    cout << std::setw(width + colorCodeLen) << s;
  }
  cout << "|" << endl << std::internal;
}

void Game::print() const
{

  cout<<"\033[31m-------------------------------------------------------------------------------------------\033[0m"<<endl;
  cout << "当前马娘：" << GameDatabase::AllUmas[umaId].name << endl;
  cout << termcolor::green << "回合数：" << turn + 1 << "/" << TOTAL_TURN << ", 第" << turn / 24 + 1 << "年" << (turn % 24) / 2 + 1 << "月" << (turn % 2 ? "后" : "前") << "半" << termcolor::reset << endl;
  //还有几个回合比赛
  {
    int turnsBeforeRace = -1;
    for (int i = turn; i < TOTAL_TURN; i++)
    {
      if (GameDatabase::AllUmas[umaId].races[i] & TURN_RACE)
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
  }
  {

    cout << endl;
  }

  //友人卡状态
  if (larc_zuoyueType == 1 || larc_zuoyueType == 2)
  {
    if (!larc_zuoyueFirstClick)
      cout << termcolor::cyan << "友人卡未点击" << termcolor::reset << endl;
    else if (larc_zuoyueOutgoingRefused)
      cout << termcolor::cyan << "友人出行已拒绝" << termcolor::reset << endl;
    else if (!larc_zuoyueOutgoingUnlocked)
      cout << termcolor::cyan << "友人出行未解锁" << termcolor::reset << endl;
    else if (larc_zuoyueOutgoingUsed < 5)
      cout << termcolor::cyan << "友人出行已走" << termcolor::yellow << larc_zuoyueOutgoingUsed << termcolor::cyan << "段" << termcolor::reset << endl;
    else if (larc_zuoyueOutgoingUsed == 5)
      cout << termcolor::cyan << "友人出行已走完" << termcolor::reset << endl;
  }

  if (turn >= 2)
  {
    cout << "第一次凯旋门允许的debuff：";
    for (int i = 0; i < 8; i++)
    {
      if (larc_allowedDebuffsFirstLarc[i])
        cout << "\033[1;32mO\033[0m";
      else
        cout << "\033[31mX\033[0m";
      if (i == 4)cout << " ";
    }
    cout << endl;
  }

  if (turn >= 2)
  {
    cout << "适性等级: ";
    for (int i = 0; i < 10; i++)
    {
      int lv = larc_levels[i];
      if (lv == 0)
        cout << "\033[31m0\033[0m";
      else if (lv == 1)
        cout << "\033[33m1\033[0m";
      else if (lv == 2)
        cout << "2";
      else if (lv == 3)
        cout << "\033[1;36m3\033[0m";
      if (i == 4)
        cout << " ";
    }
    cout << endl;
  }
  if (turn >= 2)
  {
    int qidaidu = (larc_supportPtAll + 85) / 170;
    cout << "期待度:\033[1;36m" << qidaidu / 10 << "." << qidaidu % 10 << "%\033[0m"
      << "(训练+\033[1;36m" << larc_trainBonus << "%\033[0m)   "
      << "适性pt:\033[1;36m" << larc_shixingPt << "\033[0m" << endl;

    int totalCharge = 0;
    for (int i = 0; i < 15; i++)
      totalCharge += (persons[i].larc_level - 1) * 3 + persons[i].larc_charge; // larc_level是从1开始的需要-1，如果和小黑板不一致请修改小黑板
    cout << "总格数:\033[1;36m" << totalCharge << "\033[0m   "
      << "总SS胜数:\033[1;36m" << larc_ssWin << "\033[0m   "
      << "距离上次SSS的胜数:\033[1;36m" << larc_ssWinSinceLastSSS << "\033[0m" << endl;
  }
  {
    string ganjingStr =
      motivation == 1 ? "\033[31m绝不调\033[0m" :
      motivation == 2 ? "\033[31m不调\033[0m" :
      motivation == 3 ? "\033[31m普通\033[0m" :
      motivation == 4 ? "\033[33m好调\033[0m" :
      motivation == 5 ? "\033[32m绝好调\033[0m" :
      "\033[36m未知\033[0m";


    cout << "体力|" << termcolor::green;
    for (int i = 0; i < vital / 2; i++)
      cout << "#";
    cout << termcolor::reset;
    for (int i = 0; i < maxVital / 2 - vital / 2; i++)
      cout << "-";
    cout << "| " << termcolor::green << vital << termcolor::reset << "/" << maxVital << "  干劲" << ganjingStr << endl;
  }

  if (isRacing)
  {
    cout << termcolor::red << "比赛回合" << termcolor::reset << endl;
    return;//比赛回合就不显示训练了
  }

  string divLine = "|------------------------------------------------------------------------------------|\n";
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
      else
        s = s + "(0%)";
      oneRow[i] = s;
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
  //体力
  {
    string oneRow[5];//表格中一行要显示的内容
    for (int i = 0; i < 5; i++)
    {
      int newVital = vital;
      newVital += trainValue[i][6];
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
        oneRow[i] = oneRow[i] + " (" + to_string(trainLevelCount[i] % 4) + ")";
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
  //充电量
  if (turn >= 2 && !larc_isAbroad)
  {
    string oneRow[5];//表格中一行要显示的内容
    for (int i = 0; i < 5; i++)
    {
      bool haveZuoyue = false;
      int chargeN = trainShiningNum[i] + 1;
      int totalCharge = 0;
      int totalChargeFull = 0;
      for (int j = 0; j < 5; j++)
      {
        int p = personDistribution[i][j];
        if (p < 0)break;//没人
        int personType = persons[p].personType;

        if (personType == 1)//佐岳卡
        {
          haveZuoyue = true;
        }
        else if (personType == 2|| personType == 3)//普通卡,npc
        {
          totalCharge += min(chargeN, 3 - persons[p].larc_charge);
          if (persons[p].larc_charge < 3 && persons[p].larc_charge + chargeN >= 3)
            totalChargeFull += 1;
        }
      }
      oneRow[i] = to_string(totalCharge);
      if (haveZuoyue)oneRow[i] = oneRow[i] + "+友";
      oneRow[i] = "格:\033[32m" + oneRow[i] + "\033[0m|满:\033[32m" + to_string(totalChargeFull) + "\033[0m";
    }
    printTableRow(oneRow);

  }
  if (larc_isAbroad)
  {
    string oneRow[5];//表格中一行要显示的内容
    for (int i = 0; i < 5; i++)
    {
      oneRow[i] = "适性pt:\033[32m" + to_string(larc_shixingPtGainAbroad[i]) + "\033[0m";
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
          oneRow[item] = persons[personId].getPersonStrColored(*this);
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
        oneRow[item]=s + ":" + to_string(trainValue[item][i]);
      }
      printTableRow(oneRow);
    }
  }
  cout << divLine;


  if (!larc_isAbroad && turn >= 2)
  {
    //ss人头
    int chargeFullCount = 0;
    int ssHeadIdx = 0;
    vector<string> ssHeadsStr(15);//前5个是ss里面的，之后的是满格但不在ss里的
    for (int i = 0; i < 15; i++)
    {
      if (persons[i].larc_charge != 3)
        continue;

      chargeFullCount += 1;

      bool inSS = false;
      for (int j = 0; j < 5; j++)
        if (larc_ssPersons[j] == i)
        {
          inSS = true;
          break;
        }

      string s = persons[i].getPersonStrColored(*this);
      if (inSS)
      {
        ssHeadsStr[ssHeadIdx] = s;
        ssHeadIdx += 1;
      }
      else
      {
        ssHeadsStr[4 + chargeFullCount - ssHeadIdx] = s;
      }
    
    }
    if (larc_ssPersonsCount < 5)
      cout << "SS Match" << endl;
    else if (!larc_isSSS)
      cout << "\033[1;32mSS Match\033[0m" << endl;
    else
      cout << "\033[1;36mSSS Match\033[0m" << endl;
    string oneRow[5];//表格中一行要显示的内容
    for (int i = 0; i < 5; i++)
      oneRow[i] = ssHeadsStr[i];
    printTableRow(oneRow);
    cout << endl;
    cout << "下层属性：" << " 速:" << larc_ssValue[0] << " 耐:" << larc_ssValue[1] << " 力:" << larc_ssValue[2] << " 根:" << larc_ssValue[3] << " 智:" << larc_ssValue[4] << endl;
    

    if (chargeFullCount > 5)
    {
      cout << "其他满格人头" << endl;
      for (int i = 0; i < 5; i++)
        oneRow[i] = ssHeadsStr[i + 5];
      printTableRow(oneRow);
      if(chargeFullCount > 10)
      {
        for (int i = 0; i < 5; i++)
          oneRow[i] = ssHeadsStr[i + 10];
        printTableRow(oneRow);
      }
    }

  }

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
    fiveStatusScore += GameConstants::FiveStatusFinalScore[min(fiveStatus[i], fiveStatusLimit[i])];
  int skillScore = getSkillScore();

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
  cout << termcolor::bright_red << "(直接按每pt " << (isQieZhe ? GameConstants::ScorePtRateQieZhe : GameConstants::ScorePtRate) << "分计算)" << termcolor::reset << endl;
}
