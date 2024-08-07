#pragma once

#include "iostream"
#include "fstream"
#include "string"
#include "sstream"
#include "filesystem"
#include "../External/json.hpp"

struct GameConfig 
{
  
    static double radicalFactor; // 激进度，提高会导致计算变慢
    static int searchSingleMax;  // 第一选项达到多少次搜索就停止蒙特卡洛
    static int threadNum;   // 线程数，可根据自身CPU调整，神经网络版直接设成4

    static double scorePtRate;    // 每pt的分数
    static int scoringMode;    // 打分方式

    // "localfile": checking ./thisTurn.json
    // "urafile": communicating with URA using file
    // "websocket": communicating with URA using websocket
    static std::string communicationMode;    


    //以下不经常修改

    static int eventStrength;  // 模拟器里每回合有40%概率加这么多属性，模拟支援卡事件。config里面加这个参数的目的是某种程度上代替激进度，调高了会更倾向于控属性

    static std::string modelPath;    // 神经网络文件所在目录
    static int batchSize;   // 显卡版的batchSize，非神经网络版设多少区别不大，神经网络版直接设成256

    static int searchTotalMax; //所有选项总共达到多少次搜索就停止蒙特卡洛，0为不限
    static int searchGroupSize; //搜索时每次分配多少计算量。建议128，不要小于16*线程数，太小会带来很大额外开销（每searchGroupSize次搜索要计算O(200000)次）
    static double searchCpuct; //cpuct参数，越小搜索越集中
    static int maxDepth;  // 蒙特卡洛深度，神经网络默认是10，非神经网络是直接搜到游戏结束（TOTAL_TURN=67）

    static std::string role;    // 台词风格

    static bool debugPrint; // 显示调试信息，例如进度条'.'，默认为False
    static bool noColor;    // 为True时不显示颜色

    static void load(const std::string &path);
};
