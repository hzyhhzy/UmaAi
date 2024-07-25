#pragma once
#include <random>
#include <array>
#include "../GameDatabase/GameDatabase.h"
#include "Person.h"
#include "Action.h"

struct SearchParam;


enum FarmUpgradeStrategyEnum :int16_t
{
  FUS_default,//默认（调试出来的最优）
  FUS_noGarlicLv3,//不升级Lv3大蒜
  FUS_garlicLv3,//升级Lv3大蒜
  FUS_none,//不自动升级
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
  PSID_none = -1,//未分配
  PSID_noncardYayoi = 6,//非卡理事长
  PSID_noncardReporter = 7,//非卡记者
  PSID_npc = 8//NPC
};

enum gameStageEnum :int16_t
{
  GameStage_beforeTrain = 0,//训练（或比赛）前
  GameStage_afterTrain = 1,//训练后，处理事件前
};
struct Game
{
  //显示相关
  bool playerPrint;//给人玩的时候，显示更多信息

  //参数设置

  float ptScoreRate;//每pt多少分
  float hintPtRate;//每一级hint等价多少pt
  int16_t eventStrength;//每回合有（待测）概率加这么多属性，模拟支援卡事件
  int16_t farmUpgradeStrategy;//升级农田的策略
  int16_t scoringMode;//评分方式

  //基本状态，不包括当前回合的训练信息
  int32_t umaId;//马娘编号，见KnownUmas.cpp
  bool isLinkUma;//是否为link马
  bool isRacingTurn[TOTAL_TURN];//这回合是否比赛
  int16_t fiveStatusBonus[5];//马娘的五维属性的成长率

  int16_t turn;//回合数，从0开始，到77结束
  int16_t gameStage;//游戏阶段，0是训练前，1是训练后
  int16_t vital;//体力，叫做“vital”是因为游戏里就这样叫的
  int16_t maxVital;//体力上限
  int16_t motivation;//干劲，从1到5分别是绝不调到绝好调

  int16_t fiveStatus[5];//五维属性，1200以上不减半
  int16_t fiveStatusLimit[5];//五维属性上限，1200以上不减半
  int16_t skillPt;//技能点
  int16_t skillScore;//已买技能的分数
  int16_t trainLevelCount[5];//训练等级计数，每点4下加一级

  int16_t failureRateBias;//失败率改变量。练习上手=-2，练习下手=2
  bool isQieZhe;//切者 
  bool isAiJiao;//爱娇
  bool isPositiveThinking;//ポジティブ思考，友人第三段出行选上的buff，可以防一次掉心情
  bool isRefreshMind;//+5 vital every turn

  int16_t zhongMaBlueCount[5];//种马的蓝因子个数，假设只有3星
  int16_t zhongMaExtraBonus[6];//种马的剧本因子以及技能白因子（等效成pt），每次继承加多少。全大师杯因子典型值大约是30速30力200pt
  
  bool isRacing;//这个回合是否在比赛

  int16_t friendship_noncard_yayoi;//非卡理事长羁绊
  int16_t friendship_noncard_reporter;//非卡记者羁绊

  Person persons[MAX_INFO_PERSON_NUM];//依次是6张卡。非卡理事长，记者，NPC们不单独分配person类，编号一律8
  int16_t personDistribution[5][5];//每个训练有哪些人头id，personDistribution[哪个训练][第几个人头]，空位置为-1，0~5是6张卡，非卡理事长6，记者7，NPC们编号一律8
  //int lockedTrainingId;//是否锁训练，以及锁在了哪个训练。可以先不加，等ai做完了有时间再加。

  int16_t saihou;//赛后加成

  std::discrete_distribution<> distribution_noncard;//非卡理事长/记者的分布
  std::discrete_distribution<> distribution_npc;//npc的分布

  //剧本相关--------------------------------------------------------------------------------------
  
  //状态相关
  int16_t cook_material[5];//五种菜个数
  int32_t cook_dish_pt;//料理pt
  int32_t cook_dish_pt_turn_begin;//回合刚开始（吃菜之前）的料理pt：用来检查料理pt相关的升级
  int16_t cook_farm_level[5];//五种农田的等级
  int16_t cook_farm_pt;//农田升级pt
  bool cook_dish_sure_success;//大成功确定
  int16_t cook_dish;//当前生效的菜
  int16_t cook_win_history[5];//五次试食会是否“大满足”，非大满足0，大满足1，超满足2

