#pragma once
#include <random>
#include <array>
#include "../GameDatabase/GameDatabase.h"
#include "Person.h"
#include "Action.h"

struct SearchParam;

struct Game;

enum LegendColorEnum :int16_t
{
  L_blue,//蓝
  L_green,//绿
  L_red,//红（粉）
  L_unknown=-1,
};

//剧本“心得”
//游戏里id是 4位数"XY0Z"里X是颜色，Y是星数，Z是第几个
//此处-1是空，57个buff用0~56表示
struct ScenarioBuffInfo
{
  int16_t buffId;
  bool isActive;
  int16_t coolTime;
  ScenarioBuffInfo();
  int16_t getBuffColor() const;
  int16_t getBuffStar() const;
  std::string getName() const;
  std::string getColoredState() const;

  static int16_t getBuffStarStatic(int16_t id);
  static std::string buffDescriptions[57];
  static std::string getScenarioBuffName(int16_t buffId);
  static bool defaultOrder(int16_t a, int16_t b);
};

//剧本加成   //，只考虑五个训练共有的部分，按人头的不算
//每次计算训练数值前都会重新计算这个
struct ScenarioBonus
{
  float hintProb;//红点概率提升%
  int16_t hintLv;//红点等级提升
  int16_t moreHint;//追加几个hint
  bool alwaysHint;//一定出现hint
  
  float vitalReduce;//体力消耗减少%
  float jibanAdd1;//羁绊增加量1（点击或hint）
  float jibanAdd2;//羁绊增加量2（只包括点击）
  float deyilv;//得意率提升%
  float disappearRateReduce;//消失率减少。原本的普通卡消失率是50，友人100。有对应buff时减少25

  float xunlian;//训练加成%
  float ganjing;//干劲加成%
  float youqing;//友情加成%

  int16_t extraHead;//追加人头的个数

  float xunlianPerHead;//每个人头额外多少训练加成
  float ganjingPerHead;//每个人头额外多少干劲加成
  float youqingPerShiningHead;//每个闪彩人头额外多少友情加成
  ScenarioBonus();
  void clear();
};

struct ScenarioBuffCondition //触发buff的各种条件
{
  bool isRest;
  bool isTraining;
  bool isYouqing;
  int16_t trainingSucceed;
  int16_t trainingHead;
  ScenarioBuffCondition();
  void clear();
};

struct GameSettings
{
  //显示相关
  bool playerPrint;//给人玩的时候，显示更多信息

  //参数设置

  float ptScoreRate;//每pt多少分
  float hintPtRate;//每一级hint等价多少pt
  float hintProbTimeConstant;//已经有t级技能hint时，令hint出技能的概率=0.9*(1-exp(-exp(2-t/T)))，其中T=hintProbTimeConstant，默认80
  int16_t eventStrength;//每回合有（待测）概率加这么多属性，模拟支援卡事件
  int16_t scoringMode;//评分方式

  int16_t color_priority;//优先选择哪种颜色，-1为无优先

  GameSettings();
};

enum scoringModeEnum :int16_t
{
  SM_normal,//普通(凹分、评价点)模式
  SM_race,//通用大赛模式
  SM_jjc,//竞技场模式
  SM_long,//长距离模式
  SM_2400m,//2400m模式
  SM_2000m,//2000m模式
  SM_mile,//英里模式
  SM_short,//短距离模式
  SM_debug //debug模式
};

enum personIdEnum :int16_t
{
  PS_none = -1,//未分配
  PS_noncardYayoi = 6,//非卡理事长
  PS_noncardReporter = 7,//非卡记者
  PS_npc = 8,//NPC（备用）
  PS_npc0 = 10,//NPC速
  PS_npc1 = 11,//NPC耐
  PS_npc2 = 12,//NPC力
  PS_npc3 = 13,//NPC根
  PS_npc4 = 14,//NPC智
};


