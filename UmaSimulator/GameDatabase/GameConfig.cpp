#include "GameConfig.h"
#include "../GameDatabase/GameConstants.h"

using namespace std;
using json = nlohmann::json;

bool GameConfig::noColor = false;
double GameConfig::radicalFactor = 1;
int GameConfig::eventStrength = 20;
int GameConfig::threadNum = 12;
int GameConfig::searchN = 12288;
bool GameConfig::debugPrint = false;
bool GameConfig::useWebsocket = true;
string GameConfig::role = "default";

void GameConfig::load(const string& path)
{
	try
	{
		ifstream ifs(path);
    if (!ifs) // 文件不存在的处理
    {
      // 创建默认配置JSON
      json j = {
        {"noColor", GameConfig::noColor},
				{"radicalFactor", GameConfig::radicalFactor},
				{"eventStrength", GameConfig::eventStrength},
        {"threadNum", GameConfig::threadNum},
        {"searchN", GameConfig::searchN},
        {"debugPrint", GameConfig::debugPrint},
        {"useWebsocket", GameConfig::useWebsocket},
        {"role", GameConfig::role}
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

		j.at("noColor").get_to(GameConfig::noColor);
		j.at("radicalFactor").get_to(GameConfig::radicalFactor);
		j.at("eventStrength").get_to(GameConfig::eventStrength);
		j.at("threadNum").get_to(GameConfig::threadNum);
		j.at("searchN").get_to(GameConfig::searchN);
		j.at("debugPrint").get_to(GameConfig::debugPrint);
		j.at("useWebsocket").get_to(GameConfig::useWebsocket);
		GameConfig::role = j.value("role", "default");

		cout << GameConfig::radicalFactor << endl;
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