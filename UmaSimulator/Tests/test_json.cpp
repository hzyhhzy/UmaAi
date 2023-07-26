#include <iostream>
#include <iomanip> 
#include <sstream>
#include <fstream>
#include <cassert>
#include <thread>  // for std::this_thread::sleep_for
#include <chrono>  // for std::chrono::seconds

#include "../Game/Game.h"
#include "../GameDatabase/GameConfig.h"
#include "../Search/Search.h"
#include "windows.h"
#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
using namespace std;
using json = nlohmann::json;

wchar_t buf[10240];

void main_test_json()
{
  // 检查工作目录
  GetModuleFileNameW(0, buf, 10240);
  filesystem::path exeDir = filesystem::path(buf).parent_path();
  filesystem::current_path(exeDir);
  std::cout << "当前工作目录：" << filesystem::current_path() << endl;
  cout << "当前程序目录：" << exeDir << endl;

  GameDatabase::loadUmas("db/uma");
  GameConfig::load("config.json");
  cout << "here" << endl;
}