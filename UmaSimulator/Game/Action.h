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
  TRA_outgoing,
  TRA_race,
  TRA_none = -1, //此Action不训练，只做菜
  TRA_redistributeCardsForTest = -2 //使用这个标记时，说明要randomDistributeCards，用于测试ai分数，在Search::searchSingleActionThread中使用
};
enum DishTypeEnum :int16_t
{
  DISH_none = 0,
  DISH_sandwich, //speed+power+wiz 25%
  DISH_curry, //speed+stamina+guts 25%
  DISH_speed1,//150+80  60%
  DISH_stamina1,
  DISH_power1,
  DISH_guts1,
  DISH_wiz1,
  DISH_speed2,//250+80  90%~100%
  DISH_stamina2,
  DISH_power2,
  DISH_guts2,
  DISH_wiz2,
  DISH_g1plate //5*80
};
//考虑到这个剧本做菜有随机性，可能会影响希望选的训练
//一个回合可以拆成两步：先做菜，再训练，也可以不拆
//Action类表示做菜，或者训练，或者做菜+训练
struct Action 
{
  static const std::string trainingName[8];
  static const std::string dishName[14];
  static const Action Action_RedistributeCardsForTest;

  
  int16_t dishType;//做菜，0为不做菜

  int16_t train;//-1暂时不训练，01234速耐力根智，5外出，6休息，7比赛 
  //注：外出是优先友人外出，没有再普通外出，不提供选项

  int toInt() const;
  std::string toString() const;
  static Action intToAction(int i);
};