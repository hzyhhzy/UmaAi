#pragma once
//支援卡的训练效果
//先把卡分配到对应训练里，然后才计算CardTrainingEffect
struct CardTrainingEffect
{
  double youQing;//友情加成，没闪彩就是0
  double ganJing;//干劲加成
  double xunLian;//训练加成
  double bonus[6];//速耐力根智pt的加成
  int vitalBonus;//体力回复量（主要是智彩圈）
  //int initialBonus[6];//初期速耐力根智pt的提升
  //int initialJiBan;//初始羁绊
  //double saiHou;//赛后
  //int hintBonus[6];//为了简化，把红点的技能等效成多少属性
  //double hintProbIncrease;//启发发生率提升
  //double deYiLv;//得意率
};
