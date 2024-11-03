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

  for (int i = 0; i < 5; i++)
    fiveStatusBonus[i] = GameDatabase::AllUmas[umaId].fiveStatusBonus[i];

  turn = 0;
  gameStage = GameStage_beforeTrain;
  vital = 100;
  maxVital = 100;
  motivation = 3;

  for (int i = 0; i < 5; i++)
    fiveStatus[i] = GameDatabase::AllUmas[umaId].fiveStatusInitial[i] - 10 * (5 - umaStars); //赛马娘初始值
  for (int i = 0; i < 5; i++)
    fiveStatusLimit[i] = GameConstants::BasicFiveStatusLimit[i]; //原始属性上限

  skillPt = 120;
  skillScore = umaStars >= 3 ? 170 * (umaStars - 2) : 120 * (umaStars);//固有技能

  for (int i = 0; i < 5; i++)
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

  currentDeyilvBonus = 0;
  currentLianghuaEffectEnable = false;

  for (int i = 0; i < MAX_INFO_PERSON_NUM; i++)
  {
    persons[i] = Person();
  }

  saihou = 0;
  friend_type = 0;
  friend_isSSR = false;
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

    if (persons[i].personType == PersonType_friendCard)
    {
      friend_personId = i;
      int friendCardId = cardId / 10;
      if (friendCardId == GameConstants::FriendCardLianghuaSSRId)
      {
        friend_type = FriendType_lianghua;
        friend_isSSR = true;
      }
      else if (friendCardId == GameConstants::FriendCardLianghuaRId)
      {
        friend_type = FriendType_lianghua;
        friend_isSSR = false;
      }
      else if (friendCardId == GameConstants::FriendCardYayoiSSRId)
      {
        friend_type = FriendType_yayoi;
        friend_isSSR = true;
      }
      else if (friendCardId == GameConstants::FriendCardYayoiRId)
      {
        friend_type = FriendType_yayoi;
        friend_isSSR = false;
      }
      else
        throw string("不支持带凉花/理事长以外的友人或团队卡");
      int friendLevel = cardId % 10;
      assert(friendLevel >= 0 && friendLevel <= 4);
      friend_vitalBonus = 1.0 + 0.01 * persons[i].cardParam.eventRecoveryAmountUp;
      friend_statusBonus = 1.0 + 0.01 * persons[i].cardParam.eventEffectUp;
      
      friend_vitalBonus += 1e-10;
      friend_statusBonus += 1e-10;//加个小量，避免因为舍入误差而算错
    }
  }

  std::vector<int> probs = { 100,100,100,100,100,200 }; //速耐力根智鸽
  distribution_noncard = std::discrete_distribution<>(probs.begin(), probs.end());
  probs = { 100,100,100,100,100,100 }; //速耐力根智鸽
  distribution_npc = std::discrete_distribution<>(probs.begin(), probs.end());

  for (int i = 0; i < 6; i++)//支援卡初始加成
  {
    for (int j = 0; j < 5; j++)
      addStatus(j, persons[i].cardParam.initialBonus[j]);
    skillPt += persons[i].cardParam.initialBonus[5];
  }

  mecha_linkeffect_gearProbBonus = 0;
  mecha_linkeffect_lvbonus = false;
  for (int i = 0; i < 5; i++)
    mecha_rivalLv[i] = 0;//一个link是20，2个是40，所以最后没link再改成1
  mecha_overdrive_energy = 0;
  mecha_overdrive_enabled = false;
  mecha_EN = 5;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      mecha_upgrade[i][j] = 0;
  for (int i = 0; i < 5; i++)
    mecha_hasGear[i] = false;
  for (int i = 0; i < 5; i++)
    mecha_win_history[i] = 0;


  //支援卡link
  for (int i = 0; i < 7; i++)
  {
    int chara = i < 6 ? persons[i].cardParam.charaId : umaId;
    if (GameConstants::isLinkChara_initialEN(chara))
      mecha_EN += 1;
    if (GameConstants::isLinkChara_moreGear(chara))
      mecha_linkeffect_gearProbBonus += 1;
    if (GameConstants::isLinkChara_initialOverdrive(chara))
      mecha_overdrive_energy += 3;
    if (GameConstants::isLinkChara_lvBonus(chara))
      mecha_linkeffect_lvbonus = true;
    if (GameConstants::isLinkChara_initialLv(chara))
      for (int i = 0; i < 5; i++)
        mecha_rivalLv[i] += 20;//一个link是20，2个是40
  }

  if (mecha_overdrive_energy > 6)mecha_overdrive_energy = 6;
  if (mecha_EN > 7)mecha_EN = 7;
  for (int i = 0; i < 5; i++)
    if (mecha_rivalLv[i] < 1)
      mecha_rivalLv[i] = 1;



  randomDistributeCards(rand); //随机分配卡组，包括计算属性
  
}

