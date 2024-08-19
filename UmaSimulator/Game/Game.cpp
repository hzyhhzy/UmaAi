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

    if (persons[i].personType == PersonType_scenarioCard)
    {
      friend_personId = i;
      bool isSSR = cardId > 300000;
      if (isSSR)
        friend_type = 1;
      else
        friend_type = 2;
      int friendLevel = cardId % 10;
      assert(friendLevel >= 0 && friendLevel <= 4);
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
  probs = { 100,100,100,100,100,100 }; //速耐力根智鸽
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
  {
    cook_harvest_history[i] = -1;
    cook_harvest_green_history[i] = false;
  }
  for (int i = 0; i < 5; i++)
    cook_harvest_extra[i] = 0;

  for (int i = 0; i < 8; i++)
  {
    cook_train_material_type[i] = -1;
    cook_train_green[i] = false;
  }


  randomDistributeCards(rand); //随机分配卡组，包括计算属性
  
}

void Game::randomDistributeCards(std::mt19937_64& rand)
{
  //比赛回合的人头分配，不需要置零，因为不输入神经网络
  if (isRacing)
  {
    cook_main_race_material_type = rand() % 5;
    return;//比赛不用分配卡组
  }
  
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
      if (pid >= 6)continue;

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
  bool restGreen = isXiahesu() ? true : randBool(rand, GameConstants::Cook_RestGreenRate);
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

//总数=(1+料理pt加成+吃菜加成)*(1+料理pt技能点加成)
//上层=min(总数-下层, 100)
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
bool Game::isDishLegal(int dishId) const
{
  //同一回合不能重复制作料理
  if (cook_dish != DISH_none)
    return false;

  int dishLevel = GameConstants::Cook_DishLevel[dishId];
  //检查是否达到解锁时间
  if (dishLevel == 0)
    return false;
  else if (dishLevel == 1)
  {
    //nothing
  }
  else if (dishLevel == 2)
  { 
    if (turn < 24)
      return false;
  }
  else if (dishLevel == 3)
  {
    if (turn < 48)
      return false;
  }
  else if (dishLevel == 4)
  {
    if (turn < 72)
      return false;
  }
  else
    throw "ERROR: Game::isDishLegal Unknown dish level";


  //检查材料够不够
  for (int i = 0; i < 5; i++)
  {
    int matCost = GameConstants::Cook_DishCost[dishId][i];
    if (dishId == DISH_g1plate)//如果不是超满足，G1Plate要涨价
    {
      //默认超满足
      if (cook_win_history[4] < 2)//大满足
        matCost = 100;
    }
    if (cook_material[i] < matCost)
      return false;
  }
  return true;
}

int Game::maxFarmPtUntilNow() const
{
  int totalCost = 0;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 1; j < cook_farm_level[i]; j++)
    {
      totalCost += GameConstants::Cook_FarmLvCost[j];
    }
  }

  //计算假如全程绿圈，最多多少pt
  int normalCycleNum = turn <= 39 ? turn / 4 :
    turn <= 63 ? turn / 4 - 1 :
    turn <= 72 ? turn / 4 - 2 :
    72 / 4 - 2;
  int smallCycleNum = turn <= 36 ? 0 :
    turn <= 39 ? turn - 36 :
    turn <= 60 ? 4 :
    turn <= 63 ? 4 + turn - 60 :
    turn <= 72 ? 8 :
    8 + turn - 72;
  int maxFarmPt = normalCycleNum * 160 + smallCycleNum * 75;
  return maxFarmPt - totalCost;
}

