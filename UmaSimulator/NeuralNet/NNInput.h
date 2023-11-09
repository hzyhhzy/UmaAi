#pragma once
const int LATEST_NNINPUT_VERSION = 1;


//神经网络输入结构
//一个一维向量，依次为
//(全局信息)(支援卡1参数)...(支援卡6参数)(佐岳卡参数)(支援卡人头1)...(支援卡人头6)(npc1)...(npc10)(理事长)(记者)(有卡佐岳)(无卡佐岳)
//如果带了佐岳，则 支援卡6、支援卡人头6 为空
//如果没带佐岳，则 佐岳卡参数、npc10为空
const int NNINPUT_CHANNELS_SEARCHPARAM_V1 = 3; //搜索参数占多少个通道
const int NNINPUT_CHANNELS_GAMEGLOBAL_V1 = 308;//Game里除了卡和人头的通道数
const int NNINPUT_CHANNELS_CARD_V1 = 67; //每个支援卡参数占多少通道，不包括这个人头的羁绊之类的
const int NNINPUT_CHANNELS_PERSON_V1 = 74; //每个人头多少通道，不包括支援卡参数
const int NNINPUT_CHANNELS_V1 = NNINPUT_CHANNELS_SEARCHPARAM_V1 +
                                NNINPUT_CHANNELS_GAMEGLOBAL_V1 +
                                NNINPUT_CHANNELS_CARD_V1 * 7 +
                                NNINPUT_CHANNELS_PERSON_V1 * 20;//总通道数








const int NNOUTPUT_CHANNELS_POLICY_V1 = 18;
const int NNOUTPUT_CHANNELS_VALUE_V1 = 3;
const int NNOUTPUT_CHANNELS_V1 = NNOUTPUT_CHANNELS_POLICY_V1 + NNOUTPUT_CHANNELS_VALUE_V1;