#pragma once
#include <string>


const int TOTAL_TURN = 78;//一共78回合
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
  static const double FriendEventProb;//友人事件概率
  static const double ZuoyueVitalBonusSSR[5];//佐岳SSR卡的回复量倍数（满破1.8）
  static const double ZuoyueVitalBonusR[5];//佐岳R卡的回复量倍数
  static const double ZuoyueStatusBonusSSR[5];//佐岳SSR卡的事件效果倍数（满破1.2）
  static const double ZuoyueStatusBonusR[5];//佐岳R卡的事件效果倍数
  

  //剧本相关
  static const int UpdateId50pEachTrain[5];//+50%分别对应第几个升级
  static const int SupportPtEvery5Percent;//每5%期待度对应多少总适性pt，=8500
  static const int LArcTrainBonusEvery5Percent[41];//期待度对应的训练加成，每5%一档

  //评分
  static const int FiveStatusFinalScore[1200+800*2+1];//不同属性对应的评分
  static const double ScorePtRate;//为了方便，直接视为每1pt对应多少分。
  static const double ScorePtRateQieZhe;//为了方便，直接视为每1pt对应多少分。切者


};