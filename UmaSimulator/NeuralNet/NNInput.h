#pragma once
#include "../config.h"
const int LATEST_NNINPUT_VERSION = 1;


//����������ṹ
//һ��һά����������Ϊ
//(ȫ����Ϣ)(֧Ԯ����ͷ1)...(֧Ԯ����ͷ6)��ȫ����Ϣ������֧Ԯ����ͷ
const int NNINPUT_CHANNELS_GAMEGLOBAL_V1 = 573;//Game����˿�����ͷ��ͨ����
const int NNINPUT_CHANNELS_CARD_V1 = 77; //ÿ��֧Ԯ������ռ����ͨ���������������ͷ���֮���
const int NNINPUT_CHANNELS_PERSON_V1 = 13; //ÿ����ͷ����ͨ����������֧Ԯ������
const int NNINPUT_CHANNELS_CARDPERSON_V1 = NNINPUT_CHANNELS_CARD_V1 + NNINPUT_CHANNELS_PERSON_V1; //ÿ��֧Ԯ����ͷ����ͨ��������֧Ԯ�������� ��npc�ľ籾������ϲ�������
const int NNINPUT_CHANNELS_V1 = NNINPUT_CHANNELS_GAMEGLOBAL_V1 + NNINPUT_CHANNELS_CARDPERSON_V1 * 6;//��ͨ����



#if USE_BACKEND == BACKEND_CUDA
//���������֣�kernel.cuҲҪ��
const int NNINPUT_MAX_FLOAT = 192; //nninput��������ж��ٸ���0��1����
const int NNINPUT_MAX_ONES = 192; //nninput��������ж��ٸ�1
static_assert(NNINPUT_CHANNELS_V1 < 32767);//int16
#endif


const int NNOUTPUT_CHANNELS_POLICY_V1 = 21;
const int NNOUTPUT_CHANNELS_VALUE_V1 = 3;
const int NNOUTPUT_CHANNELS_V1 = NNOUTPUT_CHANNELS_POLICY_V1 + NNOUTPUT_CHANNELS_VALUE_V1;

//should be the same as config.py in training scripts
const float NNOUTPUT_Value_Mean = 43000;
const float NNOUTPUT_Value_Scale = 400;
const float NNOUTPUT_Valuevar_Scale = 200;