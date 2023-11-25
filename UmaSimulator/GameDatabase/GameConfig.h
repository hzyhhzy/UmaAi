#pragma once

#include "iostream"
#include "fstream"
#include "string"
#include "sstream"
#include "filesystem"
#include "../External/json.hpp"

struct GameConfig 
{
    static bool noColor;    // 为True时不显示颜色
    static double radicalFactor; // 激进度，默认为5，提高会导致计算变慢
    static int eventStrength;  // 模拟器里每回合有40%概率加这么多属性，模拟支援卡事件。config里面加这个参数的目的是某种程度上代替激进度，调高了会更倾向于控属性
    static int threadNum;   // 线程数，默认为12，可根据自身CPU调整
    static int searchN;  // 迭代轮数，默认为6144，缩小该值可以加快速度，但会降低准确度
    static bool debugPrint; // 显示调试信息，例如进度条'.'，默认为False
    static bool useWebsocket;    // 是否使用websocket与小黑板通信，否则使用文件通信
    static std::string role;    // 台词风格

    static void load(const std::string &path);
};
