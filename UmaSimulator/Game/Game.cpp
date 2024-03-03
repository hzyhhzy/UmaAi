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
  eventStrength = GameConstants::EventStrengthDefault;
  turn = 0;
  vital = 100;
  maxVital = 100;
  isQieZhe = false;
  isAiJiao = false; 
  failureRateBias = 0;
  skillPt = 120;
  skillScore = umaStars >= 3 ? 170 * (umaStars - 2) : 120 * (umaStars);//固有技能
  motivation = 3;
  isPositiveThinking = false;
  for (int i = 0; i < 5; i++)
    trainLevelCount[i] = 0;
  isRacing = false;

  larc_zuoyueType = 0;
  larc_zuoyueVitalBonus = 0;
  larc_zuoyueStatusBonus = 0;
  for (int i = 0; i < 18; i++)
  {
    persons[i] = Person();
  }
  persons[15].personType = 4;
  persons[16].personType = 5;
  persons[17].personType = 6;
  for (int i = 0; i < 18; i++)
  {
    if (persons[i].personType == 0)
      persons[i].personType = 3;
  }


  normalCardCount = 0;//速耐力根智卡的数量
  saihou = 0;
  for (int i = 0; i < 6; i++)
  {
    int cardId = newCards[i];
    cardParam[i] = GameDatabase::AllCards[cardId];
    SupportCard& cardP = cardParam[i];
    saihou += cardP.saiHou;
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

        int zuoyueLevel = cardId % 10;
        if (larc_zuoyueType==1)
        {
          larc_zuoyueVitalBonus = GameConstants::ZuoyueVitalBonusSSR[zuoyueLevel];
          larc_zuoyueStatusBonus = GameConstants::ZuoyueStatusBonusSSR[zuoyueLevel];
        }
        else
        {
          larc_zuoyueVitalBonus = GameConstants::ZuoyueVitalBonusR[zuoyueLevel];
          larc_zuoyueStatusBonus = GameConstants::ZuoyueStatusBonusR[zuoyueLevel];
        }
        larc_zuoyueVitalBonus += 1e-10;
        larc_zuoyueStatusBonus += 1e-10;//加个小量，避免因为舍入误差而算错

        persons[17].personType = 1;
        persons[17].cardIdInGame = i;
        persons[17].friendship = cardP.initialJiBan;
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
  for (int i = 0; i < 9; i++)
    larc_allowedDebuffsFirstLarc[i] = false;

  //larc_zuoyueType
  //larc_zuoyueCardLevel
  larc_zuoyueFirstClick = false;
  larc_zuoyueOutgoingUnlocked = false; 
  larc_zuoyueOutgoingRefused = false;
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
    fiveStatus[i] = GameDatabase::AllUmas[umaId].fiveStatusInitial[i] - 10 * (5 - umaStars); //赛马娘初始值
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
  larc_ssPersonsCountLastTurn = 0;
  randomDistributeCards(rand); 
}

void Game::initNPCsTurn3(std::mt19937_64& rand)
{
  //int allSpecialBuffsNum[13] = {0,0,0,1,1,2,2,4,1,1,0,0,3};   // 对应巨匠前版本则需要用这个数据
  int allSpecialBuffsNum[13] = { 0,0,0,0,2,2,2,4,1,1,0,0,3 };   // 2.24 巨匠加入后更新 普通回体变成回体加上限
  int specialBuffEveryPerson[15];
  for (int i = 0; i < 15; i++)specialBuffEveryPerson[i] = 0;

  //查找这张卡固定的特殊buff
  for (int i = 0; i < normalCardCount; i++)
  {
    assert(persons[i].personType == 2);
    int s = cardParam[persons[i].cardIdInGame].larc_linkSpecialEffect;
    if (s != 0)
    {
      specialBuffEveryPerson[i] = s;
      allSpecialBuffsNum[s] -= 1;
      assert(allSpecialBuffsNum[s] >= 0);
    }
    
  }

  //没固定特殊buff的就随机分配特殊buff
  vector<int> specialBuffNotAssigned;
  for (int i = 0; i < 13; i++)
  {
    int n = allSpecialBuffsNum[i];
    if (n >= 0)
    {
      for (int j = 0; j < n; j++)
        specialBuffNotAssigned.push_back(i);
    }
  }
  std::shuffle(specialBuffNotAssigned.begin(), specialBuffNotAssigned.end(), rand);
  int c = 0;
  for (int i = 0; i < 15; i++)
  {
    if (specialBuffEveryPerson[i] == 0)
    {
      specialBuffEveryPerson[i] = specialBuffNotAssigned[c];
      c += 1;
    }
  }

  //人头属性
  vector<int> s = { 0,0,0,1,1,1,2,2,2,3,3,3,4,4,4 };
  assert(s.size() == 15);
  std::shuffle(s.begin(), s.end(), rand);

  //初始化
  for (int i = 0; i < 15; i++)
  {
    persons[i].initAtTurn3(rand, specialBuffEveryPerson[i], s[i]);
  }


}

