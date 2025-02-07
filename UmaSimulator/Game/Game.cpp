#include <iostream>
#include <cassert>
#include "Game.h"
using namespace std;
static bool randBool(mt19937_64& rand, double p)
{
  return rand() % 65536 < p * 65536;
}

//������Game���˳��һ��
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
  assert(isRacingTurn[11] == true);//������
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
    fiveStatus[i] = GameDatabase::AllUmas[umaId].fiveStatusInitial[i] - 10 * (5 - umaStars); //�������ʼֵ
  for (int i = 0; i < 5; i++)
    fiveStatusLimit[i] = GameConstants::BasicFiveStatusLimit[i]; //ԭʼ��������

  skillPt = 120;
  skillScore = umaStars >= 3 ? 170 * (umaStars - 2) : 120 * (umaStars);//���м���

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
    fiveStatusLimit[i] += int(zhongMaBlueCount[i] * 5.34 * 2); //��������--�������ֵ
  for (int i = 0; i < 5; i++)
    addStatus(i, zhongMaBlueCount[i] * 7); //����

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
      friend_statusBonus += 1e-10;//�Ӹ�С����������Ϊ�����������
    }
  }

  std::vector<int> probs = { 100,100,100,100,100,200 }; //���������Ǹ�
  distribution_noncard = std::discrete_distribution<>(probs.begin(), probs.end());
  probs = { 100,100,100,100,100,100 }; //���������Ǹ�
  distribution_npc = std::discrete_distribution<>(probs.begin(), probs.end());

  for (int i = 0; i < 6; i++)//֧Ԯ����ʼ�ӳ�
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


  randomDistributeCards(rand); //������俨�飬������������
  
}

void Game::randomDistributeCards(std::mt19937_64& rand)
{
  //�����غϵ���ͷ���䣬����Ҫ���㣬��Ϊ������������
  if (isRacing)
  {
    cook_main_race_material_type = rand() % 5;
    return;//�������÷��俨��
  }
  
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 5; j++)
      personDistribution[i][j] = -1;

  int headN[5] = { 0,0,0,0,0 };
  vector<int8_t> buckets[5];
  for (int i = 0; i < 5; i++)
    buckets[i].clear();
  //�ȷ�����/���³�/����
  for (int i = 0; i < 6 + 2; i++)
  {
    int atTrain = 5;
    if (friend_type != 0 && i == friend_personId)
    {
      //���˿�
      atTrain = persons[i].distribution(rand);
    }
    else if (i == PSID_noncardYayoi && friend_type == 0)//�ǿ����³�
    {
      atTrain = distribution_noncard(rand);
    }
    else if (i == PSID_noncardReporter)//����
    {
      if (turn < 12 || isXiahesu())//���ߵ�13�غ������ĺ���Ҳ����
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
    else if (buckets[i].size() > 1)//���ѡһ����ͷ
    {
      personDistribution[i][0] = buckets[i][rand() % buckets[i].size()];
      headN[i] += 1;
    }
    buckets[i].clear();
  }

  //Ȼ������֧ͨԮ��
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
  int npcCount = friend_type == 0 ? 6 : 7;//����֧Ԯ��һ��12��
  for (int i = 0; i < npcCount; i++)
  {
    int atTrain = distribution_npc(rand);
    if (atTrain < 5)
    {
      buckets[atTrain].push_back(PSID_npc);
    }
  }

  //ѡ��������5����ͷ
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
    else//����������5�ˣ����ѡmaxHead��
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

  //�Ƿ���hint
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

  //��Ϣ���������������֣������Ȧ
  //��Ϣ&���
  int restMaterialType = rand() % 5;
  bool restGreen = isXiahesu() ? true : randBool(rand, GameConstants::Cook_RestGreenRate);
  cook_train_material_type[TRA_rest] = restMaterialType;
  cook_train_material_type[TRA_outgoing] = restMaterialType;
  cook_train_green[TRA_rest] = restGreen;
  cook_train_green[TRA_outgoing] = restGreen;

  //����
  int raceMaterialType = rand() % 5;
  bool raceGreen = randBool(rand, GameConstants::Cook_RaceGreenRate);
  cook_train_material_type[TRA_race] = raceMaterialType;
  cook_train_green[TRA_race] = raceGreen;

  //ѵ������Ȧ��calculateTrainingValue�����

  calculateTrainingValue();
}

