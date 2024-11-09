#pragma once
struct SearchParam
{
  int searchSingleMax; //第一选项达到多少次搜索就停止蒙特卡洛
  int searchTotalMax; //所有选项总共达到多少次搜索就停止蒙特卡洛，0为不限
  int searchGroupSize; //搜索时每次分配多少计算量。建议128，不要小于16*线程数，太小会带来很大额外开销（每searchGroupSize次搜索要计算O(200000)次）
  double searchCpuct; //cpuct参数，越小搜索越集中
  int maxDepth; //蒙特卡洛多深
  double maxRadicalFactor; //最大激进度。快结束时逐渐降低

  SearchParam();
  SearchParam(int searchSingleMax, double maxRadicalFactor);
  SearchParam(
    int searchSingleMax,
    int searchTotalMax,
    int searchGroupSize,
    double searchCpuct,
    int maxDepth,
    double maxRadicalFactor
  );
};