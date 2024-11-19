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

try {
	random_device rd;
	auto rand = mt19937_64(rd());

	int lastTurn = -1;
	int scoreFirstTurn = 0;   // 第一回合分数
	int scoreLastTurn = 0;   // 上一回合分数
	string lastJsonStr;//json str of the last time

	// 检查工作目录
	char buf[10240];
	GetModuleFileNameA(0, buf, 10240);
	filesystem::path exeDir = filesystem::path(buf).parent_path();
	filesystem::current_path(exeDir);
	// 有编码问题，暂时去掉
	//cout << "当前工作目录：" << filesystem::current_path() << endl;
	//cout << "当前程序目录：" << exeDir << endl;

#if USE_BACKEND == BACKEND_NONE
	GameConfig::load("./aiConfig_cpu.json");
#else
	GameConfig::load("./aiConfig.json");
#endif
	//GameDatabase::loadTranslation("./db/text_data.json");
	GameDatabase::loadUmas("./db/umaDB.json");
	//GameDatabase::loadCards("./db/card"); // 载入并优先使用手动支援卡数据
	GameDatabase::loadDBCards("./db/cardDB.json"); //cardDB数据已经很完善了
	//loadRole();   // roleplay

	bool uraFileMode = GameConfig::communicationMode == "urafile";
	//吃菜影响决策，所以每次文件改变都刷新
	bool refreshIfAnyChanged = true;//if false, only new turns will refresh
	//bool refreshIfAnyChanged = GameConfig::communicationMode == "localfile";//if false, only new turns will refresh
	string currentGameStagePath = uraFileMode ?
		string(getenv("LOCALAPPDATA")) + "/UmamusumeResponseAnalyzer/GameData/thisTurn.json"
		: "./thisTurn.json";
	//string currentGameStagePath2 = 
	//	string(getenv("LOCALAPPDATA")) + "/UmamusumeResponseAnalyzer/GameData/turn34.json"
	//	
	//string currentGameStagePath = "./gameData/thisTurn.json";

	Model* modelptr = NULL;
	Model model(GameConfig::modelPath, GameConfig::batchSize);
	Model* modelSingleptr = NULL;
	Model modelSingle(GameConfig::modelPath, 1);
	if (GameConfig::modelPath != "")
	{
		modelptr = &model;
		modelSingleptr = &modelSingle;
	}
	else
	{
		GameConfig::maxDepth = 2 * TOTAL_TURN;
	}

	Model::printBackendInfo();

	SearchParam searchParam(
		GameConfig::searchSingleMax,
		GameConfig::searchTotalMax,
		GameConfig::searchGroupSize,
		GameConfig::searchCpuct,
		GameConfig::maxDepth,
		GameConfig::radicalFactor
	);
	Search search(modelptr, GameConfig::batchSize, GameConfig::threadNum, searchParam);
	Search search2(modelptr, GameConfig::batchSize, GameConfig::threadNum, searchParam);
	Evaluator evaSingle(modelSingleptr, 1);

	bool useWebsocket = GameConfig::communicationMode == "websocket";
	bool isLinkError = false;

	websocket ws(useWebsocket ? "http://127.0.0.1:4693" : "");
	if (useWebsocket)
	{
		do {
			Sleep(500);
			if (!isLinkError) {
				std::cout << "\x1b[93m等待URA连接\x1b[0m" << std::endl;
				isLinkError = true;
			}
		} while (ws.get_status() != "Open");
		isLinkError = false;
	}

	while (true)
	{
		Game game;
		//Game game2;
		string jsonStr;
		//string jsonStr2;
		if (useWebsocket)
		{
			jsonStr = lastFromWs;
		}
		else
		{
			while (!filesystem::exists(currentGameStagePath))
			{
				if (!isLinkError) {
					std::cout << "\x1b[93m没有检测到回合数据: " << currentGameStagePath << "\x1b[0m" << endl;
					std::cout << "\x1b[93m可能原因：（1）育成未开始，育成开始后会自动开始计算（2）小黑板没有发送数据，黑板设置应打开“生成AI数据文件”\x1b[0m" << endl;
					isLinkError = true;
				}
			}
			ifstream fs(currentGameStagePath);
			if (!fs.good())
			{
				if (!isLinkError) {
					std::cout << "读取回合数据文件错误" << endl;
					isLinkError = true;
				}
				continue;
			}
			ostringstream tmp;
			tmp << fs.rdbuf();
			fs.close();

			jsonStr = tmp.str();
			isLinkError = false;
		}

		if (lastJsonStr == jsonStr)//没有更新
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(300));//等一下
			continue;
		}
		// sanity check
		if (isLinkError)
			continue;
		try {
			bool suc = game.loadGameFromJson(jsonStr);
			game.eventStrength = GameConfig::eventStrength;
			game.ptScoreRate = GameConfig::scorePtRate;
			game.scoringMode = GameConfig::scoringMode;
			//bool suc2 = game2.loadGameFromJson(jsonStr2);
			//game2.eventStrength = GameConfig::eventStrength;

			if (!suc)
			{
				if (!isLinkError) {
					cout << "\x1b[93m小黑板通信出错\x1b[0m" << endl;
					isLinkError = true;
				}
				if (jsonStr != "[test]" && jsonStr != "{\"Result\":1,\"Reason\":null}")
				{
					auto ofs = ofstream("lastError.json");
					ofs.write(jsonStr.data(), jsonStr.size());
					ofs.close();
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(3000));//延迟几秒，避免刷屏
				continue;
			}
			else
			{
				isLinkError = false;
			}

			if (game.turn == lastTurn)
			{
				if (!refreshIfAnyChanged)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(300));//检查是否有更新
					continue;
				}
			}
			bool maybeNonTrainingTurn = true;//有时会收到一些非训练回合的信息，共同点是没人头。正常训练没人头的概率约百万分之一
			for (int i = 0; i < 5; i++)
				for (int j = 0; j < 5; j++)
				{
					if (game.personDistribution[i][j] != -1)
						maybeNonTrainingTurn = false;
				}
			if (maybeNonTrainingTurn && !refreshIfAnyChanged)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(300));//检查是否有更新
				continue;
			}
			//cout << jsonStr << endl;
			lastTurn = game.turn;
			lastJsonStr = jsonStr;
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

			auto printValue = [&ws](double p, double ref)
				{
					if (p < -5000)
					{
						cout << "---- ";
						return;
					}
					cout << fixed << setprecision(0);
					if (!GameConfig::noColor)
					{
						if (ref - p < 30) cout << "\033[41m\033[1;33m*";
						else if (ref - p < 150) cout << "\033[1;32m";
						else cout << "\033[33m";
					}
					cout << setw(4) << p;
					if (!GameConfig::noColor)cout << "\033[0m";
					cout << " ";
				};

			//search.runSearch(game, GameConfig::searchN, TOTAL_TURN, 0, rand);
			if (game.turn < TOTAL_TURN)
			{

				//备份回合信息用于debug
				try
				{
					std::filesystem::create_directories("log");
					string fname = "log/turn" + to_string(game.turn) + (game.mecha_overdrive_enabled ? "b" : game.gameStage == GameStage_beforeMechaUpgrade ? "_upgrade" : "a") + ".json";
					auto ofs = ofstream(fname);
					ofs.write(jsonStr.data(), jsonStr.size());
					ofs.close();
				}
				catch (...)
				{
					cout << "保存回合信息失败" << endl;
				}

				//game.applyAction(rand, Action(3));
				game.print();

				evaSingle.gameInput[0] = game;
				evaSingle.evaluateSelf(1, searchParam);
				Action hl = evaSingle.actionResults[0];
				if (GameConfig::modelPath == "")
					cout << "手写逻辑: " << hl.toString() << endl;
				else
					cout << "纯神经网络: " << hl.toString() << endl;

				Action bestAction = search.runSearch(game, rand);
				cout << "蒙特卡洛: " << bestAction.toString() << endl;


				//如果重新分配卡组，平均分是多少，与当前回合对比可以获得运气情况
				ModelOutputValueV1 trainAvgScore = { -1,-1,-1 };
				double trainLuckRate = -1;

				if (game.gameStage == GameStage_beforeTrain)
				{
					trainAvgScore = search2.evaluateNewGame(game, rand);

					//重新分配卡组，有多大概率比这回合好
					if (modelptr != NULL)//只有神经网络版支持此功能
					{
						int64_t count = 0;
						int64_t luckCount = 0;
						auto& eva = search2.evaluators[0];
						eva.gameInput.assign(eva.maxBatchsize, game);
						eva.evaluateSelf(0, search2.param);
						double refValue = eva.valueResults[0].scoreMean;//当前训练的平均分

						int batchN = 1 + 4 * GameConfig::searchSingleMax / eva.maxBatchsize;
						for (int b = 0; b < batchN; b++)
						{
							for (int i = 0; i < eva.maxBatchsize; i++)
							{
								eva.gameInput[i] = game;
								eva.gameInput[i].randomDistributeCards(rand);
							}
							eva.evaluateSelf(0, search2.param);
							for (int i = 0; i < eva.maxBatchsize; i++)
							{
								count++;
								if (eva.valueResults[i].scoreMean < refValue)
									luckCount++;
							}

						}
						trainLuckRate = double(luckCount) / count;
					}
				}
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

				double restValue = 0;
				if (game.gameStage == GameStage_beforeTrain)
				{
					Action restAction = Action(TRA_rest);
					Action outgoingAction = Action(TRA_outgoing);
					//休息和外出里面分最高的那个。这个数字作为显示参考
					restValue = search.allActionResults[restAction.toInt()].lastCalculate.value;
					double outgoingValue = search.allActionResults[outgoingAction.toInt()].lastCalculate.value;
					if (outgoingValue > restValue)
						restValue = outgoingValue;
				}
				wstring strToSendURA = L"UMAAI_MECHA";
				strToSendURA += L" " + to_wstring(game.turn) + L" " + to_wstring(maxMean) + L" " + to_wstring(scoreFirstTurn) + L" " + to_wstring(scoreLastTurn) + L" " + to_wstring(maxValue);
				if (game.turn == 0 || scoreFirstTurn == 0)
				{
					Action outgoingAction = Action(TRA_outgoing);
					//cout << "评分预测: 平均\033[1;32m" << int(maxMean) << "\033[0m" << "，乐观\033[1;36m+" << int(maxValue - maxMean) << "\033[0m" << endl;
					scoreFirstTurn = search.allActionResults[outgoingAction.toInt()].lastCalculate.scoreMean;
				}
				//else
				{
					cout << "运气指标：" << " | 本局：";
					print_luck(maxMean - scoreFirstTurn);
					cout << " | 本回合：" << maxMean - scoreLastTurn;
					if (trainAvgScore.value >= 0) {
						cout << "（训练：\033[1;36m" << int(maxMean - trainAvgScore.scoreMean) << "\033[0m";

						if (trainLuckRate >= 0)
						{
							cout << fixed << setprecision(2) << " 超过了\033[1;36m" << trainLuckRate * 100 << "%\033[0m";
						}
						cout << "）";
					}
					cout << " | 评分预测: \033[1;32m" << maxMean << "\033[0m"
						<< "（乐观\033[1;36m+" << int(maxValue - maxMean) << "\033[0m）" << endl;

				}
				cout.flush();
				scoreLastTurn = maxMean;

				if (game.gameStage == GameStage_beforeTrain)
				{
					string prefix[8] = { "速:", "耐:", "力:", "根:", "智:", "| 休息: ", "外出: ", "比赛: " };

					for (int tr = 0; tr < 8; tr++)
					{
						Action a(tr);
						double value = search.allActionResults[a.toInt()].lastCalculate.value;
						strToSendURA += L" " + to_wstring(tr) + L" " + to_wstring(value - restValue) + L" " + to_wstring(maxValue - restValue);
						cout << prefix[tr];
						printValue(value - restValue, maxValue - restValue);
						//cout << "(" << search.allActionResults[a.toInt()].num << ")";
						//cout << "(±" << 2 * int(Search::expectedSearchStdev / sqrt(search.allActionResults[a.toInt()].num)) << ")";
						if (tr == TRA_race && game.isLegal(a))
						{
							cout << "(比赛亏损:\033[1;36m" << maxValue - value << "\033[0m）";
						}
					}
					cout << endl;
					if (!game.mecha_overdrive_enabled && game.mecha_overdrive_energy >= 3)
					{
						Action a(0);
						a.overdrive = true;
						if (game.mecha_upgradeTotal[1] >= 15)
						{
							a.train = -1;
							double value = search.allActionResults[a.toInt()].lastCalculate.value;
							cout << "开启齿轮：";
							printValue(value - restValue, maxValue - restValue);
						}
						else
						{
							cout << "开启齿轮：";
							for (int tr = 0; tr < 5; tr++)
							{
								a.train = tr;
								double value = search.allActionResults[a.toInt()].lastCalculate.value;
								cout << prefix[tr];
								printValue(value - restValue, maxValue - restValue);
							}
						}
						cout << endl;
					}
				}
				else if (game.gameStage == GameStage_beforeMechaUpgrade)
				{
					cout << "\033[1;31mUmaAI只考虑头胸腿分别多少级，不考虑具体分到哪项，且非整3级的剩余EN也不考虑，请自己决定。\033[0m" << endl;
					for (int u = 0; u < 36; u++)
					{
						Action a(u + 14);
						if (!game.isLegal(a))continue;
						cout << "头" + std::to_string(3 * a.mechaHead) + "级胸" + std::to_string(3 * a.mechaChest) + "级腿" + std::to_string(3 * (game.mecha_EN / 3 - a.mechaChest - a.mechaHead)) + "级: ";

						double value = search.allActionResults[a.toInt()].lastCalculate.value;
						printValue(value - restValue, maxValue - restValue);
						cout << endl;
					}
				}

				cout << endl;
				//strToSendURA = L"0.1234567 5.4321";
				if (useWebsocket)
				{
					wstring s = L"{\"CommandType\":1,\"Command\":\"PrintUmaAiResult\",\"Parameters\":[\"" + strToSendURA + L"\"]}";
					//ws.send(s);
				}
			}
		}
		catch (exception& e) {
			cout << "\x1b[91m发生错误: " << endl << e.what() << endl;
			cout << "\x1b[91m** 程序即将退出**\x1b[0m" << endl;
			system("pause");
		}
		catch (...) {
			cout << "\x1b[91m发生未知错误 " << endl;
			cout << "\x1b[91m** 程序即将退出**\x1b[0m" << endl;
			system("pause");
		}
	}
}
catch (exception& e) {
	cout << "\x1b[91mAI初始化时发生错误: " << endl << e.what() << endl;
	cout << "\x1b[91m** 程序即将退出**\x1b[0m" << endl;
	system("pause");
}
catch (...) {
	cout << "\x1b[91mAI初始化时发生未知错误 " << endl;
	cout << "\x1b[91m** 程序即将退出**\x1b[0m" << endl;
	system("pause");
}
}