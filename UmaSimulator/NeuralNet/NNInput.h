#pragma once
const int LATEST_NNINPUT_VERSION = 1;
const int NNINPUT_CHANNELS_V1 = 903;
const int NNOUTPUT_CHANNELS_POLICY_V1 = (5 + 3) + 3 + 1 + 6;//5+3是五个训练和休息外出比赛，3是女神事件选颜色，1是开女神，6是6种外出。每次不一定全部生效
const int NNOUTPUT_CHANNELS_VALUE_V1 = 2;//第一个通道代表达标的概率，第二个通道代表预期分数与目标分数之差
const int NNOUTPUT_CHANNELS_V1 = NNOUTPUT_CHANNELS_POLICY_V1 + NNOUTPUT_CHANNELS_VALUE_V1;