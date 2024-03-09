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
  TRA_race
};
enum XiangtanTypeEnum :int16_t
{
  //b=blue,r=red,y=yellow, XT_ab means a to b, XT_a means all to a
  XT_none = 0,
  XT_br,
  XT_by,
  XT_rb,
  XT_ry,
  XT_yb,
  XT_yr,
  XT_b,
  XT_r,
  XT_y
};
struct Action //一个回合的操作
{
  static const int MAX_ACTION_TYPE = 10 * 5 + 3;//10种相谈*5种训练+外出休息比赛
  static const int XiangtanFromColor[10];
  static const int XiangtanToColor[10];
  static const int XiangtanNumCost[10];

  int16_t train;//01234速耐力根智，5外出，6休息，7比赛 
  //注：外出是优先友人外出，没有再普通外出，不提供选项
  
  int16_t xiangtanType;//相谈的10种方式，依次是不谈，6种单次相谈，3种两次相谈

  int toInt() const;
  static Action intToAction(int i);
};