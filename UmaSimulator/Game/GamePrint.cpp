#include <iostream>
#include <cassert>
#include <iomanip>  // for std::setw
#include "../External/termcolor.hpp"
#include "Game.h"
using std::cout;
using std::string;
using std::endl;
using std::to_string;

static string spiritStr(int x)
{
  if (x == 0)
    return "〇";

  bool doubled = x > 32;
  x = x % 32;
  int type = x % 8;
  int color = x / 8;
  string s;
  if (type == 1)
    s = "速";
  else if (type == 2)
    s = "耐";
  else if (type == 3)
    s = "力";
  else if (type == 4)
    s = "根";
  else if (type == 5)
    s = "智";
  else if (type == 6)
    s = "星";
  else
    s = "? ";

  if (color == 0)
    s = "\033[31m" + s + "\033[0m";
  else if (color == 1)
    s = "\033[34m" + s + "\033[0m";
  else if (color == 2)
    s = "\033[33m" + s + "\033[0m";

  if(doubled)
    s = s + "\033[32m" + "x2" + "\033[0m";
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
    int count = std::count(s.begin(), s.end(), '\033');
    //无论有多少组颜色代码，一律统一成10个，这样长度就统一了
    for (int j = 0; j < 10 - count / 2; j++)
      s = s + "\033[33m\033[0m";
    s = "| " + s;
    cout << std::setw(width + 10 * 9 - (i == 0 ? 0 : 0)) << s;
  }
  cout << "|" << endl << std::internal;
}

