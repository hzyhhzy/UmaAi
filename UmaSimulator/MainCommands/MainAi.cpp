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
		cout << "��ǰRP��ɫ��" << rpText["name"] << endl;
	}
	catch (...)
	{
		cout << "��ȡ������Ϣ����roleplay.json" << endl;
	}
}

void print_luck(int luck)
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

void main_ai()
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
	string lastJsonStr;//json str of the last time

	// ��鹤��Ŀ¼
	wchar_t buf[10240];
	GetModuleFileNameW(0, buf, 10240);
	filesystem::path exeDir = filesystem::path(buf).parent_path();
	filesystem::current_path(exeDir);
	std::cout << "��ǰ����Ŀ¼��" << filesystem::current_path() << endl;
	cout << "��ǰ����Ŀ¼��" << exeDir << endl;

#if USE_BACKEND == BACKEND_NONE
	GameConfig::load("./aiConfig_cpu.json");
#else
	GameConfig::load("./aiConfig.json");
#endif
	//GameDatabase::loadTranslation("./db/text_data.json");
	GameDatabase::loadUmas("./db/umaDB.json");
	//GameDatabase::loadCards("./db/card"); // ���벢����ʹ���ֶ�֧Ԯ������
	GameDatabase::loadDBCards("./db/cardDB.json"); //cardDB�����Ѿ���������
	//loadRole();   // roleplay

	bool uraFileMode = GameConfig::communicationMode == "urafile";
	//�Բ�Ӱ����ߣ�����ÿ���ļ��ı䶼ˢ��
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
	websocket ws(useWebsocket ? "http://127.0.0.1:4693" : "");
	if (useWebsocket)
	{
		do {
			Sleep(500);
			std::cout << "�ȴ�URA����" << std::endl;
		} while (ws.get_status() != "Open");
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
				std::cout << "�Ҳ���" + currentGameStagePath + "������������δ��ʼ��С�ڰ�δ��������" << endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(3000));//�ӳټ��룬����ˢ��
			}
			ifstream fs(currentGameStagePath);
			if (!fs.good())
			{
				cout << "��ȡ�ļ�����" << endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(3000));//�ӳټ��룬����ˢ��
				continue;
			}
			ostringstream tmp;
			tmp << fs.rdbuf();
			fs.close();

			jsonStr = tmp.str();
			//ifstream fs2(currentGameStagePath2);
			//ostringstream tmp2;
			//tmp2 << fs2.rdbuf();
			//fs2.close();

			//jsonStr2 = tmp2.str();
		}

		if (lastJsonStr == jsonStr)//û�и���
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(300));//��һ��
			continue;
		}

		bool suc = game.loadGameFromJson(jsonStr);
		game.eventStrength = GameConfig::eventStrength;
		game.ptScoreRate = GameConfig::scorePtRate;
		game.scoringMode = GameConfig::scoringMode;
		//bool suc2 = game2.loadGameFromJson(jsonStr2);
		//game2.eventStrength = GameConfig::eventStrength;

		if (!suc)
		{
			cout << "���ִ���" << endl;
			if (jsonStr != "[test]" && jsonStr != "{\"Result\":1,\"Reason\":null}")
			{
				auto ofs = ofstream("lastError.json");
				ofs.write(jsonStr.data(), jsonStr.size());
				ofs.close();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));//�ӳټ��룬����ˢ��
			continue;
		}
		if (game.turn == lastTurn)
		{
			if (!refreshIfAnyChanged)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(300));//����Ƿ��и���
				continue;
			}
		}
		bool maybeNonTrainingTurn = true;//��ʱ���յ�һЩ��ѵ���غϵ���Ϣ����ͬ����û��ͷ������ѵ��û��ͷ�ĸ���Լ�����֮һ
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < 5; j++)
			{
				if (game.personDistribution[i][j] != -1)
					maybeNonTrainingTurn = false;
			}
		if (maybeNonTrainingTurn && !refreshIfAnyChanged)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(300));//����Ƿ��и���
			continue;
		}
		//cout << jsonStr << endl;
		lastTurn = game.turn;
		lastJsonStr = jsonStr;
		if (game.turn == 0)//��һ�غϣ���������ai�ĵ�һ�غ�
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
				string prefix[Action::MAX_ACTION_TYPE] = { "��:", "��:", "��:", "��:", "��:", "| ��Ϣ: ", "���: ", "����: " };
				for (int dish = 1; dish < 14; dish++)
				{
					prefix[dish + TRA_race] = Action::dishName[dish] + ": ";
				}
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
		if (game.turn < TOTAL_TURN )
		{

			//���ݻغ���Ϣ����debug
			try
			{
				std::filesystem::create_directories("log");
				string fname = "log/turn" + to_string(game.turn) + (game.cook_dish == DISH_none ? "a" : "b") + ".json";
				auto ofs = ofstream(fname);
				ofs.write(jsonStr.data(), jsonStr.size());
				ofs.close();
			}
			catch (...)
			{
				cout << "����غ���Ϣʧ��" << endl;
			}

			//game.applyAction(rand, Action(DISH_none, TRA_guts));
			game.print();
			//game2.print();
			//game = game2;

			evaSingle.gameInput[0] = game;
			evaSingle.evaluateSelf(1, searchParam);
			Action hl = evaSingle.actionResults[0];
			if (GameConfig::modelPath == "")
				cout << "��д�߼�: " << hl.toString() << endl;
			else
				cout << "��������: " << hl.toString() << endl;

			search.param.maxDepth = game.turn < 70 - GameConfig::maxDepth ? GameConfig::maxDepth : 2 * TOTAL_TURN;
			Action bestAction = search.runSearch(game, rand);
			cout << "���ؿ���: " << bestAction.toString() << endl;


			//������·��俨�飬ƽ�����Ƕ��٣��뵱ǰ�غ϶Աȿ��Ի���������
			ModelOutputValueV1 trainAvgScore = { -1,-1,-1 };
			double trainLuckRate = -1;

			if (game.cook_dish == DISH_none)
			{
				trainAvgScore = search2.evaluateNewGame(game, rand);

				//���·��俨�飬�ж����ʱ���غϺ�
				if (modelptr != NULL)//ֻ���������֧�ִ˹���
				{
					int64_t count = 0;
					int64_t luckCount = 0;
					auto& eva = search2.evaluators[0];
					eva.gameInput.assign(eva.maxBatchsize, game);
					eva.evaluateSelf(0, search2.param);
					double refValue = eva.valueResults[0].scoreMean;//��ǰѵ����ƽ����

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

			Action restAction;
			restAction.dishType = DISH_none;
			restAction.train = TRA_rest;
			Action outgoingAction;
			outgoingAction.dishType = DISH_none;
			outgoingAction.train = TRA_outgoing;
			//��Ϣ������������ߵ��Ǹ������������Ϊ��ʾ�ο�
			double restValue = search.allActionResults[restAction.toInt()].lastCalculate.value;
			double outgoingValue = search.allActionResults[outgoingAction.toInt()].lastCalculate.value;
			if (outgoingValue > restValue)
				restValue = outgoingValue;


			wstring strToSendURA = L"UMAAI_COOK";
			strToSendURA += L" " + to_wstring(game.turn) + L" " + to_wstring(maxMean) + L" " + to_wstring(scoreFirstTurn) + L" " + to_wstring(scoreLastTurn) + L" " + to_wstring(maxValue);
			if (game.turn == 0 || scoreFirstTurn == 0)
			{
				//cout << "����Ԥ��: ƽ��\033[1;32m" << int(maxMean) << "\033[0m" << "���ֹ�\033[1;36m+" << int(maxValue - maxMean) << "\033[0m" << endl;
				scoreFirstTurn = search.allActionResults[outgoingAction.toInt()].lastCalculate.scoreMean;
			}
			//else
			{
				cout << "����ָ�꣺" << " | ���֣�";
				print_luck(maxMean - scoreFirstTurn);
				cout << " | ���غϣ�" << maxMean - scoreLastTurn;
				if (trainAvgScore.value >= 0) {
					cout << "��ѵ����\033[1;36m" << int(maxMean - trainAvgScore.scoreMean) << "\033[0m";

					if (trainLuckRate >= 0)
					{
						cout << fixed << setprecision(2) << " ������\033[1;36m" << trainLuckRate * 100 << "%\033[0m";
					}
					cout << "��";
				}
				cout	<< " | ����Ԥ��: \033[1;32m" << maxMean << "\033[0m"
					<< "���ֹ�\033[1;36m+" << int(maxValue - maxMean) << "\033[0m��" << endl;

			}
			cout.flush();
			scoreLastTurn = maxMean;

			for (int tr = 0; tr < 8; tr++)
			{
				Action a;
				a.dishType = DISH_none;
				a.train = tr;
				double value = search.allActionResults[a.toInt()].lastCalculate.value;
				strToSendURA += L" " + to_wstring(tr) + L" " + to_wstring(value - restValue) + L" " + to_wstring(maxValue - restValue);
				printValue(a.toInt(), value - restValue, maxValue - restValue);
				//cout << "(" << search.allActionResults[a.toInt()].num << ")";
				//cout << "(��" << 2 * int(Search::expectedSearchStdev / sqrt(search.allActionResults[a.toInt()].num)) << ")";
				if (tr == TRA_race && game.isLegal(a))
				{
					cout << "(��������:\033[1;36m" << maxValue - value << "\033[0m��";
				}
			}
			cout << endl;

			//13�ֳԲ�
			bool isAnyDishAvailable = false;
			for (int dish = 1; dish < 14; dish++)
			{
				if (game.isDishLegal(dish))
				{
					isAnyDishAvailable = true;
					break;
				}
			}
			if (isAnyDishAvailable)
			{
				cout << "��������    ";
				for (int dish = 1; dish < 14; dish++)
				{
					Action a;
					a.train = TRA_none;
					a.dishType = dish;
					if (!search.allActionResults[a.toInt()].isLegal)continue;
					double value = search.allActionResults[a.toInt()].lastCalculate.value;
					strToSendURA += L" " + to_wstring(a.toInt()) + L" " + to_wstring(value - restValue);
					printValue(a.toInt(), value - restValue, maxValue - restValue);
				}
			}
			cout << endl;

			//ũ��������Game���ڲ��Զ����еģ���Ҫ��ʾ��������������
			{
				Game game2 = game;
				game2.applyAction(rand, bestAction);
				bool anyUpgrade = false;
				for (int i = 0; i < 5; i++)
					if (game2.cook_farm_level[i] > game.cook_farm_level[i])
					{
						anyUpgrade = true;
						break;
					}
				if (anyUpgrade)
				{
					cout << "\033[1;36m";
					if (game.turn == 35 || game.turn == 59)//����ǰһ�غ�
						cout << "�Ƽ�ũ������(����ѵ��������������)��";
					else
						cout << "�Ƽ�ũ��������";

					for (int i = 0; i < 5; i++)
						if (game2.cook_farm_level[i] > game.cook_farm_level[i])
						{
							cout << GameConstants::Cook_MaterialNames[i] << "����" << game2.cook_farm_level[i] << "��  ";
						}
					cout << "\033[0m" << endl;
				}


			}

			//strToSendURA = L"0.1234567 5.4321";
			if (useWebsocket)
			{
				wstring s = L"{\"CommandType\":1,\"Command\":\"PrintUmaAiResult\",\"Parameters\":[\"" + strToSendURA + L"\"]}";
				//ws.send(s);
			}

		}

	}
}