#include <iostream>
#include <cassert>
#include "Game.h"
using namespace std;
static bool randBool(mt19937_64& rand, double p)
{
  return rand() % 65536 < p * 65536;
}

//尽量与Game类的顺序一致
void Game::newGame(mt19937_64& rand, bool enablePlayerPrint, int newUmaId, int umaStars, int newCards[6], int newZhongMaBlueCount[5], int newZhongMaExtraBonus[6])
{
  playerPrint = enablePlayerPrint;
  ptScoreRate = GameConstants::ScorePtRateDefault;
  hintPtRate = GameConstants::HintLevelPtRateDefault;
  eventStrength = GameConstants::EventStrengthDefault;
  farmUpgradeStrategy = FUS_default;
  scoringMode = SM_normal;

  umaId = newUmaId;
  isLinkUma = GameConstants::isLinkChara(umaId);
  if (!GameDatabase::AllUmas.count(umaId))
  {
    throw "ERROR Unknown character. Updating database is required.";
  }
  for (int i = 0; i < TOTAL_TURN; i++)
    isRacingTurn[i] = GameDatabase::AllUmas[umaId].races[i] == TURN_RACE;
  assert(isRacingTurn[11] == true);//出道赛
  isRacingTurn[TOTAL_TURN - 5] = true;//ura1
  isRacingTurn[TOTAL_TURN - 3] = true;//ura2
  isRacingTurn[TOTAL_TURN - 1] = true;//ura3
  isUraRace = false;

  for (int i = 0; i < 5; i++)
    fiveStatusBonus[i] = GameDatabase::AllUmas[umaId].fiveStatusBonus[i];

  turn = 0;
  vital = 100;
  maxVital = 100;
  motivation = 3;

  for (int i = 0; i < 5; i++)
    fiveStatus[i] = GameDatabase::AllUmas[umaId].fiveStatusInitial[i] - 10 * (5 - umaStars); //赛马娘初始值
  for (int i = 0; i < 5; i++)
    fiveStatusLimit[i] = GameConstants::BasicFiveStatusLimit[i]; //原始属性上限

  skillPt = 120;
  skillScore = umaStars >= 3 ? 170 * (umaStars - 2) : 120 * (umaStars);//固有技能

  for (int i = 0; i < 4; i++)
  {
    trainLevelCount[i] = 0;
  }

  failureRateBias = 0;
  isQieZhe = false;
  isAiJiao = false;
  isPositiveThinking = false;
  isRefreshMind = false;

  for (int i = 0; i < 5; i++)
    zhongMaBlueCount[i] = newZhongMaBlueCount[i];
  for (int i = 0; i < 6; i++)
    zhongMaExtraBonus[i] = newZhongMaExtraBonus[i];

  for (int i = 0; i < 5; i++)
    fiveStatusLimit[i] += int(zhongMaBlueCount[i] * 5.34 * 2); //属性上限--种马基础值
  for (int i = 0; i < 5; i++)
    addStatus(i, zhongMaBlueCount[i] * 7); //种马

  isRacing = false;

  friendship_noncard_yayoi = 0;
  friendship_noncard_reporter = 0;

  for (int i = 0; i < MAX_INFO_PERSON_NUM; i++)
  {
    persons[i] = Person();
  }

  saihou = 0;
  friend_type = 0;
  friend_personId = PSID_none;
  friend_stage = 0;
  friend_outgoingUsed = 0;
  friend_vitalBonus = 1.0;
  friend_statusBonus = 1.0;
  for (int i = 0; i < 6; i++)
  {
    int cardId = newCards[i];
    persons[i].setCard(cardId);
    saihou += persons[i].cardParam.saiHou;

    if (persons[i].personType == 1)
    {
      friend_personId = i;
      bool isSSR = cardId > 300000;
      if (isSSR)
        friend_type = 1;
      else
        friend_type = 2;
      int friendLevel = cardId % 10;
      if (friend_type ==1)
      {
        friend_vitalBonus = GameConstants::FriendVitalBonusSSR[friendLevel];
        friend_statusBonus = GameConstants::FriendStatusBonusSSR[friendLevel];
      }
      else
      {
        friend_vitalBonus = GameConstants::FriendVitalBonusR[friendLevel];
        friend_statusBonus = GameConstants::FriendStatusBonusR[friendLevel];
      }
      friend_vitalBonus += 1e-10;
      friend_statusBonus += 1e-10;//加个小量，避免因为舍入误差而算错
    }
  }

  std::vector<int> probs = { 100,100,100,100,100,200 }; //速耐力根智鸽
  distribution_noncard = std::discrete_distribution<>(probs.begin(), probs.end());
  std::vector<int> probs = { 100,100,100,100,100,100 }; //速耐力根智鸽
  distribution_npc = std::discrete_distribution<>(probs.begin(), probs.end());

  for (int i = 0; i < 6; i++)//支援卡初始加成
  {
    for (int j = 0; j < 5; j++)
      addStatus(j, persons[i].cardParam.initialBonus[j]);
    skillPt += persons[i].cardParam.initialBonus[5];
  }




  for (int i = 0; i < 5; i++)
  {
    if (isLinkUma)
      cook_material[i] = 75;
    else
      cook_material[i] = 50;
  }
  cook_dish_pt = 0;
  cook_dish_pt_turn_begin = 0;
  for (int i = 0; i < 5; i++)
    cook_farm_level[i] = 1;
  cook_farm_pt = 0;
  cook_dish_sure_success = false;
  cook_dish = 0;
  for (int i = 0; i < 5; i++)
    cook_win_history[i] = 0;

  for (int i = 0; i < 4; i++)
    cook_harvest_history[i] = -1;
  for (int i = 0; i < 5; i++)
    cook_harvest_extra[i] = 0;
  cook_harvest_green_count = 0;

  for (int i = 0; i < 8; i++)
  {
    cook_train_material_type[i] = -1;
    cook_train_green[i] = false;
  }

  updateDishPt(-1, 0);//初始化料理pt

  randomDistributeCards(rand); //随机分配卡组，包括计算属性
  
}