void Game::randomDistributeCards(std::mt19937_64& rand)
{
  //assert(stageInTurn == 0 || turn == 0);
  stageInTurn = 1;

  //比赛回合的人头分配和比赛/远征回合的ss，不需要置零，因为不输入神经网络
  if (isRacing)
    return;//比赛不用分配卡组，但要改stageInTurn
  
  //先将6张卡分配到训练中
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 5; j++)
      personDistribution[i][j] = -1;

  int headCountEveryTrain[5] = { 0,0,0,0,0 };//五个训练分别有多少人，超过5人也继续加

  //把一个人头放在某个训练里，如果人数超过5则有概率随机踢掉一个
  auto setHead = [&](int head, int whichTrain)
  {
    if (whichTrain >= 5)return;
    int p = headCountEveryTrain[whichTrain];
    if (p < 5)//还没5个头
    {
      personDistribution[whichTrain][p] = head;
    }
    else
    {
      // 纯瞎猜的公式，可以保证所有人头地位平等：5/(p+1)的概率替换掉其中一个头，(p-4)/(p+1)的概率鸽
      int r = rand() % (p + 1);
      if (r < 5)//随机换掉一个人
        personDistribution[whichTrain][r] = head;
    }
    headCountEveryTrain[whichTrain] += 1;//无论是否留在这个训练里，计数都加一
  };


  for (int i = 0; i < 18; i++)
  {
    if (turn < 2 && persons[i].personType != 2)//前两回合没有佐岳和npc和理事长记者等
      continue;
    if (larc_isAbroad && (i == 15 || i == 16 || (i == 17 && persons[i].personType != 1)))//远征时理事长记者不在，不带卡的佐岳也不在
      continue;
    if (turn < 10 && i == 16)//记者第10个回合来
      continue;

    if (i == 17 && larc_zuoyueType == 1 && persons[i].friendship >= 60)//ssr佐岳且羁绊60，会分身，因此单独处理
    {
      int whichTrain1 = rand() % 6;
      setHead(i, whichTrain1);
      if (whichTrain1 == 5)
      {
        int whichTrain2 = rand() % 6;
        setHead(i, whichTrain2);
      }
      else
      {
        //假设第二个位置和第一个不会撞车，这样与实测概率比较接近
        int whichTrain2 = rand() % 5;
        if (whichTrain2 >= whichTrain1)whichTrain2++;
        setHead(i, whichTrain2);
      }
    }
    else
    {
      int whichTrain = persons[i].distribution(rand);
      setHead(i, whichTrain);
    }

    //是否有hint
    if (persons[i].personType == 2)
    {
      double hintProb = 0.06 * (1 + 0.01 * cardParam[persons[i].cardIdInGame].hintProbIncrease);
      bernoulli_distribution d(hintProb);
      persons[i].isHint = d(rand);
    }
  }
  
  //分配ss人头
  if (!larc_isAbroad)
  {
    int fullNum = 0;
    for (int i = 0; i < 5; i++)
      larc_ssPersons[i] = -1;
    for (int i = 0; i < 15; i++)
    {
      if (persons[i].larc_charge == 3)
      {
        if (fullNum < 5)
          larc_ssPersons[fullNum] = i;
        else
        {
          int r = rand() % (fullNum + 1);
          if (r < 5)//随机换掉一个人
            larc_ssPersons[r] = i;
        }
        fullNum += 1;
      }
    }
    larc_ssPersonsCount = fullNum > 5 ? 5 : fullNum;

    bool isNewFullSS = larc_ssPersonsCount >= 5 && larc_ssPersonsCountLastTurn < 5;//为了避免满10人连出两个ss时计算错误，使用ss的时候把这个置零
    larc_ssPersonsCountLastTurn = larc_ssPersonsCount;
    if (isNewFullSS)
    {
      larc_isSSS = randBool(rand, sssProb(larc_ssWinSinceLastSSS));
    }
  }
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
//P3 = P2 + 蓝buff（具体公式未知）
//总数T = P3 * (1 + link数加成 + 大会优胜数加成)
//上层U = T - L