//int16_t stage;//分配人头前1，分配人头后c2，训练后有团卡事件（选出行或三选一）3，抽取心得前4（如果有），选心得5（如果有），固定与随机事件前6，6之后进入stage1
//对于神经网络stage2可以计算policy，stage4/6可以计算value
//stage2时神经网络直接输出选择的训练
//stage3/5时，列举所有选项分别进行一步（此过程没有随机性），然后分别调用神经网络计算value，取value最高的作为选项
enum StageEnum :int16_t
{
  ST_none,
  ST_distribute,//分配人头前
  ST_train,//训练前
  ST_decideEvent,//选择团卡事件前
  ST_pickBuff,//随机抽取心得前
  ST_chooseBuff,//选心得前
  ST_event,//处理事件前
  ST_action_randomize,//仅用于action，表示ST_train或ST_chooseBuff前的随机化
};

//int16_t decidingEvent;//需要处理的含选择项的事件。选buff 1，团卡出行 2，团卡三选一 3
enum DecidingEventEnum :int16_t
{
  DecidingEvent_none,
  DecidingEvent_RESERVED,
  DecidingEvent_outing,//团卡出行
  DecidingEvent_three,//团卡三选一
};

enum TrainActionTypeEnum :int16_t
{
  T_speed = 0,
  T_stamina,
  T_power,
  T_guts,
  T_wiz,
  T_rest,
  T_outgoing, //包括合宿的“休息&外出”
  T_race, //包括生涯比赛
  T_none = -1, //此Action不训练，只做菜
  //TRA_redistributeCardsForTest = -2 //使用这个标记时，说明要randomDistributeCards，用于测试ai分数，在Search::searchSingleActionThread中使用
};


struct Action
{
  static const std::string trainingName[8];
  //static const Action Action_RedistributeCardsForTest;
  static const int MAX_ACTION_TYPE = 8;
  
  int16_t stage;//这个action是作用于哪个stage的，如果是ST_distribute、ST_pickBuff、ST_event这三个不需要做出任何选择的stage，则无视以下内容
  int16_t idx;//stage=ST_train时为训练，01234速耐力根智，5外出，6休息，7比赛。stage=ST_decideEvent和ST_chooseBuff时是选第几个
  Action();//空Action
  Action(int st);//ST_distribute、ST_pickBuff、ST_event这三个不需要做出任何选择的stage
  Action(int st, int idx);//需要做选择的stage

  //bool isActionStandard() const;
  //int toInt() const;
  std::string toString() const;
  std::string toString(const Game& game) const;
  //static Action intToAction(int i);
};

struct Game
{
  GameSettings gameSettings;//用户设置

  //基本状态，不包括当前回合的训练信息
  int32_t umaId;//马娘编号，见KnownUmas.cpp
  bool isLinkUma;//是否为link马
  bool isRacingTurn[TOTAL_TURN];//这回合是否比赛
  int16_t fiveStatusBonus[5];//马娘的五维属性的成长率

  int16_t turn;//回合数，从0开始，到77结束
  int16_t vital;//体力，叫做“vital”是因为游戏里就这样叫的
  int16_t maxVital;//体力上限
  int16_t motivation;//干劲，从1到5分别是绝不调到绝好调，超绝好调不在这里

  int16_t fiveStatus[5];//五维属性，1200以上不减半
  int16_t fiveStatusLimit[5];//五维属性上限，1200以上不减半
  int32_t skillPt;//技能点
  int32_t skillScore;//已买技能的分数
  int32_t hintSkillLvCount;//已经有多少级hint的技能了。hintSkillLvCount越多，hint出技能的概率越小，出属性的概率越大。
  int16_t trainLevelCount[5];//训练等级计数，每点4下加一级

  int16_t failureRateBias;//失败率改变量。练习上手=-2，练习下手=2
  bool isQieZhe;//切者 
  bool isAiJiao;//爱娇
  bool isPositiveThinking;//ポジティブ思考，友人第三段出行选上的buff，可以防一次掉心情
  bool isRefreshMind;//+5 vital every turn

  bool haveCatchedDoll;//是否抓过娃娃

  int16_t zhongMaBlueCount[5];//种马的蓝因子个数，假设只有3星
  int16_t zhongMaExtraBonus[6];//种马的剧本因子以及技能白因子（等效成pt），每次继承加多少。全大师杯因子典型值大约是30速30力200pt