void Game::randomDistributeCards(std::mt19937_64& rand)
{
  //比赛回合的人头分配，不需要置零，因为不输入神经网络
  if (isRacing)
    return;//比赛不用分配卡组
  
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 5; j++)
      personDistribution[i][j] = -1;

  int headN[5] = { 0,0,0,0,0 };
  vector<int8_t> buckets[5];
  for (int i = 0; i < 5; i++)
    buckets[i].clear();
  //先放友人/理事长/记者
  for (int i = 0; i < 6 + 2; i++)
  {
    int atTrain = 5;
    if (friend_type != 0 && i == friend_personId)
    {
      //友人卡
      atTrain = persons[i].distribution(rand);
    }
    else if (i == PSID_noncardYayoi && friend_type == 0)//非卡理事长
    {
      atTrain = distribution_noncard(rand);
    }
    else if (i == PSID_noncardReporter)//记者
    {
      if (turn < 12 || isXiahesu())//记者第13回合来，夏合宿也不在
        continue;
      atTrain = distribution_noncard(rand);
    }

    if (atTrain < 5)
    {
      buckets[atTrain].push_back(i);
    }
    
  }
  for (int i = 0; i < 5; i++)
  {
    if (buckets[i].size() == 1)
    {
      personDistribution[i][0] = buckets[i][0];
      headN[i] += 1;
    }
    else if (buckets[i].size() > 1)//随机选一个人头
    {
      personDistribution[i][0] = buckets[i][rand() % buckets[i].size()];
      headN[i] += 1;
    }
    buckets[i].clear();
  }

  //然后是普通支援卡
  for (int i = 0; i < 6; i++)
  {
    Person& p = persons[i];
    if (p.personType == PersonType_card)
    {
      int atTrain = p.distribution(rand);
      if (atTrain < 5)
      {
        buckets[atTrain].push_back(i);
      }
    }
  }

  //npc
  int npcCount = friend_type == 0 ? 6 : 7;//算上支援卡一共12个
  for (int i = 0; i < npcCount; i++)
  {
    int atTrain = distribution_npc(rand);
    if (atTrain < 5)
    {
      buckets[atTrain].push_back(PSID_npc);
    }
  }

  //选出不超过5个人头
  for (int i = 0; i < 5; i++)
  {
    int maxHead = 5 - headN[i];
    if (buckets[i].size() <= maxHead)
    {
      for (int j = 0; j < buckets[i].size(); j++)
      {
        personDistribution[i][headN[i]] = buckets[i][j];
        headN[i] += 1;
      }
    }
    else//总人数超过5了，随机选maxHead个
    {
      for (int j = 0; j < maxHead; j++)
      {
        int idx = rand() % (buckets[i].size() - j);

        int count = 0;
        for (int k = 0; k < buckets[i].size(); k++)
        {
          if (buckets[i][k] != -1)
          {
            if (idx == count)
            {
              personDistribution[i][headN[i]] = buckets[i][k];
              buckets[i][k] = -1;
              headN[i] += 1;
              break;
            }
            else
              count++;
          }
        }
      }
      assert(headN[i] == 5);
    }
  }

  //是否有hint
  for (int i = 0; i < 6; i++)
    persons[i].isHint = false;

  for (int t = 0; t < 5; t++)
  {
    for (int h = 0; h < 5; h++)
    {
      int pid = personDistribution[t][h];
      if (pid < 0)break;

      if (persons[pid].personType == PersonType_card)
      {
        double hintProb = 0.06 * (1 + 0.01 * persons[pid].cardParam.hintProbIncrease);
        persons[pid].isHint = randBool(rand, hintProb);
        
      }
    }
  }

  //休息外出比赛：随机菜种，随机绿圈
  //休息&外出
  int restMaterialType = rand() % 5;
  bool restGreen = randBool(rand, GameConstants::Cook_RestGreenRate);
  cook_train_material_type[TRA_rest] = restMaterialType;
  cook_train_material_type[TRA_outgoing] = restMaterialType;
  cook_train_green[TRA_rest] = restGreen;
  cook_train_green[TRA_outgoing] = restGreen;

  //比赛
  int raceMaterialType = rand() % 5;
  bool raceGreen = randBool(rand, GameConstants::Cook_RaceGreenRate);
  cook_train_material_type[TRA_race] = raceMaterialType;
  cook_train_green[TRA_race] = raceGreen;

  //训练的绿圈在calculateTrainingValue里计算

  calculateTrainingValue();
}

//小写为一个数，大写为向量（6项，速耐力根智pt）
//k = 人头 * 训练 * 干劲 * 友情    //支援卡倍率
//B = 训练基础值 + 支援卡加成  //基础值
//C = link基础值加成，lv12345是当前属性加11223，pt均为加1
//G = 马娘成长率

//L = k * B * G //下层，先不取整
//P1 = L + k * C
//P2 = P1 * (1 + 红buff倍率)，红buff倍率只乘在当前训练的主属性上
//P3 = P2 * (1 + link数加成 + 大会优胜数加成)
//总数T = P3 + 蓝buff
//上层U = T - L

