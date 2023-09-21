#include <iostream>
#include <cassert>
#include "Game.h"
using namespace std;
static bool randBool(mt19937_64& rand, double p)
{
  return rand() % 65536 < p * 65536;
}

void Game::newGame(mt19937_64& rand, bool enablePlayerPrint, int newUmaId, int umaStars, int newCards[6], int newZhongMaBlueCount[5], int newZhongMaExtraBonus[6])
{
  playerPrint = enablePlayerPrint;

  umaId = newUmaId;
  for (int i = 0; i < 5; i++)
    fiveStatusBonus[i] = GameDatabase::AllUmas[umaId].fiveStatusBonus[i];
  turn = 0;
  vital = 100;
  maxVital = 100;
  isQieZhe = false;
  isAiJiao = false; 
  failureRateBias = 0;
  skillPt = 120;
  skillScore = umaStars >= 3 ? 170 * (1 + umaStars) : 120 * (3 + umaStars);//固有技能
  motivation = 3;
  isPositiveThinking = false;
  for (int i = 0; i < 5; i++)
    trainLevelCount[i] = 0;
  isRacing = false;

  larc_zuoyueType = 0;
  larc_zuoyueCardLevel = 0;
  for (int i = 0; i < 18; i++)
  {
    persons[i] = Person();
  }
  persons[15].personType = 4;
  persons[16].personType = 5;
  persons[17].personType = 6;


  normalCardCount = 0;//速耐力根智卡的数量
  for (int i = 0; i < 6; i++)
  {
    int cardId = newCards[i];
    cardParam[i] = GameDatabase::AllCards[cardId];
    SupportCard& cardP = cardParam[i];
    int cardType = cardP.cardType;
    if (cardType == 5 || cardType == 6)
    {

      int realCardId = cardId / 10;
      if (realCardId == 30160 || realCardId == 10094)//佐岳卡
      {
        if (realCardId == 30160)
          larc_zuoyueType = 1;
        else 
          larc_zuoyueType = 2;

        larc_zuoyueCardLevel = cardId % 10;
        persons[17].personType = 1;
        persons[17].cardIdInGame = i;
      }
      else
      {
        throw string("不支持带佐岳以外的友人或团队卡");
      }
    }
    else//速耐力根智卡
    {
      Person& p = persons[normalCardCount];
      normalCardCount += 1;
      p.personType = 2;
      p.cardIdInGame = i;
      p.friendship = cardP.initialJiBan;
      p.larc_isLinkCard = cardP.larc_isLink;

      std::vector<int> probs = { 100,100,100,100,100,50 }; //基础概率，速耐力根智鸽
      probs[cardP.cardType] += cardP.deYiLv;
      p.distribution = std::discrete_distribution<>(probs.begin(), probs.end());
    }
  }


  motivationDropCount = 0;

  larc_isAbroad = false;
  larc_supportPtAll = 0;
  larc_shixingPt = 0;
  for (int i = 0; i < 10; i++)larc_levels[i] = 0;
  larc_isSSS = false;
  larc_ssWin = 0;
  larc_ssWinSinceLastSSS = 0;
  larc_isFirstLarcWin = false;
  for (int i = 0; i < 3; i++)for (int j = 0; j < 8; j++)
    larc_allowedDebuffsFirstLarc[i][j] = false;

  //larc_zuoyueType
  //larc_zuoyueCardLevel
  larc_zuoyueFirstClick = false;
  larc_zuoyueOutgoingUnlocked = false;
  larc_zuoyueOutgoingUsed = 0;


  for (int i = 0; i < 5; i++)
    zhongMaBlueCount[i] = newZhongMaBlueCount[i];
  for (int i = 0; i < 6; i++)
    zhongMaExtraBonus[i] = newZhongMaExtraBonus[i];


  for (int i = 0; i < 5; i++)
    fiveStatusLimit[i] = GameConstants::BasicFiveStatusLimit[i]; //原始属性上限
  for (int i = 0; i < 5; i++)
    fiveStatusLimit[i] += int(zhongMaBlueCount[i] * 5.34 * 2); //属性上限--种马基础值

  //后两次继承的事情，到时候再说
  //for (int i = 0; i < 5; i++)
   // fiveStatusLimit[i] += rand() % 20; //属性上限--后两次继承随机增加


  for (int i = 0; i < 5; i++)
    fiveStatus[i] = GameDatabase::AllUmas[umaId].fiveStatusInitial[i]; //赛马娘初始值
  for (int i = 0; i < 6; i++)//支援卡初始加成
  {
    for (int j = 0; j < 5; j++)
      addStatus(j, cardParam[i].initialBonus[j]);
    skillPt += cardParam[i].initialBonus[5];
  }
  for (int i = 0; i < 5; i++)
    addStatus(i, zhongMaBlueCount[i] * 7); //种马



  //initRandomGenerators();

  stageInTurn = 0;
  randomDistributeCards(rand); 
}

void Game::initNPCsTurn3()
{
}

