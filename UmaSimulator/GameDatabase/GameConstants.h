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
  static const int TrainingBasicValue[5][5][7]; //TrainingBasicValue[颜色][第几种训练][LV几][速耐力根智pt体力]
  static const int FailRateBasic[5][5];//[第几种训练][LV几]，失败率= 0.025*(x0-x)^2 + 1.25*(x0-x)
  static const int BasicFiveStatusLimit[5];//初始上限，1200以上翻倍

  //各种游戏参数
  //static const int NormalRaceFiveStatusBonus;//常规比赛属性加成=3，特殊马娘特殊处理（狄杜斯等）
  //static const int NormalRacePtBonus;//常规比赛pt加成
  static const double EventProb;//每回合有EventProb概率随机一个属性以及pt +EventStrengthDefault，模拟支援卡事件
  static const int EventStrengthDefault;

  //剧本卡相关
  static const int FriendCardIdSSR = 30207;//SSR秋川
  static const int FriendCardIdR = 10109;//R秋川
  static const double FriendUnlockOutgoingProbEveryTurnLowFriendship;//每回合解锁外出的概率，羁绊小于60
  static const double FriendUnlockOutgoingProbEveryTurnHighFriendship;//每回合解锁外出的概率，羁绊大于等于60
  //static const double FriendEventProb;//友人事件概率//常数0.4写死在对应函数里了
  static const double FriendVitalBonusSSR[5];//友人SSR卡的回复量倍数（满破1.6）
  static const double FriendVitalBonusR[5];//友人R卡的回复量倍数
  static const double FriendStatusBonusSSR[5];//友人SSR卡的事件效果倍数（满破1.25）
  static const double FriendStatusBonusR[5];//友人R卡的事件效果倍数
  

  //剧本相关
  static const std::string Cook_MaterialNames[5];//料理原料名称

  static const std::vector<int> Cook_LinkCharas;// Link角色
  //static const int Cook_DishPtBounds[7];//料理pt的分档
  static int Cook_DishPtLevel(int dishPt);//料理pt的分档
  static const int Cook_DishPtTrainingBonus[8];//料理pt训练加成
  static const int Cook_DishPtSkillPtBonus[8];//料理pt技能点加成
  static const int Cook_DishPtDeyilvBonus[8];//料理pt得意率加成
  static const int Cook_DishPtBigSuccessRate[8];//料理大成功概率
  //大成功时，先按基础概率的比例选出一个buff（忽略无效buff），再对每个buff按追加概率决定是否追加
  static const int Cook_DishPtBigSuccessBuffProb[5][6];//料理大成功的buff基础概率，[料理等级][buff类型]，其中buff类型依次1体力，2心情，3羁绊，4分身，5体力上限
  static const int Cook_DishPtBigSuccessBuffExtraProb[5][6];//料理大成功的buff追加概率，[料理等级][buff类型]，其中buff类型依次1体力，2心情，3羁绊，4分身，5体力上限

  static const double Cook_RestGreenRate;//休息绿色概率
  static const double Cook_RaceGreenRate;//比赛绿色概率

  static const int Cook_DishLevel[14];//1级：第一年的两个，2级：第二年的5个，3级：第三年的5个，4级：第四年的G1Plate
  static const int Cook_DishMainTraining[14];//料理的主训练，1级和4级没有
  static const int Cook_DishGainPt[14];//料理获得的料理pt
  static const int Cook_DishCost[14][5];//料理原料消耗
  static const bool Cook_DishTrainingBonusEffective[14][5];//料理对哪些训练有加成
  static const int Cook_FarmLvCost[5];//农田升级消耗
  static const int Cook_HarvestBasic[6];//农田等级收获基础值
  static const int Cook_HarvestExtra[6];//农田等级收获追加值
  static const int Cook_MaterialLimit[6];//材料上限


  //评分
  static const int FiveStatusFinalScore[1200+800*2+1];//不同属性对应的评分
  static const double ScorePtRateDefault;//为了方便，直接视为每1pt对应多少分。
  static const double HintLevelPtRateDefault;//为了方便，直接视为每一级hint多少pt。
  //static const double ScorePtRateQieZhe;//为了方便，直接视为每1pt对应多少分。切者

  static bool isLinkChara(int id);

};