#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "../config.h"

const int TOTAL_TURN = 78;
const int MAX_INFO_PERSON_NUM = 6;//有单独信息的人头个数（此剧本只有支援卡）

class GameConstants
{
public:
  //Reference：https://github.com/mee1080/umasim/blob/main/core/src/commonMain/kotlin/io/github/mee1080/umasim/scenario/mecha/MechaStore.kt
  static const int TrainingBasicValue[5][5][7]; //TrainingBasicValue[颜色][第几种训练][LV几][速耐力根智pt体力]
  static const int FailRateBasic[5][5];//[第几种训练][LV几]，失败率= 0.025*(x0-x)^2 + 1.25*(x0-x)
  static const int BasicFiveStatusLimit[5];//初始上限，1200以上翻倍

  //各种游戏参数
  //static const int NormalRaceFiveStatusBonus;//常规比赛属性加成=3，特殊马娘特殊处理（狄杜斯等）
  //static const int NormalRacePtBonus;//常规比赛pt加成
  static const double EventProb;//每回合有EventProb概率随机一个属性以及pt +EventStrengthDefault，模拟支援卡事件
  static const int EventStrengthDefault;

  //剧本卡相关
  static const int FriendCardYayoiSSRId = 30207;//SSR秋川
  static const int FriendCardYayoiRId = 10109;//R秋川
  static const int FriendCardLianghuaSSRId = 30188;//SSR凉花
  static const int FriendCardLianghuaRId = 10104;//R凉花
  static const double FriendUnlockOutgoingProbEveryTurnLowFriendship;//每回合解锁外出的概率，羁绊小于60
  static const double FriendUnlockOutgoingProbEveryTurnHighFriendship;//每回合解锁外出的概率，羁绊大于等于60
  //static const double FriendEventProb;//友人事件概率//常数0.4写死在对应函数里了
  

  //剧本相关
  static const std::vector<int> Mecha_LinkCharas;// Link角色
  static const double Mecha_GearProb;// 无彩圈训练有齿轮的概率
  static const double Mecha_GearProbLinkBonus;// “容易获得更多的机械齿轮”是增加多大概率

  //lv提升公式。以下内容参考：
  //https://github.com/mee1080/umasim/blob/main/core/src/commonMain/kotlin/io/github/mee1080/umasim/scenario/mecha/MechaStore.kt
  //https://github.com/mee1080/umasim/blob/main/data/mecha_memo.md
  static const int Mecha_LvGainBasic[2][3][3][6]; //[是否合宿][无齿轮、齿轮、友情][主、副1、副2][人数]
  static const int Mecha_LvGainSubTrainIdx[5][3]; //Lv提升的副属性是什么 


  //评分
  static const int FiveStatusFinalScore[1200+800*2+1];//不同属性对应的评分
  static const double ScorePtRateDefault;//为了方便，直接视为每1pt对应多少分。
  static const double HintLevelPtRateDefault;//为了方便，直接视为每一级hint多少pt。
  //static const double ScorePtRateQieZhe;//为了方便，直接视为每1pt对应多少分。切者

  static bool isLinkChara(int id);
  static bool isLinkChara_initialEN(int id);
  static bool isLinkChara_moreGear(int id);
  static bool isLinkChara_initialOverdrive(int id);
  static bool isLinkChara_lvBonus(int id);
  static bool isLinkChara_initialLv(int id);

};