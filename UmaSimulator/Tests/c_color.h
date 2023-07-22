#pragma once
#include <windows.h>

class ColorSet {
public:
    void SetColor(int ForgC)
    {
        WORD wColor;
        //获取当前的背景属性
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;

        if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
        {
            //屏蔽其它属性，添加前景色
            wColor = (csbi.wAttributes & 0xF0) + (ForgC & 0x0F);
            SetConsoleTextAttribute(hStdOut, wColor);
        }
        return;
    }
};