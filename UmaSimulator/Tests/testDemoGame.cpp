#include <iostream>
#include <iomanip> 
#include <sstream>
#include <fstream>
#include <cassert>
#include <thread>  // for std::this_thread::sleep_for
#include <chrono>  // for std::chrono::seconds

#include "../Game/Game.h"
#include "../GameDatabase/GameDatabase.h"
#include "../GameDatabase/GameConfig.h"
#include "../Search/Search.h"
#include "../External/utils.h"
#include "tests.h"

#include "windows.h"
#include <filesystem>
#include <cstdlib>
using namespace std;
using json = nlohmann::json;

template <typename T, std::size_t N>
std::size_t findMaxIndex(const T(&arr)[N]) {
	return std::distance(arr, std::max_element(arr, arr + N));
}

void print_luck0(int luck)
{
	int u = 0;//�°�ƽ��������Լ500����Ϊ���չ�����Ƚ�һ��Ϳ�û���Ƶ��ˣ����������ai��ֻ�ƫ�ߣ��������0��
	int sigma = 1500;
	string color = "";
	if (luck > 20000) u = 32000;//�õ�Ŀ�ƽ��ֵԼΪue6

	if (!GameConfig::noColor)
	{
		if (luck > u + sigma * 1.0)
			color = "\033[32m"; // 2 3 1
		else if (luck > u - sigma * 1.0)
			color = "\033[33m";
		else
			color = "\033[31m";
		cout << color << luck << "\033[0m";
	}
	else
		cout << luck;
}