void Game::calculateTrainingValue()
{


  //剧本训练加成
  uaf_trainingBonus = 0;
  for (int color = 0; color < 3; color++)
  {
    int levelTotal = 0;
    int winCount = 0;
    for (int i = 0; i < 5; i++)
      levelTotal += uaf_trainingLevel[color][i];

    for (int i = 0; i < 5; i++)
      for (int j = 0; j < 5; j++)
        winCount += uaf_winHistory[i][color][j];

    uaf_trainLevelColorTotal[color] = levelTotal;
    uaf_colorWinCount[color] = winCount;
    uaf_trainingBonus += GameConstants::UAF_WinNumTrainingBonus[winCount];
  }

  //训练等级增加量与卡数的关系
  static const int cardNumLevelBonus[6] = { 0,1,2,2,3,3 };

  //把闪彩的卡按persons的编号记下来
  bool isCardShining_record[6] = { false,false,false,false,false,false };
  int cardNum_record[5];

  //计算等级增加量
  for (int tra = 0; tra < 5; tra++)
  {
    int cardNum = 0;
    int shiningNum = 0;
    int linkNum = 0;
    for (int h = 0; h < 5; h++)
    {
      int pIdx = personDistribution[tra][h];
      if (pIdx < 0)break;
      const Person& p = persons[pIdx];
      if (pIdx >= 6)continue;//不是支援卡
      assert(p.personType == PersonType_card || p.personType == PersonType_lianghuaCard || p.personType == PersonType_otherFriend || p.personType == PersonType_groupCard);
      
      cardNum += 1;
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
    cardNum_record[tra] = cardNum;
    trainShiningNum[tra] = shiningNum;

    int levelGain = 3 + cardNumLevelBonus[cardNum];
    if (shiningNum > 0)
      levelGain *= 2;
    levelGain += linkNum;
    if (uaf_lastTurnNotTrain)
      levelGain += 3;
    uaf_trainLevelGain[tra] = levelGain;
  }


  //link统计与加成
  int trainingColorNum[3];//三种颜色的训练个数
  int levelGainColorTotal[3];//三种颜色的训练等级提升总数
  int linkBasicValueBonus[3][6];//三种颜色的基础值增加量

  for (int color = 0; color < 3; color++)
  {
    trainingColorNum[color] = 0;
    levelGainColorTotal[color] = 0;
    for (int i = 0; i < 6; i++)
      linkBasicValueBonus[color][i] = 0;

    for (int t = 0; t < 5; t++)
    {
      if (uaf_trainingColor[t] != color)
        continue;
      trainingColorNum[color] += 1;
      levelGainColorTotal[color] += uaf_trainLevelGain[t];
      int splevel = uaf_trainingLevel[color][t];
      int gain = splevel < 30 ? 1 : splevel < 50 ? 2 : 3;
      linkBasicValueBonus[color][t] += gain;
    }
    linkBasicValueBonus[color][5] = trainingColorNum[color];
    if (trainingColorNum[color] <= 1)
    {
      for (int t = 0; t < 5; t++)
        linkBasicValueBonus[color][t] = 0;//一个link都没有吃不到加成
    }
  }

  //计算五个训练的各种数值
  for (int t = 0; t < 5; t++)
  {
    int basicValueLower[6];//训练的下层基础值，=原基础值+支援卡加成
    double valueLowerDouble[6];//训练的下层值，先不取整
    double valueDouble[6];//训练的总数，先不取整
    //int basicValue[6];//训练的基础值，= basicValueLower + linkBasicValueBonus
    int cardNum = 0;//几张卡，理事长记者不算
    int totalXunlian = 0;//训练1+训练2+...
    int totalGanjing = 0;//干劲1+干劲2+...
    double totalYouqingMultiplier = 1.0;//(1+友情1)*(1+友情2)*...
    int vitalCostBasic;//体力消耗基础量，=ReLU(基础体力消耗+link体力消耗增加-智彩体力消耗减少)
    double vitalCostMultiplier = 1.0;//(1-体力消耗减少率1)*(1-体力消耗减少率2)*...
    double failRateMultiplier = 1.0;//(1-失败率下降率1)*(1-失败率下降率2)*...

    int color = uaf_trainingColor[t]; 
    int colorNum = trainingColorNum[color];
    int splevel = uaf_trainingLevel[color][t];
    int tlevel = convertTrainingLevel(splevel);
    for (int i = 0; i < 6; i++)
      basicValueLower[i] = GameConstants::TrainingBasicValue[color][t][tlevel][i];
    vitalCostBasic = GameConstants::TrainingBasicValue[color][t][tlevel][6];
    vitalCostBasic += colorNum - 1;//link越多体力消耗越多

    for (int a = 0; a < 5; a++)
    {
      int pid = personDistribution[t][a];
      if (pid < 0)break;//没人
      if (pid >= 6)continue;//不是卡
      cardNum += 1;
      const Person& p = persons[pid];
      bool isThisCardShining = isCardShining_record[pid];//这张卡闪没闪
      bool isThisTrainingShining = trainShiningNum[t];//这个训练闪没闪
      CardTrainingEffect eff = p.cardParam.getCardEffect(*this, isThisCardShining, t, p.friendship, p.cardRecord, cardNum_record[t], trainShiningNum[t]);
        
      for (int i = 0; i < 6; i++)//基础值bonus
      {
        if (basicValueLower[i] > 0)
          basicValueLower[i] += eff.bonus[i];
      }
      if (isCardShining_record[pid])//闪彩，友情加成和智彩回复
      {
        totalYouqingMultiplier *= (1 + 0.01 * eff.youQing);
        if (t == TRA_wiz)
          vitalCostBasic -= eff.vitalBonus;
      }
      totalXunlian += eff.xunLian;
      totalGanjing += eff.ganJing;
      vitalCostMultiplier *= (1 - 0.01 * eff.vitalCostDrop);
      failRateMultiplier *= (1 - 0.01 * eff.failRateDrop);

    }

    //体力，失败率

    if (vitalCostBasic < 0)//uaf训练不会加体力，即使是很多智彩
      vitalCostBasic = 0;
    int vitalChangeInt = -int(vitalCostBasic * vitalCostMultiplier);
    if (vitalChangeInt > maxVital - vital)vitalChangeInt = maxVital - vital;
    if (vitalChangeInt < -vital)vitalChangeInt = -vital;
    trainVitalChange[t] = vitalChangeInt;
    failRate[t] = calculateFailureRate(t, failRateMultiplier);


    //k = 人头 * 训练 * 干劲 * 友情    //支援卡倍率
    double cardMultiplier = (1 + 0.05 * cardNum) * (1 + 0.01 * totalXunlian) * (1 + 0.1 * (motivation - 3) * (1 + 0.01 * totalGanjing)) * totalYouqingMultiplier;
    trainValueCardMultiplier[t] = cardMultiplier;

    //下层可以开始算了
    for (int i = 0; i < 6; i++)
    {
      if (basicValueLower[i] == 0)
        trainValueLower[t][i] = 0;
      else
      {
        double umaBonus = i < 5 ? 1 + 0.01 * fiveStatusBonus[i] : 1;
        double v = basicValueLower[i] * cardMultiplier * umaBonus;

        //经过后续实验验证，100上限在这里还没算
        //if (v < 1)v = 1;
        //if (v > 100)v = 100;

        //L = k * B * G //下层，先不取整
        valueLowerDouble[i] = v;

        trainValueLower[t][i] = std::min(int(v),100);
      }
    }
    
    //然后算总数

    //P1 = L + k * C
    for (int i = 0; i < 6; i++)//link基础值bonus
      valueDouble[i] = valueLowerDouble[i] + linkBasicValueBonus[color][i] * cardMultiplier;

    //P2 = P1 * (1 + 红buff倍率)，红buff倍率只乘在当前训练的主属性上
    if (uaf_buffNum[0] > 0)
    {
      double redBuffMultiplier = 1 + 0.01 * GameConstants::UAF_RedBuffBonus[colorNum];
      valueDouble[t] *= redBuffMultiplier;
    }

    //P3 = P2 * (1 + link数加成 + 大会优胜数加成)
    int linkNumTrainingRate = isXiahesu() ? GameConstants::UAF_LinkNumBonusXiahesu[colorNum] : GameConstants::UAF_LinkNumBonus[colorNum];
    double scenarioTrainingMultiplier = 1.0 + 0.01 * (linkNumTrainingRate + uaf_trainingBonus);

    for (int i = 0; i < 6; i++)
      valueDouble[i] *= scenarioTrainingMultiplier;
    

    //总数T = P3 + 蓝buff
    if (uaf_buffNum[1] > 0)
    {
      for (int i = 0; i < 5; i++)
      {
        int levelGain = uaf_trainLevelGain[i];
        int maxLevelGain = 100 - uaf_trainingLevel[uaf_trainingColor[i]][i];
        if (levelGain > 100 - maxLevelGain)levelGain = maxLevelGain;
        valueDouble[i] += int(levelGain / 2) + 1;
      }
      valueDouble[5] += 20;
    }


    //上层U = T - L

    for (int i = 0; i < 6; i++)
    {
      //这里还要研究一下，upper是总数减去考虑100上限的下层，还是减去没考虑上限的浮点数下层
      //之前有一个样例，为智高峰固有买了技能之后，下层+1，上层反而-1，因此应该是前者
      int upper = valueDouble[i] - trainValueLower[t][i];
      if (upper > 100)upper = 100;
      trainValue[t][i] = upper + trainValueLower[t][i];
    }


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
void Game::runSS(std::mt19937_64& rand)
{
  //ss失败的没有特殊buff，特殊buff也不会滚动，自己的支援者pt少一半，对手的没有少。除此以外（加属性，羁绊，适性pt等）完全相同
  //sss必胜

  int ssWinNum = 0;

  //体力和爱娇最后加，因为先加体力上限再加体力，先加羁绊再爱娇
  int vitalGain = 0;
  bool newAiJiao = false;

  for (int i = 0; i < larc_ssPersonsCount; i++)
  {
    int id = larc_ssPersons[i];
    Person& p = persons[id];
    p.larc_charge = 0;
    p.larc_level += 1;
    bool suc = larc_ssFailRate > 0 ? randBool(rand, 0.01 * (100 - larc_ssFailRate)) : true;
    if (suc)//特殊buff生效
    {
      ssWinNum += 1;
      int buff = p.larc_nextThreeBuffs[0];
      if (isAiJiao && buff == 8) //重复爱娇会变成属性
        buff = 11;
      if (buff == 1)//技能
      {
        int equalPt = 6;//等效pt。路人人头随机给的1级技能大概率没用，所以分给低点
        if (p.personType == 2)//支援卡
        {
          equalPt = cardParam[p.cardIdInGame].hintBonus[5];
          if (equalPt == 0)equalPt = 6;//乌拉拉之类的没技能
        }
        skillPt += equalPt;
      }
      else if (buff == 3)//体力
      {
        vitalGain += 20;
      }
      else if (buff == 4)//体力与上限
      {
        maxVital += 4;
        vitalGain += 20;
      }
      else if (buff == 5)//体力与干劲
      {
        addMotivation(1);
        vitalGain += 20;
      }
      else if (buff == 6)//充电
      {
        p.larc_charge = 3;
      }
      else if (buff == 7)//适性pt
      {
        larc_shixingPt += 50;
      }
      else if (buff == 8)//爱娇
      {
        assert(!isAiJiao && "不会重复出爱娇");
        newAiJiao = true;
      }
      else if (buff == 9)//练习上手
      {
        failureRateBias = -2;
      }
      else if (buff == 11)//属性
      {
        if(p.larc_isLinkCard)
          addAllStatus(3);//随机一个属性+15，为了方便直接平摊
        else
          addAllStatus(2);//随机一个属性+10，为了方便直接平摊
      }
      else if (buff == 12)//技能点
      {
        skillPt += 20;
      }
      else
      {
        assert(false && "未知的ss buff");
      }


      p.larc_nextBuff(rand);
    }

    if (p.personType == 2)//支援卡，加羁绊
    {
      addJiBan(id, 7);
    }
  
  }
  if (newAiJiao)isAiJiao = true;
  addVital(vitalGain);


  for (int i = 0; i < 5; i++)
    addStatus(i, larc_ssValue[i]);
  skillPt += 5 * larc_ssPersonsCount;
  if (larc_isSSS)skillPt += 25;
  larc_shixingPt += 10 * larc_ssPersonsCount;

  int supportPtFactor = larc_ssPersonsCount + ssWinNum;//输了的约等于半个人头
  int supportPtEveryHalfHead = larc_isSSS ? 1000 + rand() % 64 : 580 + rand() % 64;//半个人头的supportPt的增加量，和这几个人头在所有人中的排名有关系，非常复杂，为了省事就取了个平均再加一些随机

  int oldSupportPt = larc_supportPtAll;
  larc_supportPtAll += supportPtFactor * supportPtEveryHalfHead;
  checkSupportPtEvents(oldSupportPt, larc_supportPtAll);//期待度上升事件

  larc_ssWin += ssWinNum;

  if (larc_isSSS)
    larc_ssWinSinceLastSSS = 0;
  else
    larc_ssWinSinceLastSSS += ssWinNum;


  //清理
  larc_ssPersonsCount = 0;
  larc_ssPersonsCountLastTurn = 0;

  for (int i = 0; i < 5; i++)larc_ssPersons[i] = -1;
  larc_isSSS = false;

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
void Game::addTrainingLevelCount(int item, int value)
{
  assert(item >= 0 && item < 5 && "addTrainingLevelCount不合法训练");
  trainLevelCount[item] += value;
  if (trainLevelCount[item] > 4 * 4)trainLevelCount[item] = 4 * 4;
}
void Game::charge(int idx, int value)
{
  if (value == 0)return;
  persons[idx].larc_charge += value;
  if (persons[idx].larc_charge > 3)persons[idx].larc_charge = 3;
}
void Game::unlockUpgrade(int idx)
{
  if (larc_levels[idx] == 0)
    larc_levels[idx] = 1;
  //只会在回合结束时调用这个，所以不需要重新计算训练值
}
int Game::buyUpgradeCost(int idx, int level) const
{
  int cost = 0;

  if (larc_levels[idx] == 0)
    return -1;//没解锁
  else if (larc_levels[idx] == 1)
  {
    if (level == 2)
      cost = GameConstants::LArcUpgradesCostLv2[idx];
    else if (level == 3)
      cost = GameConstants::LArcUpgradesCostLv3[idx] + GameConstants::LArcUpgradesCostLv2[idx];
    else
      return -1;
  }
  else if (larc_levels[idx] == 2)
  {
    if (level == 2)
      cost = 0;
    else if (level == 3)
      cost = GameConstants::LArcUpgradesCostLv3[idx];
    else
      return -1;
  }
  else if (larc_levels[idx] == 3)
  {
    return -1;
  }

  return cost;
}
bool Game::tryBuyUpgrade(int idx, int level)
{
  int cost = buyUpgradeCost(idx, level);

  if (cost > larc_shixingPt)return false;
  if (cost < 0)return false;//未解锁
  if (cost == 0)return true;//已购买
  larc_shixingPt -= cost;
  larc_levels[idx] = level;
  
  if (level == 3)//需要重新计算训练值
    calculateTrainingValue();

  return true;
}
int Game::removeDebuffsFirstNCost(int n) const
{
  int totalCost = 0;

  for (int i = 0; i < n; i++)
  {
    if (larc_allowedDebuffsFirstLarc[i])
      continue;
    if (larc_levels[i] == 0)
      return 10000;
    else if (larc_levels[i] == 1)
      totalCost += GameConstants::LArcUpgradesCostLv2[i];
  }

  return totalCost;
}

bool Game::tryRemoveDebuffsFirstN(int n)
{

  if (larc_shixingPt < removeDebuffsFirstNCost(n))
    return false;

  for (int i = 0; i < n; i++)
  {
    if (larc_allowedDebuffsFirstLarc[i])
      continue;
    else if (larc_levels[i] == 1)
    {
      bool suc = tryBuyUpgrade(i, 2);
      assert(suc);
    }
  }
  return true;

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
  if (dif<0)
  {
    double f = A * dif * dif + B * dif;
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
void Game::runRace(int basicFiveStatusBonus, int basicPtBonus)
{
  double raceMultiply = 1 + 0.01 * saihou;
  int fiveStatusBonus = floor(raceMultiply * basicFiveStatusBonus);
  int ptBonus = floor(raceMultiply * basicPtBonus);
  addAllStatus(fiveStatusBonus);
  skillPt += basicPtBonus;
}

void Game::addStatusZuoyue(int idx, int value)
{
  value = int(value * larc_zuoyueStatusBonus);
  if (idx == 5)skillPt += value;
  else addStatus(idx, value);
}

void Game::addVitalZuoyue(int value)
{
  value = int(value * larc_zuoyueVitalBonus);
  addVital(value);
}


void Game::handleFriendOutgoing()
{
  assert(larc_zuoyueOutgoingUnlocked && larc_zuoyueOutgoingUsed < 5);
  if (larc_zuoyueOutgoingUsed == 0)
  {
    addVitalZuoyue(30);
    addMotivation(1);
    addStatusZuoyue(3, 5);
    addJiBan(17, 5);
  }
  else if (larc_zuoyueOutgoingUsed == 1)
  {
    addVitalZuoyue(25);
    addMotivation(1);
    addStatusZuoyue(2, 5);
    addStatusZuoyue(3, 5);
    addJiBan(17, 5);
  }
  else if (larc_zuoyueOutgoingUsed == 2)
  {
    addVitalZuoyue(35);
    addMotivation(1);
    addStatusZuoyue(3, 15);
    isPositiveThinking = true;
    addJiBan(17, 5);
  }
  else if (larc_zuoyueOutgoingUsed == 3)
  {
    addVitalZuoyue(25);
    addStatusZuoyue(3, 20);
    addJiBan(17, 5);
  }
  else if (larc_zuoyueOutgoingUsed == 4)//分为大成功和成功，取个平均
  {
    addVitalZuoyue(37);
    addStatusZuoyue(3, 7);
    addMotivation(1);
    addJiBan(17, 5);
  }
  else assert(false && "未知的出行");
  larc_zuoyueOutgoingUsed += 1;
}
void Game::handleFriendUnlock(std::mt19937_64& rand)
{
  //假设: 2选项2/3概率成功，成功时选下面，失败时选上面
  larc_zuoyueOutgoingUnlocked = true;
  larc_zuoyueOutgoingRefused = false;
  if (rand() % 3 > 0)
  {
    printEvents("友人外出解锁！2选项成功");
    addVitalZuoyue(35);
    addMotivation(1);
    addJiBan(17, 10);
  }
  else
  {
    printEvents("友人外出解锁！2选项失败，已选择1选项");
    addVitalZuoyue(15);
    addMotivation(1);
    addStatusZuoyue(3, 5);
    addJiBan(17, 5);
  }
}
void Game::handleFriendClickEvent(std::mt19937_64& rand, int atTrain)
{
  if (!larc_zuoyueFirstClick)
  {
    printEvents("第一次点友人");
    larc_zuoyueFirstClick = true;
    addJiBan(17, 10);
    addStatusZuoyue(3, 15);
    addStatusZuoyue(5, 5);
    addMotivation(1);
  }
  else
  {
    if (rand() % 5 < 3)return;//40%概率出事件，60%概率不出
    addJiBan(17, 7);
    addStatus(3, 3);
    skillPt += 3;
    bool isMotivationFull = motivation == 5;
    if (rand() % 10 == 0)
    {
      if (!isMotivationFull)
        printEvents("友人事件心情+1");
      addMotivation(1);//10%概率加心情
    }

    if (!larc_isAbroad)
    {
      printEvents("触发友人充电");
      //先在当前训练里面找人，然后从电量没满的人里面随机挑五个
      bool isNotFullCharge[15];

      for (int i = 0; i < 15; i++)
      {
        if (persons[i].larc_charge < 3)
        {
          isNotFullCharge[i] = true;
        }
        else
          isNotFullCharge[i] = false;
      }


      int toChargePersons[5] = { -1,-1,-1,-1,-1 };
      int count = 0;



      for (int i = 0; i < 5; i++)
      {
        int p = personDistribution[atTrain][i];
        if (p < 0)break;//没人
        int personType = persons[p].personType;

        if (personType == 2 || personType == 3)//可充电人头
        {
          if (persons[p].larc_charge < 3)
          {
            toChargePersons[count] = p;
            count++;
            isNotFullCharge[p] = false;
          }
        }
      }
      int fixedcount = count;//在当前训练，必充

      vector<int> randomChargePersons;//不在当前训练的非满格人头
      for (int i = 0; i < 15; i++)
      {
        if (isNotFullCharge[i])
        {
          randomChargePersons.push_back(i);
        }
      }
      if (randomChargePersons.size() >= 2)
      {
        std::shuffle(randomChargePersons.begin(), randomChargePersons.end(), rand);
      }
      int requiredRandomPersons = 5 - fixedcount;
      if (requiredRandomPersons > randomChargePersons.size())
        requiredRandomPersons = randomChargePersons.size();

      for (int i = 0; i < requiredRandomPersons; i++)
      {
        toChargePersons[count] = randomChargePersons[i];
        assert(count < 5);
        count += 1;
      }


      for (int i = 0; i < 5; i++)
      {
        if (toChargePersons[i] != -1)
          charge(toChargePersons[i], 1);
      }
    }
    else
    {
      printEvents("触发友人加适性pt");
      larc_shixingPt += 50;
    }
  }

}
void Game::calculateTrainingValueSingle(int trainType)
{
  //分配完了，接下来计算属性加值
  //failRate[trainType] = 

  //double failRateBasic = calculateFailureRate(trainType);//计算基础失败率

  for (int j = 0; j < 6; j++)
  {
    trainValue[trainType][j] = 0;
  }

  int personCount = 0;//卡+npc的人头数，不包括理事长和记者
  vector<CardTrainingEffect> effects;

  double vitalCostDrop = 1;

  //[智]真弓快车(id:30149)的固有是闪彩的训练60干劲加成，但是在把五个人头检查一遍之前并不知道闪没闪彩，因此检查完五个人头之后还需要额外对这张卡的参数进行处理
  int card30149place = -1;

  for (int i = 0; i < 5; i++)
  {
    int p = personDistribution[trainType][i];
    if (p < 0)break;//没人
    int personType = persons[p].personType;
    if (personType == 1 || personType == 2)//卡
    {
      personCount += 1; 
      CardTrainingEffect eff = cardParam[persons[p].cardIdInGame].getCardEffect(*this, trainType, persons[p].friendship, persons[p].cardRecord);
      effects.push_back(eff);
      if (eff.youQing > 0)
      {
        persons[p].isShining = true;
      }
      if (cardParam[persons[p].cardIdInGame].cardID / 10 == 30149)
        card30149place = effects.size() - 1;
    }
    else if (personType == 3)//npc
    {
      personCount += 1;
    }
  }

  trainShiningNum[trainType] = 0;
  double failRateMultiply = 1;
  for (int i = 0; i < effects.size(); ++i) {
    failRateMultiply *= (1 - 0.01 * effects[i].failRateDrop);//失败率下降
    vitalCostDrop *= (1 - 0.01 * effects[i].vitalCostDrop);//体力消耗下降
    if (effects[i].youQing > 0)trainShiningNum[trainType] += 1;//统计彩圈数
  }

  //[智]真弓快车(id:30149)的固有是闪彩的训练60干劲加成，但是在把五个人头检查一遍之前并不知道闪没闪彩，因此检查完五个人头之后还需要额外对这张卡的参数进行处理
  /*
  if (card30149place >= 0 && trainShiningNum[trainType] == 0)
  {
    effects[card30149place].ganJing -= 60;
  }*/

  failRate[trainType] = calculateFailureRate(trainType,failRateMultiply);

  if (larc_isAbroad)
    larc_shixingPtGainAbroad[trainType] = personCount * 20 + trainShiningNum[trainType] * 20 + (trainType == 4 ? 30 : 50);
  else
    larc_shixingPtGainAbroad[trainType] = 0;

  //先算下层数值
  int cardNum = effects.size();
  //1.人头数倍率，npc也算
  double cardNumMultiplying = 1 + 0.05 * personCount;
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
    growthRates[j] = 1.0 + 0.01 * fiveStatusBonus[j];
    //growthRates[j] = 1.0 + 0.01 * GameDatabase::AllUmas[umaId].fiveStatusBonus[j];

  //下层总数值
  int totalValueLower[6];
  for (int j = 0; j < 6; j++)
  {
    int v = int(totalMultiplying * basicValue[j] * growthRates[j]);//向下取整了
    if (v > 100)v = 100;
    totalValueLower[j] = v;
  }
  
  //7.上层
  double upperRate = 1;
  upperRate += 0.01 * larc_trainBonus;//期待度加成
  if (larc_isAbroad && larc_levels[GameConstants::UpgradeId50pEachTrain[trainType]] >= 3)
    upperRate += 0.5;//海外+50%
  if (larc_levels[8] >= 1)//倒数第二个升级，训练+5%
    upperRate += 0.05;
  if (larc_levels[7] >= 3 && trainShiningNum[trainType] > 0)//友情+20%
    upperRate *= 1.2;


  for (int j = 0; j < 6; j++)
  {
    int lower = totalValueLower[j];
    if (lower == 0)
    {
      trainValue[trainType][j] = 0;
      continue;
    }
    int total = int(double(lower + larc_staticBonus[j]) * upperRate);
    int upper = total - lower;
    if (upper > 100)upper = 100;
    trainValue[trainType][j] = lower + upper;
  } 
    


  //体力
  double vitalChange=GameConstants::TrainingBasicValue[trainType][trainLv][6];
  for (int i = 0; i < cardNum; i++)
    vitalChange += effects[i].vitalBonus;
  if (vitalChange < 0)//消耗体力时，检查是否购买体力-20%
  {
    vitalChange *= vitalCostDrop;
    if (larc_isAbroad && larc_levels[6] >= 3)//体力-20%
    vitalChange *= 0.8;
  }
  int vitalChangeInt = round(vitalChange);
  if (vitalChangeInt > maxVital - vital)vitalChangeInt = maxVital - vital;
  if (vitalChangeInt < - vital)vitalChangeInt = - vital;


  trainValue[trainType][6] = vitalChangeInt;
}
void Game::calculateSS()
{
  for (int i = 0; i < 5; i++)larc_ssValue[i] = 0;
  int linkn = 0;
  for (int i = 0; i < larc_ssPersonsCount; i++)
  {
    if (persons[larc_ssPersons[i]].larc_isLinkCard)
      linkn += 1;
  }
  int p = larc_ssPersonsCount;

  int totalValue =
    turn < 40 ?
    5 * p + (4 * p + 2 * linkn) * (0.8 + larc_supportPtAll * 6e-6) :
    5 * p + (5 * p + 2 * linkn) * (1.0 + larc_supportPtAll * 6e-6);//凑出来拟合的公式。误差挺大的但应该不太影响决策
  if (larc_isSSS)totalValue += 75;

  totalValue -= (4 * p + 2 * linkn);//4 * p + 2 * linkn是按照人头属性分配
  //剩下的平均分配
  int div5 = totalValue / 5;
  for (int i = 0; i < 5; i++)larc_ssValue[i] = div5;
  for (int i = 0; i < totalValue - div5 * 5; i++)larc_ssValue[i] += 1;

  //人头属性
  for (int i = 0; i < larc_ssPersonsCount; i++)
  {
    auto& p = persons[larc_ssPersons[i]];
    larc_ssValue[p.larc_statusType] += (p.larc_isLinkCard ? 6 : 4);
  }

  //ss的失败率，瞎猜的。
  //目前已知20体及以上不会失败（双圈）,10~19是单圈，1~9是三角，0是叉
  //但是具体多少概率没测过，我觉得也不重要，因为没体力的时候肯定先休息/外出再ss，否则就算ss成功了下一个回合也没法训练
  if(larc_isSSS)larc_ssFailRate = 0;
  else if (vital < 1)larc_ssFailRate = 80;
  else if (vital < 10)larc_ssFailRate = 50;
  else if (vital < 20)larc_ssFailRate = 30;
  else larc_ssFailRate = 0;
}
bool Game::applyTraining(std::mt19937_64& rand, Action action)
{
  uaf_lastTurnNotTrain = false;
  if (isRacing)
  {
    assert(false && "所有剧本比赛都在checkEventAfterTrain()里处理，不能applyTraining");
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
      handleFriendOutgoing();
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
      xiangtanAndRecalculate(action.xiangtanType);

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
      
      //buff次数-1
      for (int color = 0; color < 3; color++)
      {
        if (uaf_buffNum[color] > 0)uaf_buffNum[color] -= 1;
      }

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
  }
  else
  {
    printEvents("未知的训练项目");
    return false;
  }
  return true;
}


bool Game::isLegal(Action action) const
{
  if (isRacing)
  {
    assert(false && "凯旋门所有剧本比赛都在checkEventAfterTrain()里处理，不能applyTraining");
    return false;//凯旋门所有剧本比赛都在checkEventAfterTrain()里处理（相当于比赛回合直接跳过），不在这个函数
  }

  if (action.buy50p || action.buyFriend20 || action.buyPt10 || action.buyVital20)
  {
    if (!(action.train <= 4 && action.train >= 0 && larc_isAbroad))
      return false;
  }

  if (action.train == 6)//休息
  {
    return true;
  }
  else if (action.train == 9)//比赛
  {
    if (turn <= 12 || larc_isAbroad)
    {
      return false;
    }
    return true;
  }
  else if (action.train == 8)//普通外出
  {
    if (larc_isAbroad)
    {
      return false;
    }
    return true;
  }
  else if (action.train == 7)//友人外出
  {
    if (larc_isAbroad)
    {
      return false;
    }
    if (!larc_zuoyueOutgoingUnlocked || larc_zuoyueOutgoingUsed == 5)
    {
      return false;
    }
    return true;
  }
  else if (action.train == 5)//ss match
  {
    if (larc_isAbroad)
    {
      return false;
    }
    if (larc_ssPersonsCount == 0)
    {
      return false;
    }
    return true;
  }
  else if (action.train <= 4 && action.train >= 0)//常规训练
  {
    //先用适性pt买加成
    if (larc_isAbroad)
    {
      int remainPt = larc_shixingPt;
      if (action.buy50p)
      {
        int upgradeIdx = GameConstants::UpgradeId50pEachTrain[action.train];
        int cost = buyUpgradeCost(upgradeIdx, 3);
        if (cost <= 0)return false;
        if (cost > remainPt)return false;
        remainPt -= cost;
      }
      if (action.buyFriend20)
      {
        assert(false && "买友情20%已经改成全自动的了");
        return false;
      }
      if (action.buyPt10)
      {
        if (turn >= 41)
          return false;
        int cost = buyUpgradeCost(5, 3);
        if (cost <= 0)return false;
        if (cost > remainPt)return false;
        remainPt -= cost;
      }
      if (action.buyVital20)
      {
        if (turn <= 59)
          return false;
        int cost = buyUpgradeCost(6, 3);
        if (cost <= 0)return false;
        if (cost > remainPt)return false;
        remainPt -= cost;
      }
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
  float scorePtRate = isQieZhe ? GameConstants::ScorePtRateQieZhe : GameConstants::ScorePtRate;
  return scorePtRate * skillPt + skillScore;
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
  if(larc_isAbroad)
    level = 5;
  else
  {
    assert(trainLevelCount[item] <= 16, "训练等级计数超过16");
    level = trainLevelCount[item] / 4;
    if (level > 4)level = 4;
  }
  return level;
}

double Game::sssProb(int ssWinSinceLastSSS) const
{
  return ssWinSinceLastSSS >= 8 ? 1.0 : 0.12 + 0.056 * ssWinSinceLastSSS;
}




void Game::checkEventAfterTrain(std::mt19937_64& rand)
{
  assert(stageInTurn == 2 || isRacing);
  stageInTurn = 0;



  checkFixedEvents(rand);

  checkRandomEvents(rand);


  //回合数+1
  turn++;
  if (turn < TOTAL_TURN)
  {
    isRacing = GameConstants::LArcIsRace[turn];
    larc_isAbroad = (turn >= 36 && turn <= 42) || (turn >= 60 && turn <= 67);
    if (isRacing)
      checkEventAfterTrain(rand);//跳过这个回合
  }
  else
  {
    printEvents("育成结束!");
    printEvents("你的得分是：" + to_string(finalScore()));
  }

}
void Game::uaf_runCompetition(int n)//第n次uaf大会
{
  uaf_xiangtanRemain = 3;

  if (入赏 突破)// <12win
  {
    //依次是3，5，10，15，20
    int statusGain = n * 5;
    if (statusGain == 0)statusGain = 3;
    if (isLinkUma)statusGain += 3;
    addAllStatus(statusGain);
    skillPt += 30 + 10 * n;
  }
  else if (yousheng)// >=12win
  {
    addJiBan(6, 5);//理事长羁绊
    int statusGain = 5 + n * 5;
    if (isLinkUma)statusGain += 3;
    addAllStatus(statusGain);
    skillPt += 40 + 10 * n;
  }
  if (allwin)
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
    runRace(3, 45);
    addJiBan(6, 4);
  }

  if (turn == 11)//相谈刷新
  {
    uaf_xiangtanRemain = 3;
  }
  else if (turn == 23)//第一年年底
  {
    uaf_runCompetition(0);
    年底事件
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
    年底事件
      printEvents("uaf大会3结束");
  }
  else if (turn == 48)//抽奖
  {
    todo;
  }
  else if (turn == 49)
  {
    固有+1
  }
  else if (turn == 53)//第三年继承&larc3
  {
    runRace(7, 30);
    larc_shixingPt += 80;
    skillScore += 170;//固有技能等级+1

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

    if(理事长羁绊60)
      固有+1，心情+1
    else
      体力-5，pt+25
  }
  else if (turn == 59)//uaf4
  {
    uaf_runCompetition(3);
    printEvents("uaf大会4结束");
  }
  else if (turn == 70)
  {
    固有 + 1;
  }
  else if (turn == 71)//uaf5
  {
    uaf_runCompetition(4);
    printEvents("uaf大会5结束");
  }
  else if (turn == 73)//ura1
  {
    runRace(10, 40);
    printEvents("ura1结束");
  }
  else if (turn == 75)//ura2
  {
    runRace(10, 60);
    printEvents("ura2结束");
  }
  else if (turn == 77)//ura3，游戏结束
  {
    runRace(10, 80);

    //记者
    if (persons[16].friendship >= 100)
    {
      addAllStatus(5);
      skillPt += 20;
    }
    else if (persons[16].friendship >= 80)
    {
      addAllStatus(3);
      skillPt += 10;
    }
    else
    {
      skillPt += 5;
    }

    if (75win)
    {
      addAllStatus(55);
      skillPt += 140;
    }
    else if (有一场没win)
    {
      addAllStatus(30);
      skillPt += ？
    }
    else if (有一场没总和优胜)
    {
      addAllStatus(20);
      skillPt += 70;
    }

    if (level合计 >= 1200)
    {
      skillPt += 60;//三折金上位
    }
    else
    {
      skillPt += 20;//一折金上位
    }


    //友人卡事件
    if (larc_zuoyueOutgoingUsed == 5)//出行走完了
    {
      addStatusZuoyue(2, 15);
      addStatusZuoyue(3, 25);
      addStatusZuoyue(5, 30);
    }
    else if (larc_zuoyueOutgoingUnlocked)
    {
      addStatusZuoyue(2, 15);
      addStatusZuoyue(3, 18);
      addStatusZuoyue(5, 20);
    }
  

    addAllStatus(5);
    skillPt += 30;

    printEvents("ura3结束，游戏结算");
  }
}

void Game::checkSupportPtEvents(int oldSupportPt, int newSupportPt)
{
  int bound;
  bound = 20 * 1700 - 85;//期待度计算是SupportPt/170四舍五入，所以20.0%期待度对应的是20 * 1700 - 85
  if (oldSupportPt < bound && newSupportPt >= bound)
  {
    printEvents("期待度达到20%");
    addAllStatus(2);
    for (int i = 0; i < 5; i++)
      addTrainingLevelCount(i, 4);
  }
  bound = 40 * 1700 - 85;//期待度计算是SupportPt/170四舍五入，所以20.0%期待度对应的是20 * 1700 - 85
  if (oldSupportPt < bound && newSupportPt >= bound)
  {
    printEvents("期待度达到40%");
    addAllStatus(3);
  }
  bound = 60 * 1700 - 85;//期待度计算是SupportPt/170四舍五入，所以20.0%期待度对应的是20 * 1700 - 85
  if (oldSupportPt < bound && newSupportPt >= bound)
  {
    printEvents("期待度达到60%");
    addAllStatus(4);
    for (int i = 0; i < 5; i++)
      addTrainingLevelCount(i, 4);
  }
  bound = 80 * 1700 - 85;//期待度计算是SupportPt/170四舍五入，所以20.0%期待度对应的是20 * 1700 - 85
  if (oldSupportPt < bound && newSupportPt >= bound)
  {
    printEvents("期待度达到80%");
    addAllStatus(5);
  }
  bound = 100 * 1700 - 85;//期待度计算是SupportPt/170四舍五入，所以20.0%期待度对应的是20 * 1700 - 85
  if (oldSupportPt < bound && newSupportPt >= bound)
  {
    printEvents("期待度达到100%");
    addAllStatus(5);
    for (int i = 0; i < 5; i++)
      addTrainingLevelCount(i, 4);
  }

}

void Game::checkRandomEvents(std::mt19937_64& rand)
{
  if (larc_isAbroad)
    return;//远征期间不会发生各种随机事件

  //友人会不会解锁出行
  if (larc_zuoyueFirstClick && (!larc_zuoyueOutgoingRefused) && (!larc_zuoyueOutgoingUnlocked))
  {
    double unlockOutgoingProb = persons[17].friendship >= 60 ?
      GameConstants::FriendUnlockOutgoingProbEveryTurnHighFriendship :
      GameConstants::FriendUnlockOutgoingProbEveryTurnLowFriendship;
    if (randBool(rand, unlockOutgoingProb))//启动
    {
      handleFriendUnlock(rand);
    }
  }

  //模拟各种随机事件

  //支援卡连续事件，随机给一个卡加5羁绊
  int nonAbroadTurns = turn < 40 ? turn : turn - 7;
  double p = 0.4;
  if (randBool(rand, p))
  {
    int card = rand() % normalCardCount;
    addJiBan(card, 5);
    //addAllStatus(4);
    addStatus(rand() % 5, eventStrength);
    skillPt += eventStrength;
    printEvents("模拟支援卡随机事件：" + cardParam[persons[card].cardIdInGame].cardName + " 的羁绊+5，pt和随机属性+" + to_string(eventStrength));

    //支援卡一般是前几个事件加心情
    if (randBool(rand, 0.2 * (1.0 - turn * 1.0 / TOTAL_TURN)))
    {
      addMotivation(1);
      printEvents("模拟支援卡随机事件：心情+1");
    }
    if (randBool(rand, 0.3))
    {
      addVital(10);
      printEvents("模拟支援卡随机事件：体力+10");
    }
    else if (randBool(rand, 0.07))
    {
      addVital(-10);
      printEvents("模拟支援卡随机事件：体力-10");
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
  if (randBool(rand, 0.04))
  {
    addMotivation(-1);
    printEvents("模拟随机事件：\033[0m\033[33m心情-1\033[0m\033[32m");
  }

}

void Game::applyTrainingAndNextTurn(std::mt19937_64& rand, Action action)
{
  assert(stageInTurn == 1);
  assert(turn < TOTAL_TURN && "Game::applyTrainingAndNextTurn游戏已结束");
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

// 根据PersonDistribution cardType和Person.friendship确定训练彩圈数量
// 因为相关数据分散在各处，所以不在getCardEffect函数中时，只能在Game里以较大的开销判定
// 不能使用由getCardEffect维护的trainShiningNum成员，因为getCardEffect自己需要调用该方法。
bool Game::trainShiningCount(int train) const
{
    int count = 0;
    if (train > 0 && train <= 5)
    {
        for (int i = 0; i < 5; ++i)
        { 
            int which = personDistribution[train][i];
            if (which >= 0 && which <= 5 && cardParam[which].cardType == train && persons[which].friendship >= 80)
                ++count;
        }
    }
    return count;
}