void Game::randomDistributeCards(std::mt19937_64& rand)
{
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 5; j++)
      personDistribution[i][j] = -1;

  if (isRacing)
  {
    return;//比赛不用分配卡组
  }
  

  int headN[5] = { 0,0,0,0,0 };
  vector<int8_t> buckets[5];
  for (int i = 0; i < 5; i++)
    buckets[i].clear();
  //先放友人/理事长/记者
  for (int i = 0; i < 6 + 2; i++)
  {
    int atTrain = 5;
    if (friend_type == FriendType_yayoi && i == friend_personId)
    {
      //友人卡
      atTrain = persons[i].distribution(rand);
    }
    else if (i == PSID_noncardYayoi && friend_type != FriendType_yayoi)//非卡理事长
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
  int npcCount = 6;
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
  for (int pid = 0; pid < 6; pid++)
  {
    if (persons[pid].personType == PersonType_card)
    {
      double hintProb = 0.06 * (1 + 0.01 * persons[pid].cardParam.hintProbIncrease);
      hintProb *= (1.0 + 0.15 * mecha_upgrade[0][1]);
      persons[pid].isHint = randBool(rand, hintProb);
        
    }
  }

  //随机决定是否有齿轮。如果有彩圈，则在calculateTrainingValue()里会变成true，这里不用考虑
  double gearProb = GameConstants::Mecha_GearProb + GameConstants::Mecha_GearProbLinkBonus * mecha_linkeffect_gearProbBonus;
  for (int i = 0; i < 5; i++)
  {
    mecha_hasGear[i] = randBool(rand, gearProb);
  }

  calculateTrainingValue();
}

//
//上层=min(总数-下层, 100)
void Game::calculateTrainingValue()
{
  //重新计算统计信息
  mecha_rivalLvTotal = 0;
  for (int i = 0; i < 5; i++)
  {
    mecha_rivalLvTotal += mecha_rivalLv[i];
  }

  mecha_rivalLvLimit = turn < 24 ? 200 : turn < 36 ? 300 : turn < 48 ? 400 : turn < 60 ? 500 : turn < 72 ? 600 : 700;

  for (int i = 0; i < 3; i++)
  {
    mecha_upgradeTotal[i] = 0;
    for (int j = 0; j < 3; j++)
      mecha_upgradeTotal[i] += mecha_upgrade[i][j];
  }

  //属性加成
  for (int i = 0; i < 5; i++)
  {
    double m = 1.0;
    //double rivalLvBonus = 0.06 + 0.0006 * mecha_rivalLv[i];
    //if (mecha_linkeffect_lvbonus)rivalLvBonus * 1.5;
    //m *= (1 + rivalLvBonus);

    if (mecha_overdrive_enabled)
    {
      //m *= 1.25;
      int upgradeGroup =
        (i == 0 || i == 2) ? mecha_upgradeTotal[1] :
        (i == 1 || i == 3) ? mecha_upgradeTotal[2] :
        mecha_upgradeTotal[0];
      if (upgradeGroup >= 9)
      {
        int count = 1 + (mecha_rivalLvTotal - 1) / 200;
        double bonus = 0.03 * count;
        m *= (1 + bonus);
      }
      else if (upgradeGroup >= 6)
      {
        int count = 1 + (mecha_rivalLvTotal - 1) / 300;
        double bonus = 0.03 * count;
        m *= (1 + bonus);
      }
    }

    mecha_trainingStatusMultiplier[i] = m;
  }

  double ptb = 1.0;
  ptb *= 1 + mecha_upgrade[2][2] * 0.12;
  if (mecha_overdrive_enabled && mecha_upgradeTotal[2] >= 15)
  {
    int count = 1 + (mecha_rivalLvTotal - 1) / 150;
    double bonus = 0.03 * count;
    ptb *= (1 + bonus);
  }
  mecha_trainingStatusMultiplier[5] = ptb;

  for (int i = 0; i < 5; i++)
  {
    int upgradeLv =
      i == 0 ? mecha_upgrade[2][0] :
      i == 1 ? mecha_upgrade[1][0] :
      i == 2 ? mecha_upgrade[2][1] :
      i == 3 ? mecha_upgrade[1][1] :
      mecha_upgrade[0][0];
    double lvGainBonus =
      upgradeLv == 5 ? 40 :
      upgradeLv == 4 ? 33 :
      upgradeLv == 3 ? 26 :
      upgradeLv == 2 ? 18 :
      upgradeLv == 1 ? 10 :
      0;
    if (mecha_overdrive_enabled)
    {
      if (mecha_upgradeTotal[0] >= 12)
        lvGainBonus += 25;
      else if (mecha_upgradeTotal[0] >= 9)
        lvGainBonus += 20;
      else if (mecha_upgradeTotal[0] >= 6)
        lvGainBonus += 15;
    }

    mecha_lvGainMultiplier[i] = 1.0 + 0.01 * lvGainBonus;
  }

  for (int i = 0; i < 5; i++)
    calculateTrainingValueSingle(i);
}

void Game::addTrainingLevelCount(int trainIdx, int n)
{
  trainLevelCount[trainIdx] += n;
  if (trainLevelCount[trainIdx] > 16)trainLevelCount[trainIdx] = 16;
}