void Game::calculateTrainingValue()
{


  //剧本训练加成
  int cookDishLevel = GameConstants::Cook_DishPtLevel(cook_dish_pt);
  cook_dishpt_success_rate = GameConstants::Cook_DishPtBigSuccessRate[cookDishLevel];
  cook_dishpt_training_bonus = GameConstants::Cook_DishPtTrainingBonus[cookDishLevel];
  cook_dishpt_skillpt_bonus = GameConstants::Cook_DishPtSkillPtBonus[cookDishLevel];
  cook_dishpt_deyilv_bonus = GameConstants::Cook_DishPtDeyilvBonus[cookDishLevel];
  
  for (int i = 0; i < 8; i++)
    cook_train_material_num_extra[i] = 0;

  for (int i = 0; i < 5; i++)
    calculateTrainingValueSingle(i);
}
int Game::calculateRealStatusGain(int idx, int value) const//考虑1200以上为2的倍数的实际属性增加值
{
  if (idx == 5)return value;
  int newValue = fiveStatus[idx] + value;
  if (newValue <= 1200)return value;
  if (value == 1)return 2;
  return (newValue / 2) * 2 - fiveStatus[idx];
}
void Game::addStatus(int idx, int value)
{
  int t = fiveStatus[idx] + value;
  
  if (t > fiveStatusLimit[idx])
    t = fiveStatusLimit[idx];
  if (t < 1)
    t = 1;
  if (idx < 5 && t > 1200)
    t = (t / 2) * 2;
  fiveStatus[idx] = t;
}
void Game::addVital(int value)
{
  vital += value;
  if (vital > maxVital)
    vital = maxVital;
  if (vital < 0)
    vital = 0;
}
void Game::addMotivation(int value)
{
  if (value < 0)
  {
    if (isPositiveThinking)
      isPositiveThinking = false;
    else
    {
      motivation += value;
      if (motivation < 1)
        motivation = 1;
    }
  }
  else
  {
    motivation += value;
    if (motivation > 5)
      motivation = 5;
  }
}
void Game::addJiBan(int idx, int value)
{
  auto& p = persons[idx];
  if (p.personType == 1 || p.personType == 2)
  {
    if (isAiJiao)value += 2;
  }
  else if (p.personType == 4 || p.personType == 5 || p.personType == 6)
  {
    //不变
  }
  else //npc
    value = 0;
  p.friendship += value;
  if (p.friendship > 100)p.friendship = 100;
}
void Game::addAllStatus(int value)
{
  for (int i = 0; i < 5; i++)addStatus(i, value);
}
int Game::calculateFailureRate(int trainType, double failRateMultiply) const
{
  //粗略拟合的训练失败率，二次函数 A*(x0-x)^2+B*(x0-x)
  //误差应该在2%以内
  static const double A = 0.025;
  static const double B = 1.25;
  double x0 = 0.1 * GameConstants::FailRateBasic;
  
  double f = 0;
  double dif = x0 - vital;
  if (dif > 0)
  {
    f = A * dif * dif + B * dif;
  }
  if (f < 0)f = 0;
  if (f > 99)f = 99;//无练习下手，失败率最高99%
  f *= failRateMultiply;//支援卡的训练失败率下降词条
  int fr = round(f);
  fr += failureRateBias;
  if (fr < 0)fr = 0;
  if (fr > 100)fr = 100;
  return fr;
}
int Game::uaf_competitionFinishedNum() const
{
  if (turn < 24)
    return 0;
  else if (turn < 36)
    return 1;
  else if (turn < 48)
    return 2;
  else if (turn < 60)
    return 3;
  else if (turn < 72)
    return 4;
  return 5;
}
bool Game::isXiangtanLegal(int x) const
{
  if (x == XT_none)return true;
  if (Action::XiangtanNumCost[x] > uaf_xiangtanRemain)return false;//相谈次数不够
  bool haveColor[3] = { false,false,false };
  for (int i = 0; i < 5; i++)
    haveColor[uaf_trainingColor[i]] = true;
  if (x <= 6)//一次相谈
  {
    if (!haveColor[Action::XiangtanFromColor[x]])
      return false;
    else return true;
  }
  else if (x == XT_b)
    return haveColor[1] && haveColor[2];
  else if (x == XT_r)
    return haveColor[0] && haveColor[2];
  else if (x == XT_y)
    return haveColor[0] && haveColor[1];
  else
    assert(false);
  return false;
}
void Game::xiangtanAndRecalculate(int x, bool forHandwrittenLogic)
{
  if (x == 0)return;
  int targetC = Action::XiangtanToColor[x];
  int sourceC = Action::XiangtanFromColor[x];
  for (int i = 0; i < 5; i++)
  {
    if (sourceC == -1 || sourceC == uaf_trainingColor[i])
      uaf_trainingColor[i] = targetC;
  }
  uaf_xiangtanRemain -= Action::XiangtanNumCost[x];
  assert(uaf_xiangtanRemain >= 0);
  if (!forHandwrittenLogic)
    cardEffectCalculated = false;//保险起见，重新计算，比如速司机之类的固有可能会改变
  calculateTrainingValue();
}
void Game::runRace(int basicFiveStatusBonus, int basicPtBonus)
{
  double raceMultiply = 1 + 0.01 * saihou;
  int fiveStatusBonus = floor(raceMultiply * basicFiveStatusBonus);
  int ptBonus = floor(raceMultiply * basicPtBonus);
  addAllStatus(fiveStatusBonus);
  skillPt += basicPtBonus;
}

void Game::addStatusFriend(int idx, int value)
{
  value = int(value * lianghua_statusBonus);
  if (idx == 5)skillPt += value;
  else addStatus(idx, value);
}

void Game::addVitalFriend(int value)
{
  value = int(value * lianghua_vitalBonus);
  addVital(value);
}


