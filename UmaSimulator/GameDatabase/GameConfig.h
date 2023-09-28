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
    static int threadNum;   // 线程数，默认为12，可根据自身CPU调整
    static int searchN;  // 迭代轮数，默认为6144，缩小该值可以加快速度，但会降低准确度
    static bool debugPrint; // 显示调试信息，例如进度条'.'，默认为False
    static bool extraCardData; //是否额外读入cardDB.json的数据，默认为false
    static std::string role;    // 台词风格

    static void load(const std::string &path);
};
