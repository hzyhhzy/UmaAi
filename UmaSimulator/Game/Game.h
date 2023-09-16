#pragma once
#include <random>
#include <array>
#include "../GameDatabase/GameDatabase.h"
#include "../Game/Person.h"

struct Game
{
  //显示相关
  bool playerPrint;//给人玩的时候，显示更多信息

  //基本状态，不包括当前回合的训练信息
  int16_t umaId;//马娘编号，见KnownUmas.cpp
  int16_t fiveStatusBonus[5];//马娘的五维属性的成长率
  int16_t turn;//回合数，从0开始，到77结束
  int16_t vital;//体力，叫做“vital”是因为游戏里就这样叫的
  int16_t maxVital;//体力上限
  bool isQieZhe;//切者
  bool isAiJiao;//爱娇
  int16_t failureRateBias;//失败率改变量。练习上手=2，练习下手=-2
  int16_t fiveStatus[5];//五维属性，1200以上不减半
  //int fiveStatusUmaBonus[5];//马娘自身加成
  int16_t fiveStatusLimit[5];//五维属性上限，1200以上不减半
  int16_t skillPt;//技能点
  int16_t skillScore;//已买技能的分数
  int16_t motivation;//干劲，从1到5分别是绝不调到绝好调
  //int cardId[6];//6张卡的id
  //int cardJiBan[8];//羁绊，六张卡分别012345，理事长6，记者7
  int16_t trainLevelCount[5];//五个训练的等级的计数，实际训练等级=min(5,t/4+1)
  int16_t zhongMaBlueCount[5];//种马的蓝因子个数，假设只有3星
  int16_t zhongMaExtraBonus[6];//种马的剧本因子以及技能白因子（等效成pt），每次继承加多少。全大师杯因子典型值大约是30速30力200pt
  //bool raceTurns[TOTAL_TURN];//哪些回合是比赛 //用umaId替代，在GameDatabase::AllUmas里找
  SupportCard cardParam[6];//六张卡的参数，参数也拷贝进来。这样做的目的是训练ai时可能要随机改变卡的参数提高鲁棒性，所以每个game的卡的参数可能不一样
  Person persons[18];//如果不带其他友人团队卡，最多18个头。依次是15个可充电人头（先是支援卡（顺序随意）：0~4或5，再是npc：5或6~14），理事长15，记者16，佐岳17（带没带卡都是17）
  bool isRacing;//这个回合是否在比赛

  int motivationDropCount;//掉过几次心情了（已知同一个掉心情不会出现多次，一共3个掉心情事件，所以之前掉过越多，之后掉的概率越低）


  //凯旋门相关

  bool larc_isAbroad;//这个回合是否在海外
  //int32_t larc_supportPt;//自己的支援pt
  int32_t larc_supportPtAll;//所有人（自己+其他人）的支援pt之和，每1700支援pt对应1%期待度
  int16_t larc_shixingPt;//适性pt
  int16_t larc_levels[10];//10个海外适性的等级，0为未解锁
  bool larc_isSSS;//是否为sss
  bool larc_ssWinSinceLastSSS;//从上次sss到现在win过几次ss（决定了下一个是sss的概率）
  bool larc_isFirstLarcWin;// 第一场凯旋门赢没赢
  bool larc_allowedDebuffsFirstLarc[3][8];//第一次凯旋门可以不消哪些debuff。玩家可以设置3种组合，满足一种即可

  int16_t larc_zuoyueType;//没带佐岳卡=0，带的SSR卡=1，带的R卡=2
  int16_t larc_zuoyueCardLevel;//佐岳卡的等级
  bool larc_zuoyueFirstClick;//佐岳是否点过第一次
  bool larc_zuoyueOutgoingUnlocked;//佐岳外出解锁
  int16_t larc_zuoyueOutgoingUsed;//佐岳外出走了几段了




  //当前回合的训练信息
  //0支援卡还未分配，1支援卡分配完毕或比赛开始前，2训练结束后或比赛结束后，0检查各种固定事件与随机事件并进入下一个回合
  //stageInTurn=0时可以输入神经网络输出估值，stageInTurn=1时可以输入神经网络输出policy
  int16_t stageInTurn;
  int16_t personDistribution[5][5];//每个训练有哪些人头id，personDistribution[哪个训练][第几个人头]，空人头为-1
  bool cardHint[6];//六张卡分别有没有亮红点
  int16_t spiritDistribution[5 + 3];//碎片分布，依次是五训练01234，休息5，外出6，比赛7。若为2碎片，则加32

  //通过计算获得的信息
  int16_t trainValue[5][7];//第一个数是第几个训练，第二个数依次是速耐力根智pt体力
  int16_t failRate[5];//训练失败率
  int16_t trainShiningNum[5];//这个训练有几个彩圈
  int16_t larc_trainBonus;//期待度训练加成
  int16_t larc_ssPersonsCount;//ss有几个人
  int16_t larc_ssPersons[5];//ss有哪几个人
  int16_t larc_ssValue[7];//ss的速耐力根智pt体力（上层的属性也算，技能换算成pt）
  int16_t larc_ssSpecialEffects[13];//ss的特殊buff（去掉与上面重复的。比如“体力+心情”在此处只考虑心情）
  int16_t larc_ssSupportPtGain;//ss的支援pt总共加多少（自己+其他人头）
  int16_t larc_ssFailRate;//ss的失败率



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
    int umaStars,
    int newCards[6],
    int newZhongMaBlueCount[5],
    int newZhongMaExtraBonus[6]);//重置游戏，开局。umaId是马娘编号
  void initHeads();//第三回合初始化npc人头

  bool loadGameFromJson(std::string jsonStr);

  void initRandomGenerators();

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

