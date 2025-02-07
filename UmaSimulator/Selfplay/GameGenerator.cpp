#include <fstream>
#include "GameGenerator.h"
#include "../Search/SearchParam.h"
using namespace std;

GameGenerator::GameGenerator(SelfplayParam param, Model* model) :param(param)
{
  evaluator = Evaluator(model, param.batchsize);
  std::random_device rd;
  rand = std::mt19937_64(rd());
  gameBuf.resize(param.batchsize);
  nextGamePointer = param.batchsize;
  if (param.cardRandType == 1 || param.cardRandType == 2)
    loadCardRankFile();
}

Game GameGenerator::randomOpening()
{
  std::exponential_distribution<double> expDistr(1.0);
  std::normal_distribution<double> normDistr(0.0, 1.0);
  std::uniform_real_distribution<double> uniDistr(0.0, 1.0);

  Game game;
  int umaId = 102401;//����
  int umaStars = 5;
  int cards[6] = { 301884,301724,301614,301784,301874,301734 };
  int zhongmaBlue[5] = { 18,0,0,0,0 };
  int zhongmaBonus[6] = { 10,10,30,0,10,70 };

  if (param.cardRandType == 0)
  {
  }
  else if (param.cardRandType == 1 || param.cardRandType == 2)
  {
    //�����
    umaId = -1;
    while (!GameDatabase::AllUmas.count(umaId))
      umaId = 100000 + (rand() % 200) * 100 + rand() % 2 + 1;

    auto cards1 = getRandomCardset();
    for (int i = 0; i < 6; i++)cards[i] = cards1[i];

    //��������Ӻ;籾����
    {
      vector<int> blueStarProb = { 1,1,5,100 };
      vector<int> blueTypeProb = { 20,1,3,1,1 };
      std::discrete_distribution<> blueStarProbDis(blueStarProb.begin(), blueStarProb.end());
      std::discrete_distribution<> blueTypeProbDis(blueTypeProb.begin(), blueTypeProb.end());
      for (int i = 0; i < 5; i++)
        zhongmaBlue[i] = 0;
      for (int i = 0; i < 6; i++)
      {
        zhongmaBlue[blueTypeProbDis(rand)] += blueStarProbDis(rand);
      }

      vector<int> scenarioStarProb = { 1,2,5,15 };
      vector<int> scenarioTypeProb = { 1,1,1 };//�ഺ�� ��ʦ�� ������
      std::discrete_distribution<> scenarioStarProbDis(scenarioStarProb.begin(), scenarioStarProb.end());
      std::discrete_distribution<> scenarioTypeProbDis(scenarioTypeProb.begin(), scenarioTypeProb.end());
      for (int i = 0; i < 5; i++)
        zhongmaBonus[i] = 0;
      for (int i = 0; i < 6; i++)
      {
        int type = scenarioTypeProbDis(rand);
        int star = scenarioStarProbDis(rand);
        int bonus = star == 3 ? 8 :
          star == 2 ? 4 :
          star == 1 ? 2 :
          0;
        if (type == 0)
        {
          zhongmaBonus[2] += bonus;
          zhongmaBonus[4] += bonus;
        }
        else if (type == 1)
        {
          zhongmaBonus[0] += bonus;
          zhongmaBonus[2] += bonus;
        }
        else if (type == 2)
        {
          zhongmaBonus[1] += bonus;
          zhongmaBonus[2] += bonus;
        }
        else assert(false);
      }
      zhongmaBonus[5] = expDistr(rand) * 70.0;
      if (zhongmaBonus[5] > 300)zhongmaBonus[5] = 300;
    }
  }





  
  game.newGame(rand, false, umaId, umaStars, cards, zhongmaBlue, zhongmaBonus);
  
  if (param.cardRandType == 2)
    randomizeUmaCardParam(game);

  game.eventStrength += int(normDistr(rand) * 5.0 + 0.5);
  if (game.eventStrength < 0)game.eventStrength = 0;

  //�����Լ����
  int r = rand() % 100;
  if (r < 10)//���������
  {
    double mean = -expDistr(rand) * 30;
    for (int i = 0; i < 5; i++)
      game.addStatus(i, int(expDistr(rand) * mean));
    game.skillPt += int(2 * expDistr(rand) * mean);
  }
  else if (r < 80)//��������ԣ�ģ�����
  {
    double mean = expDistr(rand) * 50;
    for (int i = 0; i < 5; i++)
      game.addStatus(i, int(expDistr(rand) * mean));
    game.skillPt += int(2 * expDistr(rand) * mean);
  }
  if (game.skillPt < 0)game.skillPt = 0;

  if (rand() % 2 == 0)
    game.ptScoreRate = 2.1 + 0.1 * normDistr(rand);
  else
    game.ptScoreRate = 2.2 + 0.3 * normDistr(rand);


  if (game.ptScoreRate > 3.0)game.ptScoreRate = 3.0;
  if (game.ptScoreRate < 1.5)game.ptScoreRate = 1.5;

  //if (rand() % 8 == 0)
  //  game.isQieZhe = true;
  if (rand() % 4 == 0) //��ϰ����
    game.failureRateBias = -2;
  if (rand() % 128 == 0) //˫Ȧ��ϰ����
    game.failureRateBias = -5;
  if (rand() % 8 == 0)
    game.isAiJiao = true;

  game.motivation = rand() % 5 + 1;

  for (int i = 0; i < 6; i++)
  {
    int cardPerson = i;
    if (rand() % 2)
    {
      int delta = int(expDistr(rand) * 5);
      game.addJiBan(cardPerson, delta, true);
    }
  }


  if (rand() % 4)
  {
    int delta = int(expDistr(rand) * 4);
    game.maxVital += delta;
    if (game.maxVital > 120)game.maxVital = 120;
  }

  return game;
}