  int16_t stage;//int16_t stage;//分配人头前1，分配人头后2，训练后有团卡事件（选出行或三选一）3，抽取心得前4（如果有），选心得5（如果有），固定与随机事件前6，6之后进入stage1
  int16_t decidingEvent;//需要处理的含选择项的事件。选buff 1，团卡出行 2，团卡三选一 3
  bool isRacing;//这个回合是否在比赛

  int16_t friendship_noncard_yayoi;//非卡理事长羁绊
  int16_t friendship_noncard_reporter;//非卡记者羁绊

  Person persons[MAX_INFO_PERSON_NUM];//依次是6张卡。非卡理事长，记者，NPC们不单独分配person类，编号一律8
  int16_t personDistribution[5][5];//每个训练有哪些人头id，personDistribution[哪个训练][第几个人头]，空位置为-1，0~5是6张卡，非卡理事长6，记者7，NPC参见personIdEnum
  //int lockedTrainingId;//是否锁训练，以及锁在了哪个训练。可以先不加，等ai做完了有时间再加。

  int16_t saihou;//赛后加成


  //剧本相关--------------------------------------------------------------------------------------
  
  int16_t lg_mainColor;//主色
  int16_t lg_gauge[3];//三种颜色的格数
  int16_t lg_trainingColor[8];//训练的gauge颜色
  //bool lg_trainingColorBoost[8];//训练的gauge是否+3


  ScenarioBuffInfo lg_buffs[10];//10个buff，空则buffId=0
  bool lg_haveBuff[57];//有哪些buff，和lg_buffs重复但是便于查找
  int16_t lg_pickedBuffsNum;//抽取到了几个buff
  int16_t lg_pickedBuffs[9];//抽取到的buff的id


  int16_t lg_blue_active;//蓝登的超绝好调
  int16_t lg_blue_remainCount;//超绝好调还剩几个回合
  int16_t lg_blue_currentStepCount;//满3格启动超绝好调
  int16_t lg_blue_canExtendCount;//还能延长几次

  bool lg_green_active;
  int16_t lg_green_continuationZoneCount;
  int16_t lg_green_currentStepCount;
  int16_t lg_green_endRate[5];

  int16_t lg_red_friendsGauge[16];//红登的羁绊条，编号和personIdEnum对应
  int16_t lg_red_friendsLv[16];//红登的等级条，编号和personIdEnum对应



  //单独处理剧本友人卡，因为接近必带。其他友人团队卡的以后再考虑
  int16_t friend_type;//0没带友人卡，1 ssr卡，2 r卡
  int16_t friend_personId;//友人卡在persons里的编号
  double friend_vitalBonus;//友人卡的回复量倍数
  double friend_statusBonus;//友人卡的事件效果倍数

  int16_t friend_stage;//0是未点击，1是已点击但未解锁出行，2是已解锁出行
  bool friend_outgoingUsed[5];//友人的出行哪几段走过了   暂时不考虑其他友人团队卡的出行
  bool friend_qingre;//团卡是否情热
  int16_t friend_qingreTurn;//团卡连续情热多少回合了





  //可以通过上面的信息计算获得的非独立的信息，每回合更新一次，不需要录入
  ScenarioBonus lg_bonus;
  int16_t trainValue[5][6];//训练数值的总数（下层+上层），第一个数是第几个训练，第二个数依次是速耐力根智pt
  int16_t trainVitalChange[5];//训练后的体力变化（负的体力消耗）
  int16_t failRate[5];//训练失败率
  int16_t trainHeadNum[5];//训练人头个数，不包括理事长记者
  int16_t trainShiningNum[5];//训练闪彩个数


  //训练数值计算的中间变量，存下来方便手写逻辑进行估计
  int16_t trainValueLower[5][6];//训练数值的下层，第一个数是第几个训练，第二个数依次是速耐力根智pt体力
  //double trainValueCardMultiplier[5];//支援卡乘区=(1+总训练加成)(1+干劲系数*(1+总干劲加成))(1+0.05*总卡数)(1+友情1)(1+友情2)...

  //bool cardEffectCalculated;//支援卡效果是否已经计算过？吃无关菜不需要重新计算，分配卡组或者读json时需要置为false
  //CardTrainingEffect cardEffects[6];