std::vector<int> Game::calculateHarvestNum(bool isAfterTrain) const
{
  bool smallHarvest = isXiahesu() || turn >= 72;
  int harvestTurnNum = smallHarvest ? 1 : 4;
  if (!isAfterTrain)//只供训练前显示
  {
    harvestTurnNum = smallHarvest ? 0 : turn % 4;
  }

  vector<int> harvestBasic = { 0,0,0,0,0 };
  int greenNum = 0;
  for (int i = 0; i < 5; i++)
  {
    harvestBasic[i] = GameConstants::Cook_HarvestBasic[cook_farm_level[i]];
    if (smallHarvest)
      harvestBasic[i] = harvestBasic[i] / 2;
    harvestBasic[i] += cook_harvest_extra[i];
  }
  for (int i = 0; i < harvestTurnNum; i++)
  {
    int matType = cook_harvest_history[i];
    if (matType == -1)
      throw "ERROR: Game::maybeHarvest cook_harvest_history[i] == -1";

    harvestBasic[matType] += GameConstants::Cook_HarvestExtra[cook_farm_level[matType]];
    if (cook_harvest_green_history[i])
      greenNum += 1;
  }

  double multiplier = smallHarvest ? (greenNum == 0 ? 1.0000001 : 1.5000001) :
    greenNum == 0 ? 1.0000001 :
    greenNum == 1 ? 1.1000001 :
    greenNum == 2 ? 1.2000001 :
    greenNum == 3 ? 1.4000001 :
    greenNum == 4 ? 1.6000001 :
    1000;

  int farmPt = smallHarvest ? int(multiplier * 50) : int(multiplier * 100);
  for(int i=0;i<5;i++)
    harvestBasic[i] = int(harvestBasic[i] * multiplier);
  harvestBasic.push_back(farmPt);
  return harvestBasic;
}
void Game::maybeHarvest()
{
  if (!(isXiahesu() || turn >= 72 || turn % 4 == 3))
    return;//no harvest
  printEvents("农田收获");
  vector<int> harvest = calculateHarvestNum(true);

  for (int i = 0; i < 5; i++)
    addDishMaterial(i, harvest[i]);
  cook_farm_pt += harvest[5];

  //clear
  for (int i = 0; i < 4; i++)
  {
    cook_harvest_history[i] = -1;
    cook_harvest_green_history[i] = false;
  }
  for (int i = 0; i < 5; i++)
    cook_harvest_extra[i] = 0;
}

