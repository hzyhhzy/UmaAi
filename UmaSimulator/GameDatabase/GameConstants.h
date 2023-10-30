#pragma once
#include <string>
#include <unordered_map>
#include "../config.h"

const int TOTAL_TURN = 67;
static const int SHENTUAN_ID = 1;//神团卡的id

class GameConstants
{
public:
  //游戏初始化
  static const int TrainingBasicValue[5][6][7]; //TrainingBasicValue[第几种训练][LV几][速耐力根智pt体力]，红女神是LV6
  static const int BasicFiveStatusLimit[5];//初始上限，1200以上翻倍

  //各种游戏参数
  static const int NormalRaceFiveStatusBonus;//常规比赛属性加成=3
  static const int NormalRacePtBonus;//常规比赛pt加成

  //剧本卡相关
  static const double FriendUnlockOutgoingProbEveryTurn;//每回合解锁外出的概率
  //static const double FriendEventProb;//友人事件概率//常数0.4写死在对应函数里了
  static const double ZuoyueVitalBonusSSR[5];//佐岳SSR卡的回复量倍数（满破1.8）
  static const double ZuoyueVitalBonusR[5];//佐岳R卡的回复量倍数
  static const double ZuoyueStatusBonusSSR[5];//佐岳SSR卡的事件效果倍数（满破1.2）
  static const double ZuoyueStatusBonusR[5];//佐岳R卡的事件效果倍数
  static const int LArcLinkCharas[7];// Link角色
  static const int LArcLinkEffect[7]; // 对应的特殊Buff
  

  //剧本相关
  static const int UpgradeId50pEachTrain[5];//+50%分别对应第几个升级
  static const int LArcTrainBonusEvery5Percent[41];//期待度对应的训练加成，每5%一档

  static const bool LArcIsRace[TOTAL_TURN];//是否为占用回合的赛程
  static const int LArcSupportPtGainEveryTurn[TOTAL_TURN];//每回合固定的supportPt增长
  static const int LArcUpgradesCostLv2[10];//lv2要多少适性pt
  static const int LArcUpgradesCostLv3[8];//lv3要多少适性pt

  static const std::string LArcSSBuffNames[13];//ss获得的项目的名称，单字带颜色


  //评分
  static const int FiveStatusFinalScore[1200+800*2+1];//不同属性对应的评分
  static const double ScorePtRate;//为了方便，直接视为每1pt对应多少分。
  static const double ScorePtRateQieZhe;//为了方便，直接视为每1pt对应多少分。切者


};