void Game::initRandomGenerators()
{
  for (int i = 0; i < 8; i++)
  {
    std::vector<int> probs = { 100,100,100,100,100,50 }; //基础概率，速耐力根智鸽
    if (i < 6)
    {
      int cardType = cardData[i]->cardType;
      int deYiLv = cardData[i]->deYiLv;
      if (cardType >= 0 && cardType < 5)//速耐力根智卡
        probs[cardType] += deYiLv;
      else //友人卡，鸽的概率较高
        probs[5] += 50;
    }
    else //理事长，记者
      probs[5] += 50;

    cardDistributionRandom[i] = std::discrete_distribution<>(probs.begin(), probs.end());
  }

  for (int i = 0; i < 8; i++)
    venusSpiritTypeRandom[i] = std::discrete_distribution<>(GameConstants::VenusSpiritTypeProb[i], GameConstants::VenusSpiritTypeProb[i + 1]);

}

void Game::randomDistributeCards(std::mt19937_64& rand)
{
  //assert(stageInTurn == 0 || turn == 0);
  stageInTurn = 1;
  if (isRacing)
    return;//比赛不用分配卡组，但要改stageInTurn
  //先将6张卡分配到训练中
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 8; j++)
      cardDistribution[i][j] = false;

  double blueVenusHintBonus = 1 + 0.01 * GameConstants::BlueVenusLevelHintProbBonus[venusLevelBlue];

  for (int i = 0; i < 8; i++)
  {
    if (turn < 2 && i == 0)//前两回合神团不来
    {
      assert(cardData[0]->cardType == 5 && "神团卡不在第一个位置");
      continue;
    }

    int cardType = i < 6 ? cardData[i]->cardType : 6;

    int whichTrain = cardDistributionRandom[i](rand);//在哪个训练
    if (whichTrain < 5)//没鸽
      cardDistribution[whichTrain][i] = true;

    //是否有hint
    if (i < 6 && cardType >= 0 && cardType < 5)//速耐力根智卡
    {
      double hintProb = 0.06 * blueVenusHintBonus * (1 + 0.01 * cardData[i]->hintProbIncrease);
      bernoulli_distribution d(hintProb);
      cardHint[i] = d(rand);
    }
    else if (i < 6)
      cardHint[i] = false;
  }
  //分配碎片
  if (turn < 2 || venusSpiritsBottom[7] != 0)//无碎片
  {
    for (int i = 0; i < 8; i++)
      spiritDistribution[i] = 0;
  }
  else
  {
    bool allowTwoSpirits = venusSpiritsBottom[6] == 0;//有两个空位
    for (int i = 0; i < 8; i++)
    {
      int spiritType = venusSpiritTypeRandom[i](rand) + 1;
      int spiritColor = rand() % 3;
      int spirit = spiritType + spiritColor * 8;

      //看看是否为双碎片
      bool twoSpirits = false;
      if (allowTwoSpirits)
      {
        if (i < 5)//五个训练
        {
          bool isShining = false;//是否闪彩
          for (int card = 0; card < 6; card++)
          {
            if (cardDistribution[i][card])//这个卡在这个训练
            {
              isShining |= cardData[card]->getCardEffect(*this, i, cardJiBan[card], cardData[card]->effectFactor).youQing > 0;
            }
          }
          if (isShining)
          {
            if (i < 4)twoSpirits = true;
            else if (rand() % 5 < 2)twoSpirits = true;//智力彩圈40%双碎片
          }
        }
        else
        {
          if (rand() % 5 == 0)twoSpirits = true;//其他的20%双碎片
        }
      }
      if (twoSpirits)spirit += 32;//+32代表两个碎片
      spiritDistribution[i] = spirit;
    }

  }

  calculateTrainingValue();

  //如果单个训练出现6个或者更多人头，则重新分配卡组
  bool have6orMoreHeads = false;
  for (int i = 0; i < 5; i++)
  {
    int c = 0;
    for (int j = 0; j < 8; j++)
      if (cardDistribution[i][j])
        c++;
    if (c >= 6)
    {
      have6orMoreHeads = true;
      break;
    }
  }
  if (have6orMoreHeads)
    randomDistributeCards(rand);

}

