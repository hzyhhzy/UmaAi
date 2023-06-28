#include <iostream>
#include "Game.h"
using namespace std;
void Game::newGame(mt19937_64& rand, int newUmaId, int newCards[6], int newZhongMaBlueCount[5])
{
  umaId = newUmaId;
  for (int i = 0; i < 6; i++)
    cardId[i] = newCards[i];
  for (int i = 0; i < 5; i++)
    zhongMaBlueCount[i] = newZhongMaBlueCount[i];

  random_device rd;
  rand = mt19937_64(rd);
  turn = 0;
  vital = 100;
  maxVital = 100;
  isQieZhe = false;
  isAiJiao = false;
  skillPt = 120;

  for (int i = 0; i < 5; i++)
    fiveValue[i] = GameDatabase::AllUmas[umaId].fiveValueInitial[i]; //赛马娘初始值
  for (int i = 0; i < 5; i++)//支援卡初始加成
  {
    for (int j = 0; j < 5; j++)
      fiveValue[j] = GameDatabase::AllSupportCards[cardId[i]].bonusBasic[j]; 
    skillPt += GameDatabase::AllSupportCards[cardId[i]].bonusBasic[5];
  }
  for (int i = 0; i < 5; i++)
    fiveValue[i] += zhongMaBlueCount[i] * 7; //种马

  for (int i = 0; i < 5; i++)
    fiveValueLimit[i] = GameConstants::BasicFiveValueLimit[i]; //原始属性上限
  for (int i = 0; i < 5; i++)
    fiveValueLimit[i] += zhongMaBlueCount[i] * 7; //属性上限--种马
  for (int i = 0; i < 5; i++)
    fiveValueLimit[i] += rand()%10; //属性上限--白因子随机增加

  motivation = 3;
  for (int i = 0; i < 6; i++)
    cardJiBan[i] = GameDatabase::AllSupportCards[cardId[i]].initialJiBan;
  cardJiBan[6] = 0; 
  cardJiBan[7] = 0;
  for (int i = 0; i < 5; i++)
    trainLevelCount[i] = 0;
  isRacing = false;


  venusLevelYellow = 0;
  venusLevelRed = 0;
  venusLevelBlue = 0;
  venusSpiritsCount = 0;
  for (int i = 0; i < 8; i++)
    venusSpiritsBottom[i] = 0;
  for (int i = 0; i < 6; i++)
    venusSpiritsUpper[i] = 0;
  venusAvailableWisdom = 0;
  venusIsWisdomActive = false;


  venusCardFirstClick = false;
  venusCardUnlockOutgoing = false;
  venusCardIsQingRe = false;
  for (int i = 0; i < 5; i++)
    venusCardOutgoingUsed[i] = false;

  stageInTurn = 0;
  randomDistributeCardsAndCalculate(rand);
}

void Game::randomDistributeCardsAndCalculate(std::mt19937_64& rand)
{
  //先将6张卡分配到训练中
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 8; j++)
      cardDistribution[i][j] = 0;
  for (int i = 0; i < 6; i++)
  {
    std::vector<int> probs = { 100,100,100,100,100,50 }; //基础概率，速耐力根智鸽
    int cardType = GameDatabase::AllSupportCards[cardId[i]].cardType;
    int deYiLv = GameDatabase::AllSupportCards[cardId[i]].deYiLv;
    if (cardType >= 0 && cardType < 5)//速耐力根智卡
      probs[cardType] + deYiLv;
    else //友人卡，鸽的概率较高
      probs[5] += 50;


    std::discrete_distribution<> d(probs.begin(), probs.end());
    int whichTrain = d(rand);//在哪个训练
    if (whichTrain < 5)//没鸽
      cardDistribution[whichTrain][i] = true;
  }
  //理事长和记者
  {
    std::vector<int> probs = { 100,100,100,100,100,100 }; //速耐力根智鸽
    std::discrete_distribution<> d(probs.begin(), probs.end());
    int whichTrain = d(rand);//在哪个训练
    if (whichTrain < 5)//没鸽
      cardDistribution[whichTrain][6] = true;
    whichTrain = d(rand);//在哪个训练
    if (whichTrain < 5)//没鸽
      cardDistribution[whichTrain][7] = true;
  }

  //分配完了，接下来计算属性加值
  for (int trainType = 0; trainType < 5; trainType++)
  {
    vector<CardTrainingEffect> effects;
    for (int card = 0; card < 6; card++)
    {
      if (cardDistribution[trainType][card])//这个卡在这个训练
      {
        effects.push_back(GameDatabase::AllSupportCards[cardId[card]].getCardEffect(*this, trainType, cardJiBan[card]));
      }
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

  }
}

int Game::getTrainingLevel(int item) const
{
  int level = trainLevelCount[item] / 12;
  if (level > 4)level = 4;
  if (venusIsWisdomActive && venusAvailableWisdom == 1)//红女神
    level = 5;
  return level;
}
