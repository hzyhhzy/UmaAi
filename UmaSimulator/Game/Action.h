#pragma once
#include <cstdint>
#include <random>
#include <string>

enum TrainActionTypeEnum :int16_t
{
  TRA_speed = 0,
  TRA_stamina,
  TRA_power,
  TRA_guts,
  TRA_wiz,
  TRA_rest, 
  TRA_outgoing, //包括合宿的“休息&外出”
  TRA_race,
  TRA_none = -1, //此Action不训练，只做菜
  //TRA_redistributeCardsForTest = -2 //使用这个标记时，说明要randomDistributeCards，用于测试ai分数，在Search::searchSingleActionThread中使用
};

struct Action 
{
  static const std::string trainingName[8];
  static Action RedistributeCardsForTest();
  static const int MAX_ACTION_TYPE = 14 + 36;//标准的Action与编号一一对应
  
  //这个type和Game::stage对应
  //0 未初始化
  //1 普通训练，如果有overdrive则可以决定是否开，也可以先开overdrive而暂时不选训练（仅限第三年下半年点了15级胸，则需要先决定是否开overdrive根据摇人结果而选训练）。选项个数为8+5+1
  //2 升级回合。只考虑整3级，action里只包括头和胸（脚=总-头-胸），选项个数为6*6=36
  // 非整3级的部分直接手写逻辑
  //-1 使用这个标记时，说明要randomDistributeCards，用于测试ai分数，在Search::searchSingleActionThread中使用
  int16_t type;
  
  bool overdrive;//是否开overdrive（齿轮）训练。如果已经开了，这个恒为false
  
  int16_t train;//-1暂时不训练，01234速耐力根智，5外出，6休息，7比赛 
  //注：外出是优先友人外出，没有再普通外出，不提供选项

  int8_t mechaHead;//头部升级
  int8_t mechaChest;//胸部升级
  //inline int8_t mechaFoot(int8_t total) {
  //  return total - mechaHead - mechaChest;
  //}

  Action();
  Action(int id);

  bool isActionStandard() const;
  int toInt() const;
  std::string toString() const;
};