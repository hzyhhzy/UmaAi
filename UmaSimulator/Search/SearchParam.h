#pragma once
struct SearchParam
{
  int samplingNum; //每个选项蒙特卡洛多少次
  int maxDepth; //蒙特卡洛多深
  double maxRadicalFactor; //最大激进度。快结束时逐渐降低
};