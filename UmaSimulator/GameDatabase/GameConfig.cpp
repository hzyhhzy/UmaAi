#include "GameConfig.h"
#include "../GameDatabase/GameConstants.h"

using namespace std;
using json = nlohmann::json;

double GameConfig::radicalFactor = 5;
int GameConfig::eventStrength = 20;
bool GameConfig::removeDebuff5 = true;
bool GameConfig::removeDebuff7 = true;

string GameConfig::modelPath = "db/model_traced.pt";
int GameConfig::threadNum = 4;
int GameConfig::batchSize = 256;
int GameConfig::searchN = 2000;
int GameConfig::searchDepth = 10;

bool GameConfig::useWebsocket = true;
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
      // 创建默认配置JSON
      json j = {
				{"radicalFactor", GameConfig::radicalFactor},
				{"eventStrength", GameConfig::eventStrength},
				{"removeDebuff5", GameConfig::removeDebuff5},
				{"removeDebuff7", GameConfig::removeDebuff7},

				{"modelPath", GameConfig::modelPath},
        {"threadNum", GameConfig::threadNum},
				{"batchSize", GameConfig::batchSize},
				{"searchN", GameConfig::searchN},
				{"searchDepth", GameConfig::searchDepth},

        {"useWebsocket", GameConfig::useWebsocket},
        {"role", GameConfig::role},

				{"debugPrint", GameConfig::debugPrint},
				{"noColor", GameConfig::noColor}
      };
      // 写入
      ofstream ofs(path);
      ofs << j.dump(2);
      ofs.close();

      cout << "找不到配置文件，已使用默认配置: " << j.dump(2) << endl;
      return;
    }

		stringstream ss;
		ss << ifs.rdbuf();
		ifs.close();
		json j = json::parse(ss.str(),nullptr,true,true);

		j.at("radicalFactor").get_to(GameConfig::radicalFactor);
		j.at("eventStrength").get_to(GameConfig::eventStrength);
		j.at("removeDebuff5").get_to(GameConfig::removeDebuff5);
		j.at("removeDebuff7").get_to(GameConfig::removeDebuff7);


		GameConfig::modelPath = j.value("modelPath", "db/model_traced.pt");
		j.at("threadNum").get_to(GameConfig::threadNum);
		j.at("batchSize").get_to(GameConfig::batchSize);
		j.at("searchN").get_to(GameConfig::searchN);
		j.at("searchDepth").get_to(GameConfig::searchDepth);

		j.at("useWebsocket").get_to(GameConfig::useWebsocket);
		j.at("debugPrint").get_to(GameConfig::debugPrint);

		j.at("noColor").get_to(GameConfig::noColor);
		GameConfig::role = j.value("role", "default");

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