void Game::handleFriendOutgoing(std::mt19937_64& rand)
{
  assert(lianghua_type!=0 && persons[lianghua_personId].friendOrGroupCardStage>=2 && lianghua_outgoingUsed < 5);
  int pid = lianghua_personId;
  if (lianghua_outgoingUsed == 0)
  {
    addVitalFriend(35);
    addMotivation(1);
    addStatusFriend(0, 15);
    addJiBan(pid, 5);
  }
  else if (lianghua_outgoingUsed == 1)
  {
    addVitalFriend(30);
    addMotivation(1);
    addStatusFriend(0, 10);
    addStatusFriend(4, 10);
    addJiBan(pid, 5);
  }
  else if (lianghua_outgoingUsed == 2)
  {
    addVitalFriend(50);
    addMotivation(1);
    addJiBan(pid, 5);
  }
  else if (lianghua_outgoingUsed == 3)
  {
    addVitalFriend(30);
    addMotivation(1);
    addStatusFriend(0, 25);
    addJiBan(pid, 5);
  }
  else if (lianghua_outgoingUsed == 4)
  {
    //有大成功和成功
    if (rand() % 4 != 0)//粗略估计，75%大成功
    {
      addVitalFriend(40);
      addMotivation(1);
      addStatusFriend(0, 30);
      addJiBan(pid, 5);
      skillPt += 72;//金技能等价
    }
    else
    {
      addVitalFriend(35);
      addStatusFriend(0, 15);
      addJiBan(pid, 5);
      skillPt += 40;//金技能等价
    }
  }
  else assert(false && "未知的出行");

  //全体等级+1
  for(int i=0;i<3;i++)
    for (int j = 0; j < 5; j++)
    {
      uaf_trainingLevel[i][j] += 1;
      if (uaf_trainingLevel[i][j] > 100)
        uaf_trainingLevel[i][j] = 100;
    }

  lianghua_outgoingUsed += 1;
}
void Game::handleFriendUnlock(std::mt19937_64& rand)
{
  printEvents("友人外出解锁！");
  maxVital += 4;
  if (maxVital > 120)maxVital = 120;
  addVitalFriend(20);
  addMotivation(1);
  addJiBan(lianghua_personId, 5);
  persons[lianghua_personId].friendOrGroupCardStage = 2;
}
void Game::handleFriendClickEvent(std::mt19937_64& rand, int atTrain)
{
  assert(persons[lianghua_personId].personType == PersonType_lianghuaCard);
  if (persons[lianghua_personId].friendOrGroupCardStage == 0)
  {
    printEvents("第一次点友人"); 
    persons[lianghua_personId].friendOrGroupCardStage = 1;
    maxVital += 4;
    if (maxVital > 120)maxVital = 120;
    addStatusFriend(0, 5);
    addStatusFriend(4, 5);
    addJiBan(lianghua_personId, 10);
    addMotivation(1);
  }
  else
  {
    if (rand() % 5 < 3)return;//40%概率出事件，60%概率不出
    addJiBan(lianghua_personId, 5);
    addVitalFriend(7);
    if (rand() % 10 == 0)
    {
      if (motivation != 5)
        printEvents("友人事件心情+1");
      addMotivation(1);//10%概率加心情
    }

  }

}
void Game::handleFriendFixedEvent()
{
  if (lianghua_type == 0)return;//没友人卡
  if (persons[lianghua_personId].friendOrGroupCardStage < 2)return;//出行没解锁就没事件
  if (turn == 23)
  {
    maxVital += 4;
    if (maxVital > 120)maxVital = 120;
    addVitalFriend(10);
    addMotivation(1);
    addStatusFriend(0, 10);
    addJiBan(lianghua_personId, 5);
    skillPt += 40;//三级末脚，考虑到最后会给全身全灵，而且有进化，因此这个hint是有效的
  }
  else if (turn == 77)
  {
    if (lianghua_outgoingUsed >= 5)//走完出行
    {
      addStatusFriend(0, 30);
      addStatusFriend(4, 30);
      addStatusFriend(5, 45);
    }
    else
    {
      addStatusFriend(0, 25);
      addStatusFriend(4, 25);
      addStatusFriend(5, 35);
    }

  }
  else
  {
    assert(false && "其他回合没有友人固定事件");
  }
}
void Game::checkLianghuaGuyou()
{
  if (lianghua_type == 1)
  {
    if ((!lianghua_guyouEffective) && persons[lianghua_personId].friendship >= 60)
    {
      lianghua_guyouEffective = true;
      for (int card = 0; card < 6; card++)
      {
        Person& p = persons[card];
        auto probs = p.distribution.probabilities();
        probs[5] /= 2;
        p.distribution = std::discrete_distribution<>(probs.begin(), probs.end());
      }
    }
  }
}
bool Game::applyTraining(std::mt19937_64& rand, Action action)
{
  if (isRacing)
  {
    assert(false && "所有剧本比赛都在checkEventAfterTrain()里处理，不能applyTraining");
    uaf_lastTurnNotTrain = true;
    return false;//所有剧本比赛都在checkEventAfterTrain()里处理（相当于比赛回合直接跳过），不在这个函数
  }
  if (action.train == TRA_rest)//休息
  {
    if (isXiahesu())
    {
      addVital(40);
      addMotivation(1);
    }
    else
    {
      int r = rand() % 100;
      if (r < 25)
        addVital(70);
      else if (r < 82)
        addVital(50);
      else
        addVital(30);
    }
    uaf_lastTurnNotTrain = true;
  }
  else if (action.train == TRA_race)//比赛
  {
    if (turn <= 12 || turn >= 72)
    {
      printEvents("Cannot race now."); 
      return false;
    }
    addAllStatus(1);//武者振
    runRace(2, 40);//粗略的近似

    //扣体固定15
    addVital(-15);
    if (rand() % 10 == 0)
      addMotivation(1);
    uaf_lastTurnNotTrain = true;
  }
  else if (action.train == TRA_outgoing)//外出
  {
    if (isXiahesu())
    {
      printEvents("夏合宿只能休息，不能外出");
      return false;
    }

    //凉花出行
    if (lianghua_type != 0 &&  //带了凉花卡
      persons[lianghua_personId].friendOrGroupCardStage >= 2 &&  //已解锁外出
      lianghua_outgoingUsed < 5  //外出没走完
      )
    {
      handleFriendOutgoing(rand);
    }
    else //普通出行
    {
      //懒得查概率了，就50%加2心情，50%加1心情10体力
      if (rand() % 2)
        addMotivation(2);
      else
      {
        addMotivation(1);
        addVital(10);
      }
    }
    uaf_lastTurnNotTrain = true;
  }
  else if (action.train <= 4 && action.train >= 0)//常规训练
  {
    if (action.xiangtanType != XT_none)
      xiangtanAndRecalculate(action.xiangtanType, false);

    if (rand() % 100 < failRate[action.train])//训练失败
    {
      if (failRate[action.train] >= 20 && (rand() % 100 < failRate[action.train]))//训练大失败，概率是瞎猜的
      {
        printEvents("训练大失败！");
        addStatus(action.train, -10);
        if (fiveStatus[action.train] > 1200)
          addStatus(action.train, -10);//游戏里1200以上扣属性不折半，在此模拟器里对应1200以上翻倍
        //随机扣2个10，不妨改成全属性-4降低随机性
        for (int i = 0; i < 5; i++)
        {
          addStatus(i, -4);
          if (fiveStatus[i] > 1200)
            addStatus(i, -4);//游戏里1200以上扣属性不折半，在此模拟器里对应1200以上翻倍
        }
        addMotivation(-3);
        addVital(10);
      }
      else//小失败
      {
        printEvents("训练小失败！");
        addStatus(action.train, -5);
        if (fiveStatus[action.train] > 1200)
          addStatus(action.train, -5);//游戏里1200以上扣属性不折半，在此模拟器里对应1200以上翻倍
        addMotivation(-1);
      }
    }
    else
    {
      //先加上训练值
      for (int i = 0; i < 5; i++)
        addStatus(i, trainValue[action.train][i]);
      skillPt += trainValue[action.train][5];
      addVital(trainVitalChange[action.train]);

      vector<int> hintCards;//有哪几个卡出红感叹号了
      bool clickFriend = false;//这个训练有没有友人
      for (int i = 0; i < 5; i++)
      {
        int p = personDistribution[action.train][i];
        if (p < 0)break;//没人
        int personType = persons[p].personType;

        if (personType == PersonType_lianghuaCard)//友人卡
        {
          addJiBan(p, 4);
          clickFriend = true;
        }
        else if (personType== PersonType_card)//普通卡
        {
          addJiBan(p, 7);
          if(persons[p].isHint)
            hintCards.push_back(p);
        }
        else if (personType == PersonType_npc)//npc
        {
          assert(false);
        }
        else if (personType == PersonType_lishizhang)//理事长
        {
          int jiban = persons[p].friendship;
          int g = jiban < 40 ? 2 : jiban < 60 ? 3 : jiban < 80 ? 4 : 5;
          skillPt += g;
          addJiBan(p, 7);
        }
        else if (personType == PersonType_jizhe)//记者
        {
          int jiban = persons[p].friendship;
          int g = jiban < 40 ? 2 : jiban < 60 ? 3 : jiban < 80 ? 4 : 5;
          addStatus(action.train, g);
          addJiBan(p, 9);
        }
        else if (personType == PersonType_lianghuaNonCard)//无卡友人
        {
          int jiban = persons[p].friendship;
          int g = jiban < 40 ? 2 : jiban < 60 ? 3 : jiban < 80 ? 4 : 5;
          skillPt += g;
          addJiBan(p, 7);
        }
        else
        {
          //其他友人/团卡暂不支持
          assert(false);
        }
      }

      if (hintCards.size() > 0)
      {
        int hintCard = hintCards[rand() % hintCards.size()];//随机一张卡出hint

        addJiBan(hintCard, 5);
        auto& hintBonus = persons[hintCard].cardParam.hintBonus;
        for (int i = 0; i < 5; i++)
          addStatus(i, hintBonus[i]);
        skillPt += hintBonus[5];
        //黄buff，双倍
        if (uaf_buffNum[2] > 0)
        {
          for (int i = 0; i < 5; i++)
            addStatus(i, hintBonus[i]);
          skillPt += hintBonus[5];
        }
      }

      if (clickFriend)
        handleFriendClickEvent(rand, action.train);
      

      //训练等级提升
      int thisColor = uaf_trainingColor[action.train];
      for (int i = 0; i < 5; i++)
      {
        if (uaf_trainingColor[i] == thisColor)
        {
          uaf_trainingLevel[thisColor][i] += uaf_trainLevelGain[i];
          if (uaf_trainingLevel[thisColor][i] > 100)uaf_trainingLevel[thisColor][i] = 100;
        }
      }

    }

    //buff次数-1
    for (int color = 0; color < 3; color++)
    {
      if (uaf_buffNum[color] > 0)uaf_buffNum[color] -= 1;
    }
    uaf_lastTurnNotTrain = false;
  }
  else
  {
    printEvents("未知的训练项目");
    uaf_lastTurnNotTrain = true;
    return false;
  }
  return true;
}


