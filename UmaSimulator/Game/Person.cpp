#include "Person.h"
#include "../GameDatabase/GameDatabase.h"
using namespace std;
Person::Person()
{
  personType = 0; 
  //cardId = 0;
  charaId = 0;

  cardIdInGame = -1;
  friendship = 0;
  for (int i = 0; i < 5; i++)atTrain[i] = false;
  isShining = false;
  cardRecord = 0;

  larc_isLinkCard = false;
  larc_charge = 0;
  larc_atSS = false;
  larc_statusType = -1;
  larc_specialBuff = 0;
  larc_level = 0;
  for (int i = 0; i < 3; i++)larc_nextThreeBuffs[i] = 0;
  larc_assignedStatusTypeWhenFull = -1;


  std::vector<int> probs = { 1,1,1,1,1,1 }; //ËÙÄÍÁ¦¸ùÖÇ¸ë
  distribution = std::discrete_distribution<>(probs.begin(), probs.end());
}

