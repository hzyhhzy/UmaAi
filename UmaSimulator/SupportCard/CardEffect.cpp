#include "SupportCard.h"
#include "iostream"
#include "string"
#include "sstream"
using namespace std;

CardTrainingEffect::CardTrainingEffect(const SupportCard* sc)
{
    youQing = sc->youQingBasic;
    ganJing = sc->ganJingBasic;
    xunLian = sc->xunLianBasic;
    memcpy(bonus, sc->bonusBasic, sizeof(double) * 6);
    vitalBonus = sc->wizVitalBonusBasic;
    failRateDrop = sc->failRateDrop;
    vitalCostDrop = sc->vitalCostDrop;
   /*
    memcpy(initialBonus, sc->initialBonus, sizeof(int) * 6);
    initialJiBan = sc->initialJiBan;
    saiHou = sc->saiHou;
    deYiLv = sc->deYiLv;
    */
}

// 固有词条的通用计算方法
// key 对应 enum class UniqueEffectType
CardTrainingEffect& CardTrainingEffect::apply(int key, int value)
{
    switch (key)
    {
        case 1:  // 友情-乘算
            youQing = (100 + youQing) * (100 + value) / 100 - 100;
            break;
        case 2:    // 干劲-加算
            ganJing += value; break;
        case 8:    // 训练-加算
            xunLian += value; break;
    //  case 19:  // 得意率-乘算
    //     deYiLv = (100 + deYiLv) * (100 + value) / 100 - 100;
    //     break;
    //  case 15:    // 赛后-加算
    //    saiHou += value;
    //    break;
        case 27: // 失败率-减乘
            failRateDrop = 100 - (100 - failRateDrop) * (100 - value) / 100;
            break;
        case 28: // 体力-减乘
            vitalCostDrop = 100 - (100 - vitalCostDrop) * (100 - value) / 100;
            break;
        case 31:    // 智力回体UP-加算（傻不拉几）
            vitalBonus += value; break;
        case 3:    // 副属性
            bonus[0] += value; break;
        case 4:
            bonus[1] += value; break;
        case 5:
            bonus[2] += value; break;
        case 6:
            bonus[3] += value; break;
        case 7:
            bonus[4] += value; break;
        case 30:
            bonus[5] += value; break;
        /*
        case 9: // 初始属性
            initialBonus[0] += value; break;
        case 10:
            initialBonus[1] += value; break;
        case 11:
            initialBonus[2] += value; break;
        case 12:
            initialBonus[3] += value; break;
        case 13:
            initialBonus[4] += value; break;
        case 14:  // 初始羁绊
            initialJiBan += value; break;
        */
        case 17:    // Hint等级
        case 18:    // Hint率：不处理
            break;
        // 神鹰特判
        case 99:  // 速神鹰
            for (int i = 0; i < 5; ++i)
                bonus[i] += 1;
            break;
        default:
            cout << "未知词条: " << key << " = " << value << endl;
            break;
    }
    return *this;
}

const string& CardTrainingEffect::explain() {
    stringstream ss;
    ss << std::setprecision(4);
    if (youQing > 0)
        ss << "[彩圈]友情=" << youQing << ' ';
    if (ganJing > 0)
        ss << "干劲=" << ganJing << ' ';
    if (xunLian > 0)
        ss << "训练=" << xunLian << ' ';
    if (failRateDrop > 0)
        ss << "失败率-" << failRateDrop << ' ';
    if (vitalCostDrop > 0)
        ss << "体力消耗-" << vitalCostDrop << ' ';
    ss << "副属性=( ";
    for (int i = 0; i < 6; ++i)
        ss << bonus[i] << ' ';
    ss << ")";
    return ss.str();
}