bool Game::isLegal(Action action) const
{
  if (isRacing)
  {
    assert(false && "所有剧本比赛都在checkEventAfterTrain()里处理，不能applyTraining");
    return false;//所有剧本比赛都在checkEventAfterTrain()里处理（相当于比赛回合直接跳过），不在这个函数
  }
  if (action.xiangtanType != XT_none && (!(action.train >= 0 && action.train <= 4)))
    return false;//相谈但不训练
  if (action.train == TRA_rest)
  {
    return true;
  }
  else if (action.train == TRA_outgoing)
  {
    if (isXiahesu())
    {
      return false;//我将夏合宿的外出称为休息
    }
    return true;
  }
  else if (action.train == TRA_race)
  {
    if (turn <= 12 || turn >= 72)
    {
      return false;
    }
    return true;
  }
  else if (action.train >= 0 && action.train <= 4)
  {
    if (!isXiangtanLegal(action.xiangtanType))
      return false;
    //检查相谈是否有意义
    //大部分情况已经在isXiangtanLegal里排除，只剩下一种情况没排除：只相谈一种a改色b，而这个训练是c
    if (action.xiangtanType >= 1 && action.xiangtanType <= 6)
    {
      int c = uaf_trainingColor[action.train];
      if (c != Action::XiangtanFromColor[action.xiangtanType] && c != Action::XiangtanToColor[action.xiangtanType])
        return false;
    }
    return true;
  }
  else
  {
    assert(false && "未知的训练项目");
    return false;
  }
  return false;
}



float Game::getSkillScore() const
{
  return ptScoreRate * skillPt + skillScore;
}

int Game::finalScore() const
{
  int total = 0;
  for (int i = 0; i < 5; i++)
    total += GameConstants::FiveStatusFinalScore[min(fiveStatus[i],fiveStatusLimit[i])];
  
  total += getSkillScore();
  //return uaf_haveLose ? 10000 : 20000;
  return total;
}

bool Game::isEnd() const
{
  return turn >= TOTAL_TURN;
}

int Game::getTrainingLevel(int item) const
{
  if (isXiahesu())return 4;

  int splevel = uaf_trainingLevel[uaf_trainingColor[item]][item];
  return convertTrainingLevel(splevel);
}

int Game::turnIdxInHarvestLoop() const
{
  return 0;
}