void Game::calculateTrainingValue()
{
  for (int trainType = 0; trainType < 5; trainType++)
  {
    calculateTrainingValueSingle(trainType);
  }
}
void Game::addStatus(int idx, int value)
{
  fiveStatus[idx] += value;
  if (fiveStatus[idx] > fiveStatusLimit[idx])
    fiveStatus[idx] = fiveStatusLimit[idx];
  if (fiveStatus[idx] < 1)
    fiveStatus[idx] = 1;
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
  if (idx < 6 && isAiJiao)value += 2;
  cardJiBan[idx] += value;
  if (cardJiBan[idx] > 100)cardJiBan[idx] = 100;
}
void Game::addTrainingLevelCount(int item, int value)
{
  assert(item >= 0 && item < 5 && "addTrainingLevelCount不合法训练");
  trainLevelCount[item] += value;
  if (trainLevelCount[item] > 48)trainLevelCount[item] = 48;
}
void Game::addAllStatus(int value)
{
  for (int i = 0; i < 5; i++)addStatus(i, value);
}
void Game::addSpirit(std::mt19937_64& rand, int s)
{
  if (s > 32)//两个碎片
  {
    addSpirit(rand, s % 32);
    addSpirit(rand, s % 32);
    return;
  }

  int place = -1;//在第几个碎片槽
  for (int i = 0; i < 8; i++)
  {
    if (venusSpiritsBottom[i] == 0)
    {
      place = i;
      break;
    }
  }
  if (place == -1)return;//碎片槽满了
  venusSpiritsBottom[place] = s;

  //训练等级计数+1
  {
    int type = s % 8 - 1;
    if (type < 5 && type >= 0)
      addTrainingLevelCount(type, 1);
  }

  if (place % 2 == 1)//第二层有新碎片
  {
    int sL = venusSpiritsBottom[place - 1];
    int colorL = sL / 8;
    int typeL = sL % 8;
    int typeR = s % 8;

    int type = typeL;
    if (rand() % 5 == 0)
      type = typeR;//有20%概率是右侧碎片的属性
    int sU = type + 8 * colorL;//上层碎片
    int layer2Place = place / 2;
    venusSpiritsUpper[layer2Place] = sU;


    if(layer2Place%2==1)//第三层有新碎片
    {
      int sL = venusSpiritsUpper[layer2Place - 1];
      int colorL = sL / 8;
      int typeL = sL % 8;
      int typeR = sU % 8;

      int type = typeL;
      if (rand() % 5 == 0)
        type = typeR;//有20%概率是右侧碎片的属性
      int sU2 = type + 8 * colorL;//上层碎片
      int layer3Place = 4 + layer2Place / 2;
      venusSpiritsUpper[layer3Place] = sU2;

    }
  }

  if (place == 7)//碎片槽满了
  {
    int wiseColor = -1;
    int color1 = venusSpiritsBottom[0] / 8;
    int color2 = venusSpiritsBottom[4] / 8;
    if (color1 == color2)//1号位和5号位同色
    {
      wiseColor = color1;
    }
    else//数一下哪个多
    {
      int count = 0; 
      for (int i = 0; i < 8; i++)
      {
        int c = venusSpiritsBottom[i] / 8;
        if (c == color1)count += 1;
        else if (c == color2)count -= 1;
      }
      if (count > 0)wiseColor = color1;
      else if (count < 0)wiseColor = color2;
      else//个数相等
      {
        wiseColor = rand() % 2 ? color1 : color2;
      }
    }

    venusAvailableWisdom = wiseColor + 1;//123分别是红蓝黄
  }
  calculateVenusSpiritsBonus();

}
void Game::activateVenusWisdom()
{
  assert(venusAvailableWisdom != 0);
  assert(venusIsWisdomActive == false);
  venusIsWisdomActive = true;
  if (venusAvailableWisdom == 1)//开红
  {
    if (venusLevelRed < 5)
      venusLevelRed += 1;
    addVital(50);
    motivation = 5;
    //其他项目不在这里处理
  }
  if (venusAvailableWisdom == 2)//开蓝
  {
    if (venusLevelBlue < 5)
      venusLevelBlue += 1;
    for (int i = 0; i < 6; i++)
    {
      if (cardData[i]->cardType < 5)
        cardHint[i] = true;
    }
    //其他项目不在这里处理
  }
  if (venusAvailableWisdom == 3)//开黄
  {
    if (venusLevelYellow < 5)
      venusLevelYellow += 1;
    //友情训练不在这里处理
  }

  calculateTrainingValue();//重新计算训练值
}
void Game::clearSpirit()
{

}
int Game::calculateFailureRate(int trainType) const
{
  //粗略拟合的训练失败率，二次函数 A*x^2 + B*x + C + 0.5 * trainLevel
  //误差应该在2%以内
  static const double A = 0.0245;
  static const double B[5] = { -3.77,-3.74,-3.76,-3.81333,-3.286 };
  static const double C[5] = { 130,127,129,133.5,80.2 };

  double f = A * vital * vital + B[trainType] * vital + C[trainType] + 0.5 * getTrainingLevel(trainType);
  int fr = round(f);
  if (vital > 60)fr = 0;//由于是二次函数，体力超过103时算出来的fr大于0，所以需要手动修正
  if (fr < 0)fr = 0;
  if (fr > 99)fr = 99;//无练习下手，失败率最高99%
  fr += failureRateBias;
  if (fr < 0)fr = 0;
  if (fr > 100)fr = 100;
  return fr;
}
void Game::runRace(int basicFiveStatusBonus, int basicPtBonus)
{
  int cardRaceBonus = 0;
  for (int card = 0; card < 6; card++)
  {
    cardRaceBonus += cardData[card]->saiHou;
  }
  double raceMultiply = 1 + 0.01 * cardRaceBonus;
  if (venusAvailableWisdom == 1 && venusIsWisdomActive)//开红
    raceMultiply *= 1.35;
  int fiveStatusBonus = floor(raceMultiply * basicFiveStatusBonus);
  int ptBonus = floor(raceMultiply * basicPtBonus);
  addAllStatus(fiveStatusBonus);
  skillPt += basicPtBonus;
}
void Game::handleVenusOutgoing(int chosenOutgoing)
{
  venusCardOutgoingUsed[chosenOutgoing] = true;
  assert(cardData[0]->cardType == 5 && "神团卡不在第一个位置");
  if (chosenOutgoing == 0)//红
  {
    addVital(45);
    addMotivation(1);
    skillPt += 24;
    skillPt += 10;//技能等价
    addJiBan(0, 5);
  }
  else if (chosenOutgoing == 1)//蓝
  {
    addVital(32);
    addMotivation(1);
    addStatus(0, 12);
    addStatus(4, 12);
    skillPt += 10;//技能等价
    addJiBan(0, 5);
  }
  else if (chosenOutgoing == 2)//黄
  {
    maxVital += 4;
    addVital(32);
    addMotivation(1);
    addStatus(1, 8);
    addStatus(2, 8);
    addStatus(3, 8);
    skillPt += 15;//技能等价
    addJiBan(0, 5);

  }
  else if (chosenOutgoing == 3)//团1
  {
    addVital(45);
    addMotivation(1);
    addStatus(1, 15);
    addStatus(2, 15);
    addStatus(3, 15);
    addJiBan(0, 5);

  }
  else if (chosenOutgoing == 4)//团2
  {
    addVital(52);
    addMotivation(1);
    addAllStatus(9);
    skillPt += 36;
    skillPt += 50;//技能等价
    addJiBan(0, 5);
    venusCardIsQingRe = true;
  }
  else assert(false && "未知的神团出行");
}
void Game::handleVenusThreeChoicesEvent(std::mt19937_64& rand, int chosenColor)
{
  printEvents("出现女神三选一事件");
  int spiritType = chosenColor * 8 + rand() % 6 + 1;//碎片类型
  addSpirit(rand, spiritType);
  assert(cardData[0]->cardType == 5 && "神团卡不在第一个位置");
  addJiBan(0, 5);
  if (chosenColor == 0)
  {
    skillPt += 4;
    if(venusCardIsQingRe)
      skillPt += 5;
  }
  else if (chosenColor == 1)
  {
    addStatus(0, 4);
    if (venusCardIsQingRe)
      skillPt += 4;
  }
  else if (chosenColor == 2)
  {
    addStatus(1, 4);
    if (venusCardIsQingRe)
      skillPt += 4;
  }

  if (venusCardUnlockOutgoing)
    venusCardIsQingRe = true;//情热是否消失在checkEventAfterTrain里处理
}
void Game::calculateTrainingValueSingle(int trainType)
{
  //分配完了，接下来计算属性加值
  failRate[trainType] = calculateFailureRate(trainType);//计算失败率

  vector<CardTrainingEffect> effects;

  double vitalCostDrop = 1;

  for (int card = 0; card < 6; card++)
  {
    if (cardDistribution[trainType][card])//这个卡在这个训练
    {
      effects.push_back(cardData[card]->getCardEffect(*this, trainType, cardJiBan[card], cardData[card]->effectFactor));
    }
  }
  for (int i = 0; i < effects.size(); ++i) {
      failRate[trainType] *= (1-0.01*effects[i].failRateDrop);
      vitalCostDrop *= (1 - 0.01 * effects[i].vitalCostDrop);
  }

  //先算非女神的训练
  //1.人头数倍率
  int cardNum = effects.size();
  double cardNumMultiplying = 1 + 0.05 * cardNum;
  //2.彩圈(友情训练)倍率，注：是否闪彩已经在getCardEffect里考虑过了
  double youQingMultiplying = 1;
  for (int i = 0; i < cardNum; i++)
    youQingMultiplying *= (1 + 0.01 * effects[i].youQing);
  //3.训练倍率
  double xunLianBonusTotal = 0;
  for (int i = 0; i < cardNum; i++)
    xunLianBonusTotal += effects[i].xunLian;
  double xunLianMultiplying = 1 + 0.01 * xunLianBonusTotal;
  //4.干劲倍率
  double ganJingBasic = 0.1 * (motivation - 3);
  double ganJingBonusTotal = 0;
  for (int i = 0; i < cardNum; i++)
    ganJingBonusTotal += effects[i].ganJing;
  double ganJingMultiplying = 1 + ganJingBasic * (1 + 0.01 * ganJingBonusTotal);

  //与不同属性无关的总倍率
  double totalMultiplying = cardNumMultiplying * youQingMultiplying * xunLianMultiplying * ganJingMultiplying;

  //5.基础值
  int trainLv = getTrainingLevel(trainType);
  int basicValue[6] = { 0,0,0,0,0,0 };
  for (int i = 0; i < cardNum; i++)
  {
    for (int j = 0; j < 6; j++)
      basicValue[j] += effects[i].bonus[j];
  }
  for (int j = 0; j < 6; j++)
  {
    int b = GameConstants::TrainingBasicValue[trainType][trainLv][j];
    if(b>0)//关联属性
      basicValue[j] += b;
    else
      basicValue[j] = 0;
  }

  //6.成长率
  double growthRates[6] = { 1,1,1,1,1,1 };
  for (int j = 0; j < 5; j++)
    growthRates[j] = 1.0 + 0.01 * umaData->fiveStatusBonus[j];
    //growthRates[j] = 1.0 + 0.01 * GameDatabase::AllUmas[umaId].fiveStatusBonus[j];

  //下层总数值
  int totalValue[6];
  for (int j = 0; j < 6; j++)
  {
    int v = int(totalMultiplying * basicValue[j] * growthRates[j]);//向下取整了
    if (v > 100)v = 100;
    totalValue[j] = v;
  }
  
  //7.碎片
  for (int j = 0; j < 6; j++)
  {
    if (totalValue[j] > 0)//关联属性
      totalValue[j] += spiritBonus[j];
  } 
    
  //8.女神等级加成
  double venusMultiplying = 1.00 + 0.01 * (
    GameConstants::VenusLevelTrainBonus[venusLevelRed]
    + GameConstants::VenusLevelTrainBonus[venusLevelBlue]
    + GameConstants::VenusLevelTrainBonus[venusLevelYellow]
    );

  for (int j = 0; j < 6; j++)
  {
    totalValue[j] = int(venusMultiplying * totalValue[j]);
  }

  //体力
  int vitalChange=GameConstants::TrainingBasicValue[trainType][trainLv][6];
  for (int i = 0; i < cardNum; i++)
    vitalChange += effects[i].vitalBonus;
  if (vitalChange < 0)//消耗体力时，检查红女神等级
  {
    vitalChange = round(vitalChange*(1- 0.01*GameConstants::RedVenusLevelVitalCostDown[venusLevelRed])*vitalCostDrop);
  }


  for (int j = 0; j < 6; j++)
  {
    trainValue[trainType][j] = totalValue[j];
  }
  trainValue[trainType][6] = vitalChange;
}

