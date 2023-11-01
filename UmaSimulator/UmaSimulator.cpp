// UmaSimulator.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "config.h"
#include "Tests/tests.h"
#include "MainCommands/MainCommands.h"
#include "websocket.h"

int main()
{
#if defined UMAAI_TESTCARDSSINGLE
	main_testCardsSingle();
#elif defined UMAAI_TESTSCORE
	main_testAiScore();
#elif defined UMAAI_SIMULATOR
	main_playerPlay();
#elif defined UMAAI_MAINAI
	websocket ws("http://127.0.0.1:4693");
	do {
		Sleep(10);
		std::cout << ws.get_status() << std::endl;
	} while (ws.get_status() != "Open");
	ws.send("[test]");
	main_ai();
	main_ai();
#else
	//write your own test code
#endif
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