void Game::addTrainingLevelCount(int trainIdx, int n)
{
  trainLevelCount[trainIdx] += n;
  if (trainLevelCount[trainIdx] > 16)trainLevelCount[trainIdx] = 16;
}
void Game::checkDishPtUpgrade()
{
  int oldDishPt = cook_dish_pt_turn_begin;
  if (GameConstants::Cook_DishPtLevel(oldDishPt) != GameConstants::Cook_DishPtLevel(cook_dish_pt))
  {
    printEvents("料理pt达到下一阶段");
    //upgrade deyilv
    updateDeyilv();
  }
  if ((oldDishPt < 2000 && cook_dish_pt >= 2000)
   // || (oldDishPt < 5000 && cook_dish_pt >= 5000)
    || (oldDishPt < 7000 && cook_dish_pt >= 7000)
    || (oldDishPt < 12000 && cook_dish_pt >= 12000)
    )
  {
    printEvents("食意开眼，全体训练等级+1");
    for (int i = 0; i < 5; i++)
      addTrainingLevelCount(i, 4);
  }

  cook_dish_pt_turn_begin = cook_dish_pt;
}
bool Game::makeDish(int16_t dishId, std::mt19937_64& rand)
{
  if (!isDishLegal(dishId))
    return false;

  //扣除材料
  for (int i = 0; i < 5; i++)
  {
    int matCost = GameConstants::Cook_DishCost[dishId][i];
    if (dishId == DISH_g1plate)//如果不是超满足，G1Plate要涨价
    {
      //默认超满足
      if (cook_win_history[4] < 2)//大满足
        matCost = 100;
    }
    cook_material[i] -= matCost;
  }
  cook_dish = dishId;

  //升级农田
  autoUpgradeFarm(false);

  //检查是否大成功
  bool isBigSuccess = cook_dish_sure_success ? true : randBool(rand, 0.01 * cook_dishpt_success_rate);
  if (isBigSuccess)
    handleDishBigSuccess(dishId, rand);

  //计算料理pt
  assert(cook_dish_pt == cook_dish_pt_turn_begin);
  int pt = GameConstants::Cook_DishGainPt[dishId];
  cook_dish_pt += pt;

  cook_dish_sure_success = false;
  //如果跨越1500倍数了，或者大于12000，下次必为大成功
  if (cook_dish_pt >= 12000 || cook_dish_pt / 1500 != cook_dish_pt_turn_begin / 1500)
    cook_dish_sure_success = true;

  //料理的训练加成以外的效果：体力和羁绊
  int dishLevel = GameConstants::Cook_DishLevel[dishId];
  if (dishLevel == 1)
  {
    for (int i = 0; i < 6; i++)
      addJiBan(i, 2, true);
  }
  else if (dishLevel == 2)
  {
    int mainTrain = GameConstants::Cook_DishMainTraining[dishId];
    int farmLevel = cook_farm_level[mainTrain];
    if (farmLevel >= 3)
      addVital(5);
  }
  else if (dishLevel == 3)
  {
    int mainTrain = GameConstants::Cook_DishMainTraining[dishId];
    int farmLevel = cook_farm_level[mainTrain];
    if(mainTrain!=4)
      addVital(10);
    if (farmLevel >= 3)
      addVital(5);
  }
  else if (dishLevel == 4)
  {
    addVital(25);
  }
  else
    throw "ERROR: Game::makeDish Unknown dish level";

  calculateTrainingValue();
  return true;
}
void Game::handleDishBigSuccess(int dishId, std::mt19937_64& rand)
{
  //hint
  dishBigSuccess_hint(rand);

  //buff
  std::vector<int> buffs = dishBigSuccess_getBuffs(dishId, rand);
  //如果有体力最大值，则先加体力最大值
  for (int i = 0; i < buffs.size(); i++)
  {
    if (buffs[i] == 5)
    {
      addVitalMax(4);
      break;
    }
  }
  for (int i = 0; i < buffs.size(); i++)
  {
    if (buffs[i] == 1)
    {
      addVital(10);
      printEvents("料理大成功：体力+10");
    }
    else if (buffs[i] == 2)
    {
      addMotivation(1);
      printEvents("料理大成功：干劲+1");
    }
    else if (buffs[i] == 3)
    {
      for (int i = 0; i < 6; i++)
        addJiBan(i, 3, true);
      printEvents("料理大成功：全体羁绊+3");
    }
    else if (buffs[i] == 4)
    {
      int dishlevel = GameConstants::Cook_DishLevel[dishId];
      if (dishlevel == 4)
      {
        //G1Plate菜，每个训练邀请2个人
        for (int tr = 0; tr < 5; tr++)
        {
          dishBigSuccess_invitePeople(tr, rand);
          dishBigSuccess_invitePeople(tr, rand);
        }
      }
      else if (dishlevel == 3 || dishlevel == 2)
      {
        //其他菜，相应训练邀请1个人
        int mainTrain = GameConstants::Cook_DishMainTraining[dishId];
        dishBigSuccess_invitePeople(mainTrain, rand);
      }
      else
        throw "ERROR: Game::handleDishBigSuccess buffs[i] == 4 but dishlevel != 2 or 3 or 4";
      printEvents("料理大成功：摇人");
    }
    else if (buffs[i] == 5)
    {
      //已经处理过了
      printEvents("料理大成功：体力上限+4");
    }
    else
      throw "ERROR: Game::handleDishBigSuccess Unknown buff type";
  }

}
void Game::updateDeyilv()
{
  int deyilvBonus = GameConstants::Cook_DishPtDeyilvBonus[GameConstants::Cook_DishPtLevel(cook_dish_pt)];
  for (int i = 0; i < 6; i++)
  {
    if (persons[i].personType == PersonType_card)
    {
      persons[i].setExtraDeyilvBonus(deyilvBonus);
    }
  }
}
void Game::dishBigSuccess_hint(std::mt19937_64& rand)
{
  vector<int> availableHintLevels;
  //随机选一张卡hint
  for (int i = 0; i < 6; i++)
  {
    int hintLevel = persons[i].personType == PersonType_card ? persons[i].cardParam.hintLevel : 0;
    if (persons[i].cardParam.isLink && hintLevel != 0 && hintLevel < 5)hintLevel += 1;
    if (hintLevel > 0)
      availableHintLevels.push_back(i);
  }
  int hintlevel = 1;
  if (availableHintLevels.size() > 0)
    hintlevel = availableHintLevels[rand() % availableHintLevels.size()];
  printEvents("料理大成功：hint +" + to_string(hintlevel));
  skillPt += int(hintlevel * hintPtRate);
}
void Game::dishBigSuccess_invitePeople(int trainIdx, std::mt19937_64& rand)
{
  //先数一下已经有几个人
  int count = 0;
  bool cannotInvite[6] = { true, true, true, true, true, true };//不在任何一个训练，或者已经在当前训练
  for (int tra = 0; tra < 5; tra++)
  {
    if (tra == trainIdx)continue;
    for (int idx = 0; idx < 5; idx++)
    {
      int pid = personDistribution[tra][idx];
      if (pid == -1)continue;
      if (pid >= 0 && pid < 6)
      {
        cannotInvite[pid] = false;
      }
    }
  }
  for (int i = 0; i < 5; i++)
  {
    int pid = personDistribution[trainIdx][i];
    if (pid == PSID_none)
      break;
    count++;

    if (pid >= 0 && pid < 6)
    {
      cannotInvite[pid] = true;
    }
  }
  if (count >= 5)return;//已经满了


  //从不在personDistribution[trainIdx]里的人里选一个，放到里面
  vector<int> availablePeople;
  for (int i = 0; i < 6; i++)
  {
    if (!cannotInvite[i])
      availablePeople.push_back(i);
  }
  if (availablePeople.size() == 0)
  {
    return;//可能其他训练都是当前训练的复制人头，因此没有可邀请的人
    //throw "ERROR: Game::dishBigSuccess_invitePeople availablePeople.size() == 0 && turn < 72";
  }
  int pid = availablePeople[rand() % availablePeople.size()];
  personDistribution[trainIdx][count] = pid;
  //require recalculate later
}
void Game::autoUpgradeFarm(bool beforeXiahesu)
{
  if (farmUpgradeStrategy == FUS_none)
    return;

  if (isXiahesu())
    return;

  //第一年，只在确定训练后收菜前升级
  if (turn < 24)
  {
    if (turn % 4 != 3) //下回合收菜才升级
      return;
    if (gameStage != GameStage_afterTrain)//确定训练后才升级
      return;
    if (cook_farm_pt < GameConstants::Cook_FarmLvCost[1])//升级pt不够
      return;

    int value[5] = { 5,1,4,2,3 };//优先级
    //已经lv2的不升级
    for (int i = 0; i < 5; i++)
    {
      if (cook_farm_level[i] == 2)
        value[i] -= 99999;
    }
    //统计4回合内点击次数，点击一次value+10，快溢出则额外加
    int clickNums[5] = { 0,0,0,0,0 };
    for (int i = 0; i < 4; i++)
    {
      int type = cook_harvest_history[i];
      if (type == -1)
        throw "ERROR: Game::autoUpgradeFarm cook_harvest_history[i] == -1，第一年训练后收菜前才可自动升级农田";
      clickNums[type] += 1;
    }
    for (int i = 0; i < 5; i++)
    {
      value[i] += clickNums[i] * 10;
      int overflow = 30 + 25 * clickNums[i] + cook_material[i] - GameConstants::Cook_MaterialLimit[cook_farm_level[i]];
      if(overflow>0)
        value[i] += overflow;
    }
  
    if (farmUpgradeStrategy == FUS_default)
    {
      //第一年不升lv2大蒜，除非溢出太多
      value[1] -= 40;
    }
    //选出最大的
    int maxIdx = 0;
    for (int i = 0; i < 5; i++)
    {
      if (value[i] > value[maxIdx])
        maxIdx = i;
    }
    //value小于0不升级
    if (value[maxIdx] >= 0)
    {
      bool suc = upgradeFarm(maxIdx);
      assert(suc);
      autoUpgradeFarm(beforeXiahesu);//有可能再升级一个
    }
  }
  //第二年，吃菜前或者收菜前升级
  else if (turn < 48)
  {
    if (gameStage == GameStage_beforeTrain)//吃菜前升级，可以贪LV3的5体力
    {
      if (GameConstants::Cook_DishLevel[cook_dish] != 2)
        return;
      if (cook_farm_pt < GameConstants::Cook_FarmLvCost[2])
        return;//升级pt不够
      if (maxVital - vital <= 0)
        return;//体力满了
      int mainTrain = GameConstants::Cook_DishMainTraining[cook_dish];
      if (cook_farm_level[mainTrain] == 3)
        return;//已经lv3了
      //大蒜（编号1）不升级，草莓（编号4）在其他三个没升级之前不升级
      if (mainTrain == 1
        || (mainTrain == 4 && (cook_farm_level[0] != 3 || cook_farm_level[2] != 3 || cook_farm_level[3] != 3)))
        return;
      bool suc = upgradeFarm(mainTrain);
      assert(suc);
    }
    else if (gameStage == GameStage_afterTrain)//收菜前升级
    {
      if (turn % 4 != 3) //下回合收菜才升级
        return;
      int value[5] = { 25,-59,24,23,2 };//优先级
      //已经lv3的不升级，lv1的提升权重
      for (int i = 0; i < 5; i++)
      {
        if (cook_farm_level[i] == 3)
          value[i] -= 99999;
        if (cook_farm_level[i] == 1)
          value[i] += 60; //这个数恰好可以让蒜溢出时升级lv2，而不升级lv3
      }
      if(beforeXiahesu)
      {
        //nothing
      }
      else
      {
        //统计4回合内点击数，快溢出则额外加
        int clickNums[5] = { 0,0,0,0,0 };
        for (int i = 0; i < 4; i++)
        {
          int type = cook_harvest_history[i];
          if (type == -1)
            throw "ERROR: Game::autoUpgradeFarm cook_harvest_history[i] == -1，第二年训练后收菜前才可自动升级农田";
          clickNums[type] += 1;
        }
        for (int i = 0; i < 5; i++)
        {
          int overflow = 55 + 45 * clickNums[i] + cook_material[i] - GameConstants::Cook_MaterialLimit[cook_farm_level[i]];
          if (overflow > 0)
            value[i] += overflow;
        }
      }
      //选出最大的
      int maxIdx = 0;
      for (int i = 0; i < 5; i++)
      {
        if (value[i] > value[maxIdx])
          maxIdx = i;
      }
      //value小于0不升级
      if (value[maxIdx] >= 0 && cook_farm_pt >= GameConstants::Cook_FarmLvCost[cook_farm_level[maxIdx]])
      {
        bool suc = upgradeFarm(maxIdx);
        assert(suc);
        autoUpgradeFarm(beforeXiahesu);//有可能再升级一个
      }
    }
    else
      assert(false);
  }
  //第三年，收菜前升级
  else if (turn < 72)
  {
    if (gameStage == GameStage_beforeTrain)//吃菜前不升级
      return;
    if (turn % 4 != 3) //下回合收菜才升级
      return;

    //升级路线：32333 42443 43443 43453 53553
    int value[5] = { 283,140,281,282,160 };//优先级
    int priorLv5[5] = { 3,1,4,5,2 };//优先级
    //根据lv调整权重
    for (int i = 0; i < 5; i++)
    {
      if (cook_farm_level[i] == 1)
        value[i] += 1000;
      else if (cook_farm_level[i] == 2)
        value[i] += 200;
      else if (cook_farm_level[i] == 3)
        value[i] += 100;
      else if (cook_farm_level[i] == 4)
        value[i] += 0;
      else if (cook_farm_level[i] == 5)
        value[i] -= 99999;
    }

    if (beforeXiahesu)
    {
      for (int i = 0; i < 5; i++)
      {
        if (cook_farm_level[i] <= 3)
          value[i] += 0;
        else
          value[i] += 3 * priorLv5[i];
      }
    }
    else
    {
      //统计4回合内点击数，快溢出则额外加
      int clickNums[5] = { 0,0,0,0,0 };
      for (int i = 0; i < 4; i++)
      {
        int type = cook_harvest_history[i];
        if (type == -1)
          throw "ERROR: Game::autoUpgradeFarm cook_harvest_history[i] == -1，第二年训练后收菜前才可自动升级农田";
        clickNums[type] += 1;
      }
      for (int i = 0; i < 5; i++)
      {
        if (cook_farm_level[i] <= 3)
          value[i] += clickNums[i] * 15;
        else
          value[i] += 3 * priorLv5[i];
        int overflow = 55 + 45 * clickNums[i] + cook_material[i] - GameConstants::Cook_MaterialLimit[cook_farm_level[i]];
        if (overflow > 0)
          value[i] += overflow;
      }
    }
    //选出最大的
    int maxIdx = 0;
    for (int i = 0; i < 5; i++)
    {
      if (value[i] > value[maxIdx])
        maxIdx = i;
    }
    //value小于0不升级
    if (value[maxIdx] >= 0 && cook_farm_pt >= GameConstants::Cook_FarmLvCost[cook_farm_level[maxIdx]])
    {
      bool suc = upgradeFarm(maxIdx);
      assert(suc);
      autoUpgradeFarm(beforeXiahesu);//有可能再升级一个
    }
    

  }
  //ura期间，先升5
  else 
  {
    if (gameStage == GameStage_beforeTrain)//吃菜前不升级
      return;

    int value[5] = { 1000 - cook_material[0], 1000 - cook_material[1], 1000 - cook_material[2], 1000 - cook_material[3], 1000 - cook_material[4], };//优先级
    //根据lv调整权重
    for (int i = 0; i < 5; i++)
    {
      if (cook_farm_level[i] == 1)
        value[i] += 1000;
      else if (cook_farm_level[i] == 2)
        value[i] += 200;
      else if (cook_farm_level[i] == 3)
        value[i] += 0;
      else if (cook_farm_level[i] == 4)
        value[i] += 1000;
      else if (cook_farm_level[i] == 5)
        value[i] -= 99999;
    }
    //选出最大的
    int maxIdx = 0;
    for (int i = 0; i < 5; i++)
    {
      if (value[i] > value[maxIdx])
        maxIdx = i;
    }
    //value小于0不升级
    if (value[maxIdx] >= 0 && cook_farm_pt >= GameConstants::Cook_FarmLvCost[cook_farm_level[maxIdx]])
    {
      bool suc = upgradeFarm(maxIdx);
      assert(suc);
      autoUpgradeFarm(beforeXiahesu);//有可能再升级一个
    }


    }
}
void Game::addDishMaterial(int idx, int value)
{
  cook_material[idx] += value;
  int limit = GameConstants::Cook_MaterialLimit[cook_farm_level[idx]];
  if (cook_material[idx] > limit)
    cook_material[idx] = limit;
}
std::vector<int> Game::dishBigSuccess_getBuffs(int dishId, std::mt19937_64& rand)
{
  //1体力，2心情，3羁绊，4分身，5体力上限
  vector<int> buffs;

  int dishLevel = GameConstants::Cook_DishLevel[dishId];

  //写一个局部函数，判断这个buff是否允许
  auto isBuffLegal = [&](int buffType)
    {
      if (buffType == 1)//体力
        return true;
      else if (buffType == 2)//心情
        return true;
      else if (buffType == 3)//羁绊，满羁绊不触发
      {
        for (int i = 0; i < 6; i++)
          if (persons[i].friendship < 100)
            return true;
        return false;
      }
      else if (buffType == 4)//分身，已经满人的不触发
      {
        if (dishLevel == 4)
          return true;
        if (dishLevel == 1)
          return false;
        int mainTrain = GameConstants::Cook_DishMainTraining[dishId];
        if (personDistribution[mainTrain][4] == PSID_none)
        {
          //检查其他训练是否有可以邀请的人
          for (int tra = 0; tra < 5; tra++)
          {
            if (tra == mainTrain)continue;
            for (int idx = 0; idx < 5; idx++)
            {
              int pid = personDistribution[tra][idx];
              if (pid == -1)continue;
              if (pid >= 0 && pid < 6)
                return true;
            }
          }
        }
        return false;
      }
      else if (buffType == 5)//体力上限，满120不触发
        return maxVital < 120;
      
      throw "ERROR: Game::dishBigSuccess_getBuffs Unknown buff type";
    };

  vector<int> availableBuffs;
  vector<int> buffRelativeProbs;
  for (int i = 1; i <= 5; i++)
  {
    int prob = GameConstants::Cook_DishPtBigSuccessBuffProb[dishLevel][i];
    if (isBuffLegal(i) && prob > 0)
    {
      availableBuffs.push_back(i);
      buffRelativeProbs.push_back(prob);
    }
  }
  if (availableBuffs.size() == 0)
    throw "ERROR: Game::dishBigSuccess_getBuffs availableBuffs.size() == 0";
  //random pick one buff
  std::discrete_distribution<> distribution(buffRelativeProbs.begin(), buffRelativeProbs.end());
  buffs.push_back(availableBuffs[distribution(rand)]);

  //追加buff
  for (int i = 0; i < 5; i++)
  {
    int prob = GameConstants::Cook_DishPtBigSuccessBuffExtraProb[dishLevel][i];
    if (prob == 0)continue;
    if (!isBuffLegal(i))continue;
    if (randBool(rand, prob * 0.01))
    {
      //检查是否已经有这个buff
      bool hasBuff = false;
      for (int j = 0; j < buffs.size(); j++)
      {
        if (buffs[j] == i)
        {
          hasBuff = true;
          break;
        }
      }
      if (!hasBuff)
        buffs.push_back(i);
    }
  }
  return buffs;


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
  if(idx==PSID_noncardYayoi)
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
  //粗略拟合的训练失败率，二次函数 A*(x0-x)^2+B*(x0-x)
  //误差应该在2%以内
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
  double dishMultiply = 1.0;
  //dish race bonus
  if (cook_dish != DISH_none)
  {
    double extraBonus = 0;
    int dishLevel = GameConstants::Cook_DishLevel[cook_dish];
    if (dishLevel == 3)
    {
      if (turn < 60)extraBonus = 25;
      else extraBonus = 35;
    }
    else if (dishLevel == 4)
    {
      int lv5Count = 0;
      for (int i = 0; i < 5; i++)
        if (cook_farm_level[i] >= 5)
          lv5Count++;
      extraBonus = 80 + 5 * lv5Count;
    }
    dishMultiply = 1 + 0.01 * extraBonus;
  }

  int fiveStatusBonus = int(dishMultiply * int(raceMultiply * basicFiveStatusBonus));
  int ptBonus = int(dishMultiply * int(raceMultiply * basicPtBonus));
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
  else assert(false && "未知的出行");

  //全体菜+40
  for (int i = 0; i < 5; i++)
    addDishMaterial(i, 40);

  friend_outgoingUsed += 1;
}
void Game::handleFriendUnlock(std::mt19937_64& rand)
{
  assert(friend_stage == FriendStage_beforeUnlockOutgoing);
  if (maxVital - vital >= 40)
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
  friend_stage = FriendStage_afterUnlockOutgoing;
}
void Game::handleFriendClickEvent(std::mt19937_64& rand, int atTrain)
{
  assert(friend_type!=0 && (friend_personId<6&& friend_personId>=0) && persons[friend_personId].personType==PersonType_scenarioCard);
  if (friend_stage == FriendStage_notClicked)
  {
    printEvents("第一次点友人");
    friend_stage = FriendStage_beforeUnlockOutgoing;
    
    addStatusFriend(0, 14);
    addJiBan(friend_personId, 10, false);
    addMotivation(1);
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

}
void Game::handleFriendFixedEvent()
{
  if (friend_type == 0)return;//没友人卡
  if (friend_stage < FriendStage_beforeUnlockOutgoing)return;//出行没解锁就没事件
  if (turn == 23)
  {
    addMotivation(1);
    addStatusFriend(0, 24);
    addJiBan(friend_personId, 5, false);
    skillPt += 40;//三级中盘巧者，而且有进化，因此这个hint是有效的
  }
  else if (turn == 77)
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
  else
  {
    assert(false && "其他回合没有友人固定事件");
  }
}
bool Game::applyTraining(std::mt19937_64& rand, int train)
{
  assert(gameStage == GameStage_beforeTrain);
  int matType = -1;//此回合的菜的种类
  int matExtra = 0;//此回合的菜的额外加成（训练人头数）
  bool isGreen = false;//此回合的菜是否绿圈
  if (isRacing)
  {
    //比赛收益在checkEventAfterTrain()里处理，此处只处理菜
    assert(train == TRA_none || train == TRA_race);
    matType = cook_main_race_material_type;
    matExtra = 0;
    isGreen = true;
    

    //assert(false && "所有剧本比赛都在checkEventAfterTrain()里处理，不能applyTraining");
    //return false;//所有剧本比赛都在checkEventAfterTrain()里处理（相当于比赛回合直接跳过），不在这个函数
  }
  else
  {
    matType = cook_train_material_type[train];
    matExtra = cook_train_material_num_extra[train];
    isGreen = cook_train_green[train];//如果训练失败，后续将其设为false


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
        isGreen = true;
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
    }
    else if (train <= 4 && train >= 0)//常规训练
    {
      if (rand() % 100 < failRate[train])//训练失败
      {
        isGreen = false;
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
        if (friend_type == 1)
          friendshipExtra += 1;

        vector<int> hintCards;//有哪几个卡出红感叹号了
        bool clickFriend = false;//这个训练有没有友人
        //检查SSR友人在不在这里
        for (int i = 0; i < 5; i++)
        {
          int p = personDistribution[train][i];
          if (p == PSID_none)break;//没人
          if (friend_type == 1 && p == friend_personId)
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
            assert(persons[p].personType == PersonType_scenarioCard);
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
          int hintCard = hintCards[rand() % hintCards.size()];//随机一张卡出hint

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

        if (clickFriend)
          handleFriendClickEvent(rand, train);


        //训练等级提升
        addTrainingLevelCount(train, 1);

      }

    }
    else
    {
      printEvents("未知的训练项目");
      return false;
    }
  }


  //种菜
  addFarm(matType, matExtra, isGreen);
  gameStage = GameStage_afterTrain;
  autoUpgradeFarm(false);
  return true;
}


bool Game::isLegal(Action action) const
{
  if (!action.isActionStandard())
    return false;

  //是否吃得起菜
  if (action.dishType != DISH_none)
    if (!isDishLegal(action.dishType))
      return false;

  if (isRacing)
  {
    //if (isUraRace)
    //{
      if (action.train == TRA_none || action.train == TRA_race)//none是吃菜然后比赛，race是直接比赛
        return true;
      else
        return false;
    //}
    //else
    //{
      //assert(false && "所有ura以外的剧本比赛都在checkEventAfterTrain()里处理，不能applyTraining");
      //return false;//所有剧本比赛都在checkEventAfterTrain()里处理（相当于比赛回合直接跳过），不在这个函数
    //}
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
  else if (action.dishType != DISH_none)
  {
    return isDishLegal(action.dishType);
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

int Game::turnIdxInHarvestLoop() const
{
  if (isXiahesu() || turn >= 72)return 0;
  else return turn % 4;
}
bool Game::upgradeFarm(int item)
{
  if (isXiahesu())return false;
  int lv = cook_farm_level[item];
  if (lv >= 5)
    return false;
  else if (lv >= 3 && turn < 48)
    return false;
  else if (lv >= 2 && turn < 24)
    return false;

  if (cook_farm_pt < GameConstants::Cook_FarmLvCost[lv])
    return false;
  cook_farm_pt -= GameConstants::Cook_FarmLvCost[lv];
  cook_farm_level[item] += 1;
  printEvents(GameConstants::Cook_MaterialNames[item] + "升至" + to_string(cook_farm_level[item]) + "级");
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
    trainValueLower[tra][i] = lower;
    double multiplier = i < 5 ? scenarioTrainMultiplier : skillPtMultiplier;
    int total = int(lower * multiplier);
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


}

void Game::addYayoiJiBan(int value)
{
  if (friend_type != 0)
    addJiBan(friend_personId, value, true);
  else
    addJiBan(PSID_noncardYayoi, value, true);
}

int Game::getYayoiJiBan() const
{
  if (friend_type != 0)
    return persons[friend_personId].friendship;
  else
    return friendship_noncard_yayoi;
}

int Game::getDishTrainingBonus(int trainIdx) const
{
  if (cook_dish == DISH_none)return 0;
  if (!GameConstants::Cook_DishTrainingBonusEffective[cook_dish][trainIdx])
    return 0;
  int dishLevel = GameConstants::Cook_DishLevel[cook_dish];
  if (dishLevel == 1)
    return 25;
  else if (dishLevel == 2)
  {
    int b = 50;
    int mainTrainingIdx = GameConstants::Cook_DishMainTraining[cook_dish];
    if (cook_farm_level[mainTrainingIdx] >= 5)
      b += 10;
    if (turn >= 36)
      b += 10;
    return b;
  }
  else if (dishLevel == 3)
  {
    int b = 80;
    int mainTrainingIdx = GameConstants::Cook_DishMainTraining[cook_dish];
    if(cook_farm_level[mainTrainingIdx] >= 5)
      b += 10;
    if (turn >= 60)
      b += 10;
    if (mainTrainingIdx == 4)
      b += 10;
    return b;
  }
  else if (dishLevel == 4)
  {
    int lv5Count = 0;
    for (int i = 0; i < 5; i++)
      if (cook_farm_level[i] >= 5)
        lv5Count++;
    return 165 + 5 * lv5Count;
  }
  else
    throw "ERROR: Game::getDishTrainingBonus Unknown dish level";
  return 100000;
}

void Game::checkEventAfterTrain(std::mt19937_64& rand)
{
  assert(gameStage == GameStage_afterTrain);
  checkFixedEvents(rand);
  checkRandomEvents(rand);

  cook_dish = DISH_none;

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
void Game::maybeCookingMeeting()
{
  if (turn == 23)
  {
    if (cook_dish_pt >= 1000)
    {
      cook_win_history[0] = 1;
      addAllStatus(5);
      skillPt += 40;
    }
    else
    {
      addAllStatus(3);
      skillPt += 30;
    }
  }
  else if (turn == 35)
  {
    if (cook_dish_pt >= 2000)
    {
      cook_win_history[1] = 1;
      addAllStatus(10);
      skillPt += 50;
    }
    else
    {
      addAllStatus(5);
      skillPt += 40;
    }
  }
  else if (turn == 47)
  {
    if (cook_dish_pt >= 5000)
    {
      cook_win_history[2] = 1;
      addAllStatus(15);
      skillPt += 60;
    }
    else
    {
      addAllStatus(10);
      skillPt += 50;
    }
  }
  else if (turn == 59)
  {
    if (cook_dish_pt >= 7000)
    {
      cook_win_history[3] = 1;
      addAllStatus(20);
      skillPt += 70;
    }
    else
    {
      addAllStatus(15);
      skillPt += 60;
    }
  }
  else if (turn == 71)
  {
    if (cook_dish_pt >= 12000)
    {
      cook_win_history[4] = 2;
      addAllStatus(25);
      skillPt += 70;
    }
    else if(cook_dish_pt >= 10000)
    {
      cook_win_history[4] = 1;
      addAllStatus(20); //dont know, but not important
      skillPt += 60;
    }
    else
    {
      addAllStatus(20);
      skillPt += 60;
    }
  }
}
void Game::checkFixedEvents(std::mt19937_64& rand)
{
  //处理各种固定事件
  checkDishPtUpgrade();
  maybeHarvest(); 
  maybeCookingMeeting();
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
    autoUpgradeFarm(true);//合宿前升级农田
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
    autoUpgradeFarm(true);//合宿前升级农田
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
      if (c < 4 && cook_win_history[c] != 1)
        allWin = false;
      if (c == 4 && cook_win_history[c] != 2)
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
    assert(p.personType == PersonType_scenarioCard);
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
void Game::addFarm(int type, int extra, bool isGreen)
{
  int harvestLoopIdx = turnIdxInHarvestLoop();
  cook_harvest_history[harvestLoopIdx] = type;
  cook_harvest_extra[type] += extra;
  if (isGreen)
    cook_harvest_green_history[harvestLoopIdx] = true;

}
void Game::applyAction(std::mt19937_64& rand, Action action)
{
  if (isEnd()) return;
  //assert(turn < TOTAL_TURN && "Game::applyTrainingAndNextTurn游戏已结束");
  //assert(!(isRacing && !isUraRace) && "非ura的比赛回合都在checkEventAfterTrain里跳过了");
  if (action.dishType != DISH_none)//dish only, not next turn
  {
    bool suc = makeDish(action.dishType, rand);
    assert(suc && "Game::applyAction选择了不合法的菜品");
  }
  if (action.train != TRA_none || isRacing)
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