bool Game::applyTraining(std::mt19937_64& rand, int chosenTrain, bool useVenus, int chosenSpiritColor, int chosenOutgoing, int forceThreeChoicesEvent)
{
  assert(stageInTurn == 1);
  stageInTurn = 2;
  if (isRacing)
  {
    if (useVenus)
    {
      if (venusAvailableWisdom != 0)
        activateVenusWisdom();//在checkEventAfterTrain()里关闭女神并清空碎片
      else
        return false;
    }
    if (turn != TOTAL_TURN - 1)//除了GrandMaster
      runRace(GameConstants::NormalRaceFiveStatusBonus, GameConstants::NormalRacePtBonus);
    addJiBan(6, 4);//理事长羁绊+4

    int newSpirit = (rand() % 6 + 1) + (rand() % 3) * 8;//随机加两个碎片
    addSpirit(rand, newSpirit);
    addSpirit(rand, newSpirit);
    return true;//GUR,WBC,SWBC,GrandMaster四场比赛在checkEventAfterTrain()里处理，不在这个函数
  }
  if (chosenTrain == 5)//休息
  {
    if (useVenus)
    {
      if (venusAvailableWisdom != 0)
        activateVenusWisdom();//在checkEventAfterTrain()里关闭女神并清空碎片
      else
        return false;
    }
    if (isXiaHeSu())
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

    addSpirit(rand, spiritDistribution[chosenTrain]);
  }
  else if (chosenTrain == 7)//比赛
  {
    if (turn <= 12 || turn >= 72)
    {
      printEvents("前13回合和最后6回合无法比赛");
      return false;
    }
    if (useVenus)
    {
      if (venusAvailableWisdom != 0)
        activateVenusWisdom();//在checkEventAfterTrain()里关闭女神并清空碎片
      else
        return false;
    }
    runRace(2, 40);//粗略的近似
    addSpirit(rand, spiritDistribution[chosenTrain]);
  }
  else if (chosenTrain == 6)//外出
  {
    if (isXiaHeSu())
    {
      printEvents("夏合宿只能休息，不能外出");
      return false;
    }
    if(!isOutgoingLegal(chosenOutgoing))
    {
      printEvents("不合法的外出");
      return false;
    }
    assert(!isXiaHeSu() && "夏合宿不允许外出");
    assert(isOutgoingLegal(chosenOutgoing) && "不合法的外出");
    if (useVenus)
    {
      if (venusAvailableWisdom != 0)
        activateVenusWisdom();//在checkEventAfterTrain()里关闭女神并清空碎片
      else
        return false;
    }
    if (chosenOutgoing < 5)//神团外出
      handleVenusOutgoing(chosenOutgoing);
    else if (chosenOutgoing == 6)//普通外出
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

    addSpirit(rand, spiritDistribution[chosenTrain]);
  }
  else if (chosenTrain <= 4 && chosenTrain >= 0)//常规训练
  {
    if (useVenus)
    {
      if (venusAvailableWisdom != 0)
        activateVenusWisdom();//在checkEventAfterTrain()里关闭女神并清空碎片
      else
        return false;
    }
    if (rand() % 100 < failRate[chosenTrain])//训练失败
    {
      if (failRate[chosenTrain] >= 20 && (rand() % 100 < failRate[chosenTrain]))//训练大失败，概率是瞎猜的
      {
        printEvents("训练大失败！");
        addStatus(chosenTrain, -10);
        if (fiveStatus[chosenTrain] > 1200)
          addStatus(chosenTrain, -10);//游戏里1200以上扣属性不折半，在此模拟器里对应1200以上翻倍
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
        addStatus(chosenTrain, -5);
        if (fiveStatus[chosenTrain] > 1200)
          addStatus(chosenTrain, -5);//游戏里1200以上扣属性不折半，在此模拟器里对应1200以上翻倍
        addMotivation(-1);
      }
    }
    else
    {
      //先加上训练值
      for (int i = 0; i < 5; i++)
        addStatus(i, trainValue[chosenTrain][i]);
      skillPt += trainValue[chosenTrain][5];
      addVital(trainValue[chosenTrain][6]);

      //羁绊
      for (int i = 0; i < 8; i++)
      {
        if (cardDistribution[chosenTrain][i])
        {
          assert(cardData[0]->cardType == 5 && "神团卡不在第一个位置");
          if (i == 0) //神团点一次+4羁绊
            addJiBan(i, 4);
          else
            addJiBan(i, 7);
          if (i == 6)skillPt += 2;//理事长
          if (i >= 6)continue;//理事长和记者
        }
      }

      //红点(hint)
      vector<int> hintCards;
      for (int i = 0; i < 6; i++)
      {
        if (cardDistribution[chosenTrain][i] && cardHint[i])
        {
          hintCards.push_back(i);
        }
      }

      auto applyHint= [this](int i)  {
        addJiBan(i, 5);
        auto& hintBonus = cardData[i]->hintBonus;
        for (int i = 0; i < 5; i++)
          addStatus(i, hintBonus[i]);
        skillPt += hintBonus[5];
      };

      
      //开蓝
      if (venusIsWisdomActive && venusAvailableWisdom == 2)
      {
        auto blueVenusBonus = calculateBlueVenusBonus(chosenTrain);
        for (int i = 0; i < 5; i++)
          addStatus(i, blueVenusBonus[i]);
        skillPt += blueVenusBonus[5];

        for (auto i = 0; i < hintCards.size(); i++)
          applyHint(hintCards[i]);
      }
      else//一次只有一张卡的红点可以生效，除非开蓝
      {
        if (hintCards.size() != 0)
        {
          int hintCard = hintCards[rand() % hintCards.size()];
          applyHint(hintCard);
        }
      }

      //加碎片
      addSpirit(rand, spiritDistribution[chosenTrain]);

      //点击了女神所在的训练
      assert(cardData[0]->cardType == 5 && "神团卡不在第一个位置");
      if (cardDistribution[chosenTrain][0])
      {
        if (!venusCardFirstClick)//第一次点
        {
          printEvents("第一次点女神");
          venusCardFirstClick = true;
          addAllStatus(3);
          addVital(10);
          addJiBan(0, 10);
        }
        else
        {
          //三选一事件概率，暂时猜测为40%*(1+蓝女神等级加成)
          bool activateThreeChoicesEvent = randBool(rand,getThreeChoicesEventProb(useVenus));
          if (forceThreeChoicesEvent == 1)
            activateThreeChoicesEvent = true;
          else if (forceThreeChoicesEvent == -1)
              activateThreeChoicesEvent = false;
          if (activateThreeChoicesEvent)
            handleVenusThreeChoicesEvent(rand, chosenSpiritColor);
        }

      }

      //训练等级计数+2
      if(!isXiaHeSu())
        addTrainingLevelCount(chosenTrain, 2);
    }
  }
  else
  {
    printEvents("未知的训练项目");
    return false;
  }
  return true;
}

