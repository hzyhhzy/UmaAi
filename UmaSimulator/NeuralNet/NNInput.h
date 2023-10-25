#pragma once
const int LATEST_NNINPUT_VERSION = 1;


//神经网络输入结构
//一个一维向量，依次为
//(全局信息)(佐岳卡参数)(支援卡1)...(支援卡6)(npc1)...(npc10)(理事长)(记者)(无卡佐岳)
//支援卡1~6每个包括两部分：CARD和PERSON
//npc只包括PERSON部分
//如果带了佐岳，则 支援卡6、无卡佐岳 为空
//如果没带佐岳，则 佐岳卡参数、npc10为空
const int NNINPUT_CHANNELS_SEARCHPARAM_V1 = 3; //搜索参数占多少个通道
const int NNINPUT_CHANNELS_GLOBAL_V1 = 300;//全局信息
const int NNINPUT_CHANNELS_CARD_V1 = 50; //每个支援卡参数占多少通道，不包括这个人头的羁绊之类的
const int NNINPUT_CHANNELS_PERSON_V1 = 50; //每个人头多少通道，不包括支援卡参数
const int NNINPUT_CHANNELS_V1 = 2000;//总通道数








const int NNOUTPUT_CHANNELS_POLICY_V1 = 19;
const int NNOUTPUT_CHANNELS_VALUE_V1 = 3;
const int NNOUTPUT_CHANNELS_V1 = NNOUTPUT_CHANNELS_POLICY_V1 + NNOUTPUT_CHANNELS_VALUE_V1;