//����=(1+����pt�ӳ�+�Բ˼ӳ�)*(1+����pt���ܵ�ӳ�)
//�ϲ�=min(����-�²�, 100)
void Game::calculateTrainingValue()
{
  //�籾ѵ���ӳ�
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
  //ͬһ�غϲ����ظ���������
  if (cook_dish != DISH_none)
    return false;

  int dishLevel = GameConstants::Cook_DishLevel[dishId];
  //����Ƿ�ﵽ����ʱ��
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


  //�����Ϲ�����
  for (int i = 0; i < 5; i++)
  {
    int matCost = GameConstants::Cook_DishCost[dishId][i];
    if (dishId == DISH_g1plate)//������ǳ����㣬G1PlateҪ�Ǽ�
    {
      //Ĭ�ϳ�����
      if (cook_win_history[4] < 2)//������
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

  //�������ȫ����Ȧ��������pt
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
  if (!isAfterTrain)//ֻ��ѵ��ǰ��ʾ
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
  printEvents("ũ���ջ�");
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
    printEvents("����pt�ﵽ��һ�׶�");
    //upgrade deyilv
    updateDeyilv();
  }
  if ((oldDishPt < 2000 && cook_dish_pt >= 2000)
   // || (oldDishPt < 5000 && cook_dish_pt >= 5000)
    || (oldDishPt < 7000 && cook_dish_pt >= 7000)
    || (oldDishPt < 12000 && cook_dish_pt >= 12000)
    )
  {
    printEvents("ʳ�⿪�ۣ�ȫ��ѵ���ȼ�+1");
    for (int i = 0; i < 5; i++)
      addTrainingLevelCount(i, 4);
  }

  cook_dish_pt_turn_begin = cook_dish_pt;
}
bool Game::makeDish(int16_t dishId, std::mt19937_64& rand)
{
  if (!isDishLegal(dishId))
    return false;

  //�۳�����
  for (int i = 0; i < 5; i++)
  {
    int matCost = GameConstants::Cook_DishCost[dishId][i];
    if (dishId == DISH_g1plate)//������ǳ����㣬G1PlateҪ�Ǽ�
    {
      //Ĭ�ϳ�����
      if (cook_win_history[4] < 2)//������
        matCost = 100;
    }
    cook_material[i] -= matCost;
  }
  cook_dish = dishId;

  //����ũ��
  autoUpgradeFarm(false);

  //����Ƿ��ɹ�
  bool isBigSuccess = cook_dish_sure_success ? true : randBool(rand, 0.01 * cook_dishpt_success_rate);
  if (isBigSuccess)
    handleDishBigSuccess(dishId, rand);

  //��������pt
  assert(cook_dish_pt == cook_dish_pt_turn_begin);
  int pt = GameConstants::Cook_DishGainPt[dishId];
  cook_dish_pt += pt;

  cook_dish_sure_success = false;
  //�����Խ1500�����ˣ����ߴ���12000���´α�Ϊ��ɹ�
  if (cook_dish_pt >= 12000 || cook_dish_pt / 1500 != cook_dish_pt_turn_begin / 1500)
    cook_dish_sure_success = true;

  //�����ѵ���ӳ������Ч�����������
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
  //������������ֵ�����ȼ��������ֵ
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
      printEvents("�����ɹ�������+10");
    }
    else if (buffs[i] == 2)
    {
      addMotivation(1);
      printEvents("�����ɹ����ɾ�+1");
    }
    else if (buffs[i] == 3)
    {
      for (int i = 0; i < 6; i++)
        addJiBan(i, 3, true);
      printEvents("�����ɹ���ȫ���+3");
    }
    else if (buffs[i] == 4)
    {
      int dishlevel = GameConstants::Cook_DishLevel[dishId];
      if (dishlevel == 4)
      {
        //G1Plate�ˣ�ÿ��ѵ������2����
        for (int tr = 0; tr < 5; tr++)
        {
          dishBigSuccess_invitePeople(tr, rand);
          dishBigSuccess_invitePeople(tr, rand);
        }
      }
      else if (dishlevel == 3 || dishlevel == 2)
      {
        //�����ˣ���Ӧѵ������1����
        int mainTrain = GameConstants::Cook_DishMainTraining[dishId];
        dishBigSuccess_invitePeople(mainTrain, rand);
      }
      else
        throw "ERROR: Game::handleDishBigSuccess buffs[i] == 4 but dishlevel != 2 or 3 or 4";
      printEvents("�����ɹ���ҡ��");
    }
    else if (buffs[i] == 5)
    {
      //�Ѿ��������
      printEvents("�����ɹ�����������+4");
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
  //���ѡһ�ſ�hint
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
  printEvents("�����ɹ���hint +" + to_string(hintlevel));
  skillPt += int(hintlevel * hintPtRate);
}
void Game::dishBigSuccess_invitePeople(int trainIdx, std::mt19937_64& rand)
{
  //����һ���Ѿ��м�����
  int count = 0;
  bool cannotInvite[6] = { true, true, true, true, true, true };//�����κ�һ��ѵ���������Ѿ��ڵ�ǰѵ��
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
  if (count >= 5)return;//�Ѿ�����


  //�Ӳ���personDistribution[trainIdx]�������ѡһ�����ŵ�����
  vector<int> availablePeople;
  for (int i = 0; i < 6; i++)
  {
    if (!cannotInvite[i])
      availablePeople.push_back(i);
  }
  if (availablePeople.size() == 0)
  {
    return;//��������ѵ�����ǵ�ǰѵ���ĸ�����ͷ�����û�п��������
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

  //��һ�ֻ꣬��ȷ��ѵ�����ղ�ǰ����
  if (turn < 24)
  {
    if (turn % 4 != 3) //�»غ��ղ˲�����
      return;
    if (gameStage != GameStage_afterTrain)//ȷ��ѵ���������
      return;
    if (cook_farm_pt < GameConstants::Cook_FarmLvCost[1])//����pt����
      return;

    int value[5] = { 5,1,4,2,3 };//���ȼ�
    //�Ѿ�lv2�Ĳ�����
    for (int i = 0; i < 5; i++)
    {
      if (cook_farm_level[i] == 2)
        value[i] -= 99999;
    }
    //ͳ��4�غ��ڵ�����������һ��value+10�������������
    int clickNums[5] = { 0,0,0,0,0 };
    for (int i = 0; i < 4; i++)
    {
      int type = cook_harvest_history[i];
      if (type == -1)
        throw "ERROR: Game::autoUpgradeFarm cook_harvest_history[i] == -1����һ��ѵ�����ղ�ǰ�ſ��Զ�����ũ��";
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
      //��һ�겻��lv2���⣬�������̫��
      value[1] -= 40;
    }
    //ѡ������
    int maxIdx = 0;
    for (int i = 0; i < 5; i++)
    {
      if (value[i] > value[maxIdx])
        maxIdx = i;
    }
    //valueС��0������
    if (value[maxIdx] >= 0)
    {
      bool suc = upgradeFarm(maxIdx);
      assert(suc);
      autoUpgradeFarm(beforeXiahesu);//�п���������һ��
    }
  }
  //�ڶ��꣬�Բ�ǰ�����ղ�ǰ����
  else if (turn < 48)
  {
    if (gameStage == GameStage_beforeTrain)//�Բ�ǰ����������̰LV3��5����
    {
      if (GameConstants::Cook_DishLevel[cook_dish] != 2)
        return;
      if (cook_farm_pt < GameConstants::Cook_FarmLvCost[2])
        return;//����pt����
      if (maxVital - vital <= 0)
        return;//��������
      int mainTrain = GameConstants::Cook_DishMainTraining[cook_dish];
      if (cook_farm_level[mainTrain] == 3)
        return;//�Ѿ�lv3��
      //���⣨���1������������ݮ�����4������������û����֮ǰ������
      if (mainTrain == 1
        || (mainTrain == 4 && (cook_farm_level[0] != 3 || cook_farm_level[2] != 3 || cook_farm_level[3] != 3)))
        return;
      bool suc = upgradeFarm(mainTrain);
      assert(suc);
    }
    else if (gameStage == GameStage_afterTrain)//�ղ�ǰ����
    {
      if (turn % 4 != 3) //�»غ��ղ˲�����
        return;
      int value[5] = { 25,-59,24,23,2 };//���ȼ�
      //�Ѿ�lv3�Ĳ�������lv1������Ȩ��
      for (int i = 0; i < 5; i++)
      {
        if (cook_farm_level[i] == 3)
          value[i] -= 99999;
        if (cook_farm_level[i] == 1)
          value[i] += 60; //�����ǡ�ÿ����������ʱ����lv2����������lv3
      }
      if(beforeXiahesu)
      {
        //nothing
      }
      else
      {
        //ͳ��4�غ��ڵ�����������������
        int clickNums[5] = { 0,0,0,0,0 };
        for (int i = 0; i < 4; i++)
        {
          int type = cook_harvest_history[i];
          if (type == -1)
            throw "ERROR: Game::autoUpgradeFarm cook_harvest_history[i] == -1���ڶ���ѵ�����ղ�ǰ�ſ��Զ�����ũ��";
          clickNums[type] += 1;
        }
        for (int i = 0; i < 5; i++)
        {
          int overflow = 55 + 45 * clickNums[i] + cook_material[i] - GameConstants::Cook_MaterialLimit[cook_farm_level[i]];
          if (overflow > 0)
            value[i] += overflow;
        }
      }
      //ѡ������
      int maxIdx = 0;
      for (int i = 0; i < 5; i++)
      {
        if (value[i] > value[maxIdx])
          maxIdx = i;
      }
      //valueС��0������
      if (value[maxIdx] >= 0 && cook_farm_pt >= GameConstants::Cook_FarmLvCost[cook_farm_level[maxIdx]])
      {
        bool suc = upgradeFarm(maxIdx);
        assert(suc);
        autoUpgradeFarm(beforeXiahesu);//�п���������һ��
      }
    }
    else
      assert(false);
  }
  //�����꣬�ղ�ǰ����
  else if (turn < 72)
  {
    if (gameStage == GameStage_beforeTrain)//�Բ�ǰ������
      return;
    if (turn % 4 != 3) //�»غ��ղ˲�����
      return;

    //����·�ߣ�32333 42443 43443 43453 53553
    int value[5] = { 283,140,281,282,160 };//���ȼ�
    int priorLv5[5] = { 3,1,4,5,2 };//���ȼ�
    //����lv����Ȩ��
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
      //ͳ��4�غ��ڵ�����������������
      int clickNums[5] = { 0,0,0,0,0 };
      for (int i = 0; i < 4; i++)
      {
        int type = cook_harvest_history[i];
        if (type == -1)
          throw "ERROR: Game::autoUpgradeFarm cook_harvest_history[i] == -1���ڶ���ѵ�����ղ�ǰ�ſ��Զ�����ũ��";
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
    //ѡ������
    int maxIdx = 0;
    for (int i = 0; i < 5; i++)
    {
      if (value[i] > value[maxIdx])
        maxIdx = i;
    }
    //valueС��0������
    if (value[maxIdx] >= 0 && cook_farm_pt >= GameConstants::Cook_FarmLvCost[cook_farm_level[maxIdx]])
    {
      bool suc = upgradeFarm(maxIdx);
      assert(suc);
      autoUpgradeFarm(beforeXiahesu);//�п���������һ��
    }
    

  }
  //ura�ڼ䣬����5
  else 
  {
    if (gameStage == GameStage_beforeTrain)//�Բ�ǰ������
      return;

    int value[5] = { 1000 - cook_material[0], 1000 - cook_material[1], 1000 - cook_material[2], 1000 - cook_material[3], 1000 - cook_material[4], };//���ȼ�
    //����lv����Ȩ��
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
    //ѡ������
    int maxIdx = 0;
    for (int i = 0; i < 5; i++)
    {
      if (value[i] > value[maxIdx])
        maxIdx = i;
    }
    //valueС��0������
    if (value[maxIdx] >= 0 && cook_farm_pt >= GameConstants::Cook_FarmLvCost[cook_farm_level[maxIdx]])
    {
      bool suc = upgradeFarm(maxIdx);
      assert(suc);
      autoUpgradeFarm(beforeXiahesu);//�п���������һ��
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
  //1������2���飬3�4����5��������
  vector<int> buffs;

  int dishLevel = GameConstants::Cook_DishLevel[dishId];

  //дһ���ֲ��������ж����buff�Ƿ�����
  auto isBuffLegal = [&](int buffType)
    {
      if (buffType == 1)//����
        return true;
      else if (buffType == 2)//����
        return true;
      else if (buffType == 3)//��������
      {
        for (int i = 0; i < 6; i++)
          if (persons[i].friendship < 100)
            return true;
        return false;
      }
      else if (buffType == 4)//�����Ѿ����˵Ĳ�����
      {
        if (dishLevel == 4)
          return true;
        if (dishLevel == 1)
          return false;
        int mainTrain = GameConstants::Cook_DishMainTraining[dishId];
        if (personDistribution[mainTrain][4] == PSID_none)
        {
          //�������ѵ���Ƿ��п����������
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
      else if (buffType == 5)//�������ޣ���120������
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

  //׷��buff
  for (int i = 0; i < 5; i++)
  {
    int prob = GameConstants::Cook_DishPtBigSuccessBuffExtraProb[dishLevel][i];
    if (prob == 0)continue;
    if (!isBuffLegal(i))continue;
    if (randBool(rand, prob * 0.01))
    {
      //����Ƿ��Ѿ������buff
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
int Game::calculateRealStatusGain(int value, int gain) const//����1200����Ϊ2�ı�����ʵ����������ֵ
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
  //������ϵ�ѵ��ʧ���ʣ����κ��� A*(x0-x)^2+B*(x0-x)
  //���Ӧ����2%����
  static const double A = 0.025;
  static const double B = 1.25;
  double x0 = 0.1 * GameConstants::FailRateBasic[trainType][getTrainingLevel(trainType)];
  
  double f = 0;
  if (vital < x0)
  {
    f = (100 - vital) * (x0 - vital) / 40.0;
  }
  if (f < 0)f = 0;
  if (f > 99)f = 99;//����ϰ���֣�ʧ�������99%
  f *= failRateMultiply;//֧Ԯ����ѵ��ʧ�����½�����
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
    if (remainVital >= 20)//ѡ��
      addVitalFriend(43);
    else//ѡ��
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
    //�д�ɹ��ͳɹ�
    if (rand() % 4 != 0)//���Թ��ƣ�75%��ɹ�
    {
      addVitalFriend(30);
      addStatusFriend(3, 36);
      skillPt += 72;//���ܵȼ�
    }
    else
    {
      addVitalFriend(26);
      addStatusFriend(3, 24);
      skillPt += 40;//���ܵȼ�
    }
    addMotivation(1);
    addJiBan(pid, 5, false);
    isRefreshMind = true;
  }
  else assert(false && "δ֪�ĳ���");

  //ȫ���+40
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
    printEvents("�������������ѡ��");
  }
  else
  {
    addStatusFriend(0, 8);
    addStatusFriend(3, 8);
    skillPt += 10;//ֱ������+5
    printEvents("�������������ѡ��");
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
    printEvents("��һ�ε�����");
    friend_stage = FriendStage_beforeUnlockOutgoing;
    
    addStatusFriend(0, 14);
    addJiBan(friend_personId, 10, false);
    addMotivation(1);
  }
  else
  {
    if (rand() % 5 < 3)return;//40%���ʳ��¼���60%���ʲ���

    if (rand() % 10 == 0)
    {
      if (motivation != 5)
        printEvents("���˵���¼�:����+1");
      addMotivation(1);//10%���ʼ�����
    }

    if (turn < 24)
    {
      //�����͵��˼�3�
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
      printEvents("���˵���¼�:" + persons[minJiBanId].getPersonName() + " �+3, ���³��+5");

     
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
  if (friend_type == 0)return;//û���˿�
  if (friend_stage < FriendStage_beforeUnlockOutgoing)return;//����û������û�¼�
  if (turn == 23)
  {
    addMotivation(1);
    addStatusFriend(0, 24);
    addJiBan(friend_personId, 5, false);
    skillPt += 40;//�����������ߣ������н�����������hint����Ч��
  }
  else if (turn == 77)
  {
    if (friend_outgoingUsed >= 5)//�������
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
    assert(false && "�����غ�û�����˹̶��¼�");
  }
}
bool Game::applyTraining(std::mt19937_64& rand, int train)
{
  assert(gameStage == GameStage_beforeTrain);
  int matType = -1;//�˻غϵĲ˵�����
  int matExtra = 0;//�˻غϵĲ˵Ķ���ӳɣ�ѵ����ͷ����
  bool isGreen = false;//�˻غϵĲ��Ƿ���Ȧ
  if (isRacing)
  {
    //����������checkEventAfterTrain()�ﴦ���˴�ֻ�����
    assert(train == TRA_none || train == TRA_race);
    matType = cook_main_race_material_type;
    matExtra = 0;
    isGreen = true;
    

    //assert(false && "���о籾��������checkEventAfterTrain()�ﴦ������applyTraining");
    //return false;//���о籾��������checkEventAfterTrain()�ﴦ���൱�ڱ����غ�ֱ���������������������
  }
  else
  {
    matType = cook_train_material_type[train];
    matExtra = cook_train_material_num_extra[train];
    isGreen = cook_train_green[train];//���ѵ��ʧ�ܣ�����������Ϊfalse


    if (train == TRA_rest)//��Ϣ
    {
      if (isXiahesu())//����ֻ�����
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
    else if (train == TRA_race)//����
    {
      if (turn <= 12 || turn >= 72)
      {
        printEvents("Cannot race now.");
        return false;
      }
      addAllStatus(1);//������
      runRace(2, 40);//���ԵĽ���

      //����̶�15
      addVital(-15);
      if (rand() % 10 == 0)
        addMotivation(1);
    }
    else if (train == TRA_outgoing)//���
    {
      if (isXiahesu())
      {
        addVital(40);
        addMotivation(1);
      }
      else if (friend_type != 0 &&  //�������˿�
        friend_stage == FriendStage_afterUnlockOutgoing &&  //�ѽ������
        friend_outgoingUsed < 5  //���û����
        )
      {
        //���˳���
        handleFriendOutgoing(rand);
        isGreen = true;
      }
      else //��ͨ����
      {
        //���ò�����ˣ���50%��2���飬50%��1����10����
        if (rand() % 2)
          addMotivation(2);
        else
        {
          addMotivation(1);
          addVital(10);
        }
      }
    }
    else if (train <= 4 && train >= 0)//����ѵ��
    {
      if (rand() % 100 < failRate[train])//ѵ��ʧ��
      {
        isGreen = false;
        if (failRate[train] >= 20 && (rand() % 100 < failRate[train]))//ѵ����ʧ�ܣ�������Ϲ�µ�
        {
          printEvents("ѵ����ʧ�ܣ�");
          addStatus(train, -10);
          if (fiveStatus[train] > 1200)
            addStatus(train, -10);//��Ϸ��1200���Ͽ����Բ��۰룬�ڴ�ģ�������Ӧ1200���Ϸ���
          //�����2��10�������ĳ�ȫ����-4���������
          for (int i = 0; i < 5; i++)
          {
            addStatus(i, -4);
            if (fiveStatus[i] > 1200)
              addStatus(i, -4);//��Ϸ��1200���Ͽ����Բ��۰룬�ڴ�ģ�������Ӧ1200���Ϸ���
          }
          addMotivation(-3);
          addVital(10);
        }
        else//Сʧ��
        {
          printEvents("ѵ��Сʧ�ܣ�");
          addStatus(train, -5);
          if (fiveStatus[train] > 1200)
            addStatus(train, -5);//��Ϸ��1200���Ͽ����Բ��۰룬�ڴ�ģ�������Ӧ1200���Ϸ���
          addMotivation(-1);
        }
      }
      else
      {
        //�ȼ���ѵ��ֵ
        for (int i = 0; i < 5; i++)
          addStatus(i, trainValue[train][i]);
        skillPt += trainValue[train][5];
        addVital(trainVitalChange[train]);

        int friendshipExtra = 0;//�������SSR���˿���+1��������˿������ѵ������+2�������������ﴦ��
        if (friend_type == 1)
          friendshipExtra += 1;

        vector<int> hintCards;//���ļ����������̾����
        bool clickFriend = false;//���ѵ����û������
        //���SSR�����ڲ�������
        for (int i = 0; i < 5; i++)
        {
          int p = personDistribution[train][i];
          if (p == PSID_none)break;//û��
          if (friend_type == 1 && p == friend_personId)
          {
            friendshipExtra += 2;
            break;
          }
        }
        for (int i = 0; i < 5; i++)
        {
          int p = personDistribution[train][i];
          if (p < 0)break;//û��

          if (p == friend_personId && friend_type != 0)//���˿�
          {
            assert(persons[p].personType == PersonType_scenarioCard);
            addJiBan(p, 4 + friendshipExtra, false);
            clickFriend = true;
          }
          else if (p < 6)//��ͨ��
          {
            addJiBan(p, 7 + friendshipExtra, false);
            if (persons[p].isHint)
              hintCards.push_back(p);
          }
          else if (p == PSID_npc)//npc
          {
            //nothing
          }
          else if (p == PSID_noncardYayoi)//�ǿ����³�
          {
            int jiban = friendship_noncard_yayoi;
            int g = jiban < 40 ? 2 : jiban < 60 ? 3 : jiban < 80 ? 4 : 5;
            skillPt += g;
            addJiBan(PSID_noncardYayoi, 7, false);
          }
          else if (p == PSID_noncardReporter)//����
          {
            int jiban = friendship_noncard_reporter;
            int g = jiban < 40 ? 2 : jiban < 60 ? 3 : jiban < 80 ? 4 : 5;
            addStatus(train, g);
            addJiBan(PSID_noncardReporter, 7, false);
          }
          else
          {
            //��������/�ſ��ݲ�֧��
            assert(false);
          }
        }

        if (hintCards.size() > 0)
        {
          int hintCard = hintCards[rand() % hintCards.size()];//���һ�ſ���hint

          addJiBan(hintCard, 5, false);
          int hintLevel = persons[hintCard].cardParam.hintLevel;
          if (hintLevel > 0)
          {
            skillPt += int(hintLevel * hintPtRate);
          }
          else //�����������֣�ֻ������
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


        //ѵ���ȼ�����
        addTrainingLevelCount(train, 1);

      }

    }
    else
    {
      printEvents("δ֪��ѵ����Ŀ");
      return false;
    }
  }


  //�ֲ�
  addFarm(matType, matExtra, isGreen);
  gameStage = GameStage_afterTrain;
  autoUpgradeFarm(false);
  return true;
}


bool Game::isLegal(Action action) const
{
  if (!action.isActionStandard())
    return false;

  //�Ƿ�Ե����
  if (action.dishType != DISH_none)
    if (!isDishLegal(action.dishType))
      return false;

  if (isRacing)
  {
    //if (isUraRace)
    //{
      if (action.train == TRA_none || action.train == TRA_race)//none�ǳԲ�Ȼ�������race��ֱ�ӱ���
        return true;
      else
        return false;
    //}
    //else
    //{
      //assert(false && "����ura����ľ籾��������checkEventAfterTrain()�ﴦ������applyTraining");
      //return false;//���о籾��������checkEventAfterTrain()�ﴦ���൱�ڱ����غ�ֱ���������������������
    //}
  }

  if (action.train == TRA_rest)
  {
    if (isXiahesu())
    {
      return false;//���ĺ��޵ġ����&��Ϣ����Ϊ���
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
    assert(false && "δ֪��ѵ����Ŀ");
    return false;
  }
  return false;
}



float Game::getSkillScore() const
{
  float rate = isQieZhe ? ptScoreRate * 1.1 : ptScoreRate ;
  return rate * skillPt + skillScore;
}

static double scoringFactorOver1200(double x)//����ʤ������ɫʮ�֣�׷��
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
    throw "�������㷨��δʵ��";
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
  printEvents(GameConstants::Cook_MaterialNames[item] + "����" + to_string(cook_farm_level[item]) + "��");
}
void Game::calculateTrainingValueSingle(int tra)
{
  int headNum = 0;//���ſ�����npc�����³����߲���
  int shiningNum = 0;//��������
  int linkNum = 0;//����link

  int basicValue[6] = { 0,0,0,0,0,0 };//ѵ���Ļ���ֵ��=ԭ����ֵ+֧Ԯ���ӳ�

  int totalXunlian = 0;//ѵ��1+ѵ��2+...
  int totalGanjing = 0;//�ɾ�1+�ɾ�2+...
  double totalYouqingMultiplier = 1.0;//(1+����1)*(1+����2)*...
  int vitalCostBasic;//�������Ļ�������=ReLU(������������+link������������-�ǲ��������ļ���)
  double vitalCostMultiplier = 1.0;//(1-�������ļ�����1)*(1-�������ļ�����2)*...
  double failRateMultiplier = 1.0;//(1-ʧ�����½���1)*(1-ʧ�����½���2)*...

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
    if (pIdx >= 6)continue;//����֧Ԯ��

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


  //������ȡֵ
  cook_train_material_type[tra] = tra;
  cook_train_green[tra] = shiningNum > 0;
  cook_train_material_num_extra[tra] = headNum + 2 * linkNum;


  //����ֵ
  for (int i = 0; i < 6; i++)
    basicValue[i] = GameConstants::TrainingBasicValue[tra][tlevel][i];
  vitalCostBasic = -GameConstants::TrainingBasicValue[tra][tlevel][6];

  for (int h = 0; h < 5; h++)
  {
    int pid = personDistribution[tra][h];
    if (pid < 0)break;//û��
    if (pid >= 6)continue;//���ǿ�
    const Person& p = persons[pid];
    bool isThisCardShining = isCardShining_record[pid];//���ſ���û��
    bool isThisTrainingShining = shiningNum > 0;//���ѵ����û��
    CardTrainingEffect eff = p.cardParam.getCardEffect(*this, isThisCardShining, tra, p.friendship, p.cardRecord, headNum, shiningNum);
    
    for (int i = 0; i < 6; i++)//����ֵbonus
    {
      if (basicValue[i] > 0)
        basicValue[i] += int(eff.bonus[i]);
    }
    if (isCardShining_record[pid])//���ʣ�����ӳɺ��ǲʻظ�
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

  //������ʧ����

  int vitalChangeInt = vitalCostBasic > 0 ? -int(vitalCostBasic * vitalCostMultiplier) : -vitalCostBasic;
  if (vitalChangeInt > maxVital - vital)vitalChangeInt = maxVital - vital;
  if (vitalChangeInt < -vital)vitalChangeInt = -vital;
  trainVitalChange[tra] = vitalChangeInt;
  failRate[tra] = calculateFailureRate(tra, failRateMultiplier);


  //��ͷ * ѵ�� * �ɾ� * ����    //֧Ԯ������
  double cardMultiplier = (1 + 0.05 * headNum) * (1 + 0.01 * totalXunlian) * (1 + 0.1 * (motivation - 3) * (1 + 0.01 * totalGanjing)) * totalYouqingMultiplier;
  //trainValueCardMultiplier[t] = cardMultiplier;

  //�²���Կ�ʼ����
  for (int i = 0; i < 6; i++)
  {
    bool isRelated = basicValue[i] != 0;
    double bvl = basicValue[i];
    double umaBonus = i < 5 ? 1 + 0.01 * fiveStatusBonus[i] : 1;
    trainValueLower[tra][i] = bvl * cardMultiplier * umaBonus;
  }

  //�籾ѵ���ӳ�
  double scenarioTrainMultiplier = 1 + 0.01 * cook_dishpt_training_bonus;
  //����ѵ���ӳ�
  if (cook_dish != DISH_none)
    scenarioTrainMultiplier += 0.01 * getDishTrainingBonus(tra);
  double skillPtMultiplier = scenarioTrainMultiplier * (1 + 0.01 * cook_dishpt_skillpt_bonus);



  //�ϲ�=����-�²�

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

  //�غ���+1
  turn++;
  
  if (turn >= TOTAL_TURN)
  {
    printEvents("���ɽ���!");
    printEvents("��ĵ÷��ǣ�" + to_string(finalScore()));
  }
  else {
    isRacing = isRacingTurn[turn];
    gameStage = GameStage_beforeTrain;
  }
  return;

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
  //������̶ֹ��¼�
  checkDishPtUpgrade();
  maybeHarvest(); 
  maybeCookingMeeting();
  if (isRefreshMind)
  {
    addVital(5);
    if (rand() % 4 == 0) //����ÿ�غ���25%����buff��ʧ
      isRefreshMind = false;
  }
  if (isRacing)//���ı���
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

  if (turn == 11)//������
  {
    assert(isRacing);
  }
  else if (turn == 23)//��һ�����
  {
    //����¼���������ѡ������������ѡ����
    {
      int vitalSpace = maxVital - vital;//�������������
      handleFriendFixedEvent();
      if (vitalSpace >= 20)
        addVital(20);
      else
        addAllStatus(5);
    }
    printEvents("��һ�����");
  }
  else if (turn == 29)//�ڶ���̳�
  {

    for (int i = 0; i < 5; i++)
      addStatus(i, zhongMaBlueCount[i] * 6); //�����ӵ���ֵ

    double factor = double(rand() % 65536) / 65536 * 2;//�籾�������0~2��
    for (int i = 0; i < 5; i++)
      addStatus(i, int(factor*zhongMaExtraBonus[i])); //�籾����
    skillPt += int((0.5 + 0.5 * factor) * zhongMaExtraBonus[5]);//���߰��㼼�ܵĵ�Чpt

    for (int i = 0; i < 5; i++)
      fiveStatusLimit[i] += zhongMaBlueCount[i] * 2; //��������--�������ֵ��18�����μ̳й��Ӵ�Լ36���ޣ�ÿ��ÿ��������+1���ޣ�1200�۰��ٳ�2

    for (int i = 0; i < 5; i++)
      fiveStatusLimit[i] += rand() % 8; //��������--�����μ̳��������

    printEvents("�ڶ���̳�");
  }
  else if (turn == 35)
  {
    autoUpgradeFarm(true);//����ǰ����ũ��
    printEvents("�ڶ�����޿�ʼ");
  }
  else if (turn == 47)//�ڶ������
  {
    //����¼���������ѡ������������ѡ����
    {
      int vitalSpace = maxVital - vital;//�������������
      if (vitalSpace >= 30)
        addVital(30);
      else
        addAllStatus(8);
    }
    printEvents("�ڶ������");
  }
  else if (turn == 48)//�齱
  {
    int rd = rand() % 100;
    if (rd < 16)//��Ȫ��һ�Ƚ�
    {
      addVital(30);
      addAllStatus(10);
      addMotivation(2);

      printEvents("�齱�����������Ȫ/һ�Ƚ�");
    }
    else if (rd < 16 + 27)//���Ƚ�
    {
      addVital(20);
      addAllStatus(5);
      addMotivation(1);
      printEvents("�齱��������˶��Ƚ�");
    }
    else if (rd < 16 + 27 + 46)//���Ƚ�
    {
      addVital(20);
      printEvents("�齱������������Ƚ�");
    }
    else//��ֽ
    {
      addMotivation(-1);
      printEvents("�齱��������˲�ֽ");
    }
  }
  else if (turn == 49)
  {
    skillScore += 170;
    printEvents("���еȼ�+1");
  }
  else if (turn == 53)//������̳�
  {
    for (int i = 0; i < 5; i++)
      addStatus(i, zhongMaBlueCount[i] * 6); //�����ӵ���ֵ

    double factor = double(rand() % 65536) / 65536 * 2;//�籾�������0~2��
    for (int i = 0; i < 5; i++)
      addStatus(i, int(factor * zhongMaExtraBonus[i])); //�籾����
    skillPt += int((0.5 + 0.5 * factor) * zhongMaExtraBonus[5]);//���߰��㼼�ܵĵ�Чpt

    for (int i = 0; i < 5; i++)
      fiveStatusLimit[i] += zhongMaBlueCount[i] * 2; //��������--�������ֵ��18�����μ̳й��Ӵ�Լ36���ޣ�ÿ��ÿ��������+1���ޣ�1200�۰��ٳ�2

    for (int i = 0; i < 5; i++)
      fiveStatusLimit[i] += rand() % 8; //��������--�����μ̳��������

    printEvents("������̳�");

    if (getYayoiJiBan() >= 60)
    {
      skillScore += 170;//���м��ܵȼ�+1
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
    autoUpgradeFarm(true);//����ǰ����ũ��
    printEvents("��������޿�ʼ");
  }
  else if (turn == 70)
  {
    skillScore += 170;//���м��ܵȼ�+1
  }
  else if (turn == 77)//ura3����Ϸ����
  {
    //�����Ѿ���ǰ�洦����
    //����
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
      skillPt += 40;//�籾��
      addAllStatus(60);
      skillPt += 150;
    }
    else 
    {
      addAllStatus(25);
      //there should be something, but not important
    }


    //���˿��¼�
    handleFriendFixedEvent();

    addAllStatus(5);
    skillPt += 20;

    printEvents("ura3��������Ϸ����");
  }
}

void Game::checkRandomEvents(std::mt19937_64& rand)
{
  if (turn >= 72)
    return;//ura�ڼ䲻�ᷢ����������¼�

  //���˻᲻���������
  if (friend_type != 0)
  {
    Person& p = persons[friend_personId];
    assert(p.personType == PersonType_scenarioCard);
    if (friend_stage==FriendStage_beforeUnlockOutgoing)
    {
      double unlockOutgoingProb = p.friendship >= 60 ?
        GameConstants::FriendUnlockOutgoingProbEveryTurnHighFriendship :
        GameConstants::FriendUnlockOutgoingProbEveryTurnLowFriendship;
      if (randBool(rand, unlockOutgoingProb))//����
      {
        handleFriendUnlock(rand);
      }
    }
  }

  //ģ���������¼�

  //֧Ԯ�������¼��������һ������5�
  if (randBool(rand, GameConstants::EventProb))
  {
    int card = rand() % 6;
    addJiBan(card, 5, false);
    //addAllStatus(4);
    addStatus(rand() % 5, eventStrength);
    skillPt += eventStrength;
    printEvents("ģ��֧Ԯ������¼���" + persons[card].cardParam.cardName + " ���+5��pt���������+" + to_string(eventStrength));

    //֧Ԯ��һ����ǰ�����¼�������
    if (randBool(rand, 0.4 * (1.0 - turn * 1.0 / TOTAL_TURN)))
    {
      addMotivation(1);
      printEvents("ģ��֧Ԯ������¼�������+1");
    }
    if (randBool(rand, 0.5))
    {
      addVital(10);
      printEvents("ģ��֧Ԯ������¼�������+10");
    }
    else if (randBool(rand, 0.03))
    {
      addVital(-10);
      printEvents("ģ��֧Ԯ������¼�������-10");
    }
    if (randBool(rand, 0.03))
    {
      isPositiveThinking = true;
      printEvents("ģ��֧Ԯ������¼�����á�����˼����");
    }
  }

  //ģ����������¼�
  if (randBool(rand, 0.1))
  {
    addAllStatus(3);
    printEvents("ģ����������¼���ȫ����+3");
  }

  //������
  if (randBool(rand, 0.10))
  {
    addVital(5);
    printEvents("ģ������¼�������+5");
  }

  //��30�������Է��¼���
  if (randBool(rand, 0.02))
  {
    addVital(30);
    printEvents("ģ������¼�������+30");
  }

  //������
  if (randBool(rand, 0.02))
  {
    addMotivation(1);
    printEvents("ģ������¼�������+1");
  }

  //������
  if (turn >= 12 && randBool(rand, 0.04))
  {
    addMotivation(-1);
    printEvents("ģ������¼���\033[0m\033[33m����-1\033[0m\033[32m");
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
  //assert(turn < TOTAL_TURN && "Game::applyTrainingAndNextTurn��Ϸ�ѽ���");
  //assert(!(isRacing && !isUraRace) && "��ura�ı����غ϶���checkEventAfterTrain��������");
  if (action.dishType != DISH_none)//dish only, not next turn
  {
    bool suc = makeDish(action.dishType, rand);
    assert(suc && "Game::applyActionѡ���˲��Ϸ��Ĳ�Ʒ");
  }
  if (action.train != TRA_none || isRacing)
  {
    bool suc = applyTraining(rand, action.train);
    assert(suc && "Game::applyActionѡ���˲��Ϸ���ѵ��");
    
    checkEventAfterTrain(rand);
    if (isEnd()) return;

    randomDistributeCards(rand);


    //��ura�ı����غ�Ҳ���ܳԲˣ�����ˢpt�����Բ�����
    
    //if (isRacing && !isUraRace)//��ura�ı����غϣ�ֱ��������һ���غ�
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