float Game::getSkillScore() const
{
  float scorePtRate = isQieZhe ? GameConstants::ScorePtRateQieZhe : GameConstants::ScorePtRate;
  return scorePtRate * skillPt;
}

int Game::finalScore() const
{
  int total = 0;
  for (int i = 0; i < 5; i++)
    total += GameConstants::FiveStatusFinalScore[min(fiveStatus[i],fiveStatusLimit[i])];
  
  total += getSkillScore();
  return total;
}

bool Game::isEnd() const
{
  return turn >= TOTAL_TURN;
}

int Game::getTrainingLevel(int item) const
{
  int level ;
  if (venusIsWisdomActive && venusAvailableWisdom == 1)//红女神
    level = 5;
  else if(isXiaHeSu())
    level = 4;
  else
  {
    assert(trainLevelCount[item] <= 48, "训练等级计数超过48");
    level = trainLevelCount[item] / 12;
    if (level > 4)level = 4;
  }
  return level;
}

bool Game::isOutgoingLegal(int chosenOutgoing) const
{
  assert(chosenOutgoing >= 0 && chosenOutgoing <= 5 && "未知的外出");
  if (isXiaHeSu())return false;//夏合宿不允许外出
  if (chosenOutgoing == 5)return true;//普通外出
  //剩下的是神团外出
  if (!venusCardUnlockOutgoing)return false;
  if (venusCardOutgoingUsed[chosenOutgoing])return false;
  if (chosenOutgoing == 0 || chosenOutgoing == 1 || chosenOutgoing == 2)return true;
  else if (chosenOutgoing == 3)
    return venusCardOutgoingUsed[0] && venusCardOutgoingUsed[1] && venusCardOutgoingUsed[2];
  else if (chosenOutgoing == 4)
    return venusCardOutgoingUsed[3];
  else return false;//未知



}