void main_testDemoGame()
{
	//const double radicalFactor = 5;//������
	//const int threadNum = 16; //�߳���
	 // const int searchN = 12288; //ÿ��ѡ������ؿ���ģ��ľ���

	//������Ϊk��ģ��n��ʱ����׼��ԼΪsqrt(1+k^2/(2k+1))*1200/(sqrt(n))
	//��׼�����30ʱ������Ӱ���ж�׼ȷ��


	random_device rd;
	auto rand = mt19937_64(rd());

	int lastTurn = -1;
	int scoreFirstTurn = 0;   // ��һ�غϷ���
	int scoreLastTurn = 0;   // ��һ�غϷ���

	// ��鹤��Ŀ¼
	wchar_t buf[10240];
	GetModuleFileNameW(0, buf, 10240);
	filesystem::path exeDir = filesystem::path(buf).parent_path();
	filesystem::current_path(exeDir);
	//std::cout << "��ǰ����Ŀ¼��" << filesystem::current_path() << endl;
	cout << "��ǰ����Ŀ¼��" << exeDir << endl;
	GameConfig::load("./aiConfig.json");
	GameDatabase::loadTranslation("./db/text_data.json");
	GameDatabase::loadUmas("./db/umaDB.json");
	//GameDatabase::loadCards("./db/card"); // ���벢����ʹ���ֶ�֧Ԯ������
	GameDatabase::loadDBCards("./db/cardDB.json"); //cardDB�����Ѿ���������

	string currentGameStagePath = string(getenv("LOCALAPPDATA")) + "/UmamusumeResponseAnalyzer/GameData/thisTurn.json";
	//string currentGameStagePath = "./gameData/thisTurn.json";

	const int threadNum = 8;
	const double radicalFactor = 5;//������
	const int searchN = 1024;
	const int searchDepth = 1;

	SearchParam searchParam = { searchN,searchDepth,radicalFactor };

	int batchsize = 128;
#if USE_BACKEND == BACKEND_LIBTORCH
	const string modelpath =  "../training/example/model_traced.pt";
#elif USE_BACKEND == BACKEND_NONE
	const string modelpath = "";
#else
	const string modelpath = "../training/example/model.txt";
#endif

	Model* modelptr = NULL;
	Model model(modelpath, batchsize);
	if (modelpath != "")
	{
		modelptr = &model;
	}

	Search search(modelptr, batchsize, threadNum, searchParam);

	while (true)
	{

		int umaId = 106501;//̫����15��15���ӳ�
		int umaStars = 5;
		//int cards[6] = { 301604,301344,301614,300194,300114,301074 };//���ˣ��߷壬��ӥ��������������˾��
		int cards[6] = { 301604,301724,301614,301304,300114,300374 };//���ˣ�������������ӥ������˹�������񣬸��ʵ�

		int zhongmaBlue[5] = { 18,0,0,0,0 };
		int zhongmaBonus[6] = { 10,10,30,0,10,70 };
		bool allowedDebuffs[9] = { false, false, false, false, false, false, true, false, false };//�ڶ�����Բ����ڼ���debuff������������������߸���ǿ����
		Game game;

		game.newGame(rand, false, umaId, umaStars, cards, zhongmaBlue, zhongmaBonus);

		while (!game.isEnd())
		{
			string jsonStr;



			game.eventStrength = GameConfig::eventStrength;

			lastTurn = game.turn;
			//if (game.venusIsWisdomActive)
			/*
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(300));
				continue;
			}
			*/
			if (game.turn == 0)//��һ�غϣ���������ai�ĵ�һ�غ�
			{
				scoreFirstTurn = 0;
				scoreLastTurn = 0;
			}

			game.print();
			cout << endl;
			auto printPolicy = [](float p)
				{
					cout << fixed << setprecision(1);
					if (!GameConfig::noColor)
					{
						if (p >= 0.3)cout << "\033[33m";
						//else if (p >= 0.1)cout << "\033[32m";
						else cout << "\033[36m";
					}
					cout << p * 100 << "% ";
					if (!GameConfig::noColor)cout << "\033[0m";
				};

			auto printValue = [](int which, double p, double ref)
				{
					string prefix[] = { "��:", "��:", "��:", "��:", "��:", "| SS: ", "| ��Ϣ: ", "����: ", "��ͨ���: ", "����: " };
					if (p < -50000)
					{
						cout << prefix[which] << "--- ";
						return;
					}
					cout << fixed << setprecision(0);
					if (!GameConfig::noColor)
					{
						if (ref - p < 20) cout << "\033[41m\033[1;33m*";
						else if (ref - p < 100) cout << "\033[1;32m";
						else cout << "\033[33m";
					}
					cout << prefix[which] << setw(3) << p;
					if (!GameConfig::noColor)cout << "\033[0m";
					cout << " ";
				};

			//search.runSearch(game, GameConfig::searchN, TOTAL_TURN, 0, rand);
			assert(game.turn < TOTAL_TURN - 1 && !game.isRacing);


			game.playerPrint = false;
			Action bestAction = search.runSearch(game, rand);
			game.playerPrint = true;


			assert(false && "todo");
			double maxMean = -1e7;
			double maxValue = -1e7;
			/*
			for (int i = 0; i < Search::buyBuffChoiceNum(game.turn); i++)
				for (int j = 0; j < 10; j++)
				{
					auto v = search.allChoicesValue[i][j];
					if (v.value > maxValue)
						maxValue = v.value;
					if (v.scoreMean > maxMean)
						maxMean = v.scoreMean;
				}

			//��Ϣ������������ߵ��Ǹ������������Ϊ��ʾ�ο�
			double restValue = search.allChoicesValue[0][6].value;
			if (search.allChoicesValue[0][7].value > restValue)
				restValue = search.allChoicesValue[0][7].value;
			if (search.allChoicesValue[0][8].value > restValue)
				restValue = search.allChoicesValue[0][8].value;
				*/

			wstring strToSendURA = L"larc";
			strToSendURA += L" " + to_wstring(game.turn) + L" " + to_wstring(maxMean) + L" " + to_wstring(scoreFirstTurn) + L" " + to_wstring(scoreLastTurn) + L" " + to_wstring(maxValue);
			if (game.turn == 0 || scoreFirstTurn == 0)
			{
				cout << "����Ԥ��: ƽ��\033[1;32m" << int(maxMean) << "\033[0m" << "���ֹ�\033[1;36m+" << int(maxValue - maxMean) << "\033[0m" << endl;
				scoreFirstTurn = maxMean;
			}
			else
			{
				cout << "���� | ���֣�";
				print_luck0(maxMean - scoreFirstTurn);
				cout << " | ���غϣ�" << maxMean - scoreLastTurn
					<< " | ����Ԥ��: \033[1;32m" << maxMean << "\033[0m"
					<< "���ֹ�\033[1;36m+" << int(maxValue - maxMean) << "\033[0m��" << endl;

			}
			cout.flush();
			scoreLastTurn = maxMean;
			assert(false && "todo");
			/*
			for (int i = 0; i < Search::buyBuffChoiceNum(game.turn); i++)
			{
				if (Search::buyBuffChoiceNum(game.turn) > 1 && i == 0)
					cout << "����:              ";
				if (i == 1)
					cout << "��+50%:            ";
				if (i == 2 && game.turn < 50)
					cout << "��pt+10:           ";
				if (i == 2 && game.turn >= 50)
					cout << "������-20%:        ";
				if (i == 3 && game.turn < 50)
					cout << "��+50%��pt+10:     ";
				if (i == 3 && game.turn >= 50)
					cout << "��+50%������-20%:  ";
				//cout << "ѵ��: ";
				for (int j = 0; j < 10; j++)
				{
					double value = search.allChoicesValue[i][j].value;
					strToSendURA += L" " + to_wstring(value);
					printValue(j, value - restValue, maxValue - restValue);
				}
				cout << endl;
			}
			*/


			game.applyTrainingAndNextTurn(rand, bestAction);
		} // while
		game.printFinalStats();
		if (game.finalScore() < 26000)
			break;
	}
}