Game GameGenerator::randomizeBeforeOutput(const Game& game0)
{
  //assert("false" && "not implemented, TODO: GameGenerator::randomizeBeforeOutput");
  
  Game game = game0;
  std::exponential_distribution<double> expDistr(1.0);
  std::normal_distribution<double> normDistr(0.0, 1.0);
  std::uniform_real_distribution<double> uniDistr(0.0, 1.0);

  //�����Լ����
  int type0 = rand() % 100;
  if (type0 < 30)//���������
  {
    double mean = -expDistr(rand) * 50;
    for (int i = 0; i < 5; i++)
      game.addStatus(i, int(expDistr(rand) * mean));
    game.skillPt += int(2 * expDistr(rand) * mean);
  }
  else if (type0 < 80)//��������ԣ�ģ�����
  {
    double mean = expDistr(rand) * 80;
    for (int i = 0; i < 5; i++)
      game.addStatus(i, int(expDistr(rand) * mean));
    game.skillPt += int(2 * expDistr(rand) * mean);
  }
  if (game.skillPt < 0)game.skillPt = 0;

  if (rand() % 8 == 0)
    game.vital = rand() % (game.maxVital + 1);
  //��ϰ����
  if (rand() % 512 == 0)
    game.failureRateBias = 2;


  // �������
  if (rand() % 2 == 0)
  {
    double mean = expDistr(rand) * 30; //����dif�ķ�Χ����+-90������+-270ż�����ȽϺ���
      for (int i = 0; i < 5; i++)
      {
        int dif = normDistr(rand) * mean + 0.5;
        int newlevel = game.cook_material[i] + dif;
        if (newlevel > 999)newlevel = 999;
        if (newlevel < 0)newlevel = 0;
        game.cook_material[i] = newlevel;
      }
  }
  // ũ��pt���
  if (rand() % 2 == 0)
  {
    double mean = expDistr(rand) * 30; //����dif�ķ�Χ����+-90������+-270ż�����ȽϺ���
    int dif = normDistr(rand) * mean + 0.5;
    int newlevel = game.cook_farm_pt + dif;
    if (newlevel > 999)newlevel = 999;
    if (newlevel < 0)newlevel = 0;
    game.cook_farm_pt = newlevel;
  }

  // ũ��ȼ����
  if (rand() % 4 == 0)
  {
    double rand_number = rand() % 100;
    for (int i = 0; i < 5; i++)
    {
      int dif = rand_number>67?1:rand_number>33?0:-1; // 1/3����������1/3���ʲ��䣬1/3���ʽ���
      int newlevel = game.cook_farm_level[i] + dif;
      if (newlevel > 5)newlevel = 5;
      if (newlevel < 1)newlevel = 1;

      if (newlevel > 3 && game.turn <= 48)
        newlevel=3;
      if (newlevel > 2 && game.turn <= 24)
        newlevel=2;

      game.cook_farm_level[i] = newlevel;
    }
  }

  // ��Ȧ��¼���
  if (rand() % 4 == 0)
  {
    for (int i = 0; i < 4; i++)
    {
      if (rand() % 2 == 0)
        game.cook_harvest_green_history[i] = rand() % 2?true:false;
    }
  }


  //���³����ߵ�������
  for (int i = 0; i < 1; i++)
  {
    if (game.friend_type != 0 && i == 0)continue;
    if (rand() % 8 == 0)
    {
      game.addJiBan(6 + i, rand() % 8,true);
    }
  }

  return game;
}

void GameGenerator::newGameBatch()
{
  const int maxTurn = TOTAL_TURN - 4;//4���̶������غϣ�������ѵ��TOTAL_TURN - 4��
  vector<int> turnsEveryGame(param.batchsize);
  evaluator.gameInput.resize(param.batchsize);
  for (int i = 0; i < param.batchsize; i++)
  {
      evaluator.gameInput[i] = randomOpening();
      int randTurn = rand() % maxTurn;
      turnsEveryGame[i] = randTurn;
  }

  int all_end = param.batchsize; // �ܹ�����Ϸ������

  //�������һЩ�غ�
  SearchParam defaultSearchParam(1024, 5.0);//�����������ȡ��ֻ�������ɿ���ʱ����������
  for (int depth = 0; depth < maxTurn*2; depth++)
  {
    evaluator.evaluateSelf(1, defaultSearchParam);//����policy
    // assert("false" && "TODO:�¾籾applyAction��һ����һ���غϣ�Ҫ�����ɲ���");

    for (int i = 0; i < param.batchsize; i++)
    {
        if (evaluator.gameInput[i].turn < turnsEveryGame[i])
        {
            evaluator.gameInput[i].applyAction(rand, evaluator.actionResults[i]);
            if (evaluator.gameInput[i].turn == turnsEveryGame[i])
                all_end -= 1;
        }
    }
    if (all_end == 0)
        break;
  }
  for (int i = 0; i < param.batchsize; i++)
  {
    gameBuf[i] = randomizeBeforeOutput(evaluator.gameInput[i]);
  }
}
bool GameGenerator::isVaildGame(const Game& game)
{
  if (game.isEnd())return false;
  if (game.isRacing)return false;
  return true;
}

Game GameGenerator::get()
{
  while (true)
  {
    if (nextGamePointer >= param.batchsize)
    {
      nextGamePointer = 0;
      newGameBatch();
    }
    if (!isVaildGame(gameBuf[nextGamePointer]))
    {
      nextGamePointer += 1;
      continue;
    }
    else
    {
      nextGamePointer += 1;
      return gameBuf[nextGamePointer - 1];
    }
  }
}
