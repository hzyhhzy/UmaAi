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
#include "../websocket.h"

#include "windows.h"
#include <filesystem>
#include <cstdlib>
using namespace std;
using json = nlohmann::json;

template <typename T, std::size_t N>
std::size_t findMaxIndex(const T(&arr)[N]) {
	return std::distance(arr, std::max_element(arr, arr + N));
}

map<string, string> rpText;
void loadRole()
{
	try
	{
		ifstream ifs("db/roleplay.json");
		stringstream ss;
		ss << ifs.rdbuf();
		ifs.close();

		rpText.clear();
		json j = json::parse(ss.str(), nullptr, true, true);
		json entry = j.at(GameConfig::role);
		for (auto& item : entry.items())
		{
			rpText[item.key()] = UTF8_To_string(item.value());
		}
		cout << "当前RP角色：" << rpText["name"] << endl;
	}
	catch (...)
	{
		cout << "读取配置信息出错：roleplay.json" << endl;
	}
}

void print_luck(int luck)
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

void main_ai()
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
	std::cout << "当前工作目录：" << filesystem::current_path() << endl;
	cout << "当前程序目录：" << exeDir << endl;

#if USE_BACKEND == BACKEND_NONE
	GameConfig::load("./aiConfig_cpu.json");
#else
	GameConfig::load("./aiConfig.json");
