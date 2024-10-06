#include "GameConfig.h"
#include "../Game/Game.h"

using namespace std;
using json = nlohmann::json;

double GameConfig::radicalFactor = 3; 
int GameConfig::scoringMode = SM_normal;
double GameConfig::scorePtRate = GameConstants::ScorePtRateDefault;
int GameConfig::eventStrength = GameConstants::EventStrengthDefault;

#if USE_BACKEND != BACKEND_NONE      //神经网络版
#if USE_BACKEND == BACKEND_LIBTORCH      //神经网络版
string GameConfig::modelPath = "db/model_traced.pt";
#elif USE_BACKEND == BACKEND_ONNX      
string GameConfig::modelPath = "db/model.onnx";
#else
string GameConfig::modelPath = "db/model.txt";
#endif
int GameConfig::threadNum = 4;
int GameConfig::batchSize = 256;
int GameConfig::searchSingleMax = 5120;
int GameConfig::searchTotalMax = 0;
int GameConfig::searchGroupSize = 1024;
int GameConfig::maxDepth = TOTAL_TURN;
#else  //手写逻辑版
string GameConfig::modelPath = "";
int GameConfig::threadNum = 8;
int GameConfig::batchSize = 1;
int GameConfig::searchSingleMax = 4096;
int GameConfig::searchTotalMax = 0;
int GameConfig::searchGroupSize = 128;
int GameConfig::maxDepth = 2 * TOTAL_TURN;
#endif 

double GameConfig::searchCpuct = 4.0;

string GameConfig::communicationMode = "localfile";
string GameConfig::role = "default";

bool GameConfig::debugPrint = false;
bool GameConfig::noColor = false;

void GameConfig::load(const string& path)
{
	try
	{
		ifstream ifs(path);
    if (!ifs) // 文件不存在的处理
    {
      cout << "\x1b[91m缺少配置文件: "<< path <<", 请重新解压AI\x1b[0m" << endl;
      return;
    }

		stringstream ss;
		ss << ifs.rdbuf();
		ifs.close();
		json j = json::parse(ss.str(),nullptr,true,true);

		if (j.contains("radicalFactor"))
			GameConfig::radicalFactor = j.at("radicalFactor");
		if (j.contains("eventStrength"))
			GameConfig::eventStrength = j.at("eventStrength");
		if (j.contains("scorePtRate"))
			GameConfig::scorePtRate = j.at("scorePtRate");
		if (j.contains("scoringMode"))
		{
			string smStr = j.at("scoringMode");
			if (smStr == to_string(SM_normal) || smStr == "normal")
				GameConfig::scoringMode = SM_normal;
			else if (smStr == to_string(SM_race) || smStr == "race")
				GameConfig::scoringMode = SM_race;
			else if (smStr == to_string(SM_mile) || smStr == "mile")
				GameConfig::scoringMode = SM_mile;
			else
				throw "config文件里未知的scoringMode设置";
		}
		if (j.contains("modelPath"))
			GameConfig::modelPath = j.at("modelPath");
		if (j.contains("threadNum"))
			GameConfig::threadNum = j.at("threadNum");
		if (j.contains("batchSize"))
			GameConfig::batchSize = j.at("batchSize");
		if (j.contains("searchSingleMax"))
			GameConfig::searchSingleMax = j.at("searchSingleMax");
		if (j.contains("searchTotalMax"))
			GameConfig::searchTotalMax = j.at("searchTotalMax");
		if (j.contains("searchGroupSize"))
			GameConfig::searchGroupSize = j.at("searchGroupSize");
		if (j.contains("searchCpuct"))
			GameConfig::searchCpuct = j.at("searchCpuct");
		if (j.contains("maxDepth"))
			GameConfig::maxDepth = j.at("maxDepth");
		if (j.contains("communicationMode"))
			GameConfig::communicationMode = j.at("communicationMode");
		if (j.contains("role"))
			GameConfig::role = j.at("role");
		if (j.contains("debugPrint"))
			GameConfig::debugPrint = j.at("debugPrint");
		if (j.contains("noColor"))
			GameConfig::noColor = j.at("noColor");




		cout << "当前配置: " << j.dump(2) << endl;
	}
	catch (exception& e)
	{
		cout << "载入配置出错: " << e.what() << endl;
	}
	catch (...)
	{
		cout << "载入配置时发生未知错误" << endl;
	}
}