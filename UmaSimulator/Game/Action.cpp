#include "Action.h"

const Action Action::Action_RedistributeCardsForTest = { -1,false, -1 ,0,0 };

const std::string Action::trainingName[8] =
{
  "��",
  "��",
  "��",
  "��",
  "��",
  "��Ϣ",
  "���",
  "����"
};
Action::Action()
{
  type = 0;
  overdrive = false;
  train = -1;
  mechaHead = 0;
  mechaChest = 0;
}
Action::Action(int id)
{
  type = 0;
  overdrive = false;
  train = -1;
  mechaHead = 0;
  mechaChest = 0;

  if (id < 0 || id >= MAX_ACTION_TYPE)
    throw "Action::Action(): Invalid int";
  Action a;
  if (id < 14)
  {
    type = 1;
    overdrive = id >= 8;
    train = overdrive ? id - 8 : id;
    if (id == 13)train = -1;
    mechaHead = 0;
    mechaChest = 0;
  }
  else if (id < 15 + 6 * 6)
  {
    type = 2;
    overdrive = false;
    train = -1;
    mechaHead = (id - 14) / 6;
    mechaChest = (id - 14) % 6;
  }
  else
    throw "Action::Action(): Invalid int";



}
bool Action::isActionStandard() const
{
  //-1 ʹ��������ʱ��˵��ҪrandomDistributeCards�����ڲ���ai��������Search::searchSingleActionThread��ʹ��
  if (type == -1)
  {
    return false;
  }
  //1 ��ͨѵ���������overdrive����Ծ����Ƿ񿪣�Ҳ�����ȿ�overdrive����ʱ��ѡѵ�������޵������°������15���أ�����Ҫ�Ⱦ����Ƿ�overdrive����ҡ�˽����ѡѵ������ѡ�����Ϊ8+5+1
  else if (type == 1)
  {
    return mechaHead == 0 && mechaChest == 0 && (overdrive ? (train >= -1 && train < 5): (train >= 0 && train < 8));
  }
  //2 �����غϡ�ֻ������3����action��ֻ����ͷ���أ���=��-ͷ-�أ���ѡ�����Ϊ6*6=36
  else if (type == 2)
  {
    return !overdrive && train == -1 && mechaHead >= 0 && mechaHead <= 5 && mechaChest >= 0 && mechaChest <= 5;
  }
  else
    return false;
}

int Action::toInt() const
{
  if (!isActionStandard())
  {
    throw "Calling Action::toInt() for a non-standard action";
    return -1;
  }
  //1 ��ͨѵ���������overdrive����Ծ����Ƿ񿪣�Ҳ�����ȿ�overdrive����ʱ��ѡѵ�������޵������°������15���أ�����Ҫ�Ⱦ����Ƿ�overdrive����ҡ�˽����ѡѵ������ѡ�����Ϊ8+5+1
  if (type == 1)
  {
    return overdrive ? (train == -1 ? 5 + 8 : train + 8) : train;
  }
  //2 �����غϡ�ֻ������3����action��ֻ����ͷ���أ���=��-ͷ-�أ���ѡ�����Ϊ6*6=36
  else if (type == 3)
  {
    return 14 + 6 * mechaHead + mechaChest;
  }
  else
    throw "Calling Action::toInt() for a unknown type action";
}

std::string Action::toString() const
{
  //1 ��ͨѵ���������overdrive����Ծ����Ƿ񿪣�Ҳ�����ȿ�overdrive����ʱ��ѡѵ�������޵������°������15���أ�����Ҫ�Ⱦ����Ƿ�overdrive����ҡ�˽����ѡѵ������ѡ�����Ϊ8+5+1
  if (type == 1)
  {
    if (overdrive && train == -1)
      return "�ȿ�����";
    return overdrive ? "����+" + trainingName[train] : trainingName[train];
  }
  //2 �����غϡ�ֻ������3����action��ֻ����ͷ���أ���=��-ͷ-�أ���ѡ�����Ϊ6*6=36
  else if (type == 2)
  {
    return "ͷ" + std::to_string(mechaHead) + "����" + std::to_string(mechaChest) + "��";
  }
  else
    throw "Action::toString(): Unknown Action";

  return "";
}

Action Action::RedistributeCardsForTest()
{
  Action a = Action();
  a.type = -1;
  return a;
}
