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
	int u = 0;//新版平均运气大约500，但为了照顾种马比较一般和卡没满破的人（这两种情况ai打分会偏高），就设成0了
	int sigma = 1500;
	string color = "";
	if (luck > 20000) u = 32000;//好点的卡平均值约为ue6

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
	//const double radicalFactor = 5;//激进度
	//const int threadNum = 16; //线程数
	 // const int searchN = 12288; //每个选项的蒙特卡洛模拟的局数

	//激进度为k，模拟n局时，标准差约为sqrt(1+k^2/(2k+1))*1200/(sqrt(n))
	//标准差大于30时会严重影响判断准确度


	random_device rd;
	auto rand = mt19937_64(rd());

	int lastTurn = -1;
	int scoreFirstTurn = 0;   // 第一回合分数
	int scoreLastTurn = 0;   // 上一回合分数

	// 检查工作目录
	wchar_t buf[10240];
	GetModuleFileNameW(0, buf, 10240);
	filesystem::path exeDir = filesystem::path(buf).parent_path();
	filesystem::current_path(exeDir);
	//std::cout << "当前工作目录：" << filesystem::current_path() << endl;
	cout << "当前程序目录：" << exeDir << endl;
	GameConfig::load("./aiConfig.json");
	GameDatabase::loadTranslation("./db/text_data.json");
	GameDatabase::loadUmas("./db/umaDB.json");
	//GameDatabase::loadCards("./db/card"); // 载入并优先使用手动支援卡数据
	GameDatabase::loadDBCards("./db/cardDB.json"); //cardDB数据已经很完善了

	string currentGameStagePath = string(getenv("LOCALAPPDATA")) + "/UmamusumeResponseAnalyzer/GameData/thisTurn.json";
	//string currentGameStagePath = "./gameData/thisTurn.json";

	const int threadNum = 8;
	const double radicalFactor = 5;//激进度
	const int searchN = 1024;
	const int searchDepth = 1;

	SearchParam searchParam(searchN, radicalFactor);
	searchParam.maxDepth = searchDepth;

	int batchsize = 128;
#if USE_BACKEND == BACKEND_LIBTORCH
	const string modelpath = "../training/example/model_traced.pt";
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

		int umaId = 106501;//太阳神，15速15力加成
		int umaStars = 5;
		//int cards[6] = { 301604,301344,301614,300194,300114,301074 };//友人，高峰，神鹰，乌拉拉，风神，司机
		int cards[6] = { 301604,301724,301614,301304,300114,300374 };//友人，智麦昆，速神鹰，根凯斯，根风神，根皇帝

		int zhongmaBlue[5] = { 18,0,0,0,0 };
		int zhongmaBonus[6] = { 10,10,30,0,10,70 };
		bool allowedDebuffs[9] = { false, false, false, false, false, false, true, false, false };//第二年可以不消第几个debuff。第五个是智力，第七个是强心脏
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
			if (game.turn == 0)//第一回合，或者重启ai的第一回合
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
					string prefix[] = { "速:", "耐:", "力:", "根:", "智:", "| SS: ", "| 休息: ", "友人: ", "普通外出: ", "比赛: " };
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

			//休息和外出里面分最高的那个。这个数字作为显示参考
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
				cout << "评分预测: 平均\033[1;32m" << int(maxMean) << "\033[0m" << "，乐观\033[1;36m+" << int(maxValue - maxMean) << "\033[0m" << endl;
				scoreFirstTurn = maxMean;
			}
			else
			{
				cout << "运气 | 本局：";
				print_luck0(maxMean - scoreFirstTurn);
				cout << " | 本回合：" << maxMean - scoreLastTurn
					<< " | 评分预测: \033[1;32m" << maxMean << "\033[0m"
					<< "（乐观\033[1;36m+" << int(maxValue - maxMean) << "\033[0m）" << endl;

			}
			cout.flush();
			scoreLastTurn = maxMean;
			assert(false && "todo");
			/*
			for (int i = 0; i < Search::buyBuffChoiceNum(game.turn); i++)
			{
				if (Search::buyBuffChoiceNum(game.turn) > 1 && i == 0)
					cout << "不买:              ";
				if (i == 1)
					cout << "买+50%:            ";
				if (i == 2 && game.turn < 50)
					cout << "买pt+10:           ";
				if (i == 2 && game.turn >= 50)
					cout << "买体力-20%:        ";
				if (i == 3 && game.turn < 50)
					cout << "买+50%与pt+10:     ";
				if (i == 3 && game.turn >= 50)
					cout << "买+50%与体力-20%:  ";
				//cout << "训练: ";
				for (int j = 0; j < 10; j++)
				{
					double value = search.allChoicesValue[i][j].value;
					strToSendURA += L" " + to_wstring(value);
					printValue(j, value - restValue, maxValue - restValue);
				}
				cout << endl;
			}
			*/


			game.applyAction(rand, bestAction);
		} // while
		game.printFinalStats();
		if (game.finalScore() < 26000)
			break;
	}
}