void Game::maybeUpdateDeyilv()
{
  int deyilvBonus = 15 * mecha_upgrade[0][0];
  bool lianghuaEffectEnable =
    friend_type == FriendType_lianghua &&
    friend_isSSR &&
    persons[friend_personId].friendship >= 60;
  if (deyilvBonus != currentDeyilvBonus || lianghuaEffectEnable != currentLianghuaEffectEnable)
  {
    currentDeyilvBonus = deyilvBonus;
    currentLianghuaEffectEnable = lianghuaEffectEnable;
    for (int i = 0; i < 6; i++)
    {
      persons[i].setExtraDeyilvBonus(deyilvBonus, lianghuaEffectEnable);
    }
  }
}

bool Game::tryInvitePeople(std::mt19937_64& rand) 
{
  int invitePerson = rand() % 6;
  int inviteTrain = rand() % 5;

  int space = -1;
  for (int idx = 0; idx < 5; idx++)
  {
    int pid = personDistribution[inviteTrain][idx];
    if (pid == -1 && space == -1)
      space = idx;
    if (pid == invitePerson)
      return false;
  }

  if (space == -1)
    return false;

  personDistribution[inviteTrain][space] = invitePerson;
  return true;
  //require recalculate later
}
void Game::mecha_addRivalLv(int idx, int value)
{
  assert(idx >= 0 && idx < 5);
  int t = mecha_rivalLv[idx] + value;

  if (t > mecha_rivalLvLimit)
    t = mecha_rivalLvLimit;
}
void Game::mecha_distributeEN(int head3, int chest3, int foot3)
{
  throw "todo";
}
void Game::mecha_maybeRunUGE()
{
  throw "todo";
}
bool Game::mecha_activate_overdrive()
{
  throw "todo";
  return false;
}
int Game::calculateRealStatusGain(int value, int gain) const//考虑1200以上为2的倍数的实际属性增加值
{
  int newValue = value + gain;
  if (newValue <= 1200)return gain;
  if (gain == 1)return 2;
  return (newValue / 2) * 2 - value;
}
void Game::addStatus(int idx, int value)
{
  assert(idx >= 0 && idx < 5);
  int t = fiveStatus[idx] + value;
  
  if (t > fiveStatusLimit[idx])
    t = fiveStatusLimit[idx];
  if (t < 1)
    t = 1;
  if (t > 1200)
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
void Game::addVitalMax(int value)
{
  maxVital += value;
  if (maxVital > 120)
    maxVital = 120;
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
void Game::addJiBan(int idx, int value, bool ignoreAijiao)
{
  if(idx == PSID_noncardYayoi)
    friendship_noncard_yayoi += value;
  else if (idx == PSID_noncardReporter)
    friendship_noncard_reporter += value;
  else if (idx < 6)
  {
    auto& p = persons[idx];
    int gain = (isAiJiao && !ignoreAijiao) ? value + 2 : value;
    persons[idx].friendship += gain;
    if (p.friendship > 100)p.friendship = 100;
  }
  else
    throw "ERROR: Game::addJiBan Unknown person id";
}
void Game::addAllStatus(int value)
{
  for (int i = 0; i < 5; i++)addStatus(i, value);
}
int Game::calculateFailureRate(int trainType, double failRateMultiply) const
{
  static const double A = 0.025;
  static const double B = 1.25;
  double x0 = 0.1 * GameConstants::FailRateBasic[trainType][getTrainingLevel(trainType)];
  
  double f = 0;
  if (vital < x0)
  {
    f = (100 - vital) * (x0 - vital) / 40.0;
  }
  if (f < 0)f = 0;
  if (f > 99)f = 99;//无练习下手，失败率最高99%
  f *= failRateMultiply;//支援卡的训练失败率下降词条
  int fr = ceil(f);
  fr += failureRateBias;
  if (fr < 0)fr = 0;
  if (fr > 100)fr = 100;
  return fr;
}
void Game::runRace(int basicFiveStatusBonus, int basicPtBonus)
{
  double raceMultiply = 1 + 0.01 * saihou;

  int fiveStatusBonus = int(raceMultiply * basicFiveStatusBonus);
  int ptBonus = int(raceMultiply * basicPtBonus);
  //cout << fiveStatusBonus << " " << ptBonus << endl;
  addAllStatus(fiveStatusBonus);
  skillPt += ptBonus;
}

void Game::addStatusFriend(int idx, int value)
{
  value = int(value * friend_statusBonus);
  if (idx == 5)skillPt += value;
  else addStatus(idx, value);
}

void Game::addVitalFriend(int value)
{
  value = int(value * friend_vitalBonus);
  addVital(value);
}


void Game::handleFriendOutgoing(std::mt19937_64& rand)
{
  assert(friend_type!=0 && friend_stage >= FriendStage_afterUnlockOutgoing && friend_outgoingUsed < 5);
  int pid = friend_personId;
  if (friend_type == FriendType_yayoi)
  {
    if (friend_outgoingUsed == 0)
    {
      addVitalFriend(30);
      addMotivation(1);
      addStatusFriend(3, 20);
      addJiBan(pid, 5, false);
    }
    else if (friend_outgoingUsed == 1)
    {
      addVitalFriend(30);
      addMotivation(1);
      addStatusFriend(0, 10);
      addStatusFriend(3, 10);
      isRefreshMind = true;
      addJiBan(pid, 5, false);
    }
    else if (friend_outgoingUsed == 2)
    {
      int remainVital = maxVital - vital;
      if (remainVital >= 20)//选上
        addVitalFriend(43);
      else//选下
        addStatusFriend(3, 29);
      addMotivation(1);
      addJiBan(pid, 5, false);
    }
    else if (friend_outgoingUsed == 3)
    {
      addVitalFriend(30);
      addMotivation(1);
      addStatusFriend(3, 25);
      addJiBan(pid, 5, false);
    }
    else if (friend_outgoingUsed == 4)
    {
      //有大成功和成功
      if (rand() % 4 != 0)//粗略估计，75%大成功
      {
        addVitalFriend(30);
        addStatusFriend(3, 36);
        skillPt += 72;//金技能等价
      }
      else
      {
        addVitalFriend(26);
        addStatusFriend(3, 24);
        skillPt += 40;//金技能等价
      }
      addMotivation(1);
      addJiBan(pid, 5, false);
      isRefreshMind = true;
    }
    else throw string("未知的出行");
  }
  else if (friend_type == FriendType_lianghua)
  {
    throw "todo";
  }
  else throw string("未知的出行");


  friend_outgoingUsed += 1;
}
void Game::handleFriendUnlock(std::mt19937_64& rand)
{
  assert(friend_stage == FriendStage_beforeUnlockOutgoing);

  if (friend_type == FriendType_yayoi)
  { 
    if (maxVital - vital >= 15)
    {
      addVitalFriend(25);
      printEvents("友人外出解锁！选上");
    }
    else
    {
      addStatusFriend(0, 8);
      addStatusFriend(3, 8);
      skillPt += 10;//直线巧者+5
      printEvents("友人外出解锁！选下");
    }
    addMotivation(1);
    isRefreshMind = true;
    addJiBan(friend_personId, 5, false);
  }
  else if (friend_type == FriendType_lianghua)
  {
    throw "todo";
  }
  else throw string("未知的友人解锁出行");
  friend_stage = FriendStage_afterUnlockOutgoing;
}
void Game::handleFriendClickEvent(std::mt19937_64& rand, int atTrain)
{
  assert(friend_type!=0 && (friend_personId<6&& friend_personId>=0) && persons[friend_personId].personType==PersonType_friendCard);
  if (friend_stage == FriendStage_notClicked)
  {
    printEvents("第一次点友人");
    friend_stage = FriendStage_beforeUnlockOutgoing;

    if (friend_type == FriendType_yayoi)
    {
      addStatusFriend(0, 14);
      addJiBan(friend_personId, 10, false);
      addMotivation(1);
    }
    else if (friend_type == FriendType_lianghua)
    {
      throw "todo";
    }
    else throw string("未知的第一次点友人");
  }
  else
  {
    if (rand() % 5 < 3)return;//40%概率出事件，60%概率不出

    if (rand() % 10 == 0)
    {
      if (motivation != 5)
        printEvents("友人点击事件:心情+1");
      addMotivation(1);//10%概率加心情
    }

    if (friend_type == FriendType_yayoi)
    {
      if (turn < 24)
      {
        //给羁绊最低的人加3羁绊
        int minJiBan = 10000;
        int minJiBanId = -1;
        for (int i = 0; i < 6; i++)
        {
          if (persons[i].personType == PersonType_card)
          {
            if (persons[i].friendship < minJiBan)
            {
              minJiBan = persons[i].friendship;
              minJiBanId = i;
            }
          }
        }
        if (minJiBanId != -1)
        {
          addJiBan(minJiBanId, 3, false);
        }
        addJiBan(friend_personId, 5, false);
        printEvents("友人点击事件:" + persons[minJiBanId].getPersonName() + " 羁绊+3, 理事长羁绊+5");

     
      }
      else if (turn < 48)
      {
        addStatusFriend(0, 12);
        addJiBan(friend_personId, 5, false);
      }
      else
      {
        addStatusFriend(3, 12);
        addJiBan(friend_personId, 5, false);
      }
    }
    else if (friend_type == FriendType_lianghua)
    {
      throw "todo";
    }
    else throw string("未知的友人点击事件");
  }

}
void Game::handleFriendFixedEvent()
{
  if (friend_type == 0)return;//没友人卡
  if (friend_stage < FriendStage_beforeUnlockOutgoing)return;//出行没解锁就没事件
  if (turn == 23)
  {
    if (friend_type == FriendType_yayoi)
    {
      addMotivation(1);
      addStatusFriend(0, 24);
      addJiBan(friend_personId, 5, false);
      skillPt += 40;//三级中盘巧者，而且有进化，因此这个hint是有效的
    }
    else if (friend_type == FriendType_lianghua)
    {
      throw "todo";
    }
    else throw string("未知的友人固定事件");
  }
  else if (turn == 77)
  {
    if (friend_type == FriendType_yayoi)
    {
      if (friend_outgoingUsed >= 5)//走完出行
      {
        addStatusFriend(0, 20);
        addStatusFriend(3, 20);
        addStatusFriend(5, 56);
      }
      else
      {
        //just guess, to be filled
        addStatusFriend(0, 16);
        addStatusFriend(3, 16);
        addStatusFriend(5, 43);
      }
    }
    else if (friend_type == FriendType_lianghua)
    {
      throw "todo";
    }
    else throw string("未知的友人固定事件");
  }
  else
  {
    assert(false && "其他回合没有友人固定事件");
  }
}
bool Game::applyTraining(std::mt19937_64& rand, int train)
{
  if (isRacing)
  {
    //固定比赛的收益在checkEventAfterTrain()里处理
    assert(train == TRA_race);

    mecha_overdrive_energy += 1;
    if (mecha_overdrive_energy > 6)mecha_overdrive_energy = 6;
    for (int i = 0; i < 5; i++)
    {
      mecha_addRivalLv(i, 7);
    }

  }
  else
  {
    if (train == TRA_rest)//休息
    {
      if (isXiahesu())//合宿只能外出
      {
        return false;
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
      mecha_overdrive_energy += 1;
      if (mecha_overdrive_energy > 6)mecha_overdrive_energy = 6;
    }
    else if (train == TRA_race)//比赛
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

      mecha_overdrive_energy += 1;
      if (mecha_overdrive_energy > 6)mecha_overdrive_energy = 6;
      for (int i = 0; i < 5; i++)
      {
        mecha_addRivalLv(i, 7);
      }
    }
    else if (train == TRA_outgoing)//外出
    {
      if (isXiahesu())
      {
        addVital(40);
        addMotivation(1);
      }
      else if (friend_type != 0 &&  //带了友人卡
        friend_stage == FriendStage_afterUnlockOutgoing &&  //已解锁外出
        friend_outgoingUsed < 5  //外出没走完
        )
      {
        //友人出行
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
      mecha_overdrive_energy += 1;
      if (mecha_overdrive_energy > 6)mecha_overdrive_energy = 6;
    }
    else if (train <= 4 && train >= 0)//常规训练
    {
      if (rand() % 100 < failRate[train])//训练失败
      {
        if (failRate[train] >= 20 && (rand() % 100 < failRate[train]))//训练大失败，概率是瞎猜的
        {
          printEvents("训练大失败！");
          addStatus(train, -10);
          if (fiveStatus[train] > 1200)
            addStatus(train, -10);//游戏里1200以上扣属性不折半，在此模拟器里对应1200以上翻倍
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
          addStatus(train, -5);
          if (fiveStatus[train] > 1200)
            addStatus(train, -5);//游戏里1200以上扣属性不折半，在此模拟器里对应1200以上翻倍
          addMotivation(-1);
        }
      }
      else
      {
        //先加上训练值
        for (int i = 0; i < 5; i++)
          addStatus(i, trainValue[train][i]);
        skillPt += trainValue[train][5];
        addVital(trainVitalChange[train]);

        int friendshipExtra = 0;//如果带了SSR友人卡，+1。如果友人卡在这个训练，再+2。爱娇不在这里处理
        bool isSSRYayoi = friend_type == PersonType_yayoi && friend_isSSR;
        if (isSSRYayoi)
          friendshipExtra += 1;

        vector<int> hintCards;//有哪几个卡出红感叹号了
        bool clickFriend = false;//这个训练有没有友人
        //检查SSR友人在不在这里
        for (int i = 0; i < 5; i++)
        {
          int p = personDistribution[train][i];
          if (p == PSID_none)break;//没人
          if (isSSRYayoi && p == friend_personId)
          {
            friendshipExtra += 2;
            break;
          }
        }
        for (int i = 0; i < 5; i++)
        {
          int p = personDistribution[train][i];
          if (p < 0)break;//没人

          if (p == friend_personId && friend_type != 0)//友人卡
          {
            assert(persons[p].personType == PersonType_friendCard);
            addJiBan(p, 4 + friendshipExtra, false);
            clickFriend = true;
          }
          else if (p < 6)//普通卡
          {
            addJiBan(p, 7 + friendshipExtra, false);
            if (persons[p].isHint)
              hintCards.push_back(p);
          }
          else if (p == PSID_npc)//npc
          {
            //nothing
          }
          else if (p == PSID_noncardYayoi)//非卡理事长
          {
            int jiban = friendship_noncard_yayoi;
            int g = jiban < 40 ? 2 : jiban < 60 ? 3 : jiban < 80 ? 4 : 5;
            skillPt += g;
            addJiBan(PSID_noncardYayoi, 7, false);
          }
          else if (p == PSID_noncardReporter)//记者
          {
            int jiban = friendship_noncard_reporter;
            int g = jiban < 40 ? 2 : jiban < 60 ? 3 : jiban < 80 ? 4 : 5;
            addStatus(train, g);
            addJiBan(PSID_noncardReporter, 7, false);
          }
          else
          {
            //其他友人/团卡暂不支持
            assert(false);
          }
        }

        if (hintCards.size() > 0)
        {
          if (!(mecha_overdrive_enabled && mecha_upgradeTotal[0] >= 15))//随机一张卡出hint
          {
            int hintCard = hintCards[rand() % hintCards.size()];
            hintCards.clear();
            hintCards.push_back(hintCard);
          }

          for (int p = 0; p < hintCards.size(); p++)
          {
            int hintCard = hintCards[p];
            addJiBan(hintCard, 5, false);
            int hintLevel = persons[hintCard].cardParam.hintLevel;
            if (hintLevel > 0)
            {
              skillPt += int(hintLevel * hintPtRate);
            }
            else //根乌拉拉这种，只给属性
            {
              if (train == 0)
              {
                addStatus(0, 6);
                addStatus(2, 2);
              }
              else if (train == 1)
              {
                addStatus(1, 6);
                addStatus(3, 2);
              }
              else if (train == 2)
              {
                addStatus(2, 6);
                addStatus(1, 2);
              }
              else if (train == 3)
              {
                addStatus(3, 6);
                addStatus(0, 1);
                addStatus(2, 1);
              }
              else if (train == 4)
              {
                addStatus(4, 6);
                skillPt += 5;
              }
            }
          }
        }

        if (clickFriend)
          handleFriendClickEvent(rand, train);


        //训练等级提升
        addTrainingLevelCount(train, 1);

        if (mecha_hasGear[train])
        {
          mecha_overdrive_energy += 1;
          if (mecha_overdrive_energy > 6)mecha_overdrive_energy = 6;
        }

        for (int i = 0; i < 5; i++)
        {
          mecha_addRivalLv(i, mecha_lvGain[train][i]);
        }
      }

    }
    else
    {
      printEvents("未知的训练项目");
      return false;
    }
  }

  return true;
}


bool Game::isLegal(Action action) const
{
  if (!action.isActionStandard())
    return false;

  //stage是否匹配
  if (action.type != gameStage)
    return false;

  if (action.type == GameStage_beforeTrain)
  {
    if (isRacing)
    {
      if (action.train == TRA_race)
        return true;
      else
        return false;
    }

    //是否能开齿轮
    if (action.overdrive)
    {
      if (mecha_overdrive_energy < 3)
        return false;
      if (mecha_overdrive_enabled)
        return false;
      if (mecha_upgradeTotal[1] >= 15)//摇人，应该先开overdrive再选训练，分两步
        return action.train == -1;
      else
        return action.train >= 0 && action.train <= 4;
    }

    if (action.train == TRA_rest)
    {
      if (isXiahesu())
      {
        return false;//将夏合宿的“外出&休息”称为外出
      }
      return true;
    }
    else if (action.train == TRA_outgoing)
    {
      return true;
    }
    else if (action.train == TRA_race)
    {
      return isRaceAvailable();
    }
    else if (action.train >= 0 && action.train <= 4)
    {
      return true;
    }
    else
    {
      assert(false && "未知的训练项目");
      return false;
    }
    return false;
  }
  else if (action.type == GameStage_beforeMechaUpgrade)
  {
    int total3 = mecha_EN / 3;
    int mechaHeadLimit = turn >= 36 ? 5 : 3;//第二次UGE解锁头3号升级
    int mechaChestLimit = turn >= 60 ? 5 : 3;//第四次UGE解锁胸3号升级
    int mechaFootLimit = turn >= 60 ? 5 : 3;//第四次UGE解锁腿3号升级
    int mechaFoot = total3 - action.mechaHead - action.mechaChest;
    if (action.mechaHead < 0 || action.mechaHead > mechaHeadLimit)return false;
    if (action.mechaChest < 0 || action.mechaChest > mechaChestLimit)return false;
    if (mechaFoot < 0 || mechaFoot > mechaFootLimit)return false;
    return true;
  }
  else throw "unknown action.type";
}



float Game::getSkillScore() const
{
  float rate = isQieZhe ? ptScoreRate * 1.1 : ptScoreRate ;
  return rate * skillPt + skillScore;
}

static double scoringFactorOver1200(double x)//耐力胜负，脚色十分，追比
{
  if (x <= 1150)return 0;
  return tanh((x - 1150) / 100.0) * sqrt(x - 1150);
}

static double realRacingStatus(double x)
{
  if (x < 1200)return x;
  return 1200 + (x - 1200) / 4;
}

static double smoothUpperBound(double x)
{
  return (x - sqrt(x * x + 1)) / 2;
}

int Game::finalScore_mile() const
{
  double weights[5] = { 400,300,70,70,120 };
  double weights1200[5] = { 0,0,20,10,0 };


  double staminaTarget = 900;
  double staminaBonus = 5 * 100 * (smoothUpperBound((realRacingStatus(fiveStatus[1]) - staminaTarget) / 100.0) - smoothUpperBound((0 - staminaTarget) / 100.0));

  double total = 0;
  total += staminaBonus;
  for (int i = 0; i < 5; i++)
  {
    double realStat = realRacingStatus(min(fiveStatus[i], fiveStatusLimit[i]));
    total += weights[i] * sqrt(realStat);
    total += weights1200[i] * scoringFactorOver1200(realStat);
  }

  total += getSkillScore();
  if (total < 0)total = 0;
  //return uaf_haveLose ? 10000 : 20000;
  return (int)total;
}

int Game::finalScore_sum() const
{
  double weights[5] = { 5,3,3,3,3 };
  double total = 0;
  for (int i = 0; i < 5; i++)
  {
    double realStat = min(fiveStatus[i], fiveStatusLimit[i]);
    if (realStat > 1200)realStat = 1200 + (realStat - 1200) / 2;
    total += weights[i] * realStat;
  }

  total += getSkillScore();
  if (total < 0)total = 0;
  return (int)total;
}

int Game::finalScore_rank() const
{
  int total = 0;
  for (int i = 0; i < 5; i++)
    total += GameConstants::FiveStatusFinalScore[min(fiveStatus[i], fiveStatusLimit[i])];

  total += int(getSkillScore());
  //return uaf_haveLose ? 10000 : 20000;
  return total;
}

int Game::finalScore() const
{
  if (scoringMode == SM_normal)
  {
    return finalScore_rank();
  }
  else if (scoringMode == SM_race)
  {
    return finalScore_sum();
  }
  else if (scoringMode == SM_mile)
  {
    return finalScore_mile();
  }
  else
  {
    throw "此评分算法还未实现";
  }
  return 0;
}

bool Game::isEnd() const
{
  return turn >= TOTAL_TURN;
}

int Game::getTrainingLevel(int item) const
{
  if (isXiahesu())return 4;

  return trainLevelCount[item] / 4;
}

void Game::calculateLvGainSingle(int tra, int headNum, bool isShining)
{
  bool xhs = isXiahesu();
  int group = !mecha_hasGear[tra] ? 0 : !isShining ? 1 : 2;
  for (int i = 0; i < 5; i++)
    mecha_lvGain[tra][i] = 0;
  for (int sub = 0; sub < 3; sub++)
  {
    int type = GameConstants::Mecha_LvGainSubTrainIdx[tra][sub];
    int basic = GameConstants::Mecha_LvGainBasic[xhs][group][sub][headNum];
    double multiplier = mecha_lvGainMultiplier[type];
    int gain = int(multiplier * basic);
    if (gain == basic && multiplier > 1)//至少+1
      gain += 1;
    mecha_lvGain[tra][type] = gain;
  }
}

//Reference：https://github.com/mee1080/umasim/blob/main/core/src/commonMain/kotlin/io/github/mee1080/umasim/scenario/mecha/MechaStore.kt
void Game::calculateTrainingValueSingle(int tra)
{
  //先算下层------------------------------------------------------------------
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
  isTrainShining[tra] = shiningNum > 0;

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
        basicValue[i] += int(eff.bonus[i]);
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
  if (mecha_overdrive_enabled && mecha_upgradeTotal[0] >= 15)
    vitalCostMultiplier *= 0.5;
  int vitalChangeInt = vitalCostBasic > 0 ? -int(vitalCostBasic * vitalCostMultiplier) : -vitalCostBasic;
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


  //有彩圈的必有齿轮-----------------------------------------------------
  if (shiningNum > 0)
    mecha_hasGear[tra] = true;

  //算上层-----------------------------------------------------

  double scenarioTrainMultiplier = 1.0;//剧本总训练加成

  //研究等级加成
  double lvBonus = mecha_rivalLv[tra] > 1 ? 6 + 0.06 * mecha_rivalLv[tra] : 0;
  if (mecha_linkeffect_lvbonus)lvBonus *= 1.5;
  scenarioTrainMultiplier *= (1 + 0.01 * lvBonus);

  //有齿轮的训练有加成
  if (mecha_hasGear[tra])
  {
    double gearBonus =
      turn < 12 ? 3 :
      turn < 24 ? 6 :
      turn < 36 ? 10 :
      turn < 48 ? 16 :
      turn < 60 ? 20 :
      turn < 72 ? 25 :
      30;
    scenarioTrainMultiplier *= (1 + 0.01 * gearBonus);
  }

  //胸的3号升级，友情加成
  if(shiningNum > 0) 
  {
    double friendshipBonus = 2 * mecha_upgrade[1][2];
    scenarioTrainMultiplier *= (1 + 0.01 * friendshipBonus);
  }

  //overdrive
  if (mecha_overdrive_enabled)
  {
    //基础25%
    scenarioTrainMultiplier *= 1.25;

    //胸3级和12级
    double headBonus = 
      mecha_upgradeTotal[1] >= 12 ? 3 : 
      mecha_upgradeTotal[1] >= 3 ? 1 : 
      0;

    scenarioTrainMultiplier *= (1 + 0.01 * headNum * headBonus); 
  }



  //上层=总数-下层

  for (int i = 0; i < 6; i++)
  {
    int lower = trainValueLower[tra][i];
    if (lower > 100) lower = 100;
    trainValueLower[tra][i] = lower;
    int total = int(lower * scenarioTrainMultiplier * mecha_trainingStatusMultiplier[i]);
    int upper = total - lower;
    if (upper > 100)upper = 100;
    if (i < 5)
    {
      lower = calculateRealStatusGain(fiveStatus[i], lower);//consider the integer over 1200
      upper = calculateRealStatusGain(fiveStatus[i] + lower, upper);
    }
    total = upper + lower;
    trainValue[tra][i] = total;
  }

  calculateLvGainSingle(tra, headNum, shiningNum > 0);
}

void Game::addYayoiJiBan(int value)
{
  if (friend_type == FriendType_yayoi)
    addJiBan(friend_personId, value, true);
  else
    addJiBan(PSID_noncardYayoi, value, true);
}

int Game::getYayoiJiBan() const
{
  if (friend_type == FriendType_yayoi)
    return persons[friend_personId].friendship;
  else
    return friendship_noncard_yayoi;
}

void Game::checkEventAfterTrain(std::mt19937_64& rand)
{
  mecha_overdrive_enabled = false;
  checkFixedEvents(rand);
  checkRandomEvents(rand);


  //回合数+1
  turn++;
  isRacing = isRacingTurn[turn];
  gameStage = GameStage_beforeTrain;
  if (turn >= TOTAL_TURN)
  {
    printEvents("育成结束!");
    printEvents("你的得分是：" + to_string(finalScore()));
  }

}
void Game::checkFixedEvents(std::mt19937_64& rand)
{
  //处理各种固定事件

  if (isRefreshMind)
  {
    addVital(5);
    if (rand() % 4 == 0) //假设每回合有25%概率buff消失
      isRefreshMind = false;
  }
  if (isRacing)//生涯比赛
  {
    if (turn < 72)
    {
      runRace(3, 45);
      addYayoiJiBan(4);
    }
    else if (turn == 73)//ura1
    {
      runRace(10, 40);
    }
    else if (turn == 75)//ura1
    {
      runRace(10, 60);
    }
    else if (turn == 77)//ura3
    {
      runRace(10, 80);
    }

  }

  if (turn == 11)//出道赛
  {
    assert(isRacing);
  }
  else if (turn == 23)//第一年年底
  {
    //年底事件，体力低选择体力，否则选属性
    {
      int vitalSpace = maxVital - vital;//还差多少体力满
      handleFriendFixedEvent();
      if (vitalSpace >= 20)
        addVital(20);
      else
        addAllStatus(5);
    }
    printEvents("第一年结束");
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
  else if (turn == 35)
  {
    printEvents("第二年合宿开始");
  }
  else if (turn == 47)//第二年年底
  {
    //年底事件，体力低选择体力，否则选属性
    {
      int vitalSpace = maxVital - vital;//还差多少体力满
      if (vitalSpace >= 30)
        addVital(30);
      else
        addAllStatus(8);
    }
    printEvents("第二年结束");
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

    if (getYayoiJiBan() >= 60)
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
  else if (turn == 59)
  {
    printEvents("第三年合宿开始");
  }
  else if (turn == 70)
  {
    skillScore += 170;//固有技能等级+1
  }
  else if (turn == 77)//ura3，游戏结束
  {
    //比赛已经在前面处理了
    //记者
    if (friendship_noncard_reporter >= 80)
    {
      addAllStatus(5);
      skillPt += 20;
    }
    else if (friendship_noncard_reporter >= 60)
    {
      addAllStatus(3);
      skillPt += 10;
    }
    else if (friendship_noncard_reporter >= 40)
    {
      skillPt += 10;
    }
    else
    {
      skillPt += 5;
    }

    bool allWin = true;

    for (int c = 0; c < 5; c++)
    {
      if (mecha_win_history[c] != 2)
        allWin = false;
    }
    if (allWin)
    {
      skillPt += 40;//剧本金
      addAllStatus(60);
      skillPt += 150;
    }
    else 
    {
      addAllStatus(25);
      //there should be something, but not important
    }


    //友人卡事件
    handleFriendFixedEvent();

    addAllStatus(5);
    skillPt += 20;

    printEvents("ura3结束，游戏结算");
  }
}

void Game::checkRandomEvents(std::mt19937_64& rand)
{
  if (turn >= 72)
    return;//ura期间不会发生各种随机事件

  //友人会不会解锁出行
  if (friend_type != 0)
  {
    Person& p = persons[friend_personId];
    assert(p.personType == PersonType_friendCard);
    if (friend_stage==FriendStage_beforeUnlockOutgoing)
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
    addJiBan(card, 5, false);
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

  //加30体力（吃饭事件）
  if (randBool(rand, 0.02))
  {
    addVital(30);
    printEvents("模拟随机事件：体力+30");
  }

  //加心情
  if (randBool(rand, 0.02))
  {
    addMotivation(1);
    printEvents("模拟随机事件：心情+1");
  }

  //掉心情
  if (turn >= 12 && randBool(rand, 0.04))
  {
    addMotivation(-1);
    printEvents("模拟随机事件：\033[0m\033[33m心情-1\033[0m\033[32m");
  }

}
void Game::applyAction(std::mt19937_64& rand, Action action)
{
  if (isEnd()) return;
  if (action.type == GameStage_beforeMechaUpgrade)
  {
    throw "todo";
  }
  else
  {
    if (action.overdrive)//dish only, not next turn
    {
      bool suc = mecha_activate_overdrive();
      assert(suc && "Game::applyAction 无法开启overdrive");
    }
    if (action.train != TRA_none)
    {
      bool suc = applyTraining(rand, action.train);
      assert(suc && "Game::applyAction选择了不合法的训练");

      checkEventAfterTrain(rand);
      if (isEnd()) return;

      randomDistributeCards(rand);


      //非ura的比赛回合也可能吃菜，用来刷pt，所以不跳过

      //if (isRacing && !isUraRace)//非ura的比赛回合，直接跳到下一个回合
      //{
      //  Action emptyAction;
      //  emptyAction.train = TRA_none;
      //  emptyAction.dishType = DISH_none;
      //  applyAction(rand, emptyAction);
      //}
    }
  }
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
    throw "other friends or group cards are not supported";
  }
  return false;
}

