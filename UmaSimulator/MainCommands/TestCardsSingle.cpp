//����ѵ������ֵ�㷨
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <cassert>
#include <thread>
#include <atomic>
#include <mutex>
#include <cmath>
#include "../Game/Game.h"
#include "../NeuralNet/Evaluator.h"
#include "../Search/Search.h"
#include "../External/termcolor.hpp"

#include "../GameDatabase/GameDatabase.h"
#include "../GameDatabase/GameConfig.h"

using namespace std;

struct testResult
{
  string cardName;
  int cardId;
  ModelOutputValueV1 result;
};

void main_testCardsSingle()
{
  //int toTestCardType = 0;//����������
  double radicalFactor = 10;//������
  int searchN = 20000;
#if USE_BACKEND == BACKEND_LIBTORCH
  const string modelpath = "./db/model_traced.pt";
  const int threadNum = 16;
  int batchsize = 1024;
#elif USE_BACKEND == BACKEND_NONE
  const string modelpath = "";
  const int threadNum = 8;
  int batchsize = 1;
#elif USE_BACKEND == BACKEND_ONNX
  const string modelpath = "./db/model_uaf.onnx";
  const int threadNum = 4;
  int batchsize = 1024;
#else
  const string modelpath = "./db/model.txt";
  const int threadNum = 4;
  int batchsize = 1024;
#endif


  GameDatabase::loadUmas("./db/umaDB.json");
  //GameDatabase::loadCards("../db/card");
  GameDatabase::loadDBCards("./db/cardDB.json");

  random_device rd;
  auto rand = mt19937_64(rd());

  int umaId = 0;
  vector<int> cards;

  for (int toTestCardType = 0; toTestCardType < 5; toTestCardType++)
  {
    if (toTestCardType == 0)
    {
      //2��1��1��1��
      umaId = 102401;//���ڣ�20��10���ӳ�
      cards = { 301884,301724,301614,301874,301734,0 };//���ˣ�������������ӥ�����޽���������
    }

    else if (toTestCardType == 1)
    {
      //2��1��1��1��
      umaId = 102401;//���ڣ�20��10���ӳ�
      cards = { 301884,301724,301614,301784,301874,0 };//���ˣ�������������ӥ���ٴ󴸣����޽�
    }
    else if (toTestCardType == 2)
    {
      //1��1��1��1��1��
      umaId = 101101;//���Ϸɣ�20��10���ӳ�
      cards = { 301884,301724,301614,301874,301734,0 };//���ˣ�������������ӥ���ٴ󴸣����޽���������
    }
    else if (toTestCardType == 3)
    {
      //2��1��1��1��
      umaId = 102401;//���ڣ�20��10���ӳ�
      cards = { 301884,301724,301614,301784,301734,0 };//���ˣ�������������ӥ���ٴ󴸣�������
    }
    else if (toTestCardType == 4)
    {
      //2��1��1��1��
      umaId = 102401;//���ڣ�20��10���ӳ�
      cards = { 301884,301614,301784,301874,301734,0 };//���ˣ�����ӥ���ٴ󴸣����޽���������
    }

    int umaStars = 5;
    int zhongmaBlue[5] = { 18,0,0,0,0 };
    int zhongmaBonus[6] = { 10,10,30,0,10,70 };

    {
      cout << "�⿨������" << endl;
      cout << "���" << GameDatabase::AllUmas[umaId].name << "(" << umaId << ") ";
      for (int i = 0; i < 5; i++)
      {
        int gr = GameDatabase::AllUmas[umaId].fiveStatusBonus[i];
        if (gr > 0)
          cout << Action::trainingName[i] << "+" << gr << "% ";
      }
      cout << endl;
      cout << "��������";
      for (int i = 0; i < 5; i++)
      {
        int t = cards[i];
        string cardName = GameDatabase::AllCards[t].cardName;
        cardName = cardName + "(" + to_string(t / 10) + ")";
        cout << cardName << " ";
      }
      cout << endl;
    }

    SearchParam searchParam(searchN, radicalFactor);

    Model* modelptr = NULL;
    Model model(modelpath, batchsize);
    if (modelpath != "")
    {
      modelptr = &model;
    }

    Model::printBackendInfo();

    Search search(modelptr, batchsize, threadNum, searchParam);

    vector<testResult> allResult;
    for (int cardId = 39999; cardId >= 0; cardId--)
    {
      int cardIdLv50 = cardId * 10 + 4;
      if (!GameDatabase::AllCards.count(cardIdLv50))
        continue;
      auto& card = GameDatabase::AllCards[cardIdLv50];
      if (card.cardType != toTestCardType)
        continue;

      string cardName = card.cardName;

      if (cardId >= 30000)
      {
        cardName = "\033[1;37m" + cardName + "\033[0m";
      }
      else if (cardId >= 20000)
      {
        cardName = "\033[1;33m" + cardName + "SR\033[0m";
      }
      else if (cardId >= 10000)
      {
        cardName = "\033[1;36m" + cardName + "R\033[0m";
      }

      cardName = cardName + "(" + to_string(cardId) + ")";

      //û���ǹ��еĿ�
      if (card.uniqueEffectType == 5 ||
        card.uniqueEffectType == 12 ||
        card.uniqueEffectType == 15
        )
        cardName = "\033[41m\033[30m**\033[0m" + cardName;

      //������Ϊȫ���Ŀ�
      else if (card.uniqueEffectType == 6
        || cardId == 30155
        || cardId == 30171
        )
        cardName = "\033[43m\033[30m#\033[0m" + cardName;
      else
        cardName = "\033[43m\033[30m\033[0m" + cardName;
      cout << setw(55) << cardName << "��";

      cards[5] = cardIdLv50;
      Game game;
      game.newGame(rand, false, umaId, umaStars, cards.data(), zhongmaBlue, zhongmaBonus);
      //game.addAllStatus(initialStatusBonus);
      auto value = search.evaluateNewGame(game, rand);

      cout << "���ַ���=\033[1;32m" << int(value.value) << "\033[0m  ƽ������=\033[1;32m" << int(value.scoreMean) << "\033[0m" << endl;

      testResult tr;
      tr.cardId = cardId;
      tr.cardName = cardName;
      tr.result = value;

      allResult.push_back(tr);
    }

    cout << endl;

    {
      cout << "�⿨������" << endl;
      cout << "���" << GameDatabase::AllUmas[umaId].name << "(" << umaId << ") ";
      for (int i = 0; i < 5; i++)
      {
        int gr = GameDatabase::AllUmas[umaId].fiveStatusBonus[i];
        if (gr > 0)
          cout << Action::trainingName[i] << "+" << gr << "% ";
      }
      cout << endl;
      cout << "��������";
      for (int i = 0; i < 5; i++)
      {
        int t = cards[i];
        string cardName = GameDatabase::AllCards[t].cardName;
        cardName = cardName + "(" + to_string(t / 10) + ")";
        cout << cardName << " ";
      }
      cout << endl;
    }

    cout << "\033[41m\033[30m**\033[0m��û���ǹ��У�\033[43m\033[30m#\033[0m����Ϊ����ȫ��" << endl;
    cout << "�����ַ����Ӵ�С����" << endl;
    cout << "-------------------------------------------------------------------------" << endl;
    std::sort(allResult.begin(), allResult.end(), [](const testResult& a, const testResult& b) {
      return a.result.value > b.result.value;
      });

    for (int i = 0; i < allResult.size(); i++)
    {
      auto tr = allResult[i];
      cout << setw(55) << tr.cardName << "��";
      cout << "���ַ���=\033[1;32m" << int(tr.result.value) << "\033[0m  ƽ������=\033[1;32m" << int(tr.result.scoreMean) << "\033[0m" << endl;

    }
    //����������Ҫ������selfplay���ѡ��
    string resultname = "testcard_" + to_string(toTestCardType) + ".txt";
    ofstream fs(resultname);
    fs << allResult.size() << endl;
    for (int i = 0; i < allResult.size(); i++)
    {
      auto tr = allResult[i];
      fs << tr.cardId << " " << tr.result.value << " " << tr.result.scoreMean << endl;
    }
    fs.close();


    cout << endl;

    {
      cout << "�⿨������" << endl;
      cout << "���" << GameDatabase::AllUmas[umaId].name << "(" << umaId << ") ";
      for (int i = 0; i < 5; i++)
      {
        int gr = GameDatabase::AllUmas[umaId].fiveStatusBonus[i];
        if (gr > 0)
          cout << Action::trainingName[i] << "+" << gr << "% ";
      }
      cout << endl;
      cout << "��������";
      for (int i = 0; i < 5; i++)
      {
        int t = cards[i];
        string cardName = GameDatabase::AllCards[t].cardName;
        cardName = cardName + "(" + to_string(t / 10) + ")";
        cout << cardName << " ";
      }
      cout << endl;
    }

    cout << "\033[41m\033[30m**\033[0m��û���ǹ��У�\033[43m\033[30m#\033[0m����Ϊ����ȫ��" << endl;
    cout << "��ƽ�������Ӵ�С����" << endl;
    cout << "-------------------------------------------------------------------------" << endl;
    std::sort(allResult.begin(), allResult.end(), [](const testResult& a, const testResult& b) {
      return a.result.scoreMean > b.result.scoreMean;
      });

    for (int i = 0; i < allResult.size(); i++)
    {
      auto tr = allResult[i];
      cout << setw(55) << tr.cardName << "��";
      cout << "���ַ���=\033[1;32m" << int(tr.result.value) << "\033[0m  ƽ������=\033[1;32m" << int(tr.result.scoreMean) << "\033[0m" << endl;

    }
  }
}