void Game::calculateTrainingValueSingle(int tra)
{
  int headNum = 0;//几张卡或者npc，理事长记者不算
  int shiningNum = 0;//几张闪彩
  int linkNum = 0;//几张link

  int basicValue[6] = { 0,0,0,0,0,0 };//训练的基础值，=原基础值+支援卡加成

  int totalXunlian = 0;//训练1+训练2+...
  int totalGanjing = 0;//干劲1+干劲2+...
  double totalYouqingMultiplier = 1.0;//(1+友情1)*(1+友情2)*...
  int vitalCostBasic;//体力消耗基础量，=ReLU(基础体力消耗+link体力消耗增加-智彩体力消耗减少)
  double vitalCostMultiplier = 1.0;//(1-体力消耗减少率1)*(1-体力消耗减少率2)*...
  double failRateMultiplier = 1.0;//(1-失败率下降率1)*(1-失败率下降率2)*...

  int tlevel = getTrainingLevel(tra);


  bool isCardShining_record[6] = { false,false,false,false,false,false };
  for (int h = 0; h < 5; h++)
  {
    int pIdx = personDistribution[tra][h];
    if (pIdx < 0)break;
    if (pIdx == PSID_npc)
    {
      headNum += 1;
      continue;
    }
    if (pIdx >= 6)continue;//不是支援卡

    headNum += 1;
    const Person& p = persons[pIdx];
   
    if (isCardShining(pIdx, tra))
    {
      shiningNum += 1;
      isCardShining_record[pIdx] = true;
    }
    if (p.cardParam.isLink)
    {
      linkNum += 1;
    }
  }
  isTrainShining[tra] = shiningNum;

  //菜量获取值
  cook_train_material_type[tra] = tra;
  cook_train_green[tra] = shiningNum > 0;
  cook_train_material_num_extra[tra] = headNum + 2 * linkNum;


  //基础值
  for (int i = 0; i < 6; i++)
    basicValue[i] = GameConstants::TrainingBasicValue[tra][tlevel][i];
  vitalCostBasic = -GameConstants::TrainingBasicValue[tra][tlevel][6];

  for (int h = 0; h < 5; h++)
  {
    int pid = personDistribution[tra][h];
    if (pid < 0)break;//没人
    if (pid >= 6)continue;//不是卡
    const Person& p = persons[pid];
    bool isThisCardShining = isCardShining_record[pid];//这张卡闪没闪
    bool isThisTrainingShining = shiningNum > 0;//这个训练闪没闪
    CardTrainingEffect eff = p.cardParam.getCardEffect(*this, isThisCardShining, tra, p.friendship, p.cardRecord, headNum, shiningNum);
    
    for (int i = 0; i < 6; i++)//基础值bonus
    {
      if (basicValue[i] > 0)
        basicValue[i] += eff.bonus[i];
    }
    if (isCardShining_record[pid])//闪彩，友情加成和智彩回复
    {
      totalYouqingMultiplier *= (1 + 0.01 * eff.youQing);
      if (tra == TRA_wiz)
        vitalCostBasic -= eff.vitalBonus;
    }
    totalXunlian += eff.xunLian;
    totalGanjing += eff.ganJing;
    vitalCostMultiplier *= (1 - 0.01 * eff.vitalCostDrop);
    failRateMultiplier *= (1 - 0.01 * eff.failRateDrop);

  }

  //体力，失败率

  int vitalChangeInt = -int(vitalCostBasic * vitalCostMultiplier);
  if (vitalChangeInt > maxVital - vital)vitalChangeInt = maxVital - vital;
  if (vitalChangeInt < -vital)vitalChangeInt = -vital;
  trainVitalChange[tra] = vitalChangeInt;
  failRate[tra] = calculateFailureRate(tra, failRateMultiplier);


  //人头 * 训练 * 干劲 * 友情    //支援卡倍率
  double cardMultiplier = (1 + 0.05 * headNum) * (1 + 0.01 * totalXunlian) * (1 + 0.1 * (motivation - 3) * (1 + 0.01 * totalGanjing)) * totalYouqingMultiplier;
  //trainValueCardMultiplier[t] = cardMultiplier;

  //下层可以开始算了
  for (int i = 0; i < 6; i++)
  {
    bool isRelated = basicValue[i] != 0;
    double bvl = basicValue[i];
    double umaBonus = i < 5 ? 1 + 0.01 * fiveStatusBonus[i] : 1;
    trainValueLower[tra][i] = bvl * cardMultiplier * umaBonus;
  }

  //剧本训练加成
  double scenarioTrainMultiplier = 1 + 0.01 * cook_dishpt_training_bonus;
  //料理训练加成
  if (cook_dish != DISH_none)
    scenarioTrainMultiplier += 0.01 * getDishTrainingBonus(tra);
  double skillPtMultiplier = scenarioTrainMultiplier * (1 + 0.01 * cook_dishpt_skillpt_bonus);



  //上层=总数-下层

  for (int i = 0; i < 6; i++)
  {
    int lower = trainValueLower[tra][i];
    if (lower > 100) lower = 100;
    lower = calculateRealStatusGain(i, lower);//consider the integer over 1200
    trainValueLower[tra][i] = lower;
    double multiplier = i < 5 ? scenarioTrainMultiplier : skillPtMultiplier;
    int total = int(lower * multiplier);
    if (total > 100 + lower)total = 100 + lower;
    total = calculateRealStatusGain(i, total);
    trainValue[tra][i] = total;
  }


}

