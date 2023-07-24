#include "GameDatabase.h"
#include "UmaData.h"
#include "windows.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
using json = nlohmann::json;
using namespace std;

// https://www.codersrc.com/archives/15399.html
std::string string_To_UTF8(const std::string& str)
{
    int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

    wchar_t* pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
    ZeroMemory(pwBuf, nwLen * 2 + 2);

    ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

    int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

    char* pBuf = new char[nLen + 1];
    ZeroMemory(pBuf, nLen + 1);

    ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

    std::string retStr(pBuf);

    delete[]pwBuf;
    delete[]pBuf;

    pwBuf = NULL;
    pBuf = NULL;

    return retStr;
}

std::string UTF8_To_string(const std::string& str)
{
    int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

    wchar_t* pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
    memset(pwBuf, 0, nwLen * 2 + 2);

    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);

    int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

    char* pBuf = new char[nLen + 1];
    memset(pBuf, 0, nLen + 1);

    WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

    std::string retStr = pBuf;

    delete[]pBuf;
    delete[]pwBuf;

    pBuf = NULL;
    pwBuf = NULL;

    return retStr;
}

map<int, JsonUmaData> GameDatabase::jsonUmas;

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

                    JsonUmaData jdata;
                    j.get_to(jdata);
                    cout << "载入马娘 #" << jdata.gameId << " " << jdata.name << endl;
                    if (jsonUmas.count(jdata.gameId) > 0)
                        cout << "错误：重复的马娘 #" << jdata.gameId << endl;
                    else
                        jsonUmas[jdata.gameId] = jdata;
                }
                catch (exception& e)
                {
                    cout << "马娘信息JSON出错: " << entry.path() << endl << e.what() << endl;
                }
            }
        }
        cout << "共载入 " << jsonUmas.size() << " 个育成马娘数据" << endl;        
    }
    catch (exception& e)
    {
        cout << "读取马娘信息出错: " << endl << e.what() << endl;
        exit(-1);
    }
}