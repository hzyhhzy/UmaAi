#pragma once
#include <cstdint>
#include <random>
#include <string>
struct SupportCard;
struct Game;
struct Person //任何一个可能出现在训练里的人头
{
  int8_t personType;//0代表未加载（例如前两个回合的npc），1代表佐岳支援卡（R或SSR都行），2代表普通支援卡，3代表npc人头，4理事长，5记者，6不带卡的佐岳。暂不支持其他友人/团队卡
  //int16_t cardId;//支援卡id，不是支援卡就0
  int16_t charaId;//npc人头的马娘id，不是npc就0，懒得写也可以一律0（只用于获得npc的名字）

  int8_t cardIdInGame;// Game.cardParam里的支援卡序号，非支援卡为-1
  int8_t friendship;//羁绊
  bool atTrain[5];//是否在五个训练里。对于普通的卡只是one-hot或者全空，对于ssr佐岳可能有两个true
  bool isShining;//是否闪彩。无法闪彩的卡或者npc恒为false
  int8_t cardRecord;//记录一些可能随着时间而改变的参数，例如根涡轮的固有

  bool larc_isLinkCard;//是否为link支援卡
  int8_t larc_charge;//现在充了几格
  bool larc_atSS;//是否在ss对战里
  int8_t larc_statusType;//速耐力根智01234
  int8_t larc_specialBuff;//每3级的特殊固有，编号同游戏内
  int8_t larc_level;//几级
  int8_t larc_nextThreeBuffs[3];//当前以及以下两级的buff
  int8_t larc_assignedStatusTypeWhenFull;//如果对应的buff是“属性加成”，加哪个属性在满的时候已经确定

  
  std::discrete_distribution<> distribution;//distribution(rand)可以根据得意率生成0~5的整数，代表这张卡出现在速耐力根智鸽。ssr佐岳调用两次

  Person();//未加载的人头
  //更复杂的初始化一律扔到Game类里



  void writeSinglePersonNNInput(float* buf) const;//神经网络输入向量
  std::string getPersonName(const Game& game);//获得人物名称
  std::string getPersonNameColored(const Game& game);//获得人物名称并标注颜色
};