bool Game::isXiaHeSu() const
{
  return (turn >= 36 && turn <= 39) || (turn >= 60 && turn <= 63);
}

double Game::getThreeChoicesEventProb(bool useVenusIfFull) const
{
  if (!venusCardFirstClick)return 0.0;
  if (venusCardIsQingRe)return 1.0;
  if (venusAvailableWisdom == 2 && useVenusIfFull)return 1.0;
  //三选一事件概率，暂时猜测为40%*(1+蓝女神等级加成)
  return GameConstants::VenusThreeChoicesEventProb * (1 + 0.01 * GameConstants::BlueVenusLevelHintProbBonus[venusLevelBlue]);

}



void Game::checkEventAfterTrain(std::mt19937_64& rand)
{
  assert(stageInTurn == 2);
  stageInTurn = 0;
  //女神会不会启动
  if (venusCardFirstClick && (!venusCardUnlockOutgoing))
  {
    if (randBool(rand, GameConstants::VenusUnlockOutgoingProbEveryTurn))//启动
    {
      printEvents("女神外出解锁！");
      //真似び学ぶ三女神
      venusCardUnlockOutgoing = true;
      venusCardIsQingRe = true;
      addAllStatus(6);
      skillPt += 12;
      assert(cardData[0]->cardType == 5 && "神团卡不在第一个位置");
      addJiBan(0, 5);
    }
  }

  //女神情热是否结束
  if(venusCardIsQingRe)
  {
    if (randBool(rand, GameConstants::VenusQingReDeactivateProb[venusCardQingReContinuousTurns]))
    {
      printEvents("女神情热结束");
      venusCardIsQingRe = false;
      venusCardQingReContinuousTurns = 0;
    }
    else venusCardQingReContinuousTurns++;
  }

  //处理各种固定事件
  if (turn == 23)//第一年年底
  {
    //GUR
    int raceFiveStatusBonus = 10;
    int racePtBonus = 50;
    if (venusLevelYellow >= 1)
      raceFiveStatusBonus += 2;//随机1个属性+10，改成平均
    if (venusLevelRed >= 1)
      racePtBonus += 20;
    runRace(raceFiveStatusBonus, racePtBonus);
    if (venusLevelBlue >= 1)
      skillPt += 10;//获得1个跑法技能

    //升训练等级
    for (int i = 0; i < 5; i++)
      addTrainingLevelCount(i, 8);

    //年底三选一事件，选属性还是体力
    //为了简化，直接视为全属性+5
    if (maxVital - vital >= 50)
      addVital(20);
    else 
      addAllStatus(5);

    //女神さま、ひとやすみ
    if (venusCardUnlockOutgoing)
    {
      addVital(19);
      skillPt += 36;
      skillPt += 50;//技能等效
      assert(cardData[0]->cardType == 5 && "神团卡不在第一个位置");
      addJiBan(0, 5);
    }

    printEvents("GUR结束");

  }
  else if (turn == 29)//第二年继承
  {
    for (int i = 0; i < 5; i++)
      addStatus(i, zhongMaBlueCount[i] * 6); //蓝因子典型值
    for (int i = 0; i < 5; i++)
      addStatus(i, zhongMaExtraBonus[i]); //剧本因子典型值
    skillPt += zhongMaExtraBonus[5];

    printEvents("第二年继承");
  }
  else if (turn == 47)//第二年年底
  {
    //WBC
    int raceFiveStatusBonus = 15;
    int racePtBonus = 60;
    if (venusLevelYellow >= 2)
      raceFiveStatusBonus += 4;//随机2个属性+10，改成平均
    if (venusLevelRed >= 2)
      racePtBonus += 30;
    runRace(raceFiveStatusBonus, racePtBonus);
    if (venusLevelBlue >= 2)
      skillPt += 20;//获得2个跑法技能

    //升训练等级
    for (int i = 0; i < 5; i++)
      addTrainingLevelCount(i, 8);

    //年底三选一事件，选属性还是体力
    if (maxVital - vital >= 50)
      addVital(30);
    else
      addAllStatus(8);
    printEvents("WBC结束");
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
  else if (turn == 49)//升固有
  {
    skillPt += 170 / GameConstants::ScorePtRate;//固有直接等价成pt
    printEvents("固有等级+1");
  }
  else if (turn == 53)//第三年继承&理事长升固有
  {
    for (int i = 0; i < 5; i++)
      addStatus(i, zhongMaBlueCount[i] * 6); //蓝因子典型值
    for (int i = 0; i < 5; i++)
      addStatus(i, zhongMaExtraBonus[i]); //剧本因子典型值
    skillPt += zhongMaExtraBonus[5];
    printEvents("第三年继承");
    if (cardJiBan[6] >= 60)//可以升固有
    {
      addMotivation(1);
      skillPt += 170 / GameConstants::ScorePtRate;//固有直接等价成pt
      printEvents("固有等级+1");
    }
    else
    {
      addVital(-5);
      skillPt += 25;
    }
  }
  else if (turn == 70)//升固有
  {
    skillPt += 170 / GameConstants::ScorePtRate;//固有直接等价成pt
    printEvents("固有等级+1");
  }
  else if (turn == 71)//第三年年底
  {
    //SWBC
    int raceFiveStatusBonus = 20;
    int racePtBonus = 70;
    if (venusLevelYellow >= 3)
      raceFiveStatusBonus += 6;//随机3个属性+10，改成平均
    if (venusLevelRed >= 3)
      racePtBonus += 45;
    runRace(raceFiveStatusBonus, racePtBonus);
    if (venusLevelBlue >= 3)
      skillPt += 30;//获得3个跑法技能

    //升训练等级
    for (int i = 0; i < 5; i++)
      addTrainingLevelCount(i, 8);

    printEvents("SWBC结束");
  }
  else if (turn == 76)//最后一战前的三选一
  {
    int totalLevel = venusLevelRed + venusLevelBlue + venusLevelYellow;
    int maxLevel = max(venusLevelRed, max(venusLevelBlue, venusLevelYellow));
    if (maxLevel >= 4)
    {
      addAllStatus(10);
      skillPt += 50;//技能等效
      if (maxLevel >= 5)
        skillPt += 20;//技能折扣
      if (totalLevel >= 12)
        skillPt += 40;//技能折扣
    }
  }
  else if (turn == 77)//最后一战
  {
    //GrandMasters
    int raceFiveStatusBonus = 20;
    int racePtBonus = 80;
    runRace(raceFiveStatusBonus, racePtBonus);

    //女神卡事件
    if (venusCardOutgoingUsed[4])//出行走完了
    {
      addAllStatus(12);
      skillPt += 12;
    }
    else
    {
      addAllStatus(8);
    }

    //记者
    if (cardJiBan[7] >= 100)
    {
      addAllStatus(5);
      skillPt += 20;
    }
    else if (cardJiBan[7] >= 80)
    {
      addAllStatus(3);
      skillPt += 10;
    }
    else
    {
      skillPt += 5;
    }

    //各种乱七八糟
    addAllStatus(25);
    skillPt += 80;

  }

  //模拟各种随机事件

  //支援卡连续事件，随机给一个卡加5羁绊
  if (randBool(rand, 0.3))
  {
    int card = rand() % 6;
    addJiBan(card, 5);

    printEvents("模拟随机事件：" + GameDatabase::AllCards[cardId[card]].cardName + " 的羁绊+5");
  }

  //模拟乱七八糟加属性事件
  if (turn < 72)
  {
    addAllStatus(1);
    printEvents("模拟随机事件：全属性+1");
  }

  //加体力
  if (randBool(rand, 0.1))
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
  
  //如果开女神，清空碎片
  if (venusIsWisdomActive)
  {
    venusIsWisdomActive = false;
    venusAvailableWisdom = 0;
    for (int i = 0; i < 8; i++)
      venusSpiritsBottom[i] = 0;
    for (int i = 0; i < 6; i++)
      venusSpiritsUpper[i] = 0;
    for (int i = 0; i < 6; i++)
      spiritBonus[i] = 0;
  }

  //回合数+1
  turn++;
  if (turn < TOTAL_TURN)
  {
    isRacing = umaData->races[turn] & TURN_RACE;
    //isRacing = GameDatabase::AllUmas[umaId].races[turn] & TURN_RACE;
  }
  else
  {
    printEvents("育成结束!");
    printEvents("你的得分是：" + to_string(finalScore()));
  }

}

void Game::applyTrainingAndNextTurn(std::mt19937_64& rand, int chosenTrain, bool useVenus, int chosenSpiritColor, int chosenOutgoing, int forceThreeChoicesEvent)
{
  assert(stageInTurn == 1);
  assert(turn < TOTAL_TURN && "Game::applyTrainingAndNextTurn游戏已结束");
  bool suc = applyTraining(rand, chosenTrain, useVenus, chosenSpiritColor, chosenOutgoing, forceThreeChoicesEvent);
  assert(suc && "Game::applyTrainingAndNextTurn选择了不合法的训练");

  checkEventAfterTrain(rand);
  if (isEnd()) return;

  if (isRacing)
  {
    if (venusAvailableWisdom == 0)//比赛回合且无法开女神，再进行一个回合
    {
      randomDistributeCards(rand);//把stageInTurn改成1
      bool useVenus = false;
      applyTrainingAndNextTurn(rand, -1, useVenus, -1, -1, -1);
    }
  }

  randomDistributeCards(rand);

}