void Game::checkEventAfterTrain(std::mt19937_64& rand)
{

  checkFixedEvents(rand);

  checkRandomEvents(rand);


  //回合数+1
  turn++;
  if (turn < TOTAL_TURN)
  {
    isRacing = isRacingTurn[turn];
    if (isRacing)
      checkEventAfterTrain(rand);//跳过这个回合
  }
  else
  {
    printEvents("育成结束!");
    printEvents("你的得分是：" + to_string(finalScore()));
  }

}
void Game::uaf_checkNewBuffAfterLevelGain()
{
  for (int color = 0; color < 3; color++)
  {
    int leveltotal = 0;
    for (int i = 0; i < 5; i++)
      leveltotal += uaf_trainingLevel[color][i];

    int buffNumTotal = leveltotal / 50;
    int buffNumUsed = uaf_buffActivated[color];
    if (buffNumTotal > buffNumUsed)
    {
      uaf_buffNum[color] += 2 * (buffNumTotal - buffNumUsed);
      uaf_buffActivated[color] = buffNumTotal;
    }
    else if (buffNumTotal < buffNumUsed)//可能是其他地方篡改了训练等级，比如Selfplay/GameGenerator
    {
      uaf_buffActivated[color] = buffNumTotal;
    }
  }
}
void Game::uaf_runCompetition(int n)//第n次uaf大会
{
  uaf_xiangtanRemain = 3;
  int winCount = 0;
  int levelRequired = 10 + n * 10;
  for (int color = 0; color < 3; color++)
  {
    for (int i = 0; i < 5; i++)
    {
      if (uaf_trainingLevel[color][i] >= levelRequired)
      {
        uaf_winHistory[n][color][i] = true;
        winCount++;
      }
    }

  }

  if (winCount < 12)// <12win 突破/入赏
  {
    //依次是3，5，10，15，20
    int statusGain = n * 5;
    if (statusGain == 0)statusGain = 3;
    if (isLinkUma)statusGain += 3;
    addAllStatus(statusGain);
    skillPt += 30 + 10 * n;
  }
  else // >=12win 综合优胜或优胜
  {
    addJiBan(6, 5);//理事长羁绊
    int statusGain = 5 + n * 5;
    if (isLinkUma)statusGain += 3;
    addAllStatus(statusGain);
    skillPt += 40 + 10 * n;
  }
  if (winCount == 15) //全胜
  {
    addMotivation(1);
    addVital(15);
  }
}
void Game::checkFixedEvents(std::mt19937_64& rand)
{
  //处理各种固定事件
  uaf_checkNewBuffAfterLevelGain();
  if (isRacing)//生涯比赛
  {
    if (turn < 72)
    {
      runRace(3, 45);
      addJiBan(6, 4);
    }
    //否则是ura比赛，在后面有单独的处理
    uaf_lastTurnNotTrain = true;
  }

  if (turn == 11)//相谈刷新
  {
    assert(isRacing);
    uaf_xiangtanRemain = 3;
  }
  else if (turn == 23)//第一年年底
  {
    uaf_runCompetition(0);
    //年底事件，体力低选择体力，否则选属性
    {
      int vitalSpace = maxVital - vital;//还差多少体力满
      handleFriendFixedEvent();
      if (vitalSpace >= 20)
        addVital(20);
      else
        addAllStatus(5);
    }
    printEvents("uaf大会1结束");
  }
  else if (turn == 29)//第二年继承
  {

    for (int i = 0; i < 5; i++)
      addStatus(i, zhongMaBlueCount[i] * 6); //蓝因子典型值

    double factor = double(rand() % 65536) / 65536 * 2;//剧本因子随机0~2倍
    for (int i = 0; i < 5; i++)
      addStatus(i, int(factor*zhongMaExtraBonus[i])); //剧本因子
    skillPt += int((0.5 + 0.5 * factor) * zhongMaExtraBonus[5]);//乱七八糟技能的等效pt

    for (int i = 0; i < 5; i++)
      fiveStatusLimit[i] += zhongMaBlueCount[i] * 2; //属性上限--种马基础值。18蓝两次继承共加大约36上限，每次每个蓝因子+1上限，1200折半再乘2

    for (int i = 0; i < 5; i++)
      fiveStatusLimit[i] += rand() % 8; //属性上限--后两次继承随机增加

    printEvents("第二年继承");
  }
  else if (turn == 35)//uaf2
  {
    uaf_runCompetition(1);
    printEvents("uaf大会2结束");
  }

  else if (turn == 47)//第二年年底
  {
    uaf_runCompetition(2);
    //年底事件，体力低选择体力，否则选属性
    {
      int vitalSpace = maxVital - vital;//还差多少体力满
      if (vitalSpace >= 30)
        addVital(30);
      else
        addAllStatus(8);
    }
    printEvents("uaf大会3结束");
  }
  else if (turn == 48)//抽奖
  {
    int rd = rand() % 100;
    if (rd < 16)//温泉或一等奖
    {
      addVital(30);
      addAllStatus(10);
      addMotivation(2);

      printEvents("抽奖：你抽中了温泉/一等奖");
    }
    else if (rd < 16 + 27)//二等奖
    {
      addVital(20);
      addAllStatus(5);
      addMotivation(1);
      printEvents("抽奖：你抽中了二等奖");
    }
    else if (rd < 16 + 27 + 46)//三等奖
    {
      addVital(20);
      printEvents("抽奖：你抽中了三等奖");
    }
    else//厕纸
    {
      addMotivation(-1);
      printEvents("抽奖：你抽中了厕纸");
    }
  }
  else if (turn == 49)
  {
    skillScore += 170;
    printEvents("固有等级+1");
  }
  else if (turn == 53)//第三年继承
  {
    for (int i = 0; i < 5; i++)
      addStatus(i, zhongMaBlueCount[i] * 6); //蓝因子典型值

    double factor = double(rand() % 65536) / 65536 * 2;//剧本因子随机0~2倍
    for (int i = 0; i < 5; i++)
      addStatus(i, int(factor * zhongMaExtraBonus[i])); //剧本因子
    skillPt += int((0.5 + 0.5 * factor) * zhongMaExtraBonus[5]);//乱七八糟技能的等效pt

    for (int i = 0; i < 5; i++)
      fiveStatusLimit[i] += zhongMaBlueCount[i] * 2; //属性上限--种马基础值。18蓝两次继承共加大约36上限，每次每个蓝因子+1上限，1200折半再乘2

    for (int i = 0; i < 5; i++)
      fiveStatusLimit[i] += rand() % 8; //属性上限--后两次继承随机增加

    printEvents("第三年继承");

    if (persons[6].friendship >= 60)
    {
      skillScore += 170;//固有技能等级+1
      addMotivation(1);
    }
    else
    {
      addVital(-5);
      skillPt += 25;
    }
  }
  else if (turn == 59)//uaf4
  {
    uaf_runCompetition(3);
    printEvents("uaf大会4结束");
    }
  else if (turn == 65)//心情+1，决死的觉悟hint+1
  {
    addMotivation(1);
    skillPt += 5;//这个hint一般不点
  }
  else if (turn == 70)
  {
    skillScore += 170;//固有技能等级+1
  }
  else if (turn == 71)//uaf5
  {
    uaf_runCompetition(4);
    printEvents("uaf大会5结束");
  }
  else if (turn == 73)//ura1
  {
    assert(isRacing);
    runRace(10, 40);
    printEvents("ura1结束");
  }
  else if (turn == 75)//ura2
  {
    assert(isRacing);
    runRace(10, 60);
    printEvents("ura2结束");
  }
  else if (turn == 77)//ura3，游戏结束
  {
    assert(isRacing);
    runRace(10, 80);

    //记者
    if (persons[7].friendship >= 80)
    {
      addAllStatus(5);
      skillPt += 20;
    }
    else if (persons[7].friendship >= 60)
    {
      addAllStatus(3);
      skillPt += 10;
    }
    else if (persons[7].friendship >= 40)
    {
      skillPt += 10;
    }
    else
    {
      skillPt += 5;
    }

    bool allWin = true;//每一场都15win
    bool allTotallyWin = true;//每一场都12win以上

    for (int c = 0; c < 5; c++)
    {
      int thisWinNum = 0;
      for (int color = 0; color < 3; color++)
      {
        for (int j = 0; j < 5; j++)
        {
          if (uaf_winHistory[c][color][j])
            thisWinNum++;
        }
      }
      if (thisWinNum < 15)allWin = false;
      if (thisWinNum < 12)allTotallyWin = false;
    }
    if (allWin)
    {
      skillPt += 40;//全身全灵与进化
      addAllStatus(55);
      skillPt += 140;
      //cout << 1;
    }
    else if (allTotallyWin)
    {
      skillPt += 40;//全身全灵与进化
      addAllStatus(30);
      skillPt += 90;
      //cout << 2;
    }
    else
    {
      addAllStatus(20);
      skillPt += 70;
      //cout << 3;
    }


    int totalLevel = 0;
    for (int color = 0; color < 3; color++)
    {
      for (int j = 0; j < 5; j++)
      {
        totalLevel += uaf_trainingLevel[color][j];
      }
    }
    if (totalLevel >= 1200)
    {
      skillPt += 60;//三折金上位
    }
    else
    {
      skillPt += 20;//一折金上位
    }


    //友人卡事件
    handleFriendFixedEvent();

    addAllStatus(5);
    skillPt += 30;

    printEvents("ura3结束，游戏结算");
  }
}