#endif
	GameDatabase::loadTranslation("./db/text_data.json");
	GameDatabase::loadUmas("./db/umaDB.json");
	//GameDatabase::loadCards("./db/card"); // 载入并优先使用手动支援卡数据
	GameDatabase::loadDBCards("./db/cardDB.json"); //cardDB数据已经很完善了
	loadRole();   // roleplay

	string currentGameStagePath = string(getenv("LOCALAPPDATA")) + "/UmamusumeResponseAnalyzer/GameData/thisTurn.json";
	//string currentGameStagePath = "./gameData/thisTurn.json";



	Model* modelptr = NULL;
	Model model(GameConfig::modelPath, GameConfig::batchSize);
	if (GameConfig::modelPath != "")
	{
		modelptr = &model;
	}
	else
	{
		GameConfig::searchDepth = TOTAL_TURN;
	}

	Model::printBackendInfo();

	SearchParam searchParam = { GameConfig::searchN,GameConfig::searchDepth,GameConfig::radicalFactor };
	Search search(modelptr, GameConfig::batchSize, GameConfig::threadNum, searchParam);
	
	websocket ws(GameConfig::useWebsocket ? "http://127.0.0.1:4693" : "");
	if (GameConfig::useWebsocket)
	{
		do {
			Sleep(500);
			std::cout << "等待URA连接" << std::endl;
		} while (ws.get_status() != "Open");
	}

	while (true)
	{
		Game game;
		string jsonStr;
		if (GameConfig::useWebsocket)
		{
			jsonStr = lastFromWs;
		}
		else
		{

			while (!filesystem::exists(currentGameStagePath))
			{
				std::cout << "找不到" + currentGameStagePath + "，可能是育成未开始或小黑板未正常工作" << endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(3000));//延迟几秒，避免刷屏
			}
			ifstream fs(currentGameStagePath);
			if (!fs.good())
			{
				cout << "读取文件错误" << endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(3000));//延迟几秒，避免刷屏
				continue;
			}
			ostringstream tmp;
			tmp << fs.rdbuf();
			fs.close();

			jsonStr = tmp.str();
		}

		bool suc = game.loadGameFromJson(jsonStr);
		
		game.eventStrength = GameConfig::eventStrength;

		if (!suc)
		{
			cout << "出现错误" << endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));//延迟几秒，避免刷屏
			continue;
		}
		if (game.turn == lastTurn)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(300));//检查是否有更新
			continue;
		}
		bool maybeNonTrainingTurn = true;//有时会收到一些非训练回合的信息，共同点是没人头。正常训练没人头的概率约百万分之一
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < 5; j++)
			{
				if (game.personDistribution[i][j] != -1)
					maybeNonTrainingTurn = false;
			}
		if (maybeNonTrainingTurn)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(300));//检查是否有更新
			continue;
		}
		//cout << jsonStr << endl;
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

		cout << endl;
		//cout << rpText["name"] << rpText["calc"] << endl;
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

		auto printValue = [&ws](int which, double p, double ref)
			{
				string prefix[] = { "速:", "耐:", "力:", "根:", "智:", "| 休息: ", "外出: ", "比赛: " };
				if (p < -5000)
				{
					cout << prefix[which] << "---- ";
					return;
				}
				cout << fixed << setprecision(0);
				if (!GameConfig::noColor)
				{
					if (ref - p < 30) cout << "\033[41m\033[1;33m*";
					else if (ref - p < 150) cout << "\033[1;32m";
					else cout << "\033[33m";
				}
				cout << prefix[which] << setw(4) << p;
				if (!GameConfig::noColor)cout << "\033[0m";
				cout << " ";
			};

		//search.runSearch(game, GameConfig::searchN, TOTAL_TURN, 0, rand);
		if (game.turn < TOTAL_TURN - 1 && !game.isRacing)
		{
			/*
			Action handWrittenStrategy = Evaluator::handWrittenStrategy(game);
			string strategyText[10] =
			{
			  "速",
			  "耐",
			  "力",
			  "根",
			  "智",
			  "SS",
			  "休息",
			  "友人外出",
			  "普通外出",
			  "比赛"
			};
			cout << "手写逻辑：" << strategyText[handWrittenStrategy.train];
			if (game.larc_isAbroad)
			{
			  cout << "   ";
			  if (!handWrittenStrategy.buy50p)
				cout << "不";
			  cout << "购买+50%";
			}
			cout << endl;*/


			game.print();
			Action hl = Evaluator::handWrittenStrategy(game);
			cout << "手写逻辑: " << hl.toString() << endl;

			Action bestAction = search.runSearch(game, rand);
			cout << "蒙特卡洛: " << bestAction.toString() << endl;

			double maxMean = -1e7;
			double maxValue = -1e7;
			for (int i = 0; i < Action::MAX_ACTION_TYPE; i++)
			{
				if (!search.allActionResults[i].isLegal)continue;
				auto v = search.allActionResults[i].lastCalculate;
				if (v.value > maxValue)
					maxValue = v.value;
				if (v.scoreMean > maxMean)
					maxMean = v.scoreMean;
			}

			Action restAction = { TRA_rest,0 };
			Action outgoingAction = { TRA_outgoing,0 };
			//休息和外出里面分最高的那个。这个数字作为显示参考
			double restValue = search.allActionResults[restAction.toInt()].lastCalculate.value;
			double outgoingValue = search.allActionResults[outgoingAction.toInt()].lastCalculate.value;
			if (outgoingValue > restValue)
				restValue = outgoingValue;


			wstring strToSendURA = L"UAF";
			strToSendURA += L" " + to_wstring(game.turn) + L" " + to_wstring(maxMean) + L" " + to_wstring(scoreFirstTurn) + L" " + to_wstring(scoreLastTurn) + L" " + to_wstring(maxValue);
			if (game.turn == 0 || scoreFirstTurn == 0)
			{
				cout << "评分预测: 平均\033[1;32m" << int(maxMean) << "\033[0m" << "，乐观\033[1;36m+" << int(maxValue - maxMean) << "\033[0m" << endl;
				scoreFirstTurn = maxMean;
			}
			else
			{
				cout << rpText["luck"] << " | 本局：";
				print_luck(maxMean - scoreFirstTurn);
				cout << " | 本回合：" << maxMean - scoreLastTurn
					<< " | 评分预测: \033[1;32m" << maxMean << "\033[0m"
					<< "（乐观\033[1;36m+" << int(maxValue - maxMean) << "\033[0m）" << endl;

			}
			cout.flush();
			scoreLastTurn = maxMean;
			

			int xiangtannum = 0;
			for (int xt = 0; xt < 10; xt++) {
				if (!game.isXiangtanLegal(xt))continue;
				xiangtannum += 1;
			}
			strToSendURA += L" " + to_wstring(xiangtannum);
			for (int xt = 0; xt < 10; xt++)
			{
				if (!game.isXiangtanLegal(xt))continue;
				cout << "相谈:" << setw(8) << Action::xiangtanName[xt] << "  ";
				strToSendURA += L" " + to_wstring(xt)+ L" " + to_wstring(xt == XT_none ? 8 : 5);
				for (int tr = 0; tr < (xt == XT_none ? 8 : 5); tr++)
				{
					Action a = { tr,xt };
					double value = search.allActionResults[a.toInt()].lastCalculate.value;
					strToSendURA += L" " + to_wstring(tr) + L" "+ to_wstring(value-restValue) + L" " + to_wstring(maxValue - restValue);
					printValue(tr, value - restValue, maxValue - restValue);



					if (tr == TRA_race &&  game.isLegal(a))
					{
						cout<<"(比赛亏损:\033[1;36m" <<maxValue-value << "\033[0m）" ;
					}

				}
				cout << endl;
			}
			//strToSendURA = L"0.1234567 5.4321";
			if (GameConfig::useWebsocket)
			{
				wstring s = L"{\"CommandType\":1,\"Command\":\"PrintUmaAiResult\",\"Parameters\":[\"" + strToSendURA + L"\"]}";
				ws.send(s);
			}

			//提示购买友情+20%和pt+10

		}
		/*
		//cout << endl << rpText["finish"] << endl;
		cout << endl << rpText["analyze"] << " >>" << endl;
		{
		  auto policy = search.extractPolicyFromSearchResults(1);
		  if (!game.isRacing)
		  {

			cout << "速耐力根智：";
			for (int i = 0; i < 5; i++)
			  printPolicy(policy.trainingPolicy[i]);
			cout << endl;

			cout << "休息，外出，比赛：";
			for (int i = 0; i < 3; i++)
			  printPolicy(policy.trainingPolicy[5 + i]);
			cout << endl;

			// 输出运气分
			float maxScore = -1e6;
			for (int i = 0; i < 2; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					float s = search.allChoicesValue[i][j].scoreMean;
					if (s > maxScore)maxScore = s;
				}
			}
			if (game.turn == 0 || scoreFirstTurn == 0)
			{
				scoreFirstTurn = maxScore;
			}
			else
			{
				cout << rpText["luck"] << " | 本局：";
				print_luck(maxScore - scoreFirstTurn);
				cout << " | 本回合：" << maxScore - scoreLastTurn
					 << " | 评分预测: " << maxScore << endl;

				double raceLoss = maxScore - max(search.allChoicesValue[0][7].scoreMean, search.allChoicesValue[1][7].scoreMean);
				if (raceLoss < 5e5)//raceLoss大约1e6如果不能比赛
					cout << "比赛亏损（用于选择比赛回合，以完成粉丝数目标）：" << raceLoss << endl;
				cout << "----" << endl;
				cout.flush();
			}
			scoreLastTurn = maxScore;

			// 输出本回合决策
			cout << (GameConfig::noColor ? "" : "\033[1m\033[33m") << rpText["name"] << rpText["decision"] << ": "
				 << (GameConfig::noColor ? "" : "\033[32m");

			std::size_t trainChoice = findMaxIndex(policy.trainingPolicy);
			switch (trainChoice) {
				case 0:
					cout << rpText["speed"];
					break;
				case 1:
					cout << rpText["stamina"];
					break;
				case 2:
					cout << rpText["power"];
					break;
				case 3:
					cout << rpText["guts"];
					break;
				case 4:
					cout << rpText["wisdom"];
					break;
				case 5:
					cout << rpText["rest"];
					break;
				case 6:
				{
					cout << (GameConfig::noColor ? "" : "\033[0m") << " " << rpText["out"];
					break;
				}
				case 7:
					cout << rpText["race"];
					break;
			}  // switch
			cout << (GameConfig::noColor ? "" : "\033[0m") << " | ";

			cout << (GameConfig::noColor ? "" : "\033[0m") << endl;
		  } // if isRacing
		  else
		  {
			  cout << rpText["career"] << endl;
		  }
		} // 输出结果Block
		*/
		} // while

	}

//}