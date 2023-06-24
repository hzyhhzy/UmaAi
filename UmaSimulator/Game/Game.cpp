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
  randomDistributeCardsAndCalculate();
}