void Game::checkRandomEvents(std::mt19937_64& rand)
{
  if (turn>=72)
    return;//ura期间不会发生各种随机事件

  //友人会不会解锁出行
  if (lianghua_type != 0)
  {
    Person& p = persons[lianghua_personId];
    assert(p.personType == PersonType_lianghuaCard);
    if (p.friendOrGroupCardStage==1)
    {
      double unlockOutgoingProb = p.friendship >= 60 ?
        GameConstants::FriendUnlockOutgoingProbEveryTurnHighFriendship :
        GameConstants::FriendUnlockOutgoingProbEveryTurnLowFriendship;
      if (randBool(rand, unlockOutgoingProb))//启动
      {
        handleFriendUnlock(rand);
      }
    }
  }

  //模拟各种随机事件

  //支援卡连续事件，随机给一个卡加5羁绊
  if (randBool(rand, GameConstants::EventProb))
  {
    int card = rand() % 6;
    addJiBan(card, 5);
    //addAllStatus(4);
    addStatus(rand() % 5, eventStrength);
    skillPt += eventStrength;
    printEvents("模拟支援卡随机事件：" + persons[card].cardParam.cardName + " 的羁绊+5，pt和随机属性+" + to_string(eventStrength));

    //支援卡一般是前几个事件加心情
    if (randBool(rand, 0.4 * (1.0 - turn * 1.0 / TOTAL_TURN)))
    {
      addMotivation(1);
      printEvents("模拟支援卡随机事件：心情+1");
    }
    if (randBool(rand, 0.5))
    {
      addVital(10);
      printEvents("模拟支援卡随机事件：体力+10");
    }
    else if (randBool(rand, 0.03))
    {
      addVital(-10);
      printEvents("模拟支援卡随机事件：体力-10");
    }
    if (randBool(rand, 0.03))
    {
      isPositiveThinking = true;
      printEvents("模拟支援卡随机事件：获得“正向思考”");
    }
  }

  //模拟马娘随机事件
  if (randBool(rand, 0.1))
  {
    addAllStatus(3);
    printEvents("模拟马娘随机事件：全属性+3");
  }

  //加体力
  if (randBool(rand, 0.10))
  {
    addVital(5);
    printEvents("模拟随机事件：体力+5");
  }

  //加30体力（吃饭事件）,不敢吃饭了所以改成10体力
  if (randBool(rand, 0.02))
  {
    addVital(10);
    printEvents("模拟随机事件：体力+10");
  }

  //加心情
  if (randBool(rand, 0.02))
  {
    addMotivation(1);
    printEvents("模拟随机事件：心情+1");
  }

  //掉心情
  if (randBool(rand, 0.04))
  {
    addMotivation(-1);
    printEvents("模拟随机事件：\033[0m\033[33m心情-1\033[0m\033[32m");
  }

}

void Game::applyTrainingAndNextTurn(std::mt19937_64& rand, Action action)
{
  if (isEnd()) return;
  //assert(turn < TOTAL_TURN && "Game::applyTrainingAndNextTurn游戏已结束");
  assert(!isRacing && "比赛回合都在checkEventAfterTrain里跳过了");
  bool suc = applyTraining(rand, action);
  assert(suc && "Game::applyTrainingAndNextTurn选择了不合法的训练");

  checkEventAfterTrain(rand);
  if (isEnd()) return;

  assert(!isRacing && "比赛回合都在checkEventAfterTrain里跳过了");

  // 如果要支持在游戏中改变得意率，需要在这里更新得意率的值
  randomDistributeCards(rand);
}

bool Game::isCardShining(int personIdx, int trainIdx) const
{
  const Person& p = persons[personIdx];
  if(p.personType==PersonType_card)
  { 
    return p.friendship >= 80 && trainIdx == p.cardParam.cardType;
  }
  else if (p.personType == PersonType_groupCard)
  {
    return p.friendOrGroupCardStage == 3;
  }
  return false;
}

