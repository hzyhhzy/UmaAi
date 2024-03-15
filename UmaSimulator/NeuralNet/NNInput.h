#pragma once
#include "../config.h"
const int LATEST_NNINPUT_VERSION = 1;


//神经网络输入结构
//一个一维向量，依次为
//(全局信息)(支援卡人头1)...(支援卡人头6)，全局信息包含非支援卡人头
const int NNINPUT_CHANNELS_GAMEGLOBAL_V1 = 581;//Game里除了卡和人头的通道数
const int NNINPUT_CHANNELS_CARD_V1 = 77; //每个支援卡参数占多少通道，不包括这个人头的羁绊之类的
const int NNINPUT_CHANNELS_PERSON_V1 = 12; //每个人头多少通道，不包括支援卡参数
const int NNINPUT_CHANNELS_CARDPERSON_V1 = NNINPUT_CHANNELS_CARD_V1 + NNINPUT_CHANNELS_PERSON_V1; //每个支援卡人头多少通道，包括支援卡参数。 有npc的剧本不建议合并这两个
const int NNINPUT_CHANNELS_V1 = NNINPUT_CHANNELS_GAMEGLOBAL_V1 + NNINPUT_CHANNELS_CARDPERSON_V1 * 6;//总通道数

#if USE_BACKEND == BACKEND_CUDA
//改以下数字，kernel.cu也要改
const int NNINPUT_MAX_FLOAT = 128; //nninput里面最多有多少个非0非1的数
const int NNINPUT_MAX_ONES = 272; //nninput里面最多有多少个1
static_assert(NNINPUT_CHANNELS_V1 < 32767);//int16
#endif







const int NNOUTPUT_CHANNELS_POLICY_V1 = 18;
const int NNOUTPUT_CHANNELS_VALUE_V1 = 3;
const int NNOUTPUT_CHANNELS_V1 = NNOUTPUT_CHANNELS_POLICY_V1 + NNOUTPUT_CHANNELS_VALUE_V1;