  ScenarioBuffCondition lg_buffCondition;



  //游戏流程相关------------------------------------------------------------------------------------------

public:

  void newGame(std::mt19937_64& rand,
    GameSettings settings,
    int newUmaId,
    int umaStars,
    int newCards[6],
    int newZhongMaBlueCount[5],
    int newZhongMaExtraBonus[6]);//重置游戏，开局。umaId是马娘编号


  //这个操作是否允许且合理
  bool isLegal(Action action) const;


  std::vector<Action> getAllLegalActions() const;
  //无论什么stage，都往下进行一步，若action不合法则返回false
  void applyAction(
    std::mt19937_64& rand,
    Action action); 

  void continueUntilNextDecision(  //跳过不需要玩家选择的stage，直到下次需要选择
    std::mt19937_64& rand);

  void applyActionUntilNextDecision(
      std::mt19937_64& rand,
      Action action);




  int finalScore() const;//最终总分
  int finalScore_rank() const;//评价点
  int finalScore_sum() const;//极简大赛近似：属性之和*4+技能分
  int finalScore_mile() const;//大赛评分—英里
  bool isEnd() const;//是否已经终局




  //原则上这几个private就行，如果private在某些地方非常不方便那就改成public
  void calculateScenarioBonus();//计算剧本buff的各种加成
  void randomizeTurn(std::mt19937_64& rand);//回合初的随机化，随机分配人头、随机颜色等，ST_distribute->ST_train
  void undoRandomize();//准备重新分配，ST_train->ST_distribute或ST_chooseBuff->ST_pickBuff
  void randomDistributeHeads(std::mt19937_64& rand);//随机分配人头
  void randomInviteHeads(std::mt19937_64& rand, int num);//随机摇num个人头
  void inviteOneHead(std::mt19937_64& rand, int idx);//摇personId=idx这个人头
  void calculateTrainingValue();//计算所有训练分别加多少，并计算失败率、训练等级提升等
  bool applyTraining(std::mt19937_64& rand, int16_t train);//ST_train->ST_decideEvent/ST_pickBuff/ST_event。处理 训练/出行/比赛 本身，不包括友人点击事件，不包括买buff，不包括固定事件和剧本事件。如果不合法，则返回false，且保证不做任何修改
  void updateScenarioBuffAfterTrain(int16_t trainIdx, bool trainSucceed);//更新各种心得的触发条件
  void maybeSkipPickBuffStage();//训练结束后检查是否应当进入选buff阶段。每个回合必须调用，如果不是选buff回合会直接修改stage
  void decideEvent(std::mt19937_64& rand, int16_t idx);//团卡三选一/出行选择
  void decideEvent_outing(std::mt19937_64& rand, int16_t idx);//团卡出行选择
  void decideEvent_three(std::mt19937_64& rand, int16_t idx);//团卡三选一
  void randomPickBuff(std::mt19937_64& rand);//ST_pickBuff->ST_chooseBuff，从buff(心得)池里随机抽取buff
  int pickSingleBuff(std::mt19937_64& rand, int16_t color, int16_t star);//尝试随机抽取color颜色star星数的心得，如果全被抽完则返回-1
  void chooseBuff(int16_t idx); //ST_chooseBuff->ST_event，选择第几个buff
  
  void checkEvent(std::mt19937_64& rand);//ST_chooseBuff->ST_distribute检查固定事件和随机事件，并进入下一个回合
  void checkFixedEvents(std::mt19937_64& rand);//每回合的固定事件，包括剧本事件和固定比赛和部分马娘事件等
  void checkRandomEvents(std::mt19937_64& rand);//模拟支援卡事件和随机马娘事件（随机加羁绊，体力，心情，掉心情等）

  //常用接口-----------------------------------------------------------------------------------------------

  bool loadGameFromJson(std::string jsonStr);

  //神经网络输入
  void getNNInputV1(float* buf, const SearchParam& param) const;

  void print() const;//用彩色字体显示游戏内容
  void printFinalStats() const;//显示最终结果




  //各种辅助函数与接口，可以根据需要增加或者删减-------------------------------------------------------------------------------

