#pragma once

#include "iostream"
#include "fstream"
#include "string"
#include "sstream"
#include "filesystem"
#include "../External/json.hpp"

struct GameConfig 
{
    static double radicalFactor; // 激进度，默认为5，提高会导致计算变慢
    static int eventStrength;  // 模拟器里每回合有40%概率加这么多属性，模拟支援卡事件。config里面加这个参数的目的是某种程度上代替激进度，调高了会更倾向于控属性
    static bool removeDebuff5; // 第二次凯旋门前是否需要消除智力debuff
    static bool removeDebuff7; // 第二次凯旋门前是否需要消除强心脏debuff

    static std::string modelPath;    // 神经网络文件所在目录
    static int threadNum;   // 线程数，默认为12，可根据自身CPU调整，神经网络版直接设成4
    static int batchSize;   // 显卡版的batchSize，非神经网络版设多少区别不大，神经网络版直接设成256
    static int searchN;  // 蒙特卡洛数量，默认为6144，缩小该值可以加快速度，但会降低准确度
    static int searchDepth;  // 蒙特卡洛深度，神经网络默认是10，非神经网络是直接搜到游戏结束（TOTAL_TURN=67）

    static bool useWebsocket;    // 是否使用websocket与小黑板通信，否则使用文件通信
    static std::string role;    // 台词风格

    static bool debugPrint; // 显示调试信息，例如进度条'.'，默认为False
    static bool noColor;    // 为True时不显示颜色

    static void load(const std::string &path);
};
