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
  int fiveStatus[5];//五维属性，1200以上不减半
  //int fiveStatusUmaBonus[5];//马娘自身加成
  int fiveStatusLimit[5];//五维属性上限，1200以上不减半
  int skillPt;//技能点
  int motivation;//干劲，从1到5分别是绝不调到绝好调
  int cardId[6];//6张卡的id
  int cardJiBan[8];//羁绊，六张卡分别012345，理事长6，记者7
  int trainLevelCount[5];//五个训练的等级的计数，实际训练等级=min(5,t/12+1)
  int zhongMaBlueCount[5];//种马的蓝因子个数，假设只有3星
  int zhongMaExtraBonus[6];//种马的剧本因子以及技能白因子（等效成pt），每次继承加多少。全大师杯因子典型值大约是30速30力200pt
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
  int venusCardQingReContinuousTurns;//女神连着情热了几个回合
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

  //随机数发生器，用于分配卡组和碎片
  //前提是得意率为常数
  std::discrete_distribution<> cardDistributionRandom[8];
  std::discrete_distribution<> venusSpiritTypeRandom[8];


  //显示相关
  bool playerPrint;//给人玩的时候，显示更多信息

  //游戏流程:
  //newGame();
  //for (int t = 0; t < TOTAL_TURN; t++)
  //{
  //  if (!isRacing)//正常训练回合
  //  {
  //    randomDistributeCards();
  //    PLAYER_CHOICE;
  //    applyTraining();
  //    checkEventAfterTrain();
  //  }
  //  else//比赛回合
  //  {
  //    randomDistributeCards();//只把stageInTurn改成1
  //    if(venusAvailableWisdom!=0)//是否使用女神睿智，不可使用的时候直接跳过决策步
  //    {
  //      PLAYER_CHOICE;
  //    }
  //    applyTraining();//这个函数只开女神，不干别的
  //    checkEventAfterTrain();//比赛加多少在这个函数里处理
  //  }
  //}
  //finalScore();
  //



  void newGame(std::mt19937_64& rand,
    bool enablePlayerPrint,
    int newUmaId,
    int newCards[6],
    int newZhongMaBlueCount[5],
    int newZhongMaExtraBonus[6]);//重置游戏，开局。umaId是马娘编号
  void randomDistributeCards(std::mt19937_64& rand);//随机分配卡组和碎片
  void calculateTrainingValue();//计算所有训练分别加多少，并计算失败率

  // 计算训练后的变化。如果不合法，则返回false，且保证不做任何修改
  // 其中，chosenTrain代表选择的训练，01234分别是速耐力根智，5是休息，6是外出，7是比赛。
  // useVenus是假如女神已满，是否开启女神。
  // chosenSpiritColor是假如出现女神三选一事件，选择的碎片颜色。红蓝黄分别012
  // chosenOutgoing是如果外出，选择的外出项目，五个神团外出分别是01234，普通外出是5。
  //注：普通回合有14种可能（5种训练，其中一种训练可能会出现女神三选一。除此以外有休息，比赛，5种出行），比赛回合只有开不开女神两种选择
  // forceThreeChoicesEvent是强制召唤三选一事件，1为强制召唤，-1为强制不召唤，0为正常（按概率召唤）。此设置仅用于ai搜索
  bool applyTraining(
    std::mt19937_64& rand, 
    int chosenTrain, 
    bool useVenus, 
    int chosenSpiritColor, 
    int chosenOutgoing,
    int forceThreeChoicesEvent = 0);
  void checkEventAfterTrain(std::mt19937_64& rand);//检查固定事件和随机事件，并进入下一个回合

  void applyTrainingAndNextTurn(
    std::mt19937_64& rand,
    int chosenTrain,
    bool useVenus,
    int chosenSpiritColor,
    int chosenOutgoing,
    int forceThreeChoicesEvent = 0);//一直往后进行，直到下一次需要玩家决策

  int finalScore() const;//最终总分
  bool isEnd() const;//

  //辅助函数
  void activateVenusWisdom();//使用女神睿智
  int getTrainingLevel(int item) const;//计算训练等级。从0开始，游戏里的k级在这里是k-1级，红女神是5级
  bool isOutgoingLegal(int chosenOutgoing) const;//这个外出是否是可以进行的
  bool isXiaHeSu() const;//是否为夏合宿
  double getThreeChoicesEventProb(bool useVenusIfFull) const;//点击三女神出事件的概率
  //void runTestGame();

  void getNNInputV1(float* buf, float targetScore, int mode) const;//神经网络输入，mode=0是value，1是policy
  void print() const;//用彩色字体显示游戏内容
  float getSkillScore() const;//技能分，输入神经网络之前也可能提前减去
  void printFinalStats() const;//显示最终结果

  void addStatus(int idx, int value);//增加属性值，并处理溢出
  void addAllStatus(int value);//增加五个属性值
  void addVital(int value);//增加体力，并处理溢出
  void addMotivation(int value);//增加心情
  void addJiBan(int idx,int value);//增加羁绊，并考虑爱娇
  void addTrainingLevelCount(int item, int value);//增加训练等级计数（每12为1级，训练+2，碎片+1，特殊比赛
  void addSpirit(std::mt19937_64& rand, int s);//添加碎片
  void clearSpirit();//清空碎片
  int calculateFailureRate(int trainType) const;//计算训练失败率
  void calculateVenusSpiritsBonus();//计算碎片加成  
  std::array<int,6> calculateBlueVenusBonus(int trainType) const;//计算开蓝女神的加成
  void runRace(int basicFiveStatusBonus, int basicPtBonus);//把比赛奖励加到属性和pt上，输入是不计赛后加成的基础值


  //一些过于复杂的事件放在这里
  void handleVenusOutgoing(int chosenOutgoing);//女神外出
  void handleVenusThreeChoicesEvent(std::mt19937_64& rand, int chosenColor);//女神三选一事件

  //显示事件
  void printEvents(std::string s) const;//用绿色字体显示事件

  void calculateTrainingValueSingle(int trainType);//计算每个训练加多少
};

