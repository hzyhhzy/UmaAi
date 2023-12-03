
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <cassert>
#include <thread>
#include <atomic>
#include <mutex>
#include <cmath>
#include <filesystem>
#include "../Selfplay/SelfplayThread.h"



void main_selfplay()
{
  GameDatabase::loadUmas("./db/umaDB.json");
  GameDatabase::loadDBCards("./db/cardDB.json");
  SelfplayParam param;
  std::filesystem::create_directories(param.exportDataDir);

  Model* modelptr = NULL;
  Model model(param.modelPath, param.batchsize);
  if (param.modelPath != "")
  {
    modelptr = &model;
  }
  std::cout << "神经网络模型路径：" << param.modelPath << std::endl;
  Model::detect(modelptr);



  std::vector<SelfplayThread> spThreads;
  std::vector<std::thread> threads;

  for (int i = 0; i < param.threadNum; i++)
    spThreads.emplace_back(param, modelptr);

  //spThreads[0].run();
  for (auto& spt : spThreads) {
    threads.emplace_back(&SelfplayThread::run, &spt);
  }

  // 等待所有线程完成
  for (auto& th : threads) {
    th.join();
  }

  std::cout << "All threads finished." << std::endl;



}
