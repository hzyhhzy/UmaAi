#include <iostream>
#include <cassert>
#include <vector>
#include <iomanip>  // for std::setw
#include <algorithm>
#include "../External/termcolor.hpp"
#include "Game.h"
using std::cout;
using std::string;
using std::endl;
using std::to_string;
using std::vector;

std::string Person::getPersonStrColored(const Game& game, int personId, int atTrain) const
{
    
  string s =
    personType == PersonType_unknown ? "未加载" :
    personType == PersonType_lianghuaCard ? "[友]凉花" :
    personType == PersonType_card ? cardParam.cardName.substr(0, 8) :
    personType == PersonType_npc ? "NPC" :
    personType == PersonType_lishizhang ? "理事长" :
    personType == PersonType_jizhe ? "记者" :
    personType == PersonType_lianghuaNonCard ? "凉花" :
    "未知";
  if (personType != PersonType_npc)
  {
    if (friendship < 100)
      s = s + ":" + to_string(friendship);
  }

  //根据闪彩等给名称加颜色
  if (personType == PersonType_lianghuaCard)
    s = "\033[32m" + s + "\033[0m"; // 友人
  else if (personType == PersonType_card)
  {
    if(game.isCardShining(personId,atTrain))
      s = "\033[1;36m" + s + "\033[0m"; //闪彩
    else if(friendship<80)
      s = "\033[33m" + s + "\033[0m"; //需要拉羁绊
  }
  else if (personType == PersonType_npc)
  {
    assert(false && "UAF has no npc");
  }
  else if (personType == PersonType_lishizhang || personType == PersonType_jizhe || personType == PersonType_lianghuaNonCard)
  {
    s = "\033[35m" + s + "\033[0m"; //理事长记者等
  }
  else
    s = "\033[31m" + s + "\033[0m"; //unknown

  //技能启发
  if (personType == PersonType_card && isHint)
    s = "\033[31m!\033[0m" + s;

  return s;
}

void Game::printEvents(string s) const
{
#ifdef PRINT_GAME_EVENT
  if (playerPrint) //这个不删是因为模拟器里可能也要进行ai计算
    cout << "\033[32m" + s + "\033[0m" << endl;
#endif
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
    if (ptScoreRate > (GameConstants::ScorePtRate + GameConstants::ScorePtRateQieZhe) * 0.5)
      cout << termcolor::bright_yellow << "有切者" << termcolor::reset << endl;
    if (isAiJiao)
      cout << termcolor::bright_yellow << "有爱娇" << termcolor::reset << endl;
  }
  {
    cout << endl;
  }
  
  //友人卡状态
  if (lianghua_type == 1 || lianghua_type == 2)
  {
    if (persons[lianghua_personId].friendOrGroupCardStage == 0)
      cout << termcolor::cyan << "友人卡未点击" << termcolor::reset << endl;
    else if (persons[lianghua_personId].friendOrGroupCardStage == 1)
      cout << termcolor::cyan << "友人出行未解锁" << termcolor::reset << endl;
    else if (lianghua_outgoingUsed < 5)
      cout << termcolor::cyan << "友人出行已走" << termcolor::yellow << lianghua_outgoingUsed << termcolor::cyan << "段" << termcolor::reset << endl;
    else if (lianghua_outgoingUsed == 5)
      cout << termcolor::cyan << "友人出行已走完" << termcolor::reset << endl;
  }

  string color_command[3][2] = { {"\033[1;36m","\033[0m"},{"\033[1;31m","\033[0m"},{"\033[1;33m","\033[0m"} };
  
  

    cout << "三种颜色五种训练的等级: \n";

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 5; ++j) {
            if(i==0)
                cout<< "\033[1;36m" << uaf_trainingLevel[i][j]<<" "<< "\033[0m"<<" ";
            if(i==1)
                cout << "\033[1;31m" << uaf_trainingLevel[i][j] << " " << "\033[0m" << " ";
            if (i == 2)
                cout << "\033[1;33m" << uaf_trainingLevel[i][j] << " " << "\033[0m" << " ";
        }
        cout << "\n";

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
      oneRow[i] = "等级：" + color_command[uaf_trainingColor[i]][0] + to_string(getTrainingLevel(i) + 1) + color_command[uaf_trainingColor[i]][1];
      /*if (getTrainingLevel(i) < 4)
        oneRow[i] = oneRow[i] + " (" + to_string(uaf_trainingLevel[uaf_trainingColor[i]][i] % 4) + ")";
        */
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
  // lv up situation
  {
    string oneRow[5];//表格中一行要显示的内容

    int lvUpTot[3] = { 0 };
    int accLvUp[5] = { 0 };

    for (int i = 0; i < 5; ++i) {

        int dlt = std::min(100 - uaf_trainingLevel[uaf_trainingColor[i]][i], (int)uaf_trainLevelGain[i]);
        accLvUp[i] = dlt;
        lvUpTot[uaf_trainingColor[i]] += dlt;
    }

    for (int i = 0; i < 5; i++) {
      oneRow[i] = "up:"+ color_command[uaf_trainingColor[i]][0]+to_string(accLvUp[i]) 
                + " tot:" + to_string(lvUpTot[uaf_trainingColor[i]]) + "\033[0m" + color_command[uaf_trainingColor[i]][1];
    }
    printTableRow(oneRow);

  }
  /*
  if (larc_isAbroad)
  {
    string oneRow[5];//表格中一行要显示的内容
    for (int i = 0; i < 5; i++)
    {
      oneRow[i] = "适性pt:\033[32m" + to_string(larc_shixingPtGainAbroad[i]) + "\033[0m";
    }
    printTableRow(oneRow);
  }
  */

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
          oneRow[item] = persons[personId].getPersonStrColored(*this, personId, item);
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
    fiveStatusScore += GameConstants::FiveStatusFinalScore[std::min(fiveStatus[i], fiveStatusLimit[i])];
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
  cout << termcolor::bright_red << "(直接按每pt " << ptScoreRate << "分计算)" << termcolor::reset << endl;
}