  //最终收获值=f(cook_harvest_green_count)*(基本收获 + cook_harvest_history*追加收获 + cook_harvest_extra)
  int16_t cook_harvest_history[4];//此4回合分别是哪4种菜，-1是还未选择
  bool cook_harvest_green_history[4];//此4回合是不是绿菜
  int16_t cook_harvest_extra[5];//每回合收获=追加收获+人头数。cook_harvest_extra是人头数累计菜量
  


  //菜量获取相关
  int16_t cook_train_material_type[8];//训练外出比赛获得的菜的种类，编号参考TrainActionTypeEnum
  bool cook_train_green[8];//训练外出比赛是否为绿圈
  int16_t cook_main_race_material_type;//比赛回合的菜的种类

  //单独处理剧本友人卡，因为接近必带。其他友人团队卡的以后再考虑
  int16_t friend_type;//0没带友人卡，1 ssr卡，2 r卡
  int16_t friend_personId;//友人卡在persons里的编号
  int16_t friend_stage;//0是未点击，1是已点击但未解锁出行，2是已解锁出行
  int16_t friend_outgoingUsed;//友人的出行已经走了几段了   暂时不考虑其他友人团队卡的出行
  double friend_vitalBonus;//友人卡的回复量倍数
  double friend_statusBonus;//友人卡的事件效果倍数





  //可以通过上面的信息计算获得的非独立的信息，每回合更新一次，不需要录入
  int16_t trainValue[5][6];//训练数值的总数（下层+上层），第一个数是第几个训练，第二个数依次是速耐力根智pt
  int16_t trainVitalChange[5];//训练后的体力变化（负的体力消耗）
  int16_t failRate[5];//训练失败率
  bool isTrainShining[5];//训练是否闪彩

  int16_t cook_dishpt_success_rate;//大成功率
  int16_t cook_dishpt_training_bonus;//料理pt训练加成
  int16_t cook_dishpt_skillpt_bonus;//料理pt技能点加成
  int16_t cook_dishpt_deyilv_bonus;//料理pt得意率加成
  int16_t cook_train_material_num_extra[8];//训练外出比赛获得的菜的个数增加（cook_harvest_extra），训练=非link卡人头数+3*link卡数，外出比赛=0


  //训练数值计算的中间变量，存下来方便手写逻辑进行估计
  int16_t trainValueLower[5][6];//训练数值的下层，第一个数是第几个训练，第二个数依次是速耐力根智pt体力
  //double trainValueCardMultiplier[5];//支援卡乘区=(1+总训练加成)(1+干劲系数*(1+总干劲加成))(1+0.05*总卡数)(1+友情1)(1+友情2)...

  //bool cardEffectCalculated;//支援卡效果是否已经计算过？吃无关菜不需要重新计算，分配卡组或者读json时需要置为false
  //CardTrainingEffect cardEffects[6];




  //游戏流程相关------------------------------------------------------------------------------------------

public:

  void newGame(std::mt19937_64& rand,
    bool enablePlayerPrint,
    int newUmaId,
    int umaStars,
    int newCards[6],
    int newZhongMaBlueCount[5],
    int newZhongMaExtraBonus[6]);//重置游戏，开局。umaId是马娘编号


  //这个操作是否允许且合理
  //不允许的包括：菜数不够、已经做菜再次做菜，ura三个比赛回合选择训练不为空
  //不合理的包括：（无法排除做某个菜点另一个训练，还在想能不能排除一部分）
  bool isLegal(Action action) const;

  //进行Action后一直往后进行，直到下一次需要玩家决策（跳过比赛回合）。如果回合数>=78则什么都不做直接return（但不要报错或者闪退）
  //注：若Action不包含训练，则吃菜但不进行训练，也不进入下一回合
  //URA期间，比赛回合也让玩家进行吃菜决策，吃完菜进入下一回合
  void applyAction(
    std::mt19937_64& rand,
    Action action);

  int finalScore() const;//最终总分
  bool isEnd() const;//是否已经终局



  //原则上这几个private就行，如果private在某些地方非常不方便那就改成public

  void autoUpgradeFarm(bool beforeXiahesu);//农田升级策略用手写逻辑处理，就不额外计算了，beforeXiahesu是夏合宿前那个回合收菜后进行升级
  void randomDistributeCards(std::mt19937_64& rand);//随机分配人头
  void calculateTrainingValue();//计算所有训练分别加多少，并计算失败率、训练等级提升等
  bool makeDish(int16_t dishId, std::mt19937_64& rand);//做菜，并处理相关收益，计算相关数值
  bool applyTraining(std::mt19937_64& rand, int train);//处理 训练/出行/比赛 本身，包括友人点击事件，不包括做菜，不包括固定事件和剧本事件。如果不合法，则返回false，且保证不做任何修改
  void checkEventAfterTrain(std::mt19937_64& rand);//检查固定事件和随机事件，并进入下一个回合

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

