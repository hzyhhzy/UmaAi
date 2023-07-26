#include "GameDatabase.h"
#include "UmaData.h"
#include "../External/utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
using json = nlohmann::json;
using namespace std;

unordered_map<int, UmaData> GameDatabase::AllUmas;

void GameDatabase::loadUmas(const string& dir) 
{
    try
    {
        for (auto entry : filesystem::directory_iterator(dir + "/"))
        {
            //cout << entry.path() << endl;
            if (entry.path().extension() == ".json")
            {
                try
                {
                    ifstream ifs(entry.path());
                    stringstream ss;
                    ss << ifs.rdbuf();
                    ifs.close();
                    json j = json::parse(ss.str());

                    UmaData jdata;
                    j.get_to(jdata);
                    cout << "载入马娘 #" << jdata.gameId << " " << jdata.name << endl;
                    if (AllUmas.count(jdata.gameId) > 0)
                        cout << "错误：重复的马娘 #" << jdata.gameId << endl;
                    else
                        AllUmas[jdata.gameId] = jdata;
                }
                catch (exception& e)
                {
                    cout << "马娘信息JSON出错: " << entry.path() << endl << e.what() << endl;
                }
            }
        }
        cout << "共载入 " << AllUmas.size() << " 个育成马娘数据" << endl;        
    }
    catch (exception& e)
    {
        cout << "读取马娘信息出错: " << endl << e.what() << endl;
    }
    catch (...)
    {
        cout << "读取马娘信息出错：未知错误" << endl;
    }
}