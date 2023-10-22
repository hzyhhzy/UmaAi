#include "SupportCard.h"
#include "iostream"
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
    memcpy(initialBonus, sc->initialBonus, sizeof(int) * 6);
    initialJiBan = sc->initialJiBan;
    saiHou = sc->saiHou;
    deYiLv = sc->deYiLv;
}

// 固有词条的通用计算方法
// key 对应 enum class UniqueEffectType
CardTrainingEffect& CardTrainingEffect::applyUniqueEffectLine(int key, int value)
{
    switch (key)
    {
        case (int)UniqueEffectType::SpecialTagEffectUp:  // 友情-乘算
            youQing = (100 + youQing) * (100 + value) / 100 - 100;
            break;
        case (int)UniqueEffectType::MotivationUp:    // 干劲-加算
            ganJing += value;
            break;
        case (int)UniqueEffectType::GoodTrainingRateUp:  // 得意率-乘算
            deYiLv = (100 + deYiLv) * (100 + value) / 100 - 100;
            break;
        case (int)UniqueEffectType::TrainingEffectUp:    // 训练-加算
            xunLian += value; break;
        case (int)UniqueEffectType::RaceStatusUp:    // 赛后-加算
            saiHou += value;
            break;
        case (int)UniqueEffectType::TrainingFailureRateDown: // 失败率-减乘
            failRateDrop = 100 - (100 - failRateDrop) * (100 - value) / 100;
            break;
        case (int)UniqueEffectType::TrainingHPConsumptionDown: // 体力-减乘
            vitalCostDrop = 100 - (100 - vitalCostDrop) * (100 - value) / 100;
            break;
        case (int)UniqueEffectType::WizRecoverUp:    // 智力回体UP-加算（傻不拉几）
            vitalBonus += value; break;
        case (int)UniqueEffectType::TrainingSpeedUp:    // 副属性
            bonus[0] += value; break;
        case (int)UniqueEffectType::TrainingStaminaUp:
            bonus[1] += value; break;
        case (int)UniqueEffectType::TrainingPowerUp:
            bonus[2] += value; break;
        case (int)UniqueEffectType::TrainingGutsUp:
            bonus[3] += value; break;
        case (int)UniqueEffectType::TrainingWizUp:
            bonus[4] += value; break;
        case (int)UniqueEffectType::SkillPointBonus:
            bonus[5] += value; break;
        case (int)UniqueEffectType::InitialSpeedUp: // 初始属性
            initialBonus[0] += value; break;
        case (int)UniqueEffectType::InitialStaminaUp:
            initialBonus[1] += value; break;
        case (int)UniqueEffectType::InitialPowerUp:
            initialBonus[2] += value; break;
        case (int)UniqueEffectType::InitialGutsUp:
            initialBonus[3] += value; break;
        case (int)UniqueEffectType::InitialWizUp:
            initialBonus[4] += value; break;
        case (int)UniqueEffectType::InitalEvaluationUp:  // 初始羁绊
            initialJiBan += value; break;
            // 忽略的属性
        case (int)UniqueEffectType::RaceFanUp:   // 粉丝
        case (int)UniqueEffectType::SkillTipsEventRateUp:    // Hint率
        case (int)UniqueEffectType::SkillTipsLvUp:   // Hint等级
            break;
            // 神鹰特判
        case (int)UniqueEffectType::AllStatusBonus:  // 速神鹰
            for (int i = 0; i < 5; ++i)
                bonus[i] += 1;
            break;
        default:
            cout << "未知固有词条ID: " << key << " = " << value << endl;
            break;
    }
    return *this;
}
