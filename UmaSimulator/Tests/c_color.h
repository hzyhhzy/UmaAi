#pragma once
#include <windows.h>

class ColorSet {
public:
    void SetColor(int ForgC)
    {
        WORD wColor;
        //��ȡ��ǰ�ı�������
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;

        if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
        {
            //�����������ԣ����ǰ��ɫ
            wColor = (csbi.wAttributes & 0xF0) + (ForgC & 0x0F);
            SetConsoleTextAttribute(hStdOut, wColor);
        }
        return;
    }
};