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

std::string Person::getPersonName() const
{
  string s =
    personType == PersonType_unknown ? "未加载" :
    personType == PersonType_friendCard ? cardParam.cardName.substr(0, 8) :
    personType == PersonType_card ? cardParam.cardName.substr(0, 8) :
    personType == PersonType_npc ? "NPC" :
    personType == PersonType_yayoi ? "理事长" :
    personType == PersonType_reporter ? "记者" :
    "未知";
  return s;
}


std::string Game::getPersonStrColored(int personId, int atTrain) const
{
  if (personId < 0)
    return "Empty";

  //根据闪彩等给名称加颜色
  if (personId < 6)//card
  {
    const Person& p = persons[personId];
    string s = p.getPersonName();
    if (p.personType != PersonType_npc)
    {
      if (p.friendship < 100)
        s = s + ":" + to_string(p.friendship);
    }
    if (p.personType == PersonType_friendCard)
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

    return s;
  }
  else if (personId==PSID_npc)
  {
    //gray NPC
    return "\033[37mNPC\033[0m";
  }
  else if (personId == PSID_noncardYayoi)
  {
    //理事长
    int friendship = friendship_noncard_yayoi;
    string s = "理事长";
    if (friendship < 100)
      s = s + ":" + to_string(friendship);
    return "\033[35m" + s + "\033[0m";
  }
  else if (personId == PSID_noncardReporter)
  {
    //记者
    int friendship = friendship_noncard_reporter;
    string s = "记者";
    if (friendship < 100)
      s = s + ":" + to_string(friendship);
    return "\033[35m" + s + "\033[0m";
  }
  else
  {
    assert(false);
    return "\033[31mUnknown\033[0m";
  }

}

void Game::printEvents(string s) const
{
#ifdef PRINT_GAME_EVENT
  if (playerPrint) //这个不删是因为模拟器里可能也要进行ai计算
    cout << "\033[32m" + s + "\033[0m" << endl;
#endif
}


const int tableWidth = 17;
static void printTableRow(string strs[5])
{
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
    cout << std::setw(tableWidth + colorCodeLen) << s;
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
    else if (friend_outgoingUsed < 5)
      cout << termcolor::cyan << "友人出行已走" << termcolor::yellow << friend_outgoingUsed << termcolor::cyan << "段" << termcolor::reset << endl;
    else if (friend_outgoingUsed == 5)
      cout << termcolor::cyan << "友人出行已走完" << termcolor::reset << endl;
  }

  if (turn < 72)
  {
    int nextCompetition = (turn / 12 + 1) * 12;
    if (turn < 24)nextCompetition = 24;
    cout << "距离下次试食会还有 " << nextCompetition - turn << " 回合" << endl;
  }

  cout << "农田pt：" << termcolor::green << cook_farm_pt << termcolor::reset << " (断绿亏损=" << maxFarmPtUntilNow() - cook_farm_pt << ")" << endl;
  if (isXiahesu() || turn >= 72)
  {
    cout << termcolor::cyan << "夏合宿或Ura期间，每回合收获" << termcolor::reset << endl;
  }
  else
  {
    cout << "农田材料种类：";
    int farmCount = turn % 4;
    for (int i = 0; i < farmCount; i++)
    {
      bool isGreen = cook_harvest_green_history[i];
      int materialType = cook_harvest_history[i];
      if (isGreen)
        cout << termcolor::green << GameConstants::Cook_MaterialNames[materialType] << termcolor::reset;
      else
        cout << termcolor::yellow << GameConstants::Cook_MaterialNames[materialType] << termcolor::reset;
      cout << " ";
    }
    for (int i = 0; i < 4 - farmCount; i++)
    {
      cout << "__ ";
    }
    cout << endl;
  }

  cout << "料理pt：" << termcolor::bright_yellow << cook_dish_pt << termcolor::reset;
  if (cook_dish_sure_success)
    cout << termcolor::bright_green << "   大成功确定" << termcolor::reset;
  else
    cout << "   大成功 " << cook_dish_pt % 1500 << "/1500";
  cout << endl;
  
  cout << "生效料理：" << termcolor::bright_green << Action::dishName[cook_dish] << termcolor::reset << endl;
  
 

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
        motivation == 5 ? "\033[32m绝好调\033[0m" : "未知") << endl;
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
    bool isGreen = cook_train_green[i];
    if (isGreen)
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
      else
        s = s + "(0%)";
      oneRow[i] = s;
    }
    printTableRow(oneRow);
  }
  cout << divLine;
  // 菜量
  {
    string oneRow[5];//表格中一行要显示的内容
    for (int i = 0; i < 5; i++)
    {
      oneRow[i] = GameConstants::Cook_MaterialNames[i] + ":";
      oneRow[i] += to_string(cook_material[i]) + "/" + to_string(GameConstants::Cook_MaterialLimit[cook_farm_level[i]]);
    }
    printTableRow(oneRow);
  }
  {
    string oneRow[5];//表格中一行要显示的内容
    auto matGain = calculateHarvestNum(false);
    for (int i = 0; i < 5; i++)
    {
      bool willOverflow = cook_material[i] + matGain[i] > GameConstants::Cook_MaterialLimit[cook_farm_level[i]];
      if (willOverflow)
      {
        oneRow[i] = "\033[31m" + to_string(matGain[i]) + "\033[0m";
      }
      else
        oneRow[i] = to_string(matGain[i]);
      oneRow[i] = "+" + oneRow[i];
    }
    printTableRow(oneRow);
  }
  {
    string oneRow[5];//表格中一行要显示的内容
    for (int i = 0; i < 5; i++)
    {
      oneRow[i] = "LV " + to_string(cook_farm_level[i]);
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

  if (isRacing)
  {
    cout << divLineWhite;
    cout << "比赛菜种：" << "\033[32m" << GameConstants::Cook_MaterialNames[cook_main_race_material_type] << "\033[0m" << endl;
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
      Action action;
      action.dishType = DISH_none;
      action.train = t;
      if(!isLegal(action))
        s = "\033[31m" + s + ":__\033[0m";
      else
      {
        int matType = cook_train_material_type[t];
        s = s + ":";
        bool isGreen = cook_train_green[t];
        if (isGreen)
          s = s + "\033[32m" + GameConstants::Cook_MaterialNames[matType] + "\033[0m";
        else
          s = s + "\033[33m" + GameConstants::Cook_MaterialNames[matType] + "\033[0m";
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
  cout << termcolor::bright_red << "(直接按每pt " << ptScoreRate << "分计算)" << termcolor::reset << endl;
}
