#pragma once
#include <random>
#include <array>
#include "../GameDatabase/GameDatabase.h"

struct Game
{
  //基本状态，不包括当前回合的训练信息
  int umaId;//马娘编号，见KnownUmas.cpp
  int turn;//回合数，从0开始，到77结束
  int vital;//体力，叫做“vital”是因为游戏里就这样叫的
  int maxVital;//体力上限
  bool isQieZhe;//切者
  bool isAiJiao;//爱娇
  int failureRateBias;//失败率改变量。练习上手=2，练习下手=-2
  int fiveValue[5];//五维属性，1200以上不减半
  //int fiveValueUmaBonus[5];//马娘自身加成
  int fiveValueLimit[5];//五维属性上限，1200以上不减半
  int skillPt;//技能点
  int motivation;//干劲，从1到5分别是绝不调到绝好调
  int cardId[6];//6张卡的id
  int cardJiBan[8];//羁绊，六张卡分别012345，理事长6，记者7
  int trainLevelCount[5];//五个训练的等级的计数，实际训练等级=min(5,t/12+1)
  int zhongMaBlueCount[5];//种马的蓝因子个数，假设只有3星
  int isRacing;//这个回合是否在比赛
  //bool raceTurns[TOTAL_TURN];//哪些回合是比赛 //用umaId替代，在GameDatabase::AllUmas里找

  //女神杯相关
  int venusLevelYellow;//女神等级
  int venusLevelRed;
  int venusLevelBlue;

  int venusSpiritsBottom[8];//底层碎片。8*颜色+属性。颜色012对应红蓝黄，属性123456对应速耐力根智pt。叫做“spirit”是因为游戏里就这样叫的
  int venusSpiritsUpper[4 + 2];//按顺序分别是第二层和第三层的碎片，编号与底层碎片一致。*2还是*3现场算
  int venusAvailableWisdom;//顶层的女神睿智，123分别是红蓝黄，0是没有
  bool venusIsWisdomActive;//是否正在使用睿智

  //神团卡专属
  bool venusCardFirstClick;// 是否已经点击过神团卡
  bool venusCardUnlockOutgoing;// 是否解锁外出
  bool venusCardIsQingRe;// 情热zone
  bool venusCardOutgoingUsed[5];// 用过哪些出行，依次是红黄蓝和最后两个

  //当前回合的训练信息
  //0支援卡还未分配，1支援卡分配完毕或比赛开始前，2训练结束后或比赛结束后，0检查各种固定事件与随机事件并进入下一个回合
  //stageInTurn=0时可以输入神经网络输出估值，stageInTurn=1时可以输入神经网络输出policy
  int stageInTurn;
  bool cardDistribution[5][8];//支援卡分布，六张卡分别012345，理事长6，记者7
  bool cardHint[6];//六张卡分别有没有亮红点
  int spiritDistribution[5 + 3];//碎片分布，依次是五训练01234，休息5，外出6，比赛7。若为2碎片，则加32

  //通过计算获得的信息
  int spiritBonus[6];//碎片加成
  int trainValue[5][7];//第一个数是第几个训练，第二个数依次是速耐力根智pt体力
  int failRate[5];//训练失败率

  //游戏流程:
  //newGame();
  //for (int t = 0; t < TOTAL_TURN; t++)
  //{
  //  if (!isRacing)//正常训练回合
  //  {
  //    randomDistributeCards();
  //    calculateTrainingValue();
  //    PLAYER_CHOICE;
  //    applyTraining();
  //    checkEventAfterTrain();
  //  }
  //  else//比赛回合
  //  {
  //    if(venusAvailableWisdom!=0)//是否使用女神睿智，不可使用的时候直接跳过决策步
  //    {
  //      PLAYER_CHOICE;
  //    }
  //    applyTraining();//比赛
  //    checkEventAfterTrain();
  //  }
  //}
  //finalScore();
  //



  void newGame(std::mt19937_64& rand, int newUmaId,int newCards[6],int newZhongMaBlueCount[5]);//重置游戏，开局。umaId是马娘编号
  void randomDistributeCards(std::mt19937_64& rand);//随机分配卡组和碎片
  void calculateTrainingValue();//计算所有训练分别加多少，并计算失败率

  //计算训练后的变化。其中，chosenTrain代表选择的训练，01234分别是速耐力根智，5是休息，6是外出，7是比赛。useVenusIfFull是假如女神已满，是否开启女神。chosenSpiritColor是假如出现女神三选一事件，选择的碎片颜色。chosenOutgoing是如果外出，选择的外出项目，五个神团外出分别是01234，普通外出是5。
  //注：普通回合有14种可能（5种训练，其中一种训练可能会出现女神三选一。除此以外有休息，比赛，5种出行），比赛回合只有开不开女神两种选择
  void applyTraining(std::mt19937_64& rand, int chosenTrain, bool useVenusIfFull, int chosenSpiritColor, int chosenOutgoing);
  void checkEventAfterTrain(std::mt19937_64& rand);//检查固定事件和随机事件，并进入下一个回合
  int finalScore(int chosenOutgoing) const;//最终总分

  //辅助函数
  int getTrainingLevel(int item) const;//计算训练等级。从0开始，游戏里的k级在这里是k-1级，红女神是5级
  bool isOutgoingLegal(int chosenOutgoing) const;//这个外出是否是可以进行的
  //void runTestGame();

  void getNNInput(float* buf) const;//神经网络输入

private:
  void addSpirit(std::mt19937_64& rand, int s);//添加碎片
  void activateVenusWisdom();//使用女神睿智
  void clearSpirit();//清空碎片
  void calculateFailureRate(int trainType);//计算训练失败率
  void calculateVenusSpiritsBonus();//计算碎片加成  
public:
  void calculateTrainingValueSingle(int trainType);//计算每个训练加多少
};

