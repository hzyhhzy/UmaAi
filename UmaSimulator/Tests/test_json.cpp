#include <iostream>
#include <iomanip> 
#include <sstream>
#include <fstream>
#include <cassert>
#include <thread>  // for std::this_thread::sleep_for
#include <chrono>  // for std::chrono::seconds

#include "../Game/Game.h"
#include "../Search/Search.h"
#include "windows.h"
#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
using namespace std;
using json = nlohmann::json;

wchar_t buf[10240];
/*
void dumpUmas() 
{
    for (int i = 0; i < GameDatabase::ALL_UMA_NUM; ++i)
	{
		JsonUmaData udata(GameDatabase::AllUmas[i]);
		udata.name = GameDatabase::AllUmaNames[i];
		filesystem::path path = filesystem::path("db") / udata.name;
		path.concat(".json");
		ofstream ofs(path);
		auto item = find_if(
			GameDatabase::AllUmaGameIdToSimulatorId.begin(),
			GameDatabase::AllUmaGameIdToSimulatorId.end(),
			[i](pair<const int, int> it) {
				return it.second == i;
			});
		if (item != GameDatabase::AllUmaGameIdToSimulatorId.end())
			udata.gameId = item->first;
		json j = udata;
		ofs << j.dump(2) << endl;
		ofs.close();
	}
}
*/
void main_test_json()
{
  // 检查工作目录
  GetModuleFileNameW(0, buf, 10240);
  filesystem::path exeDir = filesystem::path(buf).parent_path();
  filesystem::current_path(exeDir);
  std::cout << "当前工作目录：" << filesystem::current_path() << endl;
  cout << "当前程序目录：" << exeDir << endl;
  //dumpUmas();
  GameDatabase::loadUmas("db");
}