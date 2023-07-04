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


  //女神杯
  static const int VenusLevelTrainBonus[6];//女神等级训练加成
  static const int RedVenusLevelVitalCostDown[6];//红女神体力消耗下降比例
  static const int BlueVenusLevelHintProbBonus[6];//蓝女神事件发生概率提升
  static const int YellowVenusLevelEventBonus[6];//黄女神事件数值增加量
  static const int YellowVenusLevelContinuousEventProb[6];//黄女神连续事件发生率提升

  static const int BlueVenusRelatedStatus[5][6];//蓝女神关联属性
  static const int VenusSpiritTypeProb[8][6];//碎片属性概率，01234分别是速耐力根智，5是休息，6是外出，7是比赛



  //评分
  static const int FiveStatusFinalScore[1200+800*2+1];//不同属性对应的评分
  static const double ScorePtRate;//为了方便，直接视为每1pt对应多少分。
  static const double ScorePtRateQieZhe;//为了方便，直接视为每1pt对应多少分。切者


};