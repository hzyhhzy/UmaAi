#include "Person.h"
#include "../GameDatabase/GameDatabase.h"
using namespace std;
Person::Person()
{
  cardParam = SupportCard();
  personType = 0; 
  //cardId = 0;
  charaId = 0;

  friendship = 0;
  //for (int i = 0; i < 5; i++)atTrain[i] = false;
  isHint = false;
  cardRecord = 0;

  std::vector<int> probs = { 1,1,1,1,1,1 }; //速耐力根智鸽
  distribution = std::discrete_distribution<>(probs.begin(), probs.end());
}

void Person::setCard(int cardId)
{
  cardParam = GameDatabase::AllCards[cardId];


  friendship = cardParam.initialJiBan;
  isHint = false;
  cardRecord = 0;

  int cardType = cardParam.cardType;
  if (cardType == 5)//友人卡
  {
    int realCardId = cardId / 10;

    std::vector<int> probs = { 100,100,100,100,100,100 }; //基础概率，速耐力根智鸽
    distribution = std::discrete_distribution<>(probs.begin(), probs.end());

    personType = PersonType_friendCard;
    
  }
  else if (cardType == 6)//团队卡
  {
    std::vector<int> probs = { 100,100,100,100,100,100 }; //基础概率，速耐力根智鸽
    distribution = std::discrete_distribution<>(probs.begin(), probs.end());
    throw string("不支持带凉花/理事长以外的友人或团队卡");
  }
  else if (cardType >= 0 && cardType <= 4)//速耐力根智卡
  {
    personType = 2;
    std::vector<int> probs = { 100,100,100,100,100,50 }; //基础概率，速耐力根智鸽
    probs[cardType] += int(cardParam.deYiLv);
    distribution = std::discrete_distribution<>(probs.begin(), probs.end());
  }

}
void Person::setExtraDeyilvBonus(int deyilvBonus, bool lianghuaEffect)
{
  std::vector<int> probs = { 100,100,100,100,100,50 }; //基础概率，速耐力根智鸽
  if (personType != PersonType_card)//友人卡，鸽率2倍
    probs[5] = 100;
  if (lianghuaEffect) //ssr凉花固有
    probs[5] /= 2;

  if (personType == PersonType_card)
  {
    int newDeyilv = cardParam.deYiLv + deyilvBonus; //我不知道这里应该加还是乘
    
    probs[cardParam.cardType] += newDeyilv;
  }
  distribution = std::discrete_distribution<>(probs.begin(), probs.end());
}
/*
void Person::setNonCard(int pType)
{
  personType = pType;
  if (personType != PersonType_lishizhang && personType != PersonType_jizhe && personType != PersonType_lianghuaNonCard)
  {
    assert(false && "setNonCard只用于非支援卡人头的初始化");
  }

  friendship = 0;
  isHint = false;
  cardRecord = 0;
  friendOrGroupCardStage = 0;
  groupCardShiningContinuousTurns = 0;
  std::vector<int> probs = { 100,100,100,100,100,200 }; //基础概率，速耐力根智鸽
  distribution = std::discrete_distribution<>(probs.begin(), probs.end());
}
*/