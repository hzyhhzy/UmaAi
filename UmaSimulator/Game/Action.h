#pragma once
#include <cstdint>
#include <random>
#include <string>
struct Action //一个回合的操作
{
  int8_t train;//01234速耐力根智，5ss对战，6休息或者法棍，7友人外出，8普通外出，9比赛 
  //注：不排除极个别情况下普通外出比友人外出好，比如友人走到第三段后掉心情（第四段不回心情），而且马上远征
  bool buy50p;//买不买对应训练的+50%
  bool buyPt10;//买不买pt+10
  bool buyFriend20;//买不买友情+20%
  bool buyVital20;//买不买体力消耗-20%
  //所有的2级根据larc_allowedDebuffsFirstLarc自动决定消不消，所以只考虑3级
  //非远征：只考虑train（速耐力根智，休息，外出）共7种选择。能买友情20%立刻买，买了友情20%后能买pt+10立刻买
  //第二年远征——尼尔赏：考虑train（速耐力根智，法棍。法棍则不考虑买其他buff），buy50p，buyPt10。共5*2*2+1=21种选择
  //尼尔赏——第二年凯旋门（一个回合）：考虑train（速耐力根智，法棍。法棍则不考虑买其他buff），buy50p，buyFriend20，buyPt10。共5*2*2*2+1=41种选择。但会根据当前的pt状态排除一部分情况（排除掉训练成功也没法消完debuff & 训练失败也能消完debuff且剩下的pt够买一个东西）
  //第三年远征：考虑train（速耐力根智，法棍。法棍则不考虑买其他buff），buy50p，buyVital20。共5*2*2+1=21种选择
};