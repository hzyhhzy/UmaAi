#pragma once
#include <random>
#include <array>
#include "../GameDatabase/GameDatabase.h"
#include "Person.h"
#include "Action.h"

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
  bool isPositiveThinking;//ポジティブ思考，友人第三段出行选上的buff，可以防一次掉心情
  //int cardId[6];//6张卡的id
  //int cardJiBan[8];//羁绊，六张卡分别012345，理事长6，记者7
  int16_t trainLevelCount[5];//五个训练的等级的计数，实际训练等级=min(5,t/4+1)
  int16_t zhongMaBlueCount[5];//种马的蓝因子个数，假设只有3星
  int16_t zhongMaExtraBonus[6];//种马的剧本因子以及技能白因子（等效成pt），每次继承加多少。全大师杯因子典型值大约是30速30力200pt
  //bool raceTurns[TOTAL_TURN];//哪些回合是比赛 //用umaId替代，在GameDatabase::AllUmas里找
  int normalCardCount;//速耐力根智卡的数量
  SupportCard cardParam[6];//六张卡的参数，拷贝到Game类里，一整局内不变，顺序任意。这样做的目的是训练ai时可能要随机改变卡的参数提高鲁棒性，所以每个game的卡的参数可能不一样
  int16_t saihou;//赛后加成
  Person persons[18];//如果不带其他友人团队卡，最多18个头。依次是15个可充电人头（先是支援卡（顺序随意）：0~4或5，再是npc：5或6~14），理事长15，记者16，佐岳17（带没带卡都是17）
  bool isRacing;//这个回合是否在比赛

  int motivationDropCount;//掉过几次心情了，不包括剧本事件（已知同一个掉心情不会出现多次，一共3个掉心情事件，所以之前掉过越多，之后掉的概率越低）


  //凯旋门相关

  bool larc_isAbroad;//这个回合是否在海外
  //int32_t larc_supportPt;//自己的支援pt
  int32_t larc_supportPtAll;//所有人（自己+其他人）的支援pt之和，每1700支援pt对应1%期待度
  int16_t larc_shixingPt;//适性pt
  int16_t larc_levels[10];//10个海外适性的等级，0为未解锁。顺序是游戏里从左上到右下的顺序，顺序编号在小黑板传过来的时候已经处理好了
  bool larc_isSSS;//是否为sss
  int16_t larc_ssWin;//一共多少人头的ss
  int16_t larc_ssWinSinceLastSSS;//从上次sss到现在win过几次ss（决定了下一个是sss的概率）
  bool larc_isFirstLarcWin;// 第一场凯旋门赢没赢
  bool larc_allowedDebuffsFirstLarc[9];//第一次凯旋门可以不消哪些debuff。玩家可以设置，满足则认为可以赢凯旋门

  int16_t larc_zuoyueType;//没带佐岳卡=0，带的SSR卡=1，带的R卡=2
  double larc_zuoyueVitalBonus;//佐岳卡的回复量倍数（满破1.8）
  double larc_zuoyueStatusBonus;//佐岳卡的事件效果倍数（满破1.2）
  bool larc_zuoyueFirstClick;//佐岳是否点过第一次
  bool larc_zuoyueOutgoingUnlocked;//佐岳外出解锁
  bool larc_zuoyueOutgoingRefused;//是否拒绝了佐岳外出
  int16_t larc_zuoyueOutgoingUsed;//佐岳外出走了几段了




  //当前回合的训练信息
  //0支援卡还未分配，1支援卡分配完毕或比赛开始前，2训练结束后或比赛结束后，0检查各种固定事件与随机事件并进入下一个回合
  //stageInTurn=0时可以输入神经网络输出估值，stageInTurn=1时可以输入神经网络输出policy
  int16_t stageInTurn;
  int16_t personDistribution[5][5];//每个训练有哪些人头id，personDistribution[哪个训练][第几个人头]，空人头为-1

  int16_t larc_ssPersonsCount;//ss有几个人
  int16_t larc_ssPersons[5];//ss有哪几个人
  int16_t larc_ssPersonsCountLastTurn;//上个非比赛非远征回合有几个ss人头，只用来判断这个回合是不是新的ss，用来计算sss。为了避免满10人连出两个ss时计算错误，使用ss的时候把这个置零

  //通过计算获得的信息
  int16_t trainValue[5][7];//第一个数是第几个训练，第二个数依次是速耐力根智pt体力
  int16_t failRate[5];//训练失败率
  int16_t trainShiningNum[5];//这个训练有几个彩圈
  int16_t larc_staticBonus[6];//适性升级的收益，包括前5个1级和第6个的1级3级pt+10
  int16_t larc_shixingPtGainAbroad[5];//海外训练适性pt收益
  int16_t larc_trainBonus;//期待度训练加成
  int16_t larc_ssValue[5];//ss的速耐力根智（不包括上层的属性）
  //int16_t larc_ssSpecialEffects[13];//ss的特殊buff
  //int16_t larc_ssSupportPtGain;//ss的支援pt总共加多少（自己+其他人头）
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

  void initNPCsTurn3(std::mt19937_64& rand);//第三回合初始化npc人头

  bool loadGameFromJson(std::string jsonStr);


  void randomDistributeCards(std::mt19937_64& rand);//随机分配卡组和npc
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
    Action action);
  void checkEventAfterTrain(std::mt19937_64& rand);//检查固定事件和随机事件，并进入下一个回合

  void checkFixedEvents(std::mt19937_64& rand);//每回合的固定事件，包括剧本事件和固定比赛等
  void checkSupportPtEvents(int oldSupportPt, int newSupportPt);//期待度上升事件
  void checkRandomEvents(std::mt19937_64& rand);//模拟支援卡事件和马娘事件（随机加羁绊，体力，心情，掉心情等）

  void applyTrainingAndNextTurn(
    std::mt19937_64& rand,
    Action action);//一直往后进行，直到下一次需要玩家决策

  int finalScore() const;//最终总分
  bool isEnd() const;//

  //辅助函数
  double sssProb(int ssWinSinceLastSSS) const;//出sss的概率
  int getTrainingLevel(int item) const;//计算训练等级。从0开始，游戏里的k级在这里是k-1级，远征是5级
  //void runTestGame();

  void getNNInputV1(float* buf, float targetScore, int mode) const;//神经网络输入，mode=0是value，1是policy
  void print() const;//用彩色字体显示游戏内容
  float getSkillScore() const;//技能分，输入神经网络之前也可能提前减去
  void printFinalStats() const;//显示最终结果

  void addStatus(int idx, int value);//增加属性值，并处理溢出
  void addAllStatus(int value);//同时增加五个属性值
  void addVital(int value);//增加或减少体力，并处理溢出
  void addMotivation(int value);//增加或减少心情，同时考虑“isPositiveThinking”
  void addJiBan(int idx,int value);//增加羁绊，并考虑爱娇
  void addTrainingLevelCount(int item, int value);//增加训练等级计数（每4为1级，训练+1，期待度达到某几个等级+4）
  void charge(int idx, int value);//充电
  void unlockUpgrade(int idx);//解锁某个升级
  bool tryBuyUpgrade(int idx, int level);//购买某个升级，如果买不起则返回false
  bool tryRemoveDebuffsFirstN(int n);//计算是否可以消除前n个debuff，若可以消除则消除且返回true，否则什么都不买且返回false

  int calculateFailureRate(int trainType, double failRateMultiply) const;//计算训练失败率，failRateMultiply是训练失败率乘数=(1-支援卡1的失败率下降)*(1-支援卡2的失败率下降)*...
  void calculateTrainingValueSingle(int trainType);//计算每个训练加多少
  void calculateSS();//计算ss的数值
  void runRace(int basicFiveStatusBonus, int basicPtBonus);//把比赛奖励加到属性和pt上，输入是不计赛后加成的基础值
  void runSS(std::mt19937_64& rand);//进行ss对战

  //友人卡相关事件
  void addStatusZuoyue(int idx, int value);//佐岳卡事件，增加属性值或者pt（idx=5），考虑事件加成
  void addVitalZuoyue(int value);//佐岳卡事件，增加体力，考虑回复量加成
  void handleFriendUnlock(std::mt19937_64& rand);//友人外出解锁
  void handleFriendOutgoing();//友人外出
  void handleFriendClickEvent(std::mt19937_64& rand);//友人事件（お疲れ様）
  void handleFriendFixedEvent();//友人固定事件，拜年+结算

  //显示事件
  void printEvents(std::string s) const;//用绿色字体显示事件

};