  inline bool isXiahesu() const //是否为夏合宿
  {
    return (turn >= 36 && turn <= 39) || (turn >= 60 && turn <= 63);
  }
  inline bool isRaceAvailable() const //是否可以额外比赛
  {
    return turn >= 13 && turn <= 71;
  }

  int calculateRealStatusGain(int value, int gain) const;//考虑1200以上为2的倍数的实际属性增加值
  void addStatus(int idx, int value);//增加属性值，并处理溢出
  void addAllStatus(int value);//同时增加五个属性值
  void addVital(int value);//增加或减少体力，并处理溢出
  void addVitalMax(int value);//增加体力上限，限制120
  void addMotivation(int value);//增加或减少心情，同时考虑“isPositiveThinking和蓝登
  void addJiBan(int idx,int value,int type);//增加羁绊，并考虑爱娇和buff，也考虑红登充电。type0是点击，type1是hint，type2是不吃任何加成的
  void addStatusFriend(int idx, int value);//友人卡事件，增加属性值或者pt（idx=5），考虑事件加成
  void addVitalFriend(int value);//友人卡事件，增加体力，考虑回复量加成
  void runRace(int basicFiveStatusBonus, int basicPtBonus);//把比赛奖励加到属性和pt上，输入是不计赛后加成的基础值
  void addTrainingLevelCount(int trainIdx, int n);//为某个训练增加n次计数
  void applyNormalTraining(std::mt19937_64& rand, int16_t train, bool success);//处理五种训练
  void addHintWithoutJiban(std::mt19937_64& rand, int idx);
  void jicheng(std::mt19937_64& rand);//第二三年的继承

  int getTrainingLevel(int trainIdx) const;//计算训练等级
  int calculateFailureRate(int trainType, double failRateMultiply) const;//计算训练失败率，failRateMultiply是训练失败率乘数=(1-支援卡1的失败率下降)*(1-支援卡2的失败率下降)*...

  bool isCardShining(int personIdx, int trainIdx) const;    // 判断指定卡是否闪彩。普通卡看羁绊与所在训练，团队卡看friendOrGroupCardStage
  //bool trainShiningCount(int trainIdx) const;    // 指定训练彩圈数 //uaf不一定有用
  void calculateTrainingValueSingle(int tra);//计算每个训练加多少   //uaf剧本可能五个训练一起算比较方便

  //剧本相关
  void addScenarioBuffBonus(int idx);//添加剧本心得加成到lg_bonus，包含判断部分buff的生效条件（干劲绝好调等）。“训练成功”之类的判定不在这里
  void updateScenarioBuffCondition(int idx);//更新各种心得的触发条件
  void addLgGauge(int16_t color, int num);//给color加num格，去掉大于8溢出部分
  void setMainColorTurn36(std::mt19937_64& rand);//36回合时确定主色，color_priority不为空时强制指定这个颜色，但如果原颜色与指定颜色不同则扣3000分
  void updateLgGreenStatus(std::mt19937_64& rand, int trainIdx, bool trainSucceed);//训练结束后更新绿登状态
  void endLgGreenChallenge(int turnCount, bool succeed);//处理挑战结束的收益，通过其他方式终止succeed是true，训练终止是false
  void updateLgBlueStatus();//回合末更新蓝登状态
  void calculateLgGreenEndRate();

  //友人卡相关事件
  void handleFriendUnlock(std::mt19937_64& rand);//友人外出解锁
  void handleOutgoing(std::mt19937_64& rand);//外出
  void handleFriendClickEvent(std::mt19937_64& rand, int atTrain);//友人点击事件
  void handleFriendFixedEvent();//友人固定事件，拜年+结算

  void runNormalOutgoing(std::mt19937_64& rand);//常规外出
  void runFriendOutgoing(std::mt19937_64& rand, int idx, int subIdx);//友人外出
  void runFriendClickEvent(std::mt19937_64& rand, int idx);//友人点击事件
  

  //算分
  float getSkillScore() const;//技能分，输入神经网络之前也可能提前减去


  //显示
  void printEvents(std::string s) const;//用绿色字体显示事件
  std::string getPersonStrColored(int personId, int atTrain) const;//人物名称与羁绊等整合成带颜色的字符串，在小黑板表格中显示
};