  int calculateRealStatusGain(int idx, int value) const;//考虑1200以上为2的倍数的实际属性增加值
  void addStatus(int idx, int value);//增加属性值，并处理溢出
  void addAllStatus(int value);//同时增加五个属性值
  void addVital(int value);//增加或减少体力，并处理溢出
  void addVitalMax(int value);//增加体力上限，限制120
  void addMotivation(int value);//增加或减少心情，同时考虑“isPositiveThinking”
  void addJiBan(int idx,int value,bool ignoreAijiao);//增加羁绊，并考虑爱娇。料理的羁绊不会变所以ignoreAijiao=true
  void addYayoiJiBan(int value);//增加理事长羁绊，剧本比赛等情况
  int getYayoiJiBan() const;//获得理事长羁绊
  void addStatusFriend(int idx, int value);//友人卡事件，增加属性值或者pt（idx=5），考虑事件加成
  void addVitalFriend(int value);//友人卡事件，增加体力，考虑回复量加成
  void runRace(int basicFiveStatusBonus, int basicPtBonus);//把比赛奖励加到属性和pt上，输入是不计赛后加成的基础值
  void addTrainingLevelCount(int trainIdx, int n);//为某个训练增加n次计数
  void checkDishPtUpgrade();//在回合后，检查料理pt是否跨段，如果跨段则更新得意率或提升训练等级
  void updateDeyilv();//在dishPt升级后，更新得意率

  int getTrainingLevel(int trainIdx) const;//计算训练等级
  int calculateFailureRate(int trainType, double failRateMultiply) const;//计算训练失败率，failRateMultiply是训练失败率乘数=(1-支援卡1的失败率下降)*(1-支援卡2的失败率下降)*...

  bool isCardShining(int personIdx, int trainIdx) const;    // 判断指定卡是否闪彩。普通卡看羁绊与所在训练，团队卡看friendOrGroupCardStage
  //bool trainShiningCount(int trainIdx) const;    // 指定训练彩圈数 //uaf不一定有用
  void calculateTrainingValueSingle(int tra);//计算每个训练加多少   //uaf剧本可能五个训练一起算比较方便

  //做菜相关
  int maxFarmPtUntilNow() const;//假如全程绿圈，获得的农田pt数减去已经升级的pt数
  bool upgradeFarm(int item);//把第item个农田升1级，失败返回false
  void addDishMaterial(int idx, int num);//增加菜材料，并处理溢出
  bool isDishLegal(int dishId) const;//此料理是否允许
  int getDishTrainingBonus(int trainIdx) const;//计算当前料理的训练加成
  int getDishRaceBonus() const;//计算当前料理的比赛加成
  void handleDishBigSuccess(int dishId, std::mt19937_64& rand);//处理料理大成功相关的事件
  std::vector<int> dishBigSuccess_getBuffs(int dishId, std::mt19937_64& rand);//料理大成功-获取有哪些buff
  void dishBigSuccess_hint(std::mt19937_64& rand);//料理大成功-技能hint
  void dishBigSuccess_invitePeople(int trainIdx, std::mt19937_64& rand);//料理大成功的分身效果：往trainIdx随机分配一个支援卡
  int turnIdxInHarvestLoop() const;//收获周期里的第几回合(turn%4)。夏合宿恒为0
  void addFarm(int type, int extra, bool isGreen);//种田，extra是人头附加，isGreen是绿圈
  std::vector<int> calculateHarvestNum(bool isAfterTrain) const;//收获五种菜的数量，以及农田pt数
  void maybeHarvest();//每4回合收菜，合宿每回合收菜，不是收菜回合就直接return
  void maybeCookingMeeting();//试食会



  //友人卡相关事件
  void handleFriendUnlock(std::mt19937_64& rand);//友人外出解锁
  void handleFriendOutgoing(std::mt19937_64& rand);//友人外出
  void handleFriendClickEvent(std::mt19937_64& rand, int atTrain);//友人点击事件（お疲れ様）
  void handleFriendFixedEvent();//友人固定事件，拜年+结算
  

  //算分
  float getSkillScore() const;//技能分，输入神经网络之前也可能提前减去


  //显示
  void printEvents(std::string s) const;//用绿色字体显示事件
  std::string getPersonStrColored(int personId, int atTrain) const;//人物名称与羁绊等整合成带颜色的字符串，在小黑板表格中显示
};

