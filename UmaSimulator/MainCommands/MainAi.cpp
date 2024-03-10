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
	GameDatabase::loadTranslation("./db/text_data.json");
	GameDatabase::loadUmas("./db/umaDB.json");
	//GameDatabase::loadCards("./db/card"); // ���벢����ʹ���ֶ�֧Ԯ������
	GameDatabase::loadDBCards("./db/cardDB.json"); //cardDB�����Ѿ���������
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
			std::cout << "�ȴ�URA����" << std::endl;
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
		}

		bool suc = game.loadGameFromJson(jsonStr);
		
		game.eventStrength = GameConfig::eventStrength;

		if (!suc)
		{
			cout << "���ִ���" << endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));//�ӳټ��룬����ˢ��
			continue;
		}
		if (game.turn == lastTurn)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(300));//����Ƿ��и���
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
				string prefix[] = { "��:", "��:", "��:", "��:", "��:", "| ��Ϣ: ", "���: ", "����: " };
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
			  "��",
			  "��",
			  "��",
			  "��",
			  "��",
			  "SS",
			  "��Ϣ",
			  "�������",
			  "��ͨ���",
			  "����"
			};
			cout << "��д�߼���" << strategyText[handWrittenStrategy.train];
			if (game.larc_isAbroad)
			{
			  cout << "   ";
			  if (!handWrittenStrategy.buy50p)
				cout << "��";
			  cout << "����+50%";
			}
			cout << endl;*/


			game.print();
			Action hl = Evaluator::handWrittenStrategy(game);
			cout << "��д�߼�: " << hl.toString() << endl;

			Action bestAction = search.runSearch(game, rand);
			cout << "���ؿ���: " << bestAction.toString() << endl;

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
			//��Ϣ������������ߵ��Ǹ������������Ϊ��ʾ�ο�
			double restValue = search.allActionResults[restAction.toInt()].lastCalculate.value;
			double outgoingValue = search.allActionResults[outgoingAction.toInt()].lastCalculate.value;
			if (outgoingValue > restValue)
				restValue = outgoingValue;


			wstring strToSendURA = L"UAF";
			strToSendURA += L" " + to_wstring(game.turn) + L" " + to_wstring(maxMean) + L" " + to_wstring(scoreFirstTurn) + L" " + to_wstring(scoreLastTurn) + L" " + to_wstring(maxValue);
			if (game.turn == 0 || scoreFirstTurn == 0)
			{
				cout << "����Ԥ��: ƽ��\033[1;32m" << int(maxMean) << "\033[0m" << "���ֹ�\033[1;36m+" << int(maxValue - maxMean) << "\033[0m" << endl;
				scoreFirstTurn = maxMean;
			}
			else
			{
				cout << rpText["luck"] << " | ���֣�";
				print_luck(maxMean - scoreFirstTurn);
				cout << " | ���غϣ�" << maxMean - scoreLastTurn
					<< " | ����Ԥ��: \033[1;32m" << maxMean << "\033[0m"
					<< "���ֹ�\033[1;36m+" << int(maxValue - maxMean) << "\033[0m��" << endl;

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
				cout << "��̸:" << setw(8) << Action::xiangtanName[xt] << "  ";
				strToSendURA += L" " + to_wstring(xt)+ L" " + to_wstring(xt == XT_none ? 8 : 5);
				for (int tr = 0; tr < (xt == XT_none ? 8 : 5); tr++)
				{
					Action a = { tr,xt };
					double value = search.allActionResults[a.toInt()].lastCalculate.value;
					strToSendURA += L" " + to_wstring(tr) + L" "+ to_wstring(value-restValue) + L" " + to_wstring(maxValue - restValue);
					printValue(tr, value - restValue, maxValue - restValue);

				}
				cout << endl;
			}
			//strToSendURA = L"0.1234567 5.4321";
			if (GameConfig::useWebsocket)
			{
				wstring s = L"{\"CommandType\":1,\"Command\":\"PrintUmaAiResult\",\"Parameters\":[\"" + strToSendURA + L"\"]}";
				ws.send(s);
			}

			//��ʾ��������+20%��pt+10

		}
		/*
		//cout << endl << rpText["finish"] << endl;
		cout << endl << rpText["analyze"] << " >>" << endl;
		{
		  auto policy = search.extractPolicyFromSearchResults(1);
		  if (!game.isRacing)
		  {

			cout << "���������ǣ�";
			for (int i = 0; i < 5; i++)
			  printPolicy(policy.trainingPolicy[i]);
			cout << endl;

			cout << "��Ϣ�������������";
			for (int i = 0; i < 3; i++)
			  printPolicy(policy.trainingPolicy[5 + i]);
			cout << endl;

			// ���������
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
				cout << rpText["luck"] << " | ���֣�";
				print_luck(maxScore - scoreFirstTurn);
				cout << " | ���غϣ�" << maxScore - scoreLastTurn
					 << " | ����Ԥ��: " << maxScore << endl;

				double raceLoss = maxScore - max(search.allChoicesValue[0][7].scoreMean, search.allChoicesValue[1][7].scoreMean);
				if (raceLoss < 5e5)//raceLoss��Լ1e6������ܱ���
					cout << "������������ѡ������غϣ�����ɷ�˿��Ŀ�꣩��" << raceLoss << endl;
				cout << "----" << endl;
				cout.flush();
			}
			scoreLastTurn = maxScore;

			// ������غϾ���
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
		} // ������Block
		*/
		} // while

	}

//}