void Game::print() const
{

  cout<<"\033[31m-------------------------------------------------------------------------------------------\033[0m"<<endl;
  cout << termcolor::green << "回合数：" << turn + 1 << "/" << TOTAL_TURN << ", 第" << turn / 24 + 1 << "年" << (turn % 24) / 2 + 1 << "月" << (turn % 2 ? "后" : "前") << "半" << termcolor::reset << endl;
  //还有几个回合比赛
  {
    int turnsBeforeRace = -1;
    for (int i = turn; i < TOTAL_TURN; i++)
    {
      if (GameDatabase::AllUmas[umaId].races[i])
      {
        turnsBeforeRace = i - turn;
        break;
      }
    }
    cout << "距离下一场比赛还有" << termcolor::bright_yellow << turnsBeforeRace << "个回合" << termcolor::reset << endl;

  }
  cout << "体力：" << termcolor::green << vital << termcolor::reset << "/" << maxVital << endl;
  {
    string ganjingStr =
      motivation == 1 ? "\033[31m绝不调\033[0m" :
      motivation == 2 ? "\033[31m不调\033[0m" :
      motivation == 3 ? "\033[31m普通\033[0m" :
      motivation == 4 ? "\033[33m好调\033[0m" :
      motivation == 5 ? "\033[32m绝好调\033[0m" :
      "\033[36m未知\033[0m";
    cout << "干劲" << ganjingStr << endl;
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
    cout << "当前碎片组" << endl;
    for (int i = 0; i < 8; i++)
      cout << spiritStr(venusSpiritsBottom[i]) << "  ";
    cout << endl;

    cout << "  ";
    for (int i = 0; i < 4; i++)
      cout << spiritStr(venusSpiritsUpper[i]) << "      ";
    cout << endl;

    cout << "      " << spiritStr(venusSpiritsUpper[4]) << "              " << spiritStr(venusSpiritsUpper[5]);
    cout << endl;
    cout << endl;
    cout << "碎片加成：";
    for (int i = 0; i < 6; i++)
      cout << spiritBonus[i] << " ";
    cout << endl;
    if (venusAvailableWisdom > 0)
    {
      cout << "当前可使用女神睿智：";
      if (venusAvailableWisdom == 1)
        cout << termcolor::bright_red << "红" << termcolor::reset << endl;
      else if (venusAvailableWisdom == 2)
        cout << termcolor::bright_blue << "蓝" << termcolor::reset << endl;
      else if (venusAvailableWisdom == 3)
        cout << termcolor::bright_yellow << "黄" << termcolor::reset << endl;
    }


    cout << endl;
  }
  {
    cout << "女神等级：" << termcolor::yellow << venusLevelYellow << " " << termcolor::red << venusLevelRed << " " << termcolor::blue << venusLevelBlue << termcolor::reset << endl;
  }

  //女神卡状态
  if (!venusCardFirstClick)
    cout << termcolor::cyan << "女神卡未点击" << termcolor::reset << endl;
  else if (!venusCardUnlockOutgoing)
    cout << termcolor::cyan << "女神出行未解锁" << termcolor::reset << endl;
  else if (!venusCardIsQingRe)
    cout << termcolor::cyan << "女神处于非情热状态" << termcolor::reset << endl;
  else if (!venusCardIsQingRe)
    cout << termcolor::cyan << "女神处于非情热状态" << termcolor::reset << endl;
  else
    cout << termcolor::cyan << "女神情热已持续" << termcolor::yellow << venusCardQingReContinuousTurns << termcolor::cyan << "回合" << termcolor::reset << endl;


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

      int sp = spiritDistribution[i];
      if (venusSpiritsBottom[7] > 0)//满了，不显示碎片
        sp = 0;
      else if (venusSpiritsBottom[6] > 0)//最多一个碎片
        sp = sp % 32;
      if (sp != 0)
        s = s + "-" + spiritStr(sp);
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
      oneRow[i] = "等级：" + to_string(getTrainingLevel(i) + 1) + " (" + to_string(trainLevelCount[i]) + ")";
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
  //人头
  cout << divLine;
  {
    for (int head = 0; head < 8; head++)
    {
      string oneRow[5];//表格中一行要显示的内容
      bool thisRowIsNotEmpty = false;
      for (int item = 0; item < 5; item++)
      {
        //找找这个位置要不要显示人头
        int cardIdx = -1;
        int count = 0;
        for (int i = 0; i < 8; i++)
        {
          if (cardDistribution[item][i])
          {
            if (count == head)
            {
              cardIdx = i;
              break;
            }
            count++;
          }
        }
        if (cardIdx == -1)continue;

        //找到人头了
        thisRowIsNotEmpty = true;
        string s = cardIdx < 6 ? GameDatabase::AllSupportCardNames[cardId[cardIdx]]
          : cardIdx == 6 ? "理事长"
          : cardIdx == 7 ? "记者"
          : "未知";
        int jiban = cardJiBan[cardIdx];
        if (jiban != 100)
          s = s + ":" + to_string(jiban);

        assert(cardId[0] == SHENTUAN_ID && "神团卡不在第一个位置");
        if (cardIdx == 0)//神团
        {
          if (venusCardIsQingRe)
            s = "\033[32m" + s + "\033[0m";
          else
            s = "\033[35m" + s + "\033[0m";
        }
        else if (cardIdx < 6)//其他支援卡
        {
          int cardType = GameDatabase::AllSupportCards[cardId[cardIdx]].cardType;
          assert(cardType < 5 && cardType >= 0 && "第2到第6张卡必须为速耐力根智卡");
          assert(!venusIsWisdomActive && "开女神睿智是在玩家选择之后");//排除了开黄闪彩的情况
          if(jiban<80)
            s = "\033[33m" + s + "\033[0m";
          if (item == cardType)//常规闪彩
            s = "\033[36m" + s + "\033[0m";

          if (cardHint[cardIdx])//红点
            s = "\033[31m!\033[0m" + s;
        }
        else//理事长，记者
        {
          s = "\033[34m" + s + "\033[0m";
        }
        oneRow[item] = s;
      }
      if (thisRowIsNotEmpty || head < 5)
        printTableRow(oneRow);
      else break;
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
  //休息，外出，比赛的碎片
  {
    string oneRow[5];//表格中一行要显示的内容
    oneRow[0] = "出行-" + spiritStr(spiritDistribution[6]);
    oneRow[1] = "休息-" + spiritStr(spiritDistribution[5]);
    oneRow[2] = "比赛-" + spiritStr(spiritDistribution[7]);
    if (venusCardUnlockOutgoing)
    {
      oneRow[3] = "可用女神出行：";
      string s = "1 2 3 4 5";
      for (int i = 0; i < 5; i++)
        if (venusCardOutgoingUsed[i])
          s[i * 2] = 'X';
      oneRow[4] = s;
    }
    else
      oneRow[3] = "女神出行未解锁";
    printTableRow(oneRow);
  }
  cout << divLine;


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
