
#include <iostream>
using namespace std;
void main_test1()
{
  //题目：每回合随机取5个球，黑白概率相等，拿走其中一种颜色的球
  //一共100个回合
  //黑球上限300，白球上限100，目标是总数最多
  const int MaxC1 = 1000, MaxC2 = 1000, MaxTurn = 300;
  const double probs[6] = { 
    1.0 / 32, 
    5.0 / 32, 
    10.0 / 32,
    10.0 / 32,
    5.0 / 32,
    1.0 / 32 };

  auto f = new double[MaxTurn+1][MaxC1+1][MaxC2+1]; //f是剩余turn时，当前黑球白球分别距离上限c1,c2时，最终得分距离上限的期望

  //turn=0时
  for (int c1 = 0; c1 <= MaxC1; c1++)
    for (int c2 = 0; c2 <= MaxC2; c2++)
    {
      f[0][c1][c2] = c1 + c2;
    }

  //turn>0时,递推函数
  for (int turn = 1; turn <= MaxTurn; turn++)
    for (int c1 = 0; c1 <= MaxC1; c1++)
      for (int c2 = 0; c2 <= MaxC2; c2++)
      {
        double avg = 0;

        for (int n = 0; n <= 5; n++)//n个黑球
        {
          int c1new = max(0, c1 - n);
          int c2new = max(0, c2 - (5 - n));
          double bestChoice = min(f[turn - 1][c1new][c2], f[turn - 1][c1][c2new]);
          avg += bestChoice * probs[n];
        }

        f[turn][c1][c2] = avg;
      }


  //查找turn=100时，取3黑球优于2白球的临界线
  int turn = 300;
  for (int c2 = 0; c2 < MaxC2; c2++)
  {
    for (int c1 = 0; c1 < MaxC1; c1++)
    {
      int c1new = max(0, c1 - 3);
      int c2new = max(0, c2 - (5 - 3));
      if (f[turn - 1][c1new][c2] < f[turn - 1][c1][c2new])
      {
        cout << c2 << " " << c1 << endl;
        break;
      }
    }
  }

  //计算c2=500，turn=100时，不同c1下，取3黑球与2白球的平均分的差异
  
  {
    int turn = 300;
    int c2 = 1000;
    for (int c1 = 0; c1 < MaxC1; c1++)
    {
      int c1new = max(0, c1 - 3);
      int c2new = max(0, c2 - (5 - 3));
      cout << c1 << " " << f[turn - 1][c1new][c2] - f[turn - 1][c1][c2new] << endl;
    }
  }

  for (int c = 0; c < MaxC1; c++)
  {
    cout << c << " " << f[turn][c][c